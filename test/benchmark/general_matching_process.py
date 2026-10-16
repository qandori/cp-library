import csv
import random
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

cases, reference, output = map(Path, sys.argv[1:4])
root = Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='matching-process-') as directory:
    paths = {name: Path(directory) / name for name in ['current', 'reference']}
    for name, source in [('current', root / 'test/general_matching.test.cpp'), ('reference', reference)]:
        subprocess.run(['g++', '-std=c++23', '-O2', str(source), '-o', str(paths[name])], check=True)
    rng = random.Random(39121)
    with output.open('w') as stream:
        writer = csv.writer(stream)
        writer.writerow(['case', 'variant', 'median_ms', 'matching_size'])
        for case in sorted(cases.glob('*.in')):
            expected = int(case.with_suffix('.out').read_text().split()[0])
            times = {name: [] for name in paths}
            for trial in range(9):
                order = list(paths)
                rng.shuffle(order)
                for name in order:
                    with case.open('rb') as data, tempfile.TemporaryFile() as result:
                        start = time.perf_counter_ns()
                        subprocess.run([str(paths[name])], stdin=data, stdout=result, check=True)
                        elapsed = (time.perf_counter_ns() - start) / 1000000
                        result.seek(0)
                        assert int(result.readline()) == expected, (case, name)
                    times[name].append(elapsed)
            for name in paths:
                writer.writerow([case.stem, name, statistics.median(times[name]), expected])
