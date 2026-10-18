---
title: FPS の対数の内部変換
documentation_of: //components/fps_log.hpp
---

`fps_detail::LogTransform` は `fps/fps.hpp` の対数と冪が使う x86 / AVX2 用の内部部品です。通常は `fps::log()` から利用します。AVX2 が使えない環境では FPS 側が汎用の級数除算へ戻ります。

```cpp
if (fps_detail::NTT::vectorized()) {
    auto &plan = fps_detail::ntt_plan(64 / 16);
    fps_detail::LogTransform transform{plan};
    // 64 要素の内部 Montgomery ワードを部分変換する。
    transform.forward(data, 64);
}
```

この型は x86 でのみ定義されます。各関数を呼ぶ前に AVX2 対応を確認します。保持する NTT プランの寿命は、この型より長くします。

`forward(a,n,norm,half)` は次数 8 の環を葉とする部分 NTT を行い、通常の整数 `norm` 倍を組み込みます。`norm` は $[0,p)$、入力は $[0,2p)$ とします。結果は $[0,p)$ の内部 Montgomery ワードです。`half=true` は後半を 0 として扱い、読み取る入力は前半のみです。`inverse(a,n)` は未正規化の逆変換で、両半分を復元します。比較用の `inverse_half<upper>(a,n)` は、`upper` が真なら後半、偽なら前半だけを復元します。正規化係数は $8/n$ です。入力は $[0,2p)$、復元した半分も $[0,2p)$ です。もう半分の値は未規定です。

変換長 `n` は 64 以上 $2^{23}$ 以下の 2 冪とし、配列には `n` 要素、プランには少なくとも `n/16` 個の根を用意します。

`products(dst,a,b,terms,n,side)` は同じ部分変換の 1 または 2 組の環積を加算します。`side` が非 null なら、既存の環積も加えます。`a`・`b` は `terms` 個の配列ポインタです。各入力は `n` 要素で、`a`・`b` は $[0,p)$、`side` は $[0,2p)$ とします。結果は $[0,2p)$ です。`dst` は入力の 1 配列と同一でも構いません。それ以外の部分的な重なりは認めません。出力以外の入力は変更しません。

`toeplitz(dst,side,w1,w2,q0,q1,n)` は、環積 $w_2q_0+w_1q_1$ を `dst` に、$w_3q_0+w_2q_1$ を `side` に計算します。`side` は最初に $w_3$ を保持します。全入力は $[0,p)$、出力は $[0,2p)$ です。`dst` は `q1` と同一でも構いません。他の配列同士は重ねません。両積を 3 回の環積で求めます。

`products`・`toeplitz` の `n` は 64 の倍数とし、根を `n/16` 個以上用意します。

`integrate(dst,src,prefix,first,n)` は `dst[i] = src[i] / (first+i)` を計算します。`src` は $[0,2p)$ の内部 Montgomery ワードで、出力も同範囲です。`first >= 1`、`n >= 0`、`first+n <= p` とします。`src`・`dst` はそれぞれちょうど `n` 要素で構いません。末尾はマスク付きの読み書きを使います。`prefix` は `n` を 32 の倍数へ切り上げた要素数の作業領域とし、3 配列は重ねません。ここで $p=998244353$ です。

**計算量**: `forward`・`inverse`・`inverse_half` は $O(n\log n)$、`products`・`toeplitz`・`integrate` は $O(n)$ 時間です。`integrate` の作業領域は $O(n)$、その他の追加空間は $O(1)$ です。NTT プランは $O(n)$ 空間です。
