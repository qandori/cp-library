import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

root = Path.cwd().resolve()
package = Path(__file__).resolve().parent
frozen = package / 'frozen'
out = Path(os.environ['RUNNER_TEMP']) / 'matching-components-evidence'
out.mkdir(exist_ok=False)
sha = lambda data: hashlib.sha256(data).hexdigest()
manifest = json.loads((package / 'manifest.json').read_text())
for path, expected in manifest['sha256'].items():
    assert sha((root / path).read_bytes()) == expected, path
allowed = sorted(os.sched_getaffinity(0))
assert 0 in allowed, 'Frozen protocol requires CPU 0'
cxx = shutil.which('g++')
base = subprocess.check_output(['git', 'show', '8c38d74c07610dfd1a5fb13b1beeb18313ce1e04:graph/general_matching.hpp'])
assert sha(base) == manifest['baseline_header_sha256']
assert sha((root / 'graph/general_matching.hpp').read_bytes()) == manifest['baseline_header_sha256']
meta = {'commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
        'base_main': manifest['base_main'], 'source_commit': manifest['source_commit'],
        'allowed_affinity': allowed, 'selected_cpu': 0, 'compiler_path': cxx,
        'compiler': subprocess.check_output([cxx, '--version'], text=True),
        'compiler_binary_sha256': sha(Path(cxx).read_bytes()),
        'lscpu': subprocess.check_output(['lscpu'], text=True),
        'cpuinfo': Path('/proc/cpuinfo').read_text(),
        'uname': subprocess.check_output(['uname', '-a'], text=True),
        'os_release': Path('/etc/os-release').read_text(),
        'manifest': manifest, 'manifest_sha256': sha((package / 'manifest.json').read_bytes()),
        'loadavg_before': Path('/proc/loadavg').read_text()}
(out / 'environment.json').write_text(json.dumps(meta, indent=2))
correctness = out / 'correctness'
correctness.mkdir()
shutil.copyfile(frozen / 'graph/general_matching.hpp', correctness / 'candidate.hpp')
(correctness / 'baseline.hpp').write_bytes(base.replace(b'GENERAL_MATCHING_HPP', b'BASELINE_MATCHING_HPP').replace(b'GeneralMatching', b'BaselineMatching'))
commands = []

def run(command, log, env=None):
    begin = time.perf_counter()
    with log.open('w') as f:
        result = subprocess.run(command, stdout=f, stderr=subprocess.STDOUT, env=env)
    entry = {'command': command, 'exit_code': result.returncode, 'wall_seconds': time.perf_counter() - begin}
    commands.append(entry)
    (out / 'commands.json').write_text(json.dumps(commands, indent=2))
    result.check_returncode()

try:
    for name, source, includes in [
        ('unit', frozen / 'test/unit/general_matching.cpp', [frozen]),
        ('independent', package / 'independent_correctness.cpp', [correctness])
    ]:
        for mode, flags in [('O2', ['-O2']), ('sanitizer', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]:
            binary = correctness / (name + '-' + mode + '.bin')
            command = [cxx, '-std=c++23', '-Wall', '-Wextra', *flags]
            for include in includes:
                command += ['-iquote', str(include)]
            command += [str(source), '-o', str(binary)]
            run(command, correctness / (name + '-' + mode + '-compile.log'))
            environment = dict(os.environ, ASAN_OPTIONS='detect_leaks=0:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1')
            run(['taskset', '-c', '0', str(binary)], correctness / (name + '-' + mode + '.log'), environment)
    run(['python3', str(frozen / 'test/benchmark/general_matching_components.py'), str(out / 'run'), '--stage', 'final'], out / 'protocol.log')
finally:
    meta['loadavg_after'] = Path('/proc/loadavg').read_text()
    (out / 'environment.json').write_text(json.dumps(meta, indent=2))
    hashes = {str(p.relative_to(out)): sha(p.read_bytes()) for p in sorted(out.rglob('*')) if p.is_file()}
    (out / 'evidence-hashes.json').write_text(json.dumps(hashes, indent=2))
    for p in sorted(out.rglob('*')):
        if p.is_file() and p.suffix in {'.json', '.jsonl', '.csv', '.txt', '.log'}:
            print('BEGIN_EVIDENCE', str(p.relative_to(out)), flush=True)
            print(p.read_text(), flush=True)
            print('END_EVIDENCE', str(p.relative_to(out)), flush=True)
