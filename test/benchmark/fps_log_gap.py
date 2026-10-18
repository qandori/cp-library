import re, argparse, subprocess, json, hashlib, random, statistics, time, os, fcntl
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description="Compare FPS log with submission 409452")
parser.add_argument("--reference-source", type=Path, required=True)
parser.add_argument("--official-root", type=Path, required=True)
parser.add_argument("--work-dir", type=Path, required=True)
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--suite", choices=["main", "api", "boundary"], default="main")
parser.add_argument("--cpu", type=int, default=2)
parser.add_argument("--trials", type=int, default=21)
parser.add_argument("--seed", type=int, default=20261018)
ARGS = parser.parse_args()
WORK = ARGS.work_dir.resolve()
WORK.mkdir(parents=True, exist_ok=True)
REF = ARGS.reference_source.resolve()

FLAGS = [
    "-std=c++23",
    "-O2",
    "-DEVAL",
    "-DONLINE_JUDGE",
    "-ftrivial-auto-var-init=zero",
    "-march=haswell",
]
HASH = "86ef3c3b4a4707ef5e7515e5229b2564f2a5f46bd830bb6cbe64b506a95675b6"


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


s = REF.read_text()
assert sha(REF) == HASH
prefix = s[: s.rindex("namespace {\n\nvoid solve()")]
origmain = s[s.rindex("namespace {\n\nvoid solve()") :]
normal = origmain[: origmain.index("#ifdef __ELF__")] + "}\nint main(){solve();}\n"
include = '#include "test/fps_io.hpp"\n'
adapter = """
fps reference_log(const fps &a) {
 const std::size_t n=a.size();const int lg=poly::log_log(n);
 poly::Arena arena(poly::Transform::words(lg)+poly::Arena::footprint(n)+poly::Arena::footprint(poly::log_scratch(n)));
 const poly::Transform t(arena,lg);auto g=arena.take(n),scratch=arena.take(poly::log_scratch(n));
 for(std::size_t i=0;i<n;++i)g[i]=a[i].val();
 poly::log(t,g,g,scratch);
 fps r(n);for(std::size_t i=0;i<n;++i)r[i]=fps_mint::raw(g[i]);return r;
}
"""
common = """
int main(){FastInput input;FastOutput output;int n=0;input.read(n);fps a=read_fps(input,n);fps r=CALL;write_fps(output,r);}
"""
plain = """
int main(){FastInput input;FastOutput output;unsigned n=0;input.read(n);
 const int lg=poly::log_log(n);poly::Arena arena(poly::Transform::words(lg)+poly::Arena::footprint(n)+poly::Arena::footprint(poly::log_scratch(n)));
 const poly::Transform t(arena,lg);auto g=arena.take(n),scratch=arena.take(poly::log_scratch(n));
 for(auto &x:g) input.read(x);poly::log(t,g,g,scratch);for(auto x:g){output.write(x);output.write(' ');}output.write('\\n');}
"""
fastcurrent = """
int main(){io::Reader input;const unsigned n=input.read<std::uint32_t>();
 poly::Arena arena(poly::Arena::footprint(n)+poly::Arena::footprint(fields::kTextBytes/4+1024));
 auto g=arena.take(n),scratch=arena.take(fields::kTextBytes/4+1024);io::read_bulk(input,g.data(),n);
 fps a(n);for(unsigned i=0;i<n;++i)a[i]=fps_mint::raw(g[i]);fps r=a.log(n);
 for(unsigned i=0;i<n;++i)g[i]=r[i].val();io::Writer out;
 const auto text=(reinterpret_cast<std::uintptr_t>(scratch.data())+4095)&~std::uintptr_t(4095);
 fields::write(out,g.data(),n,reinterpret_cast<char*>(text));}
"""
adapted = re.sub(r"\bntt\b", "gap_ntt", prefix)
variants = {
    "current": include + common.replace("CALL", "a.log(n)"),
    "reference_exact": s,
    "reference_normal": prefix + normal,
    "reference_common_io": adapted + include + plain,
    "reference_api": adapted
    + include
    + adapter
    + common.replace("CALL", "reference_log(a)"),
    "current_fast_io": adapted + include + fastcurrent,
}
# Reference ablations keep its I/O and regular main.
base = prefix + normal
q = base
for left, right in [
    ("inline void inverse_step(", "\n// g = 1 / f mod"),
    (
        "[[gnu::always_inline]] inline void log_derivative(",
        "\ninline int log_derivative_log",
    ),
]:
    a = q.index(left)
    b = q.index(right, a)
    part = (
        q[a:b]
        .replace("Half::kUpper", "Half::kBoth")
        .replace("Half::kLower", "Half::kBoth")
    )
    q = q[:a] + part + q[b:]
variants["reference_both_halves"] = q
variants["reference_levelwise"] = base.replace(
    "static constexpr std::size_t kTile = 256;",
    "static constexpr std::size_t kTile = 1 << 20;",
)
q = base
old = "run(out, source(in, shift, out.size()), detail::ProductBottom{roots_, inverse_roots_, b.data()}, scale, output);"
new = """forward(in,shift,out);
        const detail::ProductBottom product{roots_,inverse_roots_,b.data()};
        run(out,detail::Source(nullptr,0,0),detail::InverseProductBottom{product,out.data()},scale,output);"""
assert q.count(old) == 1
q = q.replace(old, new)
variants["reference_split_cyclic"] = q
q = base
q = q.replace("prepare(a, first, window[0]);", "")
q = q.replace(
    "if (j + 1 < count) prepare(a + 32, first + j + 1, window[(j + 1) & 1]);",
    "prepare(a,first+j,window[j & 1]);",
)
q = q.replace("prepare(first, window[0]);", "")
q = q.replace(
    "if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);",
    "prepare(first+j,window[j & 1]);",
)
q = q.replace("term.prepare(first, window[0]);", "")
# term.prepare is replaced separately because ordinary replacement is a suffix.
q = q.replace("term.\n", "\n").replace("product.\n", "\n")
q = q.replace(
    "if (j + 1 < count) product.prepare(a + 32, first + j + 1, window[(j + 1) & 1]);",
    "product.prepare(a,first+j,window[j & 1]);",
)
q = q.replace(
    "if (j + 1 < count) term.prepare(first + j + 1, window[(j + 1) & 1]);",
    "term.prepare(first+j,window[j & 1]);",
)
variants["reference_no_lookahead"] = q
q = base
a = q.index(
    "    const Vec ni = broadcast(ntt::kernels::kNI)",
    q.index("inline Vec leaf_product"),
)
b = q.index("\n    return even;", a)
body = """    const Vec ni=broadcast(ntt::kernels::kNI),p=broadcast(kP),p2=broadcast(2*kP);
    Vec even=_mm256_setzero_si256(),odd=even;
#pragma GCC unroll 8
    for(int i=0;i<8;++i){const Vec v=broadcast(b[i]);
        even=_mm256_add_epi64(even,_mm256_mul_epu32(load(window.word+8-i),v));
        odd=_mm256_add_epi64(odd,_mm256_mul_epu32(load(window.word+9-i),v));}
    even=_mm256_add_epi64(even,_mm256_mul_epu32(_mm256_mul_epu32(even,ni),p));
    odd=_mm256_add_epi64(odd,_mm256_mul_epu32(_mm256_mul_epu32(odd,ni),p));
    even=_mm256_blend_epi32(_mm256_srli_epi64(even,32),odd,0xaa);
    even=_mm256_min_epu32(even,_mm256_sub_epi32(even,p2));"""
q = q[:a] + body + q[b:]
variants["reference_intrinsic_leaf"] = q

optimized_adapter = adapter.replace(
    "fps reference_log(", '[[gnu::optimize("O3")]] fps reference_log('
)
variants["reference_api_o3"] = (
    adapted + include + optimized_adapter + common.replace("CALL", "reference_log(a)")
)
variants["reference_api_fast_io"] = (
    adapted
    + include
    + optimized_adapter
    + fastcurrent.replace("fps r=a.log(n);", "fps r=reference_log(a);")
)
variants["reference_api_o3_io"] = (
    adapted
    + include
    + '\n#pragma GCC push_options\n#pragma GCC optimize("O3")\n'
    + optimized_adapter
    + common.replace("CALL", "reference_log(a)")
    + "\n#pragma GCC pop_options\n"
)
helpers = """
[[gnu::optimize("O3")]] fps import_values(std::span<const unsigned> a){fps r(a.size());for(unsigned i=0;i<a.size();++i)r[i]=fps_mint::raw(a[i]);return r;}
[[gnu::optimize("O3")]] void export_values(std::span<unsigned> a,const fps&r){for(unsigned i=0;i<a.size();++i)a[i]=r[i].val();}
"""
boundary_main = fastcurrent.replace(
    "fps a(n);for(unsigned i=0;i<n;++i)a[i]=fps_mint::raw(g[i]);fps r=a.log(n);",
    "fps a=import_values(g);fps r=reference_log(a);",
).replace("for(unsigned i=0;i<n;++i)g[i]=r[i].val();", "export_values(g,r);")
variants["reference_api_fast_boundaries"] = (
    adapted + include + optimized_adapter + helpers + boundary_main
)
suites = {
    "main": [
        "current",
        "reference_exact",
        "reference_normal",
        "reference_common_io",
        "reference_api",
        "current_fast_io",
        "reference_both_halves",
        "reference_levelwise",
        "reference_split_cyclic",
        "reference_no_lookahead",
        "reference_intrinsic_leaf",
    ],
    "api": [
        "current",
        "reference_exact",
        "reference_normal",
        "reference_api",
        "reference_api_o3",
        "reference_api_fast_io",
        "reference_api_o3_io",
    ],
    "boundary": [
        "current",
        "reference_exact",
        "reference_normal",
        "reference_api_fast_io",
        "reference_api_fast_boundaries",
    ],
}
variants = {name: variants[name] for name in suites[ARGS.suite]}


def build():
    for name, src in variants.items():
        path = WORK / (name + ".cpp")
        path.write_text(src)
        subprocess.run(
            [
                "g++",
                *FLAGS,
                "-I" + str(ROOT),
                str(path),
                "-o",
                str(WORK / (name + ".out")),
            ],
            check=True,
        )
        print("built", name, flush=True)


def run():
    os.sched_setaffinity(0, {ARGS.cpu})
    rng = random.Random(ARGS.seed)
    folder = ARGS.official_root / "polynomial/log_of_formal_power_series"
    inp = folder / "in/max_random_00.in"
    answer = folder / "out/max_random_00.out"
    result = dict(
        baseline=subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=ROOT, text=True
        ).strip(),
        flags=FLAGS,
        cpu=ARGS.cpu,
        trials=ARGS.trials,
        seed=ARGS.seed,
        input_sha256=sha(inp),
        reference_sha256=HASH,
        compiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[
            0
        ],
        sources={
            p.relative_to(ROOT).as_posix(): sha(p)
            for folder in ["fps", "components", "math", "convolution", "test"]
            for p in (ROOT / folder).glob("*.hpp")
        },
        variants={},
        whole={},
    )
    for name in variants:
        result["variants"][name] = dict(
            source_sha256=sha(WORK / (name + ".cpp")),
            binary_sha256=sha(WORK / (name + ".out")),
        )
    samples = {k: [] for k in variants}
    with open("/tmp/cp-fps-tuning.lock", "w") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for trial in range(ARGS.trials + 1):
            order = list(variants)
            rng.shuffle(order)
            for name in order:
                out = WORK / "actual.txt"
                with inp.open("rb") as stdin, out.open("wb") as stdout:
                    start = time.perf_counter()
                    subprocess.run(
                        [str(WORK / (name + ".out"))],
                        stdin=stdin,
                        stdout=stdout,
                        check=True,
                    )
                    elapsed = (time.perf_counter() - start) * 1000
                subprocess.run(
                    [str(folder / "checker"), str(inp), str(out), str(answer)],
                    check=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                if trial:
                    samples[name].append(elapsed)
            if trial % 5 == 0:
                print("trial", trial, flush=True)
    result["harness_sha256"] = sha(Path(__file__))
    result["hardware"] = subprocess.check_output(["lscpu"], text=True)
    result["official_commit"] = subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=ARGS.official_root, text=True
    ).strip()
    result["whole"] = {
        k: dict(
            samples_ms=v, median_ms=statistics.median(v), min_ms=min(v), max_ms=max(v)
        )
        for k, v in samples.items()
    }
    ARGS.output.write_text(json.dumps(result, indent=2) + "\n")
    print({k: v["median_ms"] for k, v in result["whole"].items()}, flush=True)


if __name__ == "__main__":
    build()
    run()
