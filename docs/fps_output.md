---
title: FPS の結果配列への一括変換
documentation_of: //components/fps_output.hpp
library_category: components/fps
---

Linux x86_64 / GCC 用の内部部品です。呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `prepare_fps_output(out,total)` | 出力配列に total 個分の領域を予約 | 再確保なし: $$O(1)$$、再確保時: 既存要素数に比例＋確保時間 |
| `append_fps_words<montgomery>(out,in,n)` | 内部の 32 bit 整数値を係数へ変換して追加 | 予約済み: $$O(n)$$、再確保時: 既存要素数＋n に比例 |

`fps_detail::prepare_fps_output(out,total)` は結果の最終サイズ `total` を収容する容量を確保します。既に容量が十分なら何もしません。既存の要素や配列の長さは変更しません。

大きな結果を複数回に分けて追加する呼び出し側は、追加の前に最終サイズで一度呼びます。`append_fps_words` 自身も、容量を増やす必要がある場合にこの処理を使います。

`fps_detail::append_fps_words<montgomery>(out,in,n)` は、`in` の $$n$$ 個の `unsigned` 整数を `std::vector<Mint>` の末尾へ追加します。`montgomery=true` なら入力は内部 Montgomery 表現 $$[0,2p)$$、偽なら通常の剰余 $$[0,p)$$ とし、後者は Montgomery 表現へ変換します。ここで $$p=998244353$$ です。

入力は出力と重ねません。入力にはちょうど $$n$$ 個の要素があればよく、先頭アドレスの配置条件や末尾の余分な領域は要求しません。$$n=0$$ なら何もせず、入力が null でも構いません。結果の長さが `out.max_size()` を超える場合は `std::length_error` を送出します。

追加処理の時間は $$O(n)$$、変換バッファは $$O(1)$$ 空間です。容量の増加は、新しい総容量を $$N$$ として $$O(N)$$ 時間・追加空間を使います。既存の $$m$$ 要素を移す再確保には追加で $$O(m)$$ 時間が必要です。
