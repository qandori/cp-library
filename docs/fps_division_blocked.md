---
title: FPS と多項式除算の内部変換
documentation_of: //components/fps_division_blocked.hpp
library_category: components/fps
---

`fps_detail::blocked_poly` の除算用内部部品です。通常は `fps::div_series()`、`fps::divmod()`、`polynomial_inv_mod()` から使います。Linux x86_64 / GCC でのみ定義され、呼び出す前に `fps_detail::NTT::vectorized()` が真であることを確認します。他の環境では公開 FPS 関数が既存の実装を使います。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_div_series(a,n,b,m,out,degree)` | 級数除算の最初の degree 項を追加 | $$O(N\log N)$$ |
| `native_divide(a,n,b,m,q,r)` | 多項式の商と剰余を追加 | $$O(N\log N)$$ |
| `divide_words(n,m)` | 除算用の作業領域の 32 bit 整数の要素数 | $$O(\log N)$$ |
| `quotient_size(n,m)` | 商に必要な係数数 | $$O(1)$$ |
| `remainder_size(n,m)` | 剰余に必要な係数数 | $$O(1)$$ |
| `divide(arena,f,g,q,r)` | 通常表現での多項式除算 | $$O(N\log N)$$ |
| `division_detail::append_words(out,values,n,factor)` | factor 倍した n 係数を追加 | 予約済み: $$O(n)$$、再確保時: 既存要素数＋n に比例 |
| `division_detail::append_division_words(out,values,n,factor)` | 除算用に64係数ずつ追加 | 予約済み: $$O(n)$$、再確保時: 既存要素数＋n に比例 |

表では $$N=\max(n,m,\mathrm{degree})$$ とします。関数は `fps_detail::blocked_poly` 内にあります。入力・出力の表現と必要な長さは以下の通りです。

`native_div_series(a,n,b,m,out,degree)` は Montgomery 表現の `Mint` 配列から `a/b mod x^degree` を求め、空の `out` へ `degree` 係数を追加します。`degree > 0`、`m > 0`、`b[0] != 0` とします。入力が次数より短いときは残りを 0 として扱います。

`native_divide(a,n,b,m,q,r)` は多項式の商と剰余を `q`・`r` の末尾へ追加します。`n >= m > 0`、`b[m-1] != 0` とします。追加する商は `n-m+1` 係数、剰余は `m-1` 係数です。内部関数は末尾の 0 を削除せず、公開 `divmod()` 側が削除します。どちらの関数も入力の係数を書き換えず、呼び出し側の入力に末尾の余分な領域や先頭アドレスの配置条件は要求しません。

入力 vector 自体を追加先にしても、元の要素を保持して結果を追加します。`q`・`r` が同じ vector なら商、剰余の順に追加します。

内部の `divide_words(n,m)` と `divide(arena,f,g,q,r)` は通常の剰余値 $$[0,p)$$ を扱います。$$p=998244353$$ です。`f`・`g`・`q`・`r` は重ねず、`q` の長さを `quotient_size(n,m)`、`r` の長さを `remainder_size(n,m)` にします。`g` は非空で最高次係数が非零です。`n<m` の場合は商を空にして、剰余へ `f` をコピーします。作業用の整列と領域の間隔は `Arena` が管理します。

`division_detail::append_words(out,values,n,factor)` は、通常の剰余値の配列を `factor` 倍し、その整数値を `Mint` の内部表現として `out` に追加します。入力値と `factor` は $$[0,p)$$ とします。`append_division_words` も同じ結果を追加します。

公開 FPS から呼ぶ経路では係数数・要求次数を $$2^{23}$$ 以下に制限します。変換長と積の仕様は [専用変換](fps_blocked.md) に従います。

**計算量**: 入力係数数と要求次数の最大を $$n$$ として、級数除算・多項式除算は $$O(n\log n)$$ 時間、追加空間は $$O(n)$$ です。短い入力の直接除算は商と除数の係数数の積に比例します。
