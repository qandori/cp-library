import hashlib
import json
import re
import sys
import urllib.request
from pathlib import Path

out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = json.load(urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions/404900', timeout=60))['source']
(out / 'reference-source.cpp').write_text(source)
(out / 'reference-source.sha256').write_text(hashlib.sha256(source.encode()).hexdigest() + '\n')
source = source[source.index('struct flower {'):source.index('\nvoid Yorisou()')]
source = re.sub(r'\bvc\b', 'std::vector', source).replace('.ep(', '.emplace_back(')
source = re.sub(r'all\((\w+)\)', r'\1.begin(), \1.end()', source)
source = source.replace('FOR(i, N)', 'for (int i = 0; i < N; ++i)')
source = '#include <algorithm>\n#include <numeric>\n#include <utility>\n#include <vector>\nnamespace matching_reference_code {\nusing namespace std;\n' + source + '\n}\n'
source += '''
class Matching_reference {
	matching_reference_code::flower core;
public:
	std::vector<int> mate;
	explicit Matching_reference(int n) : core(n), mate(n) {}
	void reserve(std::size_t) {}
	void add_edge(int u, int v) { if (u != v) core.add(u, v); }
	int build() {
		int result = core.slv();
		for (int i = 0; i < core.N; ++i) mate[i] = core.ma[i] == core.N ? -1 : core.ma[i];
		return result;
	}
};
'''
(out / 'reference.hpp').write_text(source)
