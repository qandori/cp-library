---
title: Flat Hash Map
documentation_of: //data_structure/flat_hash_map.hpp
library_category: data_structure/associative
---

キーに対応する値を追加・更新・削除する連想配列です。

```cpp
#include "lib/data_structure/flat_hash_map.hpp"
#include <cstdint>

FlatHashMap<std::uint64_t, std::uint64_t> values;
values.reserve(200000);
values.set(10, 3);
values[20] += 5;
auto a = values.get(10); // optional(3)
auto b = values.get(30); // nullopt
if (auto *value = values.find(20)) *value += 2;
values.erase(10);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `FlatHashMap<Key,T,Hash=std::hash<Key>,KeyEqual=std::equal_to<Key>>()` | 空のマップを作成 |
| `FlatHashMap(n)` | `n` 種類のキーを格納するための領域を確保 |
| `set(key,value)` | 値を追加または上書き |
| `operator[](key)` | 値の `T&`。未登録なら `T{}` を追加 |
| `find(key)` | 値へのポインタ。未登録なら `nullptr` |
| `get(key)` | 値を `std::optional<T>` で返す。未登録なら `nullopt` |
| `at(key)` | 値への参照。未登録なら `std::out_of_range` |
| `contains(key)` / `count(key)` | 存在判定／存在すれば 1、なければ 0 |
| `erase(key)` | キーを削除。存在すれば `true` |
| `size()` / `empty()` | 登録されたキー数／空判定 |
| `prefetch(key)` | 後でアクセスするキーを先読みするヒント。内容は変更しない |
| `reserve(n)` | `n` 種類のキーを格納するための領域を確保 |
| `clear()` | 全要素を削除 |
| `swap(other)` | 内容を交換 |

`prefetch` は `const` オブジェクトでも呼び出せる `noexcept` の操作で、未登録のキーも指定できます。環境によっては何も行いません。

`find`・`at` は `const` オブジェクトでは `const T*`・`const T&` を返します。取得したポインタや参照は、挿入・`set`・`operator[]`・`reserve`・削除・`clear`・代入の後には取得し直してください。既存キーの更新でも領域を確保し直すことがあります。

64 bit 以下の標準整数キーと、格納されているバイト列のコピーで複製できる値では専用のハッシュテーブルを使います。値の型は `std::is_trivially_copyable_v<T>` を満たし、コピー・`T{}` による構築・代入が例外を送出しないことが必要です。その他の型や独自の `Hash`・`KeyEqual` では `std::unordered_map` を使います。`Hash`・`KeyEqual` は既定構築し、同値なキーのハッシュ値は等しくしてください。

`get` は値のコピー、`operator[]` は値の既定構築を必要とします。コピーは独立したマップを作り、移動元は再利用できる空のマップになります。`size_type` は `std::size_t` です。

引数なしの構築・`size`・`empty`・`prefetch`・`swap`・移動構築は $$O(1)$$ 時間です。検索・追加・更新・削除は期待償却 $$O(1)$$ 時間です。確保した領域を $$C$$ として、使用空間・コピー・`clear` は $$O(C)$$、`FlatHashMap(m)` は $$O(m)$$、`reserve(m)` は期待 $$O(C+m)$$ 時間です。既に十分な領域があれば `reserve` は定数時間です。コピー・移動代入には、代入先がそれまで確保していた領域の破棄時間も加わります。空のマップの構築・コピー・削除にも定数時間かかります。値・キー・ハッシュ・比較の演算を定数時間とします。

「期待」はハッシュ値が偏らないときの平均時間を、「償却」は一連の操作の合計時間から求めた 1 回あたりの計算量を表します。
