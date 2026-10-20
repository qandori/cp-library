import argparse
import fcntl
import json
from pathlib import Path

from cases import oracle


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--directory', type=Path, required=True)
    args = parser.parse_args()
    args.directory.mkdir(parents=True, exist_ok=True)
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        rows = []
        for n in [8192, 32768, 131072, 196608]:
            for shape in ['random30', 'short']:
                row = oracle.large_case(args.directory, shape, n, 100000, 20261030 + n)
                name = f'{shape}_n{n}'
                for field in ['input', 'expected']:
                    old = Path(row[field]['path'])
                    target = old.with_stem(name)
                    old.rename(target)
                    row[field]['path'] = str(target)
                (args.directory / (shape + '.json')).unlink()
                row['name'] = name
                (args.directory / (name + '.json')).write_text(json.dumps(row, indent=2) + '\n')
                rows.append(row)
        (args.directory / 'manifest.json').write_text(json.dumps(rows, indent=2) + '\n')


if __name__ == '__main__':
    main()
