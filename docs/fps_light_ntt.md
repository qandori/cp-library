---
title: "軽量 FPS の NTT"
documentation_of: //components/fps_light_ntt.hpp
library_category: components
---

通常は [軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) を利用します。法 998244353 の共有 `mint` と小さい NTT を使う内部部品です。軽量 FPS と軽量畳み込みが共有し、このヘッダー単体では FPS 本体を読み込みません。通常版 FPS の変換処理は読み込みません。

| 関数 | 内容・条件 | 計算量 |
| --- | --- | --- |
| `fps_light_detail::multiply(a,b)` | `std::vector<mint>` 同士の畳み込みを同じ型で返す。入力が空なら空配列 | $$O(n\log n)$$ |
| `fps_light_detail::transform(a,inverse = false)` | 順変換／逆変換を上書き。返り値は `void`。長さは正の 2 冪で $$2^{23}$$ 以下 | $$O(n\log n)$$ |
| `fps_light_detail::pointwise(a,b,vectorized=simd_available())` | `std::vector<mint>` の要素積を `a` に格納。`a.size() <= b.size()` とし、`b` の先頭 `a.size()` 項を使う。返り値は `void` | $$O(n)$$ |
| `fps_light_detail::scale(a,c,vectorized=simd_available())` | 全要素を `mint c` 倍。返り値は `void` | $$O(n)$$ |

`pointwise` と `scale` は空配列にも使えます。`pointwise` の `b` は変更しませんが、`a` と同じ配列を渡して平方を上書きすることもできます。`vectorized` は通常省略します。`false` は 1 要素ずつの計算を選び、`true` は AVX2（複数の整数をまとめて処理する CPU 命令）対応 CPU でのみ指定できます。表の $$n$$ は `multiply` では結果長、それ以外では `a.size()` です。

NTT は素数を法として係数列を変換し、多項式の積を計算しやすくする方法です。順変換後の値は、添字を `log2(n)` 桁の 2 進数で書いて左右反転した順序で格納します。変換後の同じ位置同士を掛けて逆変換する用途に使え、逆変換には元の係数列へ戻すための長さによる除算も含みます。

積の結果の長さは両入力が非空なら `a.size()+b.size()-1` です。各入力と結果の長さを $$2^{23}$$ 以下にしてください。

積の結果長または変換長を $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。根の表は、これまでに必要になった最大変換長を $$L$$ として $$O(L)$$ 空間を保持します。
