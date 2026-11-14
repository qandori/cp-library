---
title: FPS の一括出力
documentation_of: //components/fps_write.hpp
library_category: components/fps
---

法 998244353 の共有 `mint` の係数列を [Fast I/O](fast_io.md) でまとめて出力します。通常版・軽量版 FPS のどちらでも使え、FPS 本体を読み込まずにこの部品だけを include することもできます。

```cpp
#include "lib/fps/fps_light.hpp"
#include "lib/components/fps_write.hpp"

int main() {
	fps f{1, 2, 3};
	const auto g = f.log(5).exp();
	FastOutput out;
	fps_detail::write_coefficients(out, g.data(), g.size());
	return out.flush() ? 0 : 1;
}
```

出力を空白区切りで読むと、係数列は `1 2 3 0 0` です。整形のために入る空白の個数は実行環境で異なります。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::write_coefficients(output,in,n)` | `FastOutput& output` へ、`const mint* in` の先頭 `size_t n` 係数を順に出力する。返り値は `void` | $$O(n)$$ |

引数の順序は出力先・係数列の先頭・係数数です。既定引数はありません。`n>0` なら、`in` から始まる添字 `0` 以上 `n` 未満の `mint` が有効であることが必要です。`n=0` は `in=nullptr` でもよく、改行だけを出力します。

入力は変更しません。入力の寿命は呼び出しが終わるまで保ってください。入力の先頭アドレスの配置条件や末尾の余分な領域は要求せず、指定範囲外を読みません。同じ値や零を含む係数列も、その順序を保って出力します。係数型は共有する `mint` 専用です。`mint::word` で係数 $$c$$ を直接用意する場合は、$$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数を使い、$$[0,2p)$$ に収めてください。

各係数を `val()` の十進整数値として空白で区切り、末尾に改行を付けます。値の間に空白が複数入る場合があります。

通常の `output.write`・`output.println` と混在できます。出力はバッファに残る場合があるため、最後に `output.flush()` を呼ぶか、`output` を破棄します。バッファを空ける処理や直接書き込みで失敗すると `std::runtime_error` を送出します。最後の `flush()` は成功時に `true` を返します。作業領域は呼び出しごとに所有し、共有する可変バッファはありません。同じ `FastOutput` を複数スレッドから同時に使わないでください。

係数数を $$n$$ として時間は $$O(n)$$、追加の作業空間は $$O(1)$$ です。空入力にも定数時間で対応します。
