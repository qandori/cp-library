---
title: "XOR 基底"
documentation_of: //math/xor_basis.hpp
---

```cpp
#include "lib/math/xor_basis.hpp"

XorBasis<> basis{3, 5};
auto maximum = basis.max_xor();
auto third = basis.kth(2);
auto coefficients = basis.coordinates(6);
auto perpendicular = basis.orthogonal_complement();

XorBasisWithWitness<> witness;
witness.insert(3);
witness.insert(5);
auto indices = witness.represent(6);
```

`XorBasis<UInt = uint64_t, Bits = numeric_limits<UInt>::digits>` は XOR で張られる空間を扱います。`UInt` は 64 bit 以下の符号なし整数型で、`bool` は除きます。`1 <= Bits <= UInt のビット数` とし、挿入値は下位 `Bits` bit に収めます。既定では 64 bit 全てを使います。

以下で $$S$$ は現在の空間、$$S_x=\{x\mathbin{\mathrm{xor}}s\mid s\in S\}$$ です。空集合の XOR は 0 とし、重複する値は一つに数えます。`empty()` が真でも $$S=\{0\}$$ です。

| 操作・型 | 内容・返り値 |
| --- | --- |
| `XorBasis()` | 零空間を構築 |
| `XorBasis{a,b,...}` / `XorBasis(vector<UInt>)` | 与えた値で空間を構築 |
| `value_type` / `count_type` / `bits` | `UInt`／`__uint128_t`／`Bits` |
| `rank()` / `empty()` | 次元／次元が 0 か |
| `count()` | $$S$$ の異なる値の個数。$$2^{\mathrm{rank}}$$ |
| `clear()` | 零空間に戻す |
| `insert(value)` | 値を追加。次元が増えたときのみ `true` |
| `contains(value)` | $$S$$ に含まれるか |
| `reduce(value)` | 同じ剰余類の最小代表。`min_xor(value)` と同じ |
| `min_xor(x = 0)` / `max_xor(x = 0)` | $$S_x$$ の最小値／最大値 |
| `min_nonzero()` | $$S$$ の最小の非零値。存在しなければ `nullopt` |
| `mex(x = 0)` | $$S_x$$ に含まれない最小の非負整数。`count_type` |
| `vectors()` | 現在の独立な基底を `vector<UInt>` で返す |
| `canonical()` | 同じ空間の一意な簡約基底を持つ `XorBasis` を返す。`vectors()` は昇順 |
| `coordinates(value)` | `vectors()` の何番目を XOR するかをビットで表す。作れなければ `nullopt` |
| `combine(coefficients)` | `coordinates` の逆。`rank()` 以上のビットは 0 とする |
| `kth(k,x = 0)` | $$S_x$$ の昇順で 0 始まり `k` 番目。範囲外なら `nullopt` |
| `count_less(value,x = 0)` / `count_leq(value,x = 0)` | $$S_x$$ のうち、`value` 未満／以下の個数 |
| `count_range(lower,upper,x = 0)` | $$S_x$$ のうち半開区間 `[lower,upper)` に入る個数。`lower <= upper` |
| `index_of(value,x = 0)` | $$S_x$$ での昇順の添字。含まれなければ `nullopt` |
| `lower_bound(value,x = 0)` / `upper_bound(value,x = 0)` | $$S_x$$ で `value` 以上／より大きい最小値。なければ `nullopt` |
| `predecessor(value,x = 0)` | $$S_x$$ で `value` 未満の最大値。なければ `nullopt` |
| `merge(b)` | `b` との和空間に更新し、増えた次元数を返す |
| <code>a &#124; b</code> / <code>a &#124;= b</code> | 和空間／その代入 |
| `intersection(b)` / `a & b` / `a &= b` | 共通部分の空間／その代入 |
| `is_subspace_of(b)` | `b` の部分空間か |
| `a == b` / `a != b` | 張る空間が同じか。挿入順は問わない |
| `orthogonal_complement()` | 下位 `Bits` bit の標準内積による直交補空間 |

`kth` の添字、順序・個数クエリの境界値と返される個数・添字、`mex` は `count_type` を使います。64 次元の $$2^{64}$$ 個も正確に扱えます。例えば全範囲の個数は `count_range(0, __uint128_t(1) << 64)` と書けます。`UInt` の最大値を超える境界値も利用できます。`x` は `UInt` の範囲で指定でき、`Bits` より上のビットはそのまま保持されます。

`coordinates` は元の入力の添字ではなく、現在の `vectors()` に対する係数です。元の入力による復元には次のクラスを使います。元の空間の任意の `u` と、直交補空間の任意の `v` に対して `popcount(u & v)` は偶数です。

`XorBasisWithWitness<UInt,Bits>` は、独立な入力の添字を保持します。テンプレート引数の既定値は `XorBasis` と同じです。

| 操作 | 内容・返り値 |
| --- | --- |
| `XorBasisWithWitness()` | 初期化 |
| `insert(value)` | 値を追加。次元が増えたときのみ `true` |
| `size()` | 追加した回数。0 や従属な値も含む |
| `basis()` | 現在の `XorBasis` の const 参照。最大値・順序統計などのクエリに利用 |
| `represent(value)` | XOR が `value` になる元の入力の添字を `optional<vector<size_t>>` で返す |
| `clear()` | 基底と入力の添字を初期化 |

添字は挿入順の 0 始まりで、返り値は昇順です。作れなければ `nullopt`、0 なら空の配列です。復元には高々 `rank()` 個の独立な入力を使います。

`RollbackXorBasis<UInt,Bits>` は挿入を巻き戻せます。

| 操作 | 内容 |
| --- | --- |
| `RollbackXorBasis()` | 初期化 |
| `insert(value)` | 値を追加。次元が増えたときのみ `true`。従属でも履歴に残す |
| `basis()` | 現在の `XorBasis` の const 参照 |
| `snapshot()` | 現在の履歴の長さを `size_t` で返す |
| `undo()` | 最後の挿入を 1 回取り消す。履歴が空のときは呼ばない |
| `rollback(token)` | 指定した履歴の長さまで巻き戻す |
| `clear()` | 基底と履歴を初期化 |

`token` は現在の履歴の先頭部分を指すものにします。巻き戻して捨てた履歴や `clear()` より前の履歴は復元できません。過去の基底自体を保持する場合は `basis()` をコピーします。

`PrefixXorBasis<UInt,Bits>` は右端を順に進める区間クエリに使います。追加済み列を $$a_0,\ldots,a_{n-1}$$ とすると、対象は `[lower,n)` です。

| 操作 | 内容・返り値 |
| --- | --- |
| `PrefixXorBasis()` | 初期化 |
| `push_back(value)` | 末尾に値を追加。列全体の次元が増えたときのみ `true` |
| `size()` | 追加した要素数 |
| `rank(lower = 0)` | `[lower,size())` の次元 |
| `basis(lower = 0)` | `[lower,size())` の `XorBasis` を返す |
| `min_xor(lower = 0,x = 0)` / `max_xor(lower = 0,x = 0)` | 区間内の部分集合の XOR と `x` の XOR の最小値／最大値 |
| `contains(lower,value)` | 区間内の部分集合の XOR で `value` を作れるか |
| `clear()` | 列を初期化 |

`0 <= lower <= size()` とします。過去の右端の状態は保存しません。任意の `[l,r)` のクエリは右端 `r` の昇順に処理し、`r` 個まで追加してから `basis(l)` などを使います。全 prefix の基底を保存せず、固定サイズの作業領域で処理できます。

$$B=\mathrm{Bits}$$、$$r=\mathrm{rank()}$$ とします。通常基底の追加・所属・最小最大・係数変換は $$O(r)$$、順序統計・mex は $$O(B)$$、最小非零値と基底の列挙は $$O(r)$$ です。正規化・結合・共通部分・直交補空間・空間の比較は $$O(B^2)$$、入力数 $$n$$ からの構築は $$O(B+nB)$$、コピー・clear と保存領域は $$O(B)$$ です。`rank`・`empty`・`count` は $$O(1)$$。次元が $$B$$ のときは追加・所属・最小最大・`kth`・個数クエリを $$O(1)$$ で処理します。復元付き基底の追加・復元は $$O(B)$$、保存領域は $$O(B)$$。ロールバックの追加は償却 $$O(B)$$、`undo` は $$O(1)$$、`rollback` は取り消す回数に比例し、履歴の領域は挿入回数に比例します。prefix 用の追加・各区間クエリ・基底の抽出は $$O(B)$$、保存領域は $$O(B)$$ です。
