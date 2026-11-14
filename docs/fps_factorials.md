---
title: 階乗・逆階乗生成
documentation_of: //components/fps_factorials.hpp
library_category: components/fps
---

旧 `fps_detail` 名を利用するコード向けの互換ヘッダーです。FPS 本体を含むライブラリ内部では `mint_detail` の共通部品を直接使います。実装は [modint998244353 の階乗・逆階乗生成](https://qandori.github.io/cp-library/components/mint_factorials.hpp.html) と共有し、NTT には依存しません。通常は [階乗・逆階乗と二項係数](https://qandori.github.io/cp-library/math/comb.hpp.html) や [Taylor Shift](https://qandori.github.io/cp-library/fps/taylor_shift.hpp.html) を利用します。`factorials` には `mint*` または法 998244353 の内部 Montgomery 表現の整数値へのポインタを渡せます。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::factorials(fact,invfact,n)` | `fact[i] = i!`、`invfact[i] = 1/i!` を先頭 `n` 要素へ書き込む | $$O(n)$$ |
| `fps_detail::factorials8(fact,invfact,n)` | AVX2（複数の整数をまとめて処理する CPU 命令）版。`n >= 256` | $$O(n)$$ |
| `fps_detail::factorial_chain<reverse>(out,chunk,value,factor)` | 8 本の積の列を同時に生成。各列の `value` を書き、`factor` を掛けて進める | $$O(\mathrm{chunk})$$ |
| `fps_detail::factorial_transpose(x)` | 8×8 個の 32 bit 整数の行と列を入れ替える | $$O(1)$$ |

`factorials` の引数は内部の 32 bit 整数値版では $$0\le n<998244353$$、`mint*` 版では $$0\le n\le998244353$$ とし、出力先はそれぞれ `n` 要素を持つ重ならない領域にします。`n = 0` では書き込みません。Mac ARM でも `factorials` を利用できます。

末尾に `8` が付く関数と `factorial_chain`・`factorial_transpose` は x86 のみで定義され、AVX2 に対応する CPU から呼び出します。`factorial_chain` は `chunk` を正の 8 の倍数とし、`out` に `8 * chunk` 要素を用意します。第 `s` 列は第 `s` 区間に置かれ、`reverse = true` のときは逆順で書き、乗数を 1 ずつ減らします。偽のときは順に書き、乗数を 1 ずつ増やします。

`factorials`・`factorials8` は時間 $$O(n)$$、`factorial_chain` は時間 $$O(\mathrm{chunk})$$、`factorial_transpose` は時間 $$O(1)$$ です。出力領域を除く追加空間はいずれも $$O(1)$$ です。
