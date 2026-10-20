#!/usr/bin/env python3
"""Validate every supplied Nim control against independent oracle answers."""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import subprocess


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--cases', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    record = json.loads((args.reference / 'preparation.json').read_text())
    assert record['compiled']
    assert sha(args.reference / 'Supplied.nim') == record['source_sha256']
    assert sha(args.reference / 'Instrumented.nim') == record['api_source_sha256']
    result = {'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'helper_sha256': sha(Path(__file__)), 'source_sha256': record['source_sha256'],
              'preparation_sha256': sha(args.reference / 'preparation.json'), 'rows': []}
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {2})
        for case_path in sorted(args.cases.glob('*.json')):
            if case_path.name == 'manifest.json':
                continue
            case = json.loads(case_path.read_text())
            input_path, expected_path = Path(case['input']['path']), Path(case['expected']['path'])
            assert sha(input_path) == case['input']['sha256']
            expected = expected_path.read_bytes()
            assert sha(expected_path) == case['expected']['sha256']
            for variant, binary in record['binaries'].items():
                path = Path(binary['path'])
                assert sha(path) == binary['sha256']
                with input_path.open('rb') as source:
                    process = subprocess.run([str(path)], stdin=source, capture_output=True,
                                             check=True, timeout=120)
                assert process.stdout == expected, (case['name'], variant, 'wrong answer')
                row = {'case': case['name'], 'variant': variant,
                       'input_sha256': case['input']['sha256'], 'output_sha256': case['expected']['sha256'],
                       'answers': case['stats']['query'], 'binary_sha256': binary['sha256']}
                if variant.endswith('_api'):
                    row['timers'] = json.loads(process.stderr)
                    assert row['timers']['answers'] == row['answers']
                result['rows'].append(row)
                args.output.parent.mkdir(parents=True, exist_ok=True)
                args.output.write_text(json.dumps(result, indent=2) + '\n')
            print('validated', case['name'], len(record['binaries']), flush=True)


if __name__ == '__main__':
    main()
