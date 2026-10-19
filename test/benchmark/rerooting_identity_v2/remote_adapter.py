#!/usr/bin/env python3
"""Exact one-shot source binding and bounded, lossless log transport.

This module never builds or runs a worker. It has no campaign or retry mode.
"""
from __future__ import annotations

import argparse
import base64
import gzip
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import signal
import stat
import subprocess
import sys
import time

REPOSITORY = 'qandori/cp-library'
BRANCH = 'codex/rerooting-identity-screen-v2-20261019'
EXPERIMENT = 'rerooting-identity-public-20261019-v2'
BASE = '9f05f01500656c65e394a8aa11c3f5afa4d9495a'
BASE_TREE = 'a2d8f814ffb2df1010396d9b487e8b9d41ceeaa3'
PACKAGE = 'test/benchmark/rerooting_identity_v2'
WORKFLOW = '.github/workflows/rerooting-identity-screen-v2.yml'
OUTPUT_NAME = 'rerooting-identity-screen-v2'
MARKER = PACKAGE + '/launch.json'
BINDING = PACKAGE + '/binding.json'
PACKET_FILES = (
    'base/tree/rerooting.hpp', 'candidate/tree/rerooting.hpp',
    'base/components/undirected_csr.hpp', 'candidate/components/undirected_csr.hpp',
    'candidate/test/unit/rerooting.cpp', 'prospective-benchmark.md', 'decode_transport.py',
    *('benchmark/' + name for name in (
        'build.py', 'collector.py', 'collector_test.py', 'kernel.cpp', 'kernel.hpp',
        'provenance_test.py', 'run_experiment.py', 'seal_manifest.py',
        'validate_worker.py', 'worker.cpp')),
)
STAGE_FILES = frozenset({PACKAGE + '/' + name for name in PACKET_FILES} | {
    PACKAGE + '/remote_adapter.py', PACKAGE + '/REMOTE-INTEGRATION.md', WORKFLOW,
    '.github/workflows/verify.yml', '.github/workflows/macos-portability.yml',
})
FIXED_SOURCES = {
    'base/tree/rerooting.hpp': 'f7b2af0d128eb794a1342acc328becb82d1fd49075daa6db273d09dda308b828',
    'candidate/tree/rerooting.hpp': '63ff812240127afd3313ac0001652cf01173c66d212063813f8d47fc3a27f5d4',
    'base/components/undirected_csr.hpp': '2e53a2ce4fbaf18b7f30808cade253977b6cfe422eb3a199b1c79384a0093ec7',
    'candidate/components/undirected_csr.hpp': '2e53a2ce4fbaf18b7f30808cade253977b6cfe422eb3a199b1c79384a0093ec7',
    'candidate/test/unit/rerooting.cpp': '122d62eb83c84ee2f0771a52ebe6ffb3b4a7f7406051df7b5326128dcbd861fc',
    'prospective-benchmark.md': '434d67065f3a7a3de061732c589a38af8f64a3197252ac144ae4b5956219e1ca',
}
PREFIX = 'RR_EVIDENCE_V2 '
CHUNK_BYTES = 12288
# Transport safety limits only; no worker, collector or measurement cap changes.
IO_BYTES = 65536
MAX_TRANSPORT_FILES = 4096
MAX_TRANSPORT_ENTRIES = 16384
MAX_PATH_BYTES = 1024
MAX_EXACT_FILE_BYTES = 256 * 1024 * 1024
MAX_ORIGINAL_TOTAL_BYTES = 256 * 1024 * 1024
MAX_EXACT_TOTAL_BYTES = MAX_ORIGINAL_TOTAL_BYTES
MAX_COMPRESSED_FILE_BYTES = MAX_EXACT_FILE_BYTES + 1024 * 1024
MAX_COMPRESSED_TOTAL_BYTES = MAX_EXACT_TOTAL_BYTES + 16 * 1024 * 1024
MAX_LOG_LINE_BYTES = 32768
MAX_LOG_BYTES = 1024 * 1024 * 1024


class AdapterError(RuntimeError):
    pass


def require(value, message):
    if not value:
        raise AdapterError(message)


def strict_json(data):
    def pairs(items):
        result = {}
        for key, value in items:
            require(key not in result, 'Duplicate JSON key: ' + key)
            result[key] = value
        return result
    def invalid(value):
        raise AdapterError('Nonfinite JSON value: ' + value)
    return json.loads(data, object_pairs_hook=pairs, parse_constant=invalid)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as source:
        for chunk in iter(lambda: source.read(65536), b''):
            h.update(chunk)
    return h.hexdigest()


def emit(event, **fields):
    print(PREFIX + json.dumps({'event': event, **fields}, sort_keys=True,
                             allow_nan=False, separators=(',', ':')), flush=True)


def regular(path):
    info = path.lstat()
    require(stat.S_ISREG(info.st_mode), 'Not a regular file: ' + str(path))
    return info


def safe_path(root, name):
    relative = PurePosixPath(name)
    require(not relative.is_absolute() and '..' not in relative.parts
            and relative.as_posix() == name, 'Invalid bound relative path')
    path = root / name
    require(path.resolve(strict=True).is_relative_to(root.resolve(strict=True)),
            'Bound path leaves its root: ' + name)
    # Reject symlinks in every component, not just the final file.
    current = root
    for part in relative.parts:
        current /= part
        require(not current.is_symlink(), 'Symlink in bound path: ' + name)
    regular(path)
    return path


def git(root, *args):
    result = subprocess.run(['git', '-C', str(root), *args], stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, timeout=8, check=False)
    require(result.returncode == 0, 'Read-only Git query failed: ' + ' '.join(args))
    return result.stdout


def object_id(root, revision):
    return git(root, 'rev-parse', '--verify', revision).decode().strip()


def audit(root, env):
    expected = {
        'GITHUB_ACTIONS': 'true', 'GITHUB_REPOSITORY': REPOSITORY,
        'GITHUB_EVENT_NAME': 'push', 'GITHUB_REF': 'refs/heads/' + BRANCH,
        'GITHUB_RUN_NUMBER': '1', 'GITHUB_RUN_ATTEMPT': '1',
        'RUNNER_ENVIRONMENT': 'github-hosted',
        'RUNNER_OS': 'Linux', 'RUNNER_ARCH': 'X64', 'ImageOS': 'ubuntu26',
    }
    require(all(env.get(key) == value for key, value in expected.items()),
            'Unexpected one-shot runner/event context')
    require(bool(env.get('ImageVersion')), 'Missing runner image version')
    event = strict_json(Path(env['GITHUB_EVENT_PATH']).read_bytes())
    repository = event.get('repository', {})
    require(repository.get('full_name') == REPOSITORY and repository.get('private') is False
            and repository.get('visibility') == 'public', 'Expected exact public repository')
    require(all(event.get(key) is False for key in ('created', 'deleted', 'forced')),
            'Launch must update an existing branch without force/deletion')
    head = env.get('GITHUB_SHA', '')
    require(re.fullmatch('[0-9a-f]{40}', head), 'Invalid launch commit')
    require(event.get('after') == head and event.get('ref') == expected['GITHUB_REF'],
            'Push event does not match checkout context')
    require(object_id(root, 'HEAD') == head, 'Checkout must be the exact launch head')
    require(not git(root, 'status', '--porcelain', '--untracked-files=all'), 'Checkout is not clean')
    binding_path = safe_path(root, BINDING)
    binding = strict_json(binding_path.read_bytes())
    require(set(binding) == {'schema_version', 'experiment', 'base_commit', 'base_tree', 'branch',
                            'stage_files', 'packet_files', 'baseline_workflows', 'fixed_sources'},
            'Unexpected binding schema')
    require(binding['schema_version'] == 1 and binding['experiment'] == EXPERIMENT
            and binding['branch'] == BRANCH and binding['base_commit'] == BASE
            and binding['base_tree'] == BASE_TREE, 'Binding identity differs')
    require(binding['fixed_sources'] == FIXED_SOURCES, 'Fixed sources differ')
    require(set(binding['stage_files']) == STAGE_FILES, 'Unexpected stage allowlist')
    require(set(binding['packet_files']) == set(PACKET_FILES), 'Unexpected packet allowlist')
    for name, value in binding['stage_files'].items():
        require(type(value) is str and re.fullmatch('[0-9a-f]{64}', value), 'Invalid stage hash')
    for name in PACKET_FILES:
        require(binding['packet_files'][name] == binding['stage_files'][PACKAGE + '/' + name],
                'Packet and stage hashes differ')
    for name, value in FIXED_SOURCES.items():
        require(binding['packet_files'][name] == value, 'Frozen source hash differs')
    launch = strict_json(safe_path(root, MARKER).read_bytes())
    require(set(launch) == {'experiment', 'stage_commit', 'stage_tree', 'binding_sha256'},
            'Unexpected marker schema')
    require(launch['experiment'] == EXPERIMENT and launch['binding_sha256'] == sha(binding_path),
            'Marker does not bind the reviewed packet')
    stage = launch['stage_commit']
    require(type(stage) is str and re.fullmatch('[0-9a-f]{40}', stage), 'Invalid stage commit')
    require(re.fullmatch('[0-9a-f]{40}', launch['stage_tree']), 'Invalid stage tree')
    require(event.get('before') == stage, 'Launch must follow exact stage')
    require(git(root, 'show', '-s', '--format=%P', head).decode().strip() == stage,
            'Launch must have exactly the stage parent')
    require(git(root, 'show', '-s', '--format=%P', stage).decode().strip() == BASE,
            'Stage must have exactly the reviewed main parent')
    require(object_id(root, stage + '^{tree}') == launch['stage_tree'], 'Stage tree differs')
    require(object_id(root, 'refs/remotes/origin/main') == BASE
            and object_id(root, BASE + '^{tree}') == BASE_TREE, 'Fetched main drifted')
    require(git(root, 'diff', '--name-status', stage, head, '--').decode() == 'A\t' + MARKER + '\n',
            'Launch may only add the marker')
    require(not git(root, 'ls-tree', '--name-only', stage, '--', MARKER), 'Stage already has marker')
    changed = set(git(root, 'diff', '--name-only', BASE, stage, '--').decode().splitlines())
    require(changed == STAGE_FILES | {BINDING}, 'Stage diff differs from exact allowlist')
    for name, expected_hash in binding['stage_files'].items():
        require(sha(safe_path(root, name)) == expected_hash, 'Checkout hash differs: ' + name)
        require(digest(git(root, 'show', stage + ':' + name)) == expected_hash,
                'Stage blob hash differs: ' + name)
        entry = git(root, 'ls-tree', stage, '--', name).decode()
        require(entry.startswith('100644 blob ') and entry.endswith('\t' + name + '\n'),
                'Stage file mode differs: ' + name)
    require(git(root, 'show', stage + ':' + BINDING) == binding_path.read_bytes(), 'Stage binding differs')
    workflows = set(git(root, 'ls-tree', '-r', '--name-only', BASE, '--', '.github/workflows').decode().splitlines())
    require(workflows == set(binding['baseline_workflows']) and len(workflows) == 19,
            'Baseline workflow inventory differs')
    for name, value in binding['baseline_workflows'].items():
        require(re.fullmatch('[0-9a-f]{40}', value) and object_id(root, BASE + ':' + name) == value,
                'Baseline workflow blob differs: ' + name)
    for name in ('.github/workflows/verify.yml', '.github/workflows/macos-portability.yml'):
        baseline = git(root, 'show', BASE + ':' + name)
        require(baseline.count(b'  push:\n') == 1, 'Unexpected baseline push syntax')
        expected_bytes = baseline.replace(b'  push:\n',
            ('  push:\n    branches-ignore: [' + BRANCH + "]\n    tags: ['**']\n").encode(), 1)
        require(safe_path(root, name).read_bytes() == expected_bytes,
                'Temporary workflow exclusion changed other semantics')
    for name in ('tree/rerooting.hpp', 'components/undirected_csr.hpp'):
        require(digest(git(root, 'show', BASE + ':' + name)) == FIXED_SOURCES['base/' + name],
                'Baseline source differs')
    return binding, {'experiment': EXPERIMENT, 'head': head, 'stage': stage,
                     'stage_tree': launch['stage_tree'], 'base_commit': BASE,
                     'binding_sha256': sha(binding_path), 'workflow_sha256': binding['stage_files'][WORKFLOW],
                     'run_id': env.get('GITHUB_RUN_ID'), 'run_number': env['GITHUB_RUN_NUMBER'],
                     'run_attempt': env['GITHUB_RUN_ATTEMPT'],
                     'image_os': env['ImageOS'], 'image_version': env['ImageVersion']}


def output_path(root, env, *, exists=False):
    temporary = Path(env['RUNNER_TEMP']).resolve(strict=True)
    require(temporary.is_dir(), 'Runner temporary root is not a directory')
    out = temporary / OUTPUT_NAME
    require(not out.is_symlink() and not out.resolve().is_relative_to(root),
            'Output must be outside checkout without symlink')
    if exists:
        require(out.is_dir(), 'No prepared output exists')
    return out


def prepare(root, env):
    binding, receipt = audit(root, env)
    out = output_path(root, env)
    out.mkdir(exist_ok=False)
    for name in PACKET_FILES:
        source = safe_path(root / PACKAGE, name)
        require(sha(source) == binding['packet_files'][name], 'Source changed before copy')
        target = out / name
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open('xb') as dest:
            dest.write(source.read_bytes())
        require(sha(target) == binding['packet_files'][name], 'Copy hash differs')
    # All selected bytes, including the two Python-only compatibility changes,
    # are rechecked after the copy. No compiler/worker process is started here.
    for name in PACKET_FILES:
        require(sha(safe_path(root / PACKAGE, name)) == binding['packet_files'][name],
                'Source changed after copy')
    receipt.update(status='prepared', packet_files=binding['packet_files'], binding=binding)
    with (out / 'prepare-receipt.json').open('x') as dest:
        json.dump(receipt, dest, sort_keys=True, allow_nan=False)
        dest.write('\n')
    workflow_hash = receipt['workflow_sha256']
    require(re.fullmatch('[0-9a-f]{64}', workflow_hash), 'Unsafe workflow output')
    with Path(env['GITHUB_OUTPUT']).open('a') as dest:
        dest.write('workflow_sha256=' + workflow_hash + '\n')
    emit('prepared', **receipt)
    return receipt


def classification(name):
    if name in PACKET_FILES:
        return 'bound_source_hash_only'
    if re.fullmatch(r'benchmark/fixtures/(baseline|candidate)/cell-(0[1-9]|10)-seed-85712[3-5]\.(input|output)\.bin', name):
        return 'canonical_fixture_hash_only'
    if re.fullmatch(r'benchmark/build/(worker\.o|kernel-(baseline|candidate)\.o|worker-(baseline|candidate))', name):
        return 'compiled_binary_hash_only'
    if re.fullmatch(r'benchmark/__pycache__/[A-Za-z_][A-Za-z_0-9]*\.cpython-[0-9]+\.pyc', name):
        return 'python_bytecode_hash_only'
    # No extension-based fallback: every unrecognized generated file is sent.
    # In particular every raw, partial, failure, compiler and control log is exact.
    return 'exact_bytes'


def file_inventory(out):
    result = {}
    paths = []
    for path in out.rglob('*'):
        require(len(paths) < MAX_TRANSPORT_ENTRIES, 'Too many evidence tree entries')
        paths.append(path)
    original_bytes = exact_bytes = 0
    for path in sorted(paths):
        require(not path.is_symlink(), 'Symlink in evidence tree')
        if path.is_dir():
            continue
        info = regular(path)
        original_bytes += info.st_size
        require(original_bytes <= MAX_ORIGINAL_TOTAL_BYTES,
                'Original evidence inventory exceeds artifact budget')
        name = path.relative_to(out).as_posix()
        transport_name(name)
        require(len(result) < MAX_TRANSPORT_FILES, 'Too many evidence files')
        kind = classification(name)
        if kind == 'exact_bytes':
            exact_bytes += info.st_size
            require(info.st_size <= MAX_EXACT_FILE_BYTES
                    and exact_bytes <= MAX_EXACT_TOTAL_BYTES, 'Exact evidence exceeds transport bound')
        result[name] = {'bytes': info.st_size, 'sha256': sha(path), 'transport': kind}
    return result


def transport_name(name):
    require(type(name) is str and 0 < len(name.encode('utf-8')) <= MAX_PATH_BYTES
            and re.fullmatch(r'[A-Za-z0-9_][A-Za-z0-9_+./-]*', name), 'Invalid transport path')
    relative = PurePosixPath(name)
    require(not relative.is_absolute() and '..' not in relative.parts
            and relative.as_posix() == name, 'Noncanonical transport path')
    return relative


class GzipSink:
    """Hash/count gzip bytes; optionally emit fixed-size chunks without spooling."""
    def __init__(self, on_chunk=None):
        self.on_chunk = on_chunk
        self.size = 0
        self.hash = hashlib.sha256()
        self.pending = bytearray()

    def write(self, data):
        self.size += len(data)
        require(self.size <= MAX_COMPRESSED_FILE_BYTES, 'Compressed file exceeds transport bound')
        self.hash.update(data)
        if self.on_chunk is not None:
            view = memoryview(data)
            while view:
                take = min(CHUNK_BYTES - len(self.pending), len(view))
                self.pending.extend(view[:take])
                view = view[take:]
                if len(self.pending) == CHUNK_BYTES:
                    self.on_chunk(bytes(self.pending))
                    self.pending.clear()
        return len(data)

    def finish(self):
        if self.pending:
            self.on_chunk(bytes(self.pending))
            self.pending.clear()
        return {'encoding': 'gzip+base64', 'compressed_bytes': self.size,
                'compressed_sha256': self.hash.hexdigest()}


def gzip_file(path, entry, on_chunk=None):
    """Deterministic gzip of the exact original bytes, with bounded memory."""
    sink = GzipSink(on_chunk)
    original = hashlib.sha256()
    count = 0
    with path.open('rb') as source:
        with gzip.GzipFile(filename='', mode='wb', compresslevel=6, mtime=0,
                           fileobj=sink) as compressed:
            for chunk in iter(lambda: source.read(IO_BYTES), b''):
                count += len(chunk)
                require(count <= entry['bytes'] and count <= MAX_EXACT_FILE_BYTES,
                        'Evidence grew while compressing: ' + str(path))
                original.update(chunk)
                compressed.write(chunk)
    require(count == entry['bytes'] and original.hexdigest() == entry['sha256'],
            'Evidence changed while compressing: ' + str(path))
    return sink.finish()


def transport(root, env):
    out = output_path(root, env, exists=True)
    before = file_inventory(out)
    require(before, 'Prepared evidence directory is empty')
    inventory = {}
    compressed_total = 0
    # First pass records the gzip size/hash without retaining payload or a spool.
    # The unchanged 60-second main alarm includes both passes and every emit.
    for name, original in before.items():
        entry = dict(original)
        if entry['transport'] == 'exact_bytes':
            entry.update(gzip_file(out / name, entry))
            compressed_total += entry['compressed_bytes']
            require(compressed_total <= MAX_COMPRESSED_TOTAL_BYTES,
                    'Compressed evidence exceeds transport bound')
        inventory[name] = entry
    inventory_hash = digest(json.dumps(inventory, sort_keys=True, separators=(',', ':')).encode())
    emit('transport_begin', experiment=EXPERIMENT, inventory_sha256=inventory_hash,
         transport_schema_version=2,
         files=len(before), total_file_bytes=sum(value['bytes'] for value in before.values()),
         total_compressed_bytes=compressed_total,
         has_experiment_result='benchmark/experiment/experiment-result.json' in before)
    transported_bytes = 0
    for name, entry in inventory.items():
        emit('file_start', path=name, **entry)
        count, offset = 0, 0
        if entry['transport'] == 'exact_bytes':
            def send_chunk(chunk):
                nonlocal count, offset
                emit('file_chunk', path=name, index=count, offset=offset,
                     bytes=len(chunk), data_base64=base64.b64encode(chunk).decode('ascii'))
                count += 1
                offset += len(chunk)
            compressed = gzip_file(out / name, entry, send_chunk)
            require(all(entry[key] == value for key, value in compressed.items())
                    and offset == entry['compressed_bytes'],
                    'Compressed evidence changed between passes: ' + name)
        emit('file_end', path=name, chunks=count, transported_bytes=offset, sha256=entry['sha256'])
        transported_bytes += offset
    require(file_inventory(out) == before, 'Evidence changed after transport')
    require(transported_bytes == compressed_total, 'Compressed transport total differs')
    emit('transport_complete', experiment=EXPERIMENT, inventory_sha256=inventory_hash,
         files=len(before), transported_bytes=transported_bytes,
         performance_interpretation='Requires complete reconstructed logs and validated experiment outcome; transport is not a performance pass.')
    return inventory


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=('prepare', 'transport'))
    args = parser.parse_args(argv)
    seconds = 30 if args.mode == 'prepare' else 60
    def expired(signum, frame):
        # Never block while printing a timeout into a stalled log sink.
        # No prepare output or transport footer means terminal/inconclusive.
        os._exit(2)
    signal.signal(signal.SIGALRM, expired)
    signal.setitimer(signal.ITIMER_REAL, seconds)
    try:
        root = Path(os.environ['GITHUB_WORKSPACE']).resolve(strict=True)
        require(root == Path(__file__).resolve().parents[3], 'Adapter is not in exact checkout')
        if args.mode == 'prepare':
            prepare(root, os.environ)
        else:
            transport(root, os.environ)
        return 0
    except (AdapterError, OSError, ValueError, KeyError, subprocess.TimeoutExpired) as exc:
        emit('adapter_failure', mode=args.mode, status='inconclusive',
             error_type=type(exc).__name__, message=str(exc))
        return 2
    finally:
        signal.setitimer(signal.ITIMER_REAL, 0)


if __name__ == '__main__':
    sys.exit(main())
