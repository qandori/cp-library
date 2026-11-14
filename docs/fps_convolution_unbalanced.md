---
title: FPS の非対称畳み込み
documentation_of: //components/fps_convolution_unbalanced.hpp
library_category: components/fps
---

Linux x86_64 / GCC の AVX2（複数の整数をまとめて処理する CPU 命令）用内部部品です。公開の `fps::multiply` と `convolution` から利用します。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `fps_detail::multiply_detail::unbalanced::fits(n,m)` | 短辺が 32 より大きく、長辺が 32768 以上かつ短辺の 32 倍以上か判定 | $$O(1)$$ |
| `fps_detail::multiply_detail::unbalanced::multiply(a,b)` | 読み取り専用の二つの係数列の積を返す | $$O((n+m)\log m)$$ |

ここで $$n$$ は長辺、$$m$$ は短辺です。`multiply` の呼び出し側は `fits(a.size(),b.size())`、AVX2 対応、両 span が生存中の `Mint` 要素を指すこと、非空入力、入力の内部 Montgomery 値が $$[0,2p)$$、出力長が $$2^{23}$$ 以下であることを保証します。公開 dispatch は `fits` を満たす場合だけこの経路を使います。

入力を変更せず、公開入力に先頭アドレスの配置条件や末尾の余分な領域を要求しません。作業配列は `Arena` が所有し、出力は独立した `vector<Mint>` の生存中の要素として構築します。同じ入力を二回渡しても安全です。追加の作業空間は $$O(m)$$、返す配列は $$O(n+m)$$ です。
