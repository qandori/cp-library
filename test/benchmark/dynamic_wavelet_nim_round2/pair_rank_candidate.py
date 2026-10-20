def _replace(source, signature, following, replacement):
    begin = source.index(signature)
    end = source.index(following, begin + len(signature))
    return source[:begin] + replacement + source[end:]


def apply(source):
    source = _replace(
        source,
        "\tstd::pair<int, int> rank(int l, int r, int digit) const {\n",
        "\tCounts ranks(int p) const {\n",
        r"""	std::pair<int, int> rank(int l, int r, int digit) const {
		assert(0 <= l && l <= r && r <= n && 0 <= digit && digit < 4);
		if (!total[digit]) return {0, 0};
		if (total[digit] == n) return {l, r};
		if (!l) return {0, rank(r, digit)};
		if (r == n) return {rank(l, digit), total[digit]};
		auto prefix = [&](int id, int p) {
			int before = 0;
			while (!(id & 1)) {
				if (!p) return before;
				const auto& node = nodes[id >> 1];
				if (p == node.sizes[node.count]) return before + node.sums[digit][node.count];
				int j = child(node, p);
				p -= node.sizes[j], before += node.sums[digit][j];
				id = node.ids[j];
			}
			return before + rank_leaf(leaves[id >> 1], p, digit);
		};
		int id = root, before = 0;
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			if (!l) return {before, before + prefix(id, r)};
			if (r == node.sizes[node.count]) return {before + prefix(id, l), before + node.sums[digit][node.count]};
			int a = child(node, l);
			int b = r <= node.sizes[a + 1] ? a : child(node, r - 1);
			if (a != b) return {before + node.sums[digit][a] + prefix(node.ids[a], l - node.sizes[a]),
				before + node.sums[digit][b] + prefix(node.ids[b], r - node.sizes[b])};
			l -= node.sizes[a], r -= node.sizes[a], before += node.sums[digit][a];
			id = node.ids[a];
		}
		const auto& leaf = leaves[id >> 1];
		return {before + rank_leaf(leaf, l, digit), before + rank_leaf(leaf, r, digit)};
	}
""".lstrip("\n"),
    )
    return _replace(
        source,
        "\tstd::pair<Counts, Counts> ranks(int l, int r) const {\n",
        "\tstd::pair<int, int> access_rank(int p) const {\n",
        r"""	std::pair<Counts, Counts> ranks(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if (uniform >= 0) { Counts x{}, y{}; x[uniform] = l, y[uniform] = r; return {x, y}; }
		if (!l) return {Counts{}, ranks(r)};
		if (r == n) return {ranks(l), total};
		auto prefix = [&](int id, int p) {
			Counts before{};
			while (!(id & 1)) {
				if (!p) return before;
				const auto& node = nodes[id >> 1];
				if (p == node.sizes[node.count]) {
					for (int d = 0; d < 4; ++d) before[d] += node.sums[d][node.count];
					return before;
				}
				int j = child(node, p);
				p -= node.sizes[j];
				for (int d = 0; d < 4; ++d) before[d] += node.sums[d][j];
				id = node.ids[j];
			}
			auto result = ranks_leaf(leaves[id >> 1], p);
			for (int d = 0; d < 4; ++d) result[d] += before[d];
			return result;
		};
		int id = root;
		Counts before{};
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			if (!l || r == node.sizes[node.count]) {
				auto x = prefix(id, l), y = prefix(id, r);
				for (int d = 0; d < 4; ++d) x[d] += before[d], y[d] += before[d];
				return {x, y};
			}
			int a = child(node, l);
			int b = r <= node.sizes[a + 1] ? a : child(node, r - 1);
			if (a != b) {
				auto x = prefix(node.ids[a], l - node.sizes[a]);
				auto y = prefix(node.ids[b], r - node.sizes[b]);
				for (int d = 0; d < 4; ++d) x[d] += before[d] + node.sums[d][a], y[d] += before[d] + node.sums[d][b];
				return {x, y};
			}
			l -= node.sizes[a], r -= node.sizes[a];
			for (int d = 0; d < 4; ++d) before[d] += node.sums[d][a];
			id = node.ids[a];
		}
		auto x = ranks_leaf(leaves[id >> 1], l), y = ranks_leaf(leaves[id >> 1], r);
		for (int d = 0; d < 4; ++d) x[d] += before[d], y[d] += before[d];
		return {x, y};
	}
""".lstrip("\n"),
    )
