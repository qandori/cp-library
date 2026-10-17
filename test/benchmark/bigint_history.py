import io
from pathlib import Path
import subprocess
import tarfile


BASE_REVISION = 'a6c027bfe8b8c35a809e24a8429f53ece621cd5f'
LAST_REVISION = 'b15be5484cf6397d754d5df3eca7d30fd3b84713'


def materialize(root, destination, revision=BASE_REVISION):
    destination = destination.resolve()
    destination.mkdir(parents=True, exist_ok=True)
    archive = subprocess.check_output(['git', 'archive', revision, 'components', 'math', 'graph'], cwd=root)
    with tarfile.open(fileobj=io.BytesIO(archive)) as source:
        for member in source:
            if not member.isfile() or not member.name.endswith('.hpp'):
                continue
            target = (destination / member.name).resolve()
            if destination not in target.parents:
                raise ValueError(member.name)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(source.extractfile(member).read())
    return destination


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--revision', default=LAST_REVISION)
    args = parser.parse_args()
    materialize(Path(__file__).resolve().parents[2], args.output, args.revision)
