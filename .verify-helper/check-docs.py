import pathlib
import re
import subprocess

import yaml


root = pathlib.Path(__file__).resolve().parents[1]
headers = sorted(path for folder in ('convolution', 'data_structure', 'graph', 'math') for path in (root / folder).glob('*.hpp'))
for header in headers:
    path = header.relative_to(root)
    doc = root / '.verify-helper/markdown' / (str(path) + '.md')
    data = yaml.safe_load(doc.read_text().split('---', 2)[1])
    assert data['title'] != str(path), f'{path}: documentation is missing'
    code = data['data']['bundledCode']
    assert 'Traceback (most recent call last)' not in code, f'{path}: bundle failed'
    assert not re.search(r'^\s*#\s*include\s*"', code, re.MULTILINE), f'{path}: local include remains'
    subprocess.run(['g++', '-std=c++23', '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True, check=True, cwd=root)
print(f'Checked documentation and bundled code for {len(headers)} libraries')
