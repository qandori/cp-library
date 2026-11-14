---
title: FPS の定数係数畳み込み
documentation_of: //components/fps_convolution_constant.hpp
library_category: components/fps
---

Linux x86_64 / GCC の AVX2（複数の整数をまとめて処理する CPU 命令）用内部部品です。公開の `fps::multiply` と `convolution` が、両入力の全係数がそれぞれ一定の場合に利用します。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `fps_detail::multiply_detail::constant::fits(a,b)` | 両入力が 4096 項以上で、各列の全係数がそれぞれ同じ剰余か厳密に判定 | 最悪 $$O(n+m)$$ |
| `fps_detail::multiply_detail::constant::multiply(a,b)` | 確認済みの二つの定数係数列の積を返す | $$O(n+m)$$ |

`fits` は空入力や 4096 項未満の入力には `false` を返します。各列の全係数について、法で同じ値を表しているかを確認します。

`multiply` の呼び出し側は、AVX2 対応、両 span が生存中の `Mint` 要素を指すこと、各列がそれぞれ定数であること、非空入力、内部 Montgomery 値が $$[0,2p)$$、出力長 $$n+m-1$$ が $$2^{23}$$ 以下であることを保証します。公開 dispatch は `fits` が true のときだけこの関数を呼びます。

各列の係数を $$a_0,b_0$$ とすると、出力の第 $$i$$ 係数は $$a_0b_0\min(i+1,n,m,n+m-1-i)$$ です。零の場合も公開サイズを保ち、返す内部整数値は全て $$[0,p)$$ にそろえます。

入力を変更せず、先頭アドレスの配置条件や末尾の余分な領域を要求しません。同一入力や長短の交換にも対応します。返り値は入力とは独立した `vector<Mint>` です。返す配列を除く追加作業領域は $$O(1)$$、返す配列は $$O(n+m)$$ です。
