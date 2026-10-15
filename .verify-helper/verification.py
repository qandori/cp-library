from pathlib import Path


def get_marker():
    from onlinejudge_verify import marker
    from onlinejudge_verify.languages import list as languages

    languages.get(Path("source.cpp"))
    return marker.get_verification_marker(jobs=2)
