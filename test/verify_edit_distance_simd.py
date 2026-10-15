import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--simde', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--library', choices=('edit_distance', 'lcs'), default='edit_distance')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    assert (args.simde / 'simde/x86/avx512.h').is_file()
    source = (root / f'string/{args.library}.hpp').read_text()
    source = source.replace('#if defined(__x86_64__)', '#if 1')
    source = source.replace('#include <immintrin.h>', '''
#define SIMDE_NO_NATIVE
#define SIMDE_ENABLE_NATIVE_ALIASES
#include <simde/x86/avx512.h>
#if !defined(_mm512_alignr_epi64)
inline simde__m512i edit_distance_test_alignr(simde__m512i a, simde__m512i b, int k) {
    auto x = simde__m512i_to_private(a), y = simde__m512i_to_private(b);
    simde__m512i_private result{};
    for (int j = 0; j < 8; ++j) result.u64[j] = j + k < 8 ? y.u64[j + k] : x.u64[j + k - 8];
    return simde__m512i_from_private(result);
}
#define _mm512_alignr_epi64(a, b, k) edit_distance_test_alignr(a, b, k)
#endif
''')
    for isa in ('avx2', 'avx512f'):
        source = source.replace(f'__attribute__((target("{isa}"))) ', '')
        source = source.replace(f'__builtin_cpu_supports("{isa}")', 'false')
    with tempfile.TemporaryDirectory(prefix='edit-distance-simd-') as directory:
        directory = Path(directory)
        header = directory / f'string/{args.library}.hpp'
        header.parent.mkdir()
        header.write_text(source)
        executable = directory / 'test'
        flags = ['-std=c++23', '-O2', '-Wall', '-Wextra', f'-D{args.library.upper()}_EMULATE_SIMD',
                 '-I', str(directory), '-isystem', str(args.simde.resolve()), '-I', str(root)]
        if args.sanitize:
            flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']
        subprocess.run([args.cxx, *flags, str(root / f'test/unit/{args.library}.cpp'), '-o', str(executable)], check=True)
        subprocess.run([str(executable)], check=True, timeout=120)
    print(f'{args.library}: AVX2 and AVX-512 emulation passed')


if __name__ == '__main__':
    main()
