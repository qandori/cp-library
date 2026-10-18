import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
from fps_tuning_reference import get

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--metadata', type=Path, default=ROOT / 'test/benchmark/results/fps_speed_reference.json')
    args = parser.parse_args()
    metadata = json.loads(args.metadata.read_text())
    tasks = [(problem, reference) for problem, data in metadata['problems'].items() for reference in data.get('references', [])]

    def fetch(task):
        problem, reference = task
        target = args.cache / problem / (str(reference['id']) + '.cpp')
        if target.exists() and hashlib.sha256(target.read_bytes()).hexdigest() == reference['source_sha256']:
            return
        detail = get('https://v3.api.judge.yosupo.jp/submissions/' + str(reference['id']))
        source = detail['source'].encode()
        if hashlib.sha256(source).hexdigest() != reference['source_sha256']:
            raise RuntimeError(f'Reference source changed: {reference["id"]}')
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(source)

    list(ThreadPoolExecutor(max_workers=4).map(fetch, tasks))
    print('Verified reference sources:', len(tasks))


if __name__ == '__main__':
    main()
