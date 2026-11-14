---
title: 多項式 Half-GCD の専用変換
documentation_of: //components/polynomial_gcd_blocked.hpp
library_category: components/fps
---

`polynomial_gcd.hpp` が使う Linux x86_64 / GCC 用の内部部品です。通常は公開関数 `polynomial_gcd()`、`polynomial_extended_gcd()`、`polynomial_inv_mod()` を使います。専用経路を呼ぶ前に `fps_detail::NTT::vectorized()` を確認します。他の環境では既存の Half-GCD へ戻ります。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `HalfGcd::words(n)` | 必要な作業領域の 32 bit 整数の要素数 | $$O(1)$$ |
| `HalfGcd(arena,n)` | 変換表と作業領域を準備 | $$O(n)$$ |
| `matrix(k,room)` | 変換行列用の領域を取得 | $$O(1)$$ |
| `jump(a,n,b,m,k,out,need)` | 互除法をまとめた変換行列を計算 | $$O(n\log^2 n)$$ |
| `native_inv_mod(a,n,b,m,out)` | 法多項式での逆元。存在すれば true | $$O(n\log^2 n)$$ |
| `native_gcd_matrix(a,n,b,m,need,sink)` | 変換行列が有効な間に sink を呼ぶ | $$O(n\log^2 n)$$ |

`fps_detail::blocked_poly::gcd_detail::HalfGcd` は次数ごとの jump を求めます。`HalfGcd::words(n)` 個の 32 bit 整数を持つ `Arena` を用意し、`HalfGcd(arena,n)` で変換表と作業領域を確保します。作業領域が不足したときは `std::bad_alloc` を送出します。

`matrix(k,room)` は次数の低下量が `k` 以下の変換行列用領域を返します。`room` は保存する変換の最大長で、保存しないときは 0 にします。返した行列の係数配列は `HalfGcd` の作業領域を参照するため、その寿命内だけ使います。

`jump(a,n,b,m,k,out,need)` は `deg(a)=n > deg(b)=m`、`0 <= k <= n` の入力で、互除法を `deg(b) < n-k` になるまでまとめます。零多項式の `b` は `m=-1` で表します。入力係数は $$[0,998244353)$$、最高次係数は非零とします。`out` は `matrix(k,room)` で確保します。`need` の bit `2*i+j` は行列の $$(i,j)$$ 成分を求めるかを指定し、既定値は 15 です。指定しない成分の係数と長さは使用しません。

`Matrix::progress` は最初の多項式の次数低下量です。`entry[i]`・`size[i]` は係数配列とその長さ、`at0[i]` は定数項です。通常の互除法で得る行列に対し、全成分に同じ 0 でない値を掛けた行列を返すことがあります。公開関数は、最高次係数が 1 の GCD と、`a*u+b*v=gcd` を満たす係数多項式 `u,v` にそろえて返します。行列式が常に $$\pm1$$ であるとは仮定しません。

`native_inv_mod(a,n,b,m,out)` は `Mint` 配列を受け取り、逆元が存在すれば空の `out` に追加して真を返します。`b` は非空で最高次係数が非零です。定数の法では空の結果を返します。`native_gcd_matrix(a,n,b,m,need,sink)` は `n>m>0` の入力の jump を求め、行列が有効な間に `sink(matrix)` を呼びます。呼び出し側の入力は変更せず、追加の先頭アドレスの配置条件や末尾の余分な領域を要求しません。

**計算量**: 最大係数数を $$n$$ として、Half-GCD・GCD・拡張 GCD・法多項式に対する逆元は $$O(n\log^2 n)$$ 時間、追加空間は $$O(n)$$ です。
