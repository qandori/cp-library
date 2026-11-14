---
title: "Interval Map"
documentation_of: //data_structure/interval_map.hpp
library_category: data_structure/associative
---

整数の半開区間 $$[l,r)$$ に値を代入するデータ構造です。重なった部分は上書きし、隣接する同じ値の区間は自動で結合します。値が設定されていない場所も扱えます。

```cpp
#include "lib/data_structure/interval_map.hpp"
#include <string>

IntervalMap<int, std::string> colors(0, 10, "red");
colors.assign(3, 7, "blue");
auto value = colors.get(4); // optional("blue")
auto missing = colors.get(10); // nullopt

colors.erase(4, 6);
auto size = colors.size(); // 8: 値が設定されている整数の個数
for (const auto &[left, segment] : colors) {
    // [left, segment.right) の値が segment.value
}

colors.assign(3, 7, "red");
auto count = colors.interval_count(); // 1: [0,10) がすべて red
colors.set(2, "green"); // [2,3) に代入
```

`IntervalMap<T,V,Size,Equal>` の `T` は座標型、`V` は値の型です。どちらも既定は `std::int64_t` です。`T` は `bool` を除く整数型を使います。負の座標、`__int128_t`・`__uint128_t`、Boost.Multiprecision の整数型にも対応し、ヘッダー自体は Boost に依存しません。

## API

| 操作 | 内容・返り値 |
| --- | --- |
| `IntervalMap()` | 空のマップを作成 |
| `IntervalMap(l,r,value)` | 初めに `[l,r)` へ `value` を代入 |
| `IntervalMap(intervals)` | `vector<interval_type>` または初期化子リストの区間を順番に代入 |
| `assign(l,r,value)` | `[l,r)` を `value` で上書き |
| `set(x,value)` | 整数 `x` の値を設定 |
| `assign(intervals)` | 全内容を、区間列を順番に代入した結果で置き換える |
| `erase(l,r)` / `erase(x)` | 区間／整数 `x` の値を削除。値がなくなった整数の個数を返す |
| `get(x)` | `x` の値を `std::optional<V>` で返す。未設定なら `nullopt` |
| `at(x)` | `x` の値の `const V&` を返す。値が設定されていることが必要 |
| `find(x)` | `x` を含む区間のイテレータ。未設定なら `end()` |
| `lower_bound(x)` | `x` を含む区間、なければ右側の最初の区間のイテレータ。なければ `end()` |
| `contains(x)` | `x` に値が設定されているか |
| `contains(l,r)` | 値を問わず `[l,r)` 全体に値が設定されているか |
| `contains(l,r,value)` | `[l,r)` 全体が `value` か |
| `intersects(l,r)` | `[l,r)` 内に値が設定された整数があるか |
| `count(l,r)` | `[l,r)` 内で値が設定された整数の個数 |
| `for_each(l,r,visit)` | 重なる区間を `[l,r)` で切り詰め、`visit(left,right,const V&)` を左から呼ぶ。未設定部分は省く |
| `size()` / `interval_count()` | 値が設定された整数の個数／区間数 |
| `empty()` | 区間がないか |
| `begin()` / `end()` | 左端の昇順に区間を読み取り専用で走査 |
| `rbegin()` / `rend()` | 左端の降順に区間を読み取り専用で走査 |
| `clear()` | 全内容を削除 |
| コピー／代入 | 独立したマップとして複製 |
| 移動／移動代入 | 内容を移動。移動元は空になり、再利用可能 |
| `swap(t)` / `swap(s,t)` | 内容と値の比較関数を交換 |

`interval_type` は `{left,right,value}` の 3 メンバーを持つ構造体です。区間列に重なりがあれば、後の要素の値を優先します。イテレータは左端をキーとし、`second.right` が右端、`second.value` が値です。更新で分割・結合・削除された区間のイテレータや参照は取得し直してください。`for_each` のコールバック内で同じマップを更新しないでください。

`V` はコピー・移動・代入ができる型です。等値比較には `Equal`（既定は `std::equal_to<V>`）を使い、同値な値は同じ値として扱います。独自の比較を使う場合は、2 つの値を同じとみなすかを `bool` で返す関数を `Equal` に指定し、コンストラクタの最後の引数で渡します。自分自身との比較は `true`、引数を交換しても結果は同じとし、`a` と `b`、`b` と `c` を同じとみなすなら `a` と `c` も同じとみなしてください。`assign(intervals)`・`clear` は比較関数を維持します。

## 範囲と計算量

区間は $$l\le r$$ とし、両端とも `T` で表します。空区間の更新・列挙は何もせず、`contains(l,r)`・`contains(l,r,value)` は `true`、`intersects(l,r)` は `false`、`count(l,r)` は 0 です。

上限のある座標型では、設定できる整数は `T` の最小値以上、最大値未満です。最大値を区間の右端や検索座標には使えます。`set(x,value)` は最大値未満の `x` に対して使い、`erase(x)` は最大値なら 0 を返します。

`Size` は区間の長さと個数の型です。既定は、64 bit 以下の組み込み整数座標では `std::uint64_t`、128 bit 座標では `__uint128_t`、その他では `T` です。`T`・`Size` は `std::numeric_limits` の `is_integer` が `true` になる型を使います。独自の `Size` を指定する場合は、端点からの変換・長さの差・個数の加減算を正確に計算できる型を選んでください。固定幅の多倍長整数で広い区間を扱う場合は、`Size` に `cpp_int` などを指定できます。

保持する区間数を $$n$$、対象範囲に重なる区間数を $$k$$ とします。区間の代入・削除、`contains(l,r)`・`count`・`for_each` は $$O(\log n+k)$$ 時間、単点更新・検索・`contains(l,r,value)`・`intersects` は $$O(\log n)$$ 時間です。`size`・`interval_count`・`empty` とイテレータの取得は $$O(1)$$、コピー・`clear` は $$O(n)$$、移動構築・交換は $$O(1)$$ 時間です。移動代入は移動先の区間数に比例します。空のマップに対する操作は定数時間です。

長さ $$m$$ の区間列からの構築は $$O(m\log m)$$、`assign(intervals)` は $$O(n+m\log m)$$ 時間です。使用空間は $$O(n)$$ で、区間列からの構築・置換中は $$O(n+m)$$ です。以上は座標・値の比較、コピー、演算を定数時間とした計算量です。
