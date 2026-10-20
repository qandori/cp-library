#!/usr/bin/env python3
"""Export portable provenance for the frozen Nim comparison controls."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import urllib.request


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    directory = args.reference.resolve()
    preparation = directory / 'preparation.json'
    record = json.loads(preparation.read_text())
    assert record['compiled']
    assert sha(directory / 'Supplied.nim') == record['source_sha256']
    assert sha(directory / 'Instrumented.nim') == record['api_source_sha256']
    upstream = []
    for url in record['embedded_references']:
        relative = url.split('/src/', 1)[1]
        target = directory / 'upstream' / relative
        raw_url = url.replace('https://github.com/kemuniku/cplib/blob/',
                              'https://raw.githubusercontent.com/kemuniku/cplib/')
        if not target.is_file():
            request = urllib.request.Request(raw_url, headers={'User-Agent': 'cp-library-benchmark'})
            with urllib.request.urlopen(request, timeout=60) as response:
                data = response.read()
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        upstream.append({'path': relative, 'url': raw_url, 'sha256': sha(target),
                         'bytes': target.stat().st_size})

    def portable(value):
        if isinstance(value, str):
            return value.replace(str(directory), '${REFERENCE}')
        if isinstance(value, dict):
            return {k: portable(v) for k, v in value.items()}
        if isinstance(value, list):
            return [portable(v) for v in value]
        return value

    binaries = {}
    for name, binary in record['binaries'].items():
        assert sha(binary['path']) == binary['sha256']
        build = binary['backend_build']
        binaries[name] = {'sha256': binary['sha256'], 'command': record['commands'][name],
                          'backend_compile': build['compile'], 'backend_link': build['linkcmd'],
                          'build_log_sha256': binary['build_log_sha256']}
    result = {'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'export_helper_sha256': sha(__file__), 'preparation_sha256': sha(preparation),
              'prepared_utc': record['utc'], 'preparation_helper_sha256': record['helper_sha256'],
              'supplied_source_sha256': record['source_sha256'], 'instrumented_source_sha256': record['api_source_sha256'],
              'source_origin': 'user supplied self-contained Nim source, including originalSource metadata template',
              'upstream_commit': record['library_commit'], 'upstream_files': upstream,
              'license': {'url': record['license']['url'], 'sha256': record['license']['sha256'],
                          'bytes': record['license']['bytes'], 'name': 'CC0-1.0'},
              'nim_version': record['nim_version'], 'nim_binary_sha256': record['nim_sha256'],
              'nim_distribution': record['nim_archive'], 'backend_compilers': record['backend_compilers'],
              'semantics': record['semantics'], 'official_control': record['official'],
              'optimized_control': record['optimized'], 'binaries': binaries,
              'portable_path': '${REFERENCE} is the prepared reference directory'}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(portable(result), indent=2) + '\n')
    print('exported', len(upstream), 'upstream files and', len(binaries), 'binary controls')


if __name__ == '__main__':
    main()
