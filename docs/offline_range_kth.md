---
title: 区間順位の一括処理（補助部品）
documentation_of: //components/offline_range_kth.hpp
library_category: components/data_structure
---

[区間の k 番目の値](offline_range_kth_smallest.md)で使う、圧縮済み整数列の順位問い合わせの共通処理です。

`offline_range_kth_detail::Frontier(q)` に問い合わせ数を渡し、`left`・`right`・`rank` に半開区間の両端と 0 始まりの順位を設定します。`run(frontier,codes,bits)` は、各答えの圧縮済みの値を `frontier.answer` に書き込みます。問い合わせの作業領域は更新されます。

`codes` は `vector<uint32_t>`、各値は `bits` bit 以下とし、`1 <= bits <= 31` を満たすものとします。列長・問い合わせ数は `INT_MAX` 以下、各問い合わせは非空の有効な区間とその区間内の順位を指定します。

対応する CPU では AVX2 命令による高速化を使い、それ以外では通常の整数演算を使います。呼び出し側による CPU の指定は不要です。

列長を $$N$$、問い合わせ数を $$Q$$ として、時間は $$O((N+Q)\,\mathrm{bits})$$、追加空間は $$O(N+Q)$$ です。
