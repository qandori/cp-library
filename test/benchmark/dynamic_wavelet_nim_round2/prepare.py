#!/usr/bin/env python3
"""Freeze the supplied Nim source and build distinct compiler controls."""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import urllib.request

NIM_URL = 'https://nim-lang.org/download/nim-2.2.4-linux_x64.tar.xz'
NIM_SHA256 = '791802138aaf19c8579232c50b4998ce2ae2928b791127ce5b4ef3c7af53fb46'
PIN = '244c855f5961a2ce03a3acebed8eac1f5f4099fc'
LICENSE_URL = f'https://raw.githubusercontent.com/kemuniku/cplib/{PIN}/LICENSE'
OFFICIAL = ['cpp', '-d:release', '--opt:speed', '--multimethods:on',
            '--warning[SmallLshouldNotBeUsed]:off', '--hints:off',
            '--maxLoopIterationsVM:10000000000000', '--maxCallDepthVM:10000000000000',
            '-d:second_compile']
OPTIMIZED = ['c', '-d:danger', '-d:second_compile', '-d:useMalloc', '--gc:arc',
             '--panics:on', '--opt:speed', '--checks:off',
             '--passC:-flto -m64 -march=native -ffast-math -funroll-loops -fipa-pta',
             '--passL:-flto', '--hints:off']
API_MAIN = '''
import std/times
let N = ii()
let Q = ii()
let A = lii(N)
var operations = newSeq[tuple[kind, a, b: int]](Q)
var answers = newSeq[int](Q)
for i in 0..<Q:
    operations[i] = (ii(), ii(), ii())
let kernel = if kernelAvailable(bpkAvx512): bpkAvx512 else: bpkAuto
let startWall = epochTime()
let startCpu = cpuTime()
var wm = initDynamicQuadWaveletMatrixFast(A, H = 30, kernel = kernel)
let builtCpu = cpuTime()
let builtWall = epochTime()
var answerCount = 0
for op in operations:
    if op.kind == 1:
        wm[op.a - 1] = op.b
    else:
        let l = op.a - 1
        let r = op.b
        let mx = wm.kth_largest(l, r, 0)
        let x = wm.prev_value(l, r, mx)
        answers[answerCount] = if x.isNone: 0 else: wm.count(l, r, x.get())
        inc answerCount
let doneCpu = cpuTime()
let doneWall = epochTime()
stderr.writeLine("{\\\"build_cpu\\\":", builtCpu - startCpu,
    ",\\\"operation_cpu\\\":", doneCpu - builtCpu,
    ",\\\"build_wall\\\":", builtWall - startWall,
    ",\\\"operation_wall\\\":", doneWall - builtWall,
    ",\\\"answers\\\":", answerCount, "}")
for i in 0..<answerCount:
    print(answers[i])
'''


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def download(url, target):
    if not target.is_file():
        request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'})
        with urllib.request.urlopen(request, timeout=90) as response:
            target.write_bytes(response.read())
    return {'url': url, 'path': str(target), 'sha256': sha(target),
            'bytes': target.stat().st_size}


def toolchain(directory):
    directory.mkdir(parents=True, exist_ok=True)
    archive = directory / 'nim-2.2.4-linux_x64.tar.xz'
    record = download(NIM_URL, archive)
    if record['sha256'] != NIM_SHA256:
        raise RuntimeError('Official Nim archive SHA256 mismatch')
    record['sha256_url'] = NIM_URL + '.sha256'
    nim = directory / 'nim-2.2.4/bin/nim'
    if not nim.exists():
        with tarfile.open(archive) as stream:
            for member in stream.getmembers():
                target = directory / member.name
                if not target.resolve().is_relative_to(directory.resolve()):
                    raise RuntimeError('Unsafe archive member: ' + member.name)
                if member.issym() or member.islnk():
                    link = target.parent / member.linkname if member.issym() else directory / member.linkname
                    if not link.resolve().is_relative_to(directory.resolve()):
                        raise RuntimeError('Unsafe archive link: ' + member.name)
            stream.extractall(directory)
    return nim, record


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--nim', type=Path)
    parser.add_argument('--compile', action='store_true')
    parser.add_argument('--variants', nargs='+', choices=['official_stream', 'official_api', 'optimized_stream', 'optimized_api'],
                        default=['official_stream', 'official_api', 'optimized_stream', 'optimized_api'])
    parser.add_argument('--lock', type=Path, default=Path('/tmp/cp-wm-opt.lock'))
    args = parser.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    frozen = work / 'Supplied.nim'
    original = args.source.read_bytes()
    if frozen.exists() and frozen.read_bytes() != original:
        raise RuntimeError('Refusing to replace a different frozen source')
    frozen.write_bytes(original)
    source = original.decode('utf-8-sig')
    if source.count('\noptimize()\n') != 1:
        raise RuntimeError('The supplied main boundary changed')
    prefix, stream_main = source.split('\noptimize()\n')
    if 'initDynamicQuadWaveletMatrixFast(A, H = 30, kernel = kernel)' not in stream_main:
        raise RuntimeError('Unexpected supplied main')
    # The unused metadata template stays in both sources. The real optimize call
    # stays in the stream source, guarded only by -d:second_compile.
    api = work / 'Instrumented.nim'
    api.write_text(prefix + '\n' + API_MAIN)
    nim, archive = (args.nim.resolve(), None) if args.nim else toolchain(work / 'tools')
    version = subprocess.check_output([str(nim), '--version'], text=True)
    if 'Nim Compiler Version 2.2.4 ' not in version:
        raise RuntimeError('Nim2.2.4 is required')
    license_record = download(LICENSE_URL, work / 'LICENSE')
    references = sorted(set(re.findall(r'https://github\.com/kemuniku/cplib/blob/[^\s]+', source)))
    metadata = {'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                'helper_sha256': sha(__file__), 'source_origin': str(args.source.resolve()),
                'source_sha256': sha(frozen), 'api_source_sha256': sha(api),
                'library_commit': PIN, 'embedded_references': references, 'license': license_record,
                'nim_version': version, 'nim_path': str(nim), 'nim_sha256': sha(nim),
                'nim_archive': archive, 'uname': list(os.uname()),
                'lscpu': subprocess.check_output(['lscpu'], text=True),
                'gcc': subprocess.check_output(['gcc', '--version'], text=True),
                'gxx': subprocess.check_output(['g++', '--version'], text=True),
                'backend_compilers': {name: {'path': shutil.which(name), 'sha256': sha(shutil.which(name))}
                                      for name in ['gcc', 'g++']},
                'semantics': 'unsigned/nonnegative H=30, FastQuadVector, kth_largest(0), prev_value, count; explicit AVX512 if available, otherwise auto',
                'official': 'AtCoder Nim cpp release flags; second_compile only bypasses the source self-recompilation macro',
                'optimized': 'Exact flags from the supplied optimize() second-stage nim c command, C backend, ARC, danger, LTO, native; not an equal compiler control',
                'compiled': args.compile, 'commands': {}, 'binaries': {}}
    for variant in args.variants:
        flags = OFFICIAL if variant.startswith('official') else OPTIMIZED
        source_path = api if variant.endswith('_api') else frozen
        binary = work / variant
        command = [str(nim), *flags, '--listCmd:on', '--nimcache:' + str(work / (variant + '-cache')),
                   '--out:' + str(binary), str(source_path)]
        metadata['commands'][variant] = command
    if args.compile:
        with args.lock.open('w') as lock:
            fcntl.flock(lock, fcntl.LOCK_EX)
            for variant, command in metadata['commands'].items():
                print('compile', variant, flush=True)
                with (work / (variant + '-build.log')).open('w') as log:
                    subprocess.run(command, cwd=work, stdout=log, stderr=subprocess.STDOUT, check=True)
                backend_path = work / (variant + '-cache') / (variant + '.json')
                metadata['binaries'][variant] = {'path': str(work / variant), 'sha256': sha(work / variant),
                                                 'build_log_sha256': sha(work / (variant + '-build.log')),
                                                 'backend_build': json.loads(backend_path.read_text())}
    (work / 'preparation.json').write_text(json.dumps(metadata, indent=2) + '\n')
    print(json.dumps({'work': str(work), 'compiled': args.compile,
                      'source_sha256': metadata['source_sha256'], 'binaries': metadata['binaries']}, indent=2))


if __name__ == '__main__':
    main()
