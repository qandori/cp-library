from pathlib import Path
from functools import lru_cache
import re
import subprocess


BASE = '5012542'
HEADERS = ['components/dynamic_bit_vector.hpp', 'components/dynamic_quad_vector.hpp',
           'components/dynamic_quad_wavelet.hpp', 'data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp',
           'components/fast_io.hpp']


def sources(repo, baseline=False):
    return {p: subprocess.check_output(['git', 'show', f'{BASE}:{p}'], cwd=repo).decode() if baseline
            else (repo / p).read_text() for p in HEADERS}


@lru_cache(None)
def baseline_vector():
    return subprocess.check_output(
        ['git', 'show', BASE + ':components/dynamic_quad_vector.hpp'],
        cwd=Path(__file__).resolve().parents[3]).decode()


def configure(headers, block=1024, quarter=False, full=False, pair=False, compact=False):
    result = dict(headers)
    vector = result['components/dynamic_quad_vector.hpp']
    matrix = result['components/dynamic_quad_wavelet.hpp']
    vector = vector.replace('size) / (Block / 4) + 2', 'size) / (Block / 2) + 2')
    vector = vector.replace('.length < Block / 4', '.length < Block / 2')
    vector = vector.replace('left.length <= Block / 4', 'left.length <= Block / 2')
    vector = vector.replace('int target = Block,', 'int target = Block * 3 / 4,')
    vector = vector.replace(
        '\t\tstd::size_t capacity = 1;\n'
        '\t\twhile (capacity < (values.size() - 1) / Block + 1) capacity *= Branch;\n'
        '\t\tcount = int(std::min(std::size_t(count), capacity));\n', '')
    if not pair:
        for first, following in [
            ('\tstd::pair<int, int> rank(int l, int r, int digit) const {\n', '\tCounts ranks(int p) const {\n'),
            ('\tstd::pair<Counts, Counts> ranks(int l, int r) const {\n', '\tstd::pair<int, int> access_rank(int p) const {\n')]:
            begin, end = vector.index(first), vector.index(following, vector.index(first))
            base = baseline_vector()
            vector = vector[:begin] + base[base.index(first):base.index(following, base.index(first))] + vector[end:]
    if quarter:
        vector = vector.replace('size) / (Block / 2) + 2', 'size) / (Block / 4) + 2')
        vector = vector.replace('.length < Block / 2', '.length < Block / 4')
        vector = vector.replace('left.length <= Block / 2', 'left.length <= Block / 4')
    if full:
        vector = vector.replace('int target = Block * 3 / 4', 'int target = Block')
    if compact:
        vector = vector.replace(
            '\t\tleaves.reserve(count);',
            '\t\tstd::size_t capacity = 1;\n'
            '\t\twhile (capacity < (values.size() - 1) / Block + 1) capacity *= Branch;\n'
            '\t\tcount = int(std::min(std::size_t(count), capacity));\n'
            '\t\tleaves.reserve(count);')
    if pair:
        from pair_rank_candidate import apply
        vector = apply(vector)
    matrix = re.sub(r'int Block = \d+, int Branch = 32', f'int Block = {block}, int Branch = 32', matrix)
    result['components/dynamic_quad_vector.hpp'] = vector
    result['components/dynamic_quad_wavelet.hpp'] = matrix
    return result


def save(directory, headers):
    directory = Path(directory)
    for name, text in headers.items():
        path = directory / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
