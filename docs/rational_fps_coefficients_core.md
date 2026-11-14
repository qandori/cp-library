---
title: 有理 FPS の区間係数取得の共通処理
documentation_of: //components/rational_fps_coefficients.hpp
library_category: components/fps
---

通常版と軽量版で共有する、有理式 $$p(x)/q(x)$$ を形式的冪級数として展開した係数を求める処理です。通常は [有理 FPS の区間係数取得](rational_fps_coefficients.md) を使います。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `rational_fps_detail::Coefficients<F,Transform>::get(p,q,l,r)` | $$p/q$$ の半開区間 `[l,r)` の係数を取得 | $$O(N\log N\log r+(N+M)\log(N+M))$$ |
| `Coefficients::middle(f,g)` | 積の次数 `f.size()-1` から `g.size()-1` までを取得。`0<f.size()<=g.size()` | $$O(S\log S)$$ |

`F` は係数を次数の小さい順に持つ形式的冪級数の型です。`Transform` は順変換・逆変換・成分ごとの積を提供します。逆変換には変換長で割る処理を含め、順変換後に逆変換すると元の配列へ戻るものにします。変換結果は、2 進数で表した添字のビットの並びを逆順にした順番（bit reverse 順）に並べます。例えば長さ 8 なら、通常の添字の `0,4,2,6,1,5,3,7` の順です。

`get` の `l,r` は `long long` で、$$0\le l\le r$$ とします。`q` は空でなく、`q[0] != 0` でその逆元が存在することが必要です。分子 `p` は空でも使え、その場合は長さ `r-l` の零列を返します。`l == r` なら空列です。引数は値渡しで、元の入力は変更しません。

$$N=\max(\lvert p\rvert,\lvert q\rvert)$$、$$M=r-l$$、$$S=\lvert g\rvert$$ として、区間取得の追加空間は $$O(N\log r+M)$$、中央部分の積の追加空間は $$O(S)$$ です。

区間取得には $$N+M\le2^{22}$$ が必要です。空区間は、引数の受け渡しを除き $$O(1)$$ 時間・追加空間です。$$r=1$$ では対数の項を定数として扱います。係数の四則演算を定数時間とし、長さ $$k$$ の変換を $$O(k\log k)$$ 時間・$$O(k)$$ 空間とした計算量です。
