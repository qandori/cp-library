---
title: 多項式の積・打ち切り積の共通処理
documentation_of: //components/polynomial_multiply.hpp
library_category: components/fps
---

`polynomial_detail::Products<F,Limit>` は、積の木と FPS 合成が共有する内部部品です。`F` は `F::multiply` を持つ多項式型、`Limit` はその畳み込みの最大係数数です。

`multiply(a,b)` は通常の積、`multiply_low(a,b,n)` は積の低次 `n` 係数を返します。後者の返り値は長さ `n` で、不足分は 0 です。積が `Limit` を超える場合は係数列を分割して計算します。

`Limit >= 2` とします。入力の係数は変更しません。どちらかの入力が空なら `multiply` は空列を返し、`multiply_low` は指定した長さの零列を返します。`n == 0` の打ち切り積は空列です。係数のコピー・加算を定数時間とします。

結果が高々 $$k$$ 係数となる `F::multiply` の時間と追加空間を、それぞれ $$M(k),W(k)$$ とします。入力長を $$p,q$$ とすると、分割しない `multiply` の時間・追加空間は $$O(M(p+q-1))$$・$$O(W(p+q-1))$$ です。

打ち切り積の出力長を $$n$$、ブロック幅を $$B=\lfloor\mathrm{Limit}/2\rfloor$$、調べる入力のブロック数の上界を $$\displaystyle A=\left\lceil\frac{\min(p,n)}B\right\rceil$$、$$\displaystyle C=\left\lceil\frac{\min(q,n)}B\right\rceil$$ とします。`multiply_low` は、時間 $$O(n+AC(M(2B)+B))$$、返り値を含む追加空間 $$O(n+B+W(2B))$$ です。実際には出力に寄与するブロック対だけを計算します。空列を返す場合は定数時間です。
