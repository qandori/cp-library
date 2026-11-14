---
title: FPS の対数の内部変換
documentation_of: //components/fps_log.hpp
library_category: components/fps
---

`fps_detail::LogTransform` は `fps/fps.hpp` の対数と冪が使う x86 / AVX2（複数の整数をまとめて処理する CPU 命令）用の内部部品です。通常は `fps::log()` から利用します。AVX2 が使えない環境では FPS 側が汎用の級数除算へ戻ります。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

```cpp
#include "lib/components/fps_log.hpp"

#if defined(__x86_64__) || defined(__i386__)
if (fps_detail::NTT::vectorized()) {
	std::vector<fps_detail::NTT::U> data(64, 0);
	auto &plan = fps_detail::ntt_plan(64 / 16);
	fps_detail::LogTransform transform{plan};
	transform.forward(data.data(), 64);
}
#endif
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `LogTransform{plan}` | 変換に使う定数を保持した `plan` を参照して作成 | $$O(1)$$ |
| `forward(a,n,norm = 1,half = false)` | `a` を部分 NTT し、`norm` 倍を組み込む | $$O(n\log n)$$ |
| `inverse(a,n)` | `a` を逆変換。最後の倍率 $$8/n$$ は掛けない | $$O(n\log n)$$ |
| `inverse_half<upper>(a,n)` | 指定した半分だけを逆変換 | $$O(n\log n)$$ |
| `products(dst,a,b,terms,n,side = nullptr)` | `terms` 組の変換済み配列の環積の和を `dst` に格納 | $$O(n)$$ |
| `toeplitz(dst,side,w1,w2,q0,q1,n)` | 二つの環積の和を `dst`・`side` に格納 | $$O(n)$$ |
| `integrate(dst,src,prefix,first,n)` | `src[i] / (first+i)` を `dst[i]` に格納 | $$O(n)$$ |

この型は x86 でのみ定義されます。各関数を呼ぶ前に AVX2 対応を確認します。参照先の NTT オブジェクト `plan` は、この型の使用終了まで破棄しないでください。

`forward(a,n,norm,half)` は 8 係数ずつの組を残して途中まで変換し、通常の整数 `norm` 倍を組み込みます。`norm` は $$[0,p)$$、入力は $$[0,2p)$$ とします。結果は $$[0,p)$$ の内部 Montgomery 表現の整数値です。`half=true` は後半を 0 として扱い、読み取る入力は前半のみです。`inverse(a,n)` は両半分を逆変換しますが、元の値に戻すための倍率 $$8/n$$ はまだ掛けません。 `inverse_half<upper>(a,n)` は、`upper` が真なら後半、偽なら前半だけを復元します。逆変換した値には、必要に応じて $$8/n$$ を掛けてください。入力は $$[0,2p)$$、復元した半分も $$[0,2p)$$ です。もう半分の値は未規定です。

変換長 `n` は 64 以上 $$2^{23}$$ 以下の 2 冪とし、配列には `n` 要素、`plan` には少なくとも `n/16` 個の変換用の定数を用意します。

`products(dst,a,b,terms,n,side)` は同じ途中段階まで変換した 1 または 2 組の配列を掛けて加算します。第 $$k$$ 組の 8 係数を、次数 0 から 7 の多項式の係数とみなします。その積を $$z^8-w_k$$ で割った余りを、このページでは環積と呼びます。ここで $$w_k=(-1)^k r_{\lfloor k/2\rfloor}$$、$$r_i=\texttt{plan.roots[i]}$$ です。`side` が非 null なら、その 8 係数ずつの多項式も加えます。`a`・`b` は `terms` 個の配列ポインタです。各入力は `n` 要素で、`a`・`b` は $$[0,p)$$、`side` は $$[0,2p)$$ とします。結果は $$[0,2p)$$ です。`dst` は入力の 1 配列と同一でも構いません。それ以外の部分的な重なりは認めません。出力以外の入力は変更しません。

`toeplitz(dst,side,w1,w2,q0,q1,n)` は、環積 $$w_2q_0+w_1q_1$$ を `dst` に、$$w_3q_0+w_2q_1$$ を `side` に計算します。`side` は最初に $$w_3$$ を保持します。全入力は $$[0,p)$$、出力は $$[0,2p)$$ です。`dst` は `q1` と同一でも構いません。他の配列同士は重ねません。両積を 3 回の環積で求めます。

`products`・`toeplitz` の `n` は 64 の倍数とし、`plan` に変換用の定数を `n/16` 個以上用意します。

`integrate(dst,src,prefix,first,n)` は `dst[i] = src[i] / (first+i)` を計算します。`src` は $$[0,2p)$$ の内部 Montgomery 表現の整数値で、出力も同範囲です。`first >= 1`、`n >= 0`、`first+n <= p` とします。`src`・`dst` はそれぞれちょうど `n` 要素で構いません。`prefix` は `n` を 32 の倍数へ切り上げた要素数の作業領域とし、3 配列は重ねません。ここで $$p=998244353$$ です。

**計算量**: `forward`・`inverse`・`inverse_half` は $$O(n\log n)$$、`products`・`toeplitz`・`integrate` は $$O(n)$$ 時間です。`integrate` の作業領域は $$O(n)$$、その他の追加空間は $$O(1)$$ です。変換用の定数を保持する NTT オブジェクトは $$O(n)$$ 空間です。
