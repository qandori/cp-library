#!/usr/bin/env python3
"""Strict, bounded offline reconstruction of v2 lossless evidence transport.

This checks transport only, never performance or experiment validity. It does not
run workers. An incomplete/corrupt transport never publishes the output directory.
"""
from __future__ import annotations

import argparse
import base64
import binascii
import hashlib
import json
from pathlib import Path
import re
import shutil
import sys
import tempfile
import zlib

import remote_adapter as adapter


def fields(event, expected):
    adapter.require(type(event) is dict and set(event) == set(expected.split()),
                    'Unexpected transport record schema')


def integer(value, maximum, label):
    adapter.require(type(value) is int and 0 <= value <= maximum, 'Invalid ' + label)
    return value


def sha256(value):
    adapter.require(type(value) is str and re.fullmatch(r'[0-9a-f]{64}', value),
                    'Invalid SHA-256')
    return value


def records(source):
    """Read bounded lines, allowing the log provider's prefix before our marker."""
    total = 0
    marker = adapter.PREFIX.encode('ascii')
    while True:
        line = source.readline(adapter.MAX_LOG_LINE_BYTES + 1)
        if not line:
            return
        total += len(line)
        adapter.require(total <= adapter.MAX_LOG_BYTES, 'Input log exceeds bound')
        adapter.require(len(line) <= adapter.MAX_LOG_LINE_BYTES, 'Input log line exceeds bound')
        if marker not in line:
            continue
        adapter.require(line.count(marker) == 1, 'Ambiguous transport marker')
        payload = line.split(marker, 1)[1]
        event = adapter.strict_json(payload)
        adapter.require(type(event) is dict and type(event.get('event')) is str,
                        'Invalid transport record')
        yield event


class ExactFile:
    """One strictly single-member gzip stream, never accumulated in memory."""
    def __init__(self, target, entry):
        target.parent.mkdir(parents=True, exist_ok=True)
        self.dest = target.open('xb')
        self.entry = entry
        self.original_size = 0
        self.compressed_size = 0
        self.chunks = 0
        self.original_hash = hashlib.sha256()
        self.compressed_hash = hashlib.sha256()
        self.decoder = zlib.decompressobj(16 + zlib.MAX_WBITS)

    def chunk(self, event):
        fields(event, 'event path index offset bytes data_base64')
        integer(event['index'], adapter.MAX_COMPRESSED_FILE_BYTES, 'chunk index')
        integer(event['offset'], adapter.MAX_COMPRESSED_FILE_BYTES, 'chunk offset')
        integer(event['bytes'], adapter.CHUNK_BYTES, 'chunk bytes')
        adapter.require(event['index'] == self.chunks and event['offset'] == self.compressed_size,
                        'Noncontiguous chunk index or offset')
        expected = min(adapter.CHUNK_BYTES,
                       self.entry['compressed_bytes'] - self.compressed_size)
        adapter.require(event['bytes'] == expected and expected > 0,
                        'Unexpected compressed chunk length')
        encoded = event['data_base64']
        adapter.require(type(encoded) is str and len(encoded) == 4 * ((expected + 2) // 3),
                        'Invalid base64 length')
        try:
            data = base64.b64decode(encoded, validate=True)
        except (ValueError, binascii.Error) as exc:
            raise adapter.AdapterError('Invalid base64 chunk') from exc
        adapter.require(len(data) == expected
                        and base64.b64encode(data).decode('ascii') == encoded,
                        'Noncanonical base64 or chunk length')
        self.compressed_hash.update(data)
        self.compressed_size += len(data)
        self.chunks += 1
        adapter.require(not self.decoder.eof, 'Data after gzip end')
        pending = data
        while pending:
            # The +1 detects a dishonest declared size before writing that byte.
            bound = min(adapter.IO_BYTES, self.entry['bytes'] - self.original_size + 1)
            try:
                plain = self.decoder.decompress(pending, bound)
            except zlib.error as exc:
                raise adapter.AdapterError('Corrupt gzip stream') from exc
            adapter.require(self.original_size + len(plain) <= self.entry['bytes'],
                            'Decompressed file exceeds declared size')
            self.original_size += len(plain)
            self.original_hash.update(plain)
            self.dest.write(plain)
            adapter.require(not self.decoder.unused_data, 'Trailing data or multiple gzip members')
            tail = self.decoder.unconsumed_tail
            adapter.require(not tail or plain or len(tail) < len(pending),
                            'Gzip decoder made no progress')
            pending = tail

    def finish(self):
        adapter.require(self.decoder.eof and not self.decoder.unused_data
                        and not self.decoder.unconsumed_tail, 'Truncated or trailing gzip stream')
        adapter.require(self.compressed_size == self.entry['compressed_bytes']
                        and self.compressed_hash.hexdigest() == self.entry['compressed_sha256'],
                        'Compressed size or SHA-256 differs')
        adapter.require(self.original_size == self.entry['bytes']
                        and self.original_hash.hexdigest() == self.entry['sha256'],
                        'Original size or SHA-256 differs')

    def close(self):
        self.dest.close()


def decode(source, output):
    output = Path(output)
    adapter.require(not output.exists() and not output.is_symlink(),
                    'Output already exists; preserved evidence must not be overwritten')
    adapter.require(output.parent.is_dir(), 'Output parent must already exist')
    staging = Path(tempfile.mkdtemp(prefix='.rr-transport-', dir=output.parent))
    begin = footer = active = writer = None
    prepared = False
    inventory = {}
    total_original = total_compressed = total_file_bytes = 0
    exact_files = record_count = 0
    try:
        for event in records(source):
            record_count += 1
            kind = event['event']
            adapter.require(footer is None, 'Evidence record after completion footer')
            if kind == 'prepared':
                adapter.require(begin is None and not prepared, 'Duplicate or misplaced prepared record')
                adapter.require(event.get('experiment') == adapter.EXPERIMENT,
                                'Prepared experiment differs')
                prepared = True
                continue
            if kind == 'transport_begin':
                fields(event, 'event experiment inventory_sha256 transport_schema_version files '
                              'total_file_bytes total_compressed_bytes has_experiment_result')
                adapter.require(begin is None and active is None, 'Duplicate transport begin')
                adapter.require(event['experiment'] == adapter.EXPERIMENT
                                and type(event['transport_schema_version']) is int
                                and event['transport_schema_version'] == 2, 'Wrong transport identity')
                integer(event['files'], adapter.MAX_TRANSPORT_FILES, 'file count')
                adapter.require(event['files'] > 0, 'Empty transport')
                integer(event['total_file_bytes'], adapter.MAX_ORIGINAL_TOTAL_BYTES, 'total file bytes')
                integer(event['total_compressed_bytes'], adapter.MAX_COMPRESSED_TOTAL_BYTES,
                        'total compressed bytes')
                adapter.require(type(event['has_experiment_result']) is bool, 'Invalid result presence')
                sha256(event['inventory_sha256'])
                begin = event
                continue
            adapter.require(begin is not None, 'Missing transport begin')
            if kind == 'file_start':
                adapter.require(active is None, 'Overlapping file records')
                name = event.get('path')
                adapter.transport_name(name)
                adapter.require(name not in inventory and len(inventory) < begin['files'],
                                'Duplicate file or excess file count')
                exact = event.get('transport') == 'exact_bytes'
                fields(event, 'event path bytes sha256 transport' +
                       (' encoding compressed_bytes compressed_sha256' if exact else ''))
                integer(event['bytes'], adapter.MAX_EXACT_FILE_BYTES if exact else adapter.MAX_ORIGINAL_TOTAL_BYTES,
                        'original file bytes')
                sha256(event['sha256'])
                adapter.require(event['transport'] == adapter.classification(name),
                                'Incorrect exact/hash-only classification')
                active = {key: value for key, value in event.items() if key not in ('event', 'path')}
                active_name = name
                total_file_bytes += active['bytes']
                adapter.require(total_file_bytes <= begin['total_file_bytes'], 'Total file bytes exceeded')
                if exact:
                    adapter.require(active['encoding'] == 'gzip+base64', 'Wrong compression encoding')
                    integer(active['compressed_bytes'], adapter.MAX_COMPRESSED_FILE_BYTES,
                            'compressed file bytes')
                    adapter.require(active['compressed_bytes'] >= 20, 'Gzip stream too short')
                    sha256(active['compressed_sha256'])
                    total_original += active['bytes']
                    total_compressed += active['compressed_bytes']
                    adapter.require(total_original <= adapter.MAX_EXACT_TOTAL_BYTES
                                    and total_compressed <= begin['total_compressed_bytes'],
                                    'Total evidence exceeds bound')
                    writer = ExactFile(staging / name, active)
                    exact_files += 1
                continue
            if kind == 'file_chunk':
                adapter.require(active is not None and writer is not None
                                and event.get('path') == active_name, 'Chunk without matching exact file')
                writer.chunk(event)
                continue
            if kind == 'file_end':
                fields(event, 'event path chunks transported_bytes sha256')
                adapter.require(active is not None and event['path'] == active_name,
                                'End without matching file')
                integer(event['chunks'], adapter.MAX_COMPRESSED_FILE_BYTES, 'chunk count')
                integer(event['transported_bytes'], adapter.MAX_COMPRESSED_FILE_BYTES,
                        'transported file bytes')
                adapter.require(event['sha256'] == active['sha256'], 'File-end original SHA differs')
                adapter.require(event['chunks'] == (writer.chunks if writer else 0)
                                and event['transported_bytes'] ==
                                (writer.compressed_size if writer else 0), 'File-end counts differ')
                if writer is not None:
                    writer.finish()
                    writer.close()
                    writer = None
                inventory[active_name] = active
                active = None
                continue
            if kind == 'transport_complete':
                fields(event, 'event experiment inventory_sha256 files transported_bytes '
                              'performance_interpretation')
                adapter.require(active is None, 'Footer before file completion')
                integer(event['files'], adapter.MAX_TRANSPORT_FILES, 'footer file count')
                integer(event['transported_bytes'], adapter.MAX_COMPRESSED_TOTAL_BYTES,
                        'footer transported bytes')
                sha256(event['inventory_sha256'])
                adapter.require(event['experiment'] == adapter.EXPERIMENT
                                and type(event['performance_interpretation']) is str,
                                'Wrong footer identity')
                footer = event
                continue
            raise adapter.AdapterError('Unexpected evidence event: ' + kind)
        adapter.require(begin is not None and footer is not None and active is None,
                        'Missing complete transport footer')
        inventory_hash = adapter.digest(json.dumps(inventory, sort_keys=True,
                                                   separators=(',', ':')).encode())
        adapter.require(inventory_hash == begin['inventory_sha256'] == footer['inventory_sha256'],
                        'Inventory SHA-256 differs')
        adapter.require(len(inventory) == begin['files'] == footer['files'], 'Inventory count differs')
        adapter.require(total_file_bytes == begin['total_file_bytes'], 'Original inventory bytes differ')
        adapter.require(total_compressed == begin['total_compressed_bytes'] == footer['transported_bytes'],
                        'Compressed transport bytes differ')
        adapter.require(begin['has_experiment_result'] ==
                        ('benchmark/experiment/experiment-result.json' in inventory),
                        'Experiment-result presence differs')
        adapter.require(not output.exists() and not output.is_symlink(), 'Output appeared during decode')
        staging.rename(output)
        return {'status': 'transport_complete_only', 'not_performance_qualification': True,
                'file_count': len(inventory), 'exact_files_reconstructed': exact_files,
                'exact_bytes': total_original, 'compressed_bytes': total_compressed,
                'inventory_sha256': inventory_hash, 'record_count': record_count,
                'inventory': inventory}
    finally:
        if writer is not None:
            writer.close()
        if staging.exists():
            shutil.rmtree(staging)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args(argv)
    try:
        with args.log.open('rb') as source:
            report = decode(source, args.output)
        print(json.dumps(report, sort_keys=True, allow_nan=False))
        return 0
    except (adapter.AdapterError, OSError, ValueError, KeyError, TypeError,
            RecursionError) as exc:
        print(json.dumps({'status': 'inconclusive', 'error_type': type(exc).__name__,
                          'message': str(exc)}, sort_keys=True), file=sys.stderr)
        return 2


if __name__ == '__main__':
    sys.exit(main())
