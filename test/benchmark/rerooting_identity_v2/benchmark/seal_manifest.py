#!/usr/bin/env python3
"""Seal complete untimed evidence. No timing, compilation, installation or network."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

from build import (HERE, MAX_ARTIFACT_BYTES, PreparationError, artifact_bytes,
                   inventory, sha, verify_manifest, verify_map, write_json_new)

SEEDS = (857123, 857124, 857125)


def fixture_inventory(manifest: dict) -> dict[str, str]:
    expected = set()
    for label in ('baseline', 'candidate'):
        root = Path(manifest['fixture_roots'][label]).resolve(strict=True)
        if root != (HERE / 'fixtures' / label).resolve():
            raise PreparationError('Unexpected fixture root: ' + str(root))
        for cell in range(1, 11):
            for seed in SEEDS:
                for suffix in ('input', 'output'):
                    path = root / f'cell-{cell:02d}-seed-{seed}.{suffix}.bin'
                    if path.is_symlink() or not path.is_file() or path.stat().st_size == 0:
                        raise PreparationError('Missing/empty/symlink fixture: ' + str(path))
                    expected.add(path)
        actual = {p for p in root.rglob('*') if p.is_file()}
        if actual != {p for p in expected if p.parent == root}:
            raise PreparationError('Unexpected or missing files in fixture root: ' + str(root))
    frozen = inventory(expected)
    for cell in range(1, 11):
        for seed in SEEDS:
            for suffix in ('input', 'output'):
                name = f'cell-{cell:02d}-seed-{seed}.{suffix}.bin'
                a = Path(manifest['fixture_roots']['baseline']) / name
                b = Path(manifest['fixture_roots']['candidate']) / name
                # Compare full canonical bytes, not just hashes.
                if a.read_bytes() != b.read_bytes():
                    raise PreparationError('Baseline/candidate canonical mismatch: ' + name)
    return frozen


def seal(compiled=None, validation=None, test_logs=None, output=None):
    compiled = Path(compiled or HERE / 'build-manifest.compiled.json').resolve(strict=True)
    validation = Path(validation or HERE / 'worker-validation.json').resolve(strict=True)
    output = Path(output or HERE / 'build-manifest.json')
    if output.exists():
        raise PreparationError('Refusing to overwrite or silently reseal a manifest')
    before = sha(compiled)
    manifest = json.loads(compiled.read_text())
    if manifest.get('sealed') is not False or manifest.get('manifest_kind') != 'compiled-provisional':
        raise PreparationError('A preserved compiled provisional manifest is required')
    if manifest.get('real_timing_executed') is not False:
        raise PreparationError('Preparation record must not contain real timing')
    verify_manifest(manifest)
    interpreted = inventory(HERE.glob('*.py'))
    if interpreted != manifest['interpreted_sources']:
        raise PreparationError('Interpreted code inventory changed after compilation')
    result = json.loads(validation.read_text())
    if result.get('status') != 'passed' or result.get('real_timing_executed') is not False:
        raise PreparationError('Complete untimed worker validation must pass')
    if result.get('full_size_cases') != 60:
        raise PreparationError('Exactly 60 full-size build/cell/seed preparations required')
    test_logs = [Path(p).resolve(strict=True) for p in (test_logs or [HERE / 'collector-tests.log', HERE / 'provenance-tests.log'])]
    for path in test_logs:
        text = path.read_text()
        if not text.rstrip().endswith('OK') or '\nFAILED' in text or '\nERROR' in text:
            raise PreparationError('Synthetic/unit test log does not end in OK: ' + str(path))
    fixtures = fixture_inventory(manifest)
    if len(fixtures) != 120:
        raise PreparationError('Exactly 120 canonical fixture files required')
    declared = result.get('fixtures')
    if declared is not None and declared != fixtures:
        raise PreparationError('Validator fixture inventory disagrees with seal inventory')
    manifest['fixtures'] = fixtures
    manifest['preparation_evidence'].update(inventory([compiled, validation, *test_logs]))
    for name in ('worker-validation.log', 'build.log', 'build-resume.log'):
        path = HERE / name
        if path.exists():
            manifest['preparation_evidence'].update(inventory([path]))
    manifest['sealed'] = True
    manifest['manifest_kind'] = 'sealed-full-fixtures'
    manifest['pre_seal_manifest_sha256'] = before
    manifest['status'] = 'Compiled and fully validated with untimed/synthetic checks only; real campaign not executed.'
    verify_manifest(manifest)
    verify_map(fixtures)
    if sha(compiled) != before:
        raise PreparationError('Compiled manifest changed during sealing')
    if artifact_bytes(HERE) > MAX_ARTIFACT_BYTES:
        raise PreparationError('Complete artifact storage exceeds 256 MiB')
    write_json_new(output, manifest)
    verify_manifest(manifest)
    print('SEALED FULL FIXTURES: ' + sha(output), flush=True)
    return manifest


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiled', type=Path)
    parser.add_argument('--validation', type=Path)
    parser.add_argument('--test-log', action='append', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args(argv)
    try:
        seal(args.compiled, args.validation, args.test_log, args.output)
    except (PreparationError, OSError, ValueError, KeyError) as exc:
        print('SEAL BLOCKED: ' + str(exc), file=sys.stderr)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main())
