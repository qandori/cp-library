import argparse
import json
import pathlib
import re
import subprocess

import yaml


root = pathlib.Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--selection', type=pathlib.Path)
args = parser.parse_args()
selection = json.loads(args.selection.read_text()) if args.selection else None
selected = set(selection.get('bundles', selection['headers'])) if selection is not None else None
headers = sorted(path for folder in ('convolution', 'data_structure', 'graph', 'math', 'string') for path in (root / folder).rglob('*.hpp'))
compiled = 0
for header in headers:
    path = header.relative_to(root)
    doc = root / '.verify-helper/markdown' / (str(path) + '.md')
    data = yaml.safe_load(doc.read_text().split('---', 2)[1])
    assert data['title'] != str(path), f'{path}: documentation is missing'
    code = data['data']['bundledCode']
    assert 'Traceback (most recent call last)' not in code, f'{path}: bundle failed'
    assert not re.search(r'^\s*#\s*include\s*"', code, re.MULTILINE), f'{path}: local include remains'
    if selected is None or path.as_posix() in selected:
        subprocess.run(['g++', '-std=c++23', '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True, check=True, cwd=root)
        compiled += 1
print(f'Checked documentation for {len(headers)} libraries; compiled {compiled} affected bundles')
