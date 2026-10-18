import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def allocator_probe(work, flags):
    source = work / 'allocator.cpp'
    source.write_text('''#include "components/fps_workspace.hpp"
#include <fstream>
#include <iostream>
#include <string>
int main() {
    constexpr std::size_t n = (1U << 20) + 17;
    auto storage = fps_detail::allocate_fps_words(n);
    volatile unsigned int *p = storage.get();
    for (std::size_t i = 0; i < n; ++i) p[i] = unsigned(i);
    std::ifstream input("/proc/self/smaps_rollup");
    for (std::string line; std::getline(input, line);)
        if (line.starts_with("Rss:") || line.starts_with("AnonHugePages:")) std::cout << line << '\\n';
}
''')
    binary = work / 'allocator.out'
    subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
    return subprocess.check_output([str(binary)], text=True).splitlines()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='e348f396c801964e72d4b1460a39dae91fdc88a3')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(baseline=args.baseline, flags=flags, size=524288, samples=[], sources={})
    with tempfile.TemporaryDirectory(prefix='fps-workspace-memory-') as temporary:
        work = Path(temporary)
        before, current = work / 'before', work / 'current'
        data = subprocess.check_output(['git', 'archive', args.baseline, 'components', 'fps', 'math', 'convolution'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data)) as archive:
            archive.extractall(before, filter='data')
        for folder in ['components', 'fps', 'math', 'convolution']:
            shutil.copytree(ROOT / folder, current / folder)
        for name, tree in [('before', before), ('current', current)]:
            binary = work / (name + '.out')
            result['sources'][name] = {p.relative_to(tree).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                                       for p in tree.rglob('*.hpp')}
            subprocess.run(['g++', *flags, '-I', str(tree), str(ROOT / 'test/benchmark/fps_tuning_basic.cpp'), '-o', str(binary)], check=True)
            for operation in ['inv', 'log', 'exp', 'sqrt', 'pow']:
                with subprocess.Popen([str(binary), operation, '524288'], stdout=subprocess.PIPE, text=True) as child:
                    _, status, usage = os.wait4(child.pid, 0)
                    child.returncode = os.waitstatus_to_exitcode(status)
                    if child.returncode:
                        raise subprocess.CalledProcessError(child.returncode, child.args)
                    raw = child.stdout.read()
                result['samples'].append(dict(version=name, operation=operation, max_rss_kib=usage.ru_maxrss, checksum=raw.split()[1]))
        result['allocator_probe'] = allocator_probe(work, flags)
    for operation in ['inv', 'log', 'exp', 'sqrt', 'pow']:
        assert len({row['checksum'] for row in result['samples'] if row['operation'] == operation}) == 1
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
