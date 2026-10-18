---
title: "Predecessor Set（64 分木）"
documentation_of: //components/predecessor_set.hpp
redirect_from: /data_structure/fast_set.hpp.html
---

```cpp
#include "lib/components/predecessor_set.hpp"

PredecessorSet set("00100100");
set.insert(7);
set.erase(2);
auto a = set.next(3); // 5
auto b = set.prev(6); // 5
auto lo = set.min(); // 5
auto hi = set.max(); // 7
set.for_each(0, 8, [](int x) { (void)x; });
```

固定範囲 `[0,n)` の整数集合を 64 分木で管理します。各階層の 1 bit は、下位階層の対応する 64 bit 語が空でないかを表します。全階層を連続した配列で保持します。区間ソート木も同じ実装を使います。

| 操作 | 内容・返り値 |
| --- | --- |
| `PredecessorSet(n = 0,filled = false)` | 範囲 `[0,n)` の空集合。`filled=true` なら全要素を入れる |
| `PredecessorSet(bits)` | `string_view` の 0/1 列から構築。位置 `i` が `'1'` なら `i` を入れる |
| `PredecessorSet(n,words)` | `span<const uint64_t>` のビット列から構築 |
| `assign(n,filled = false)` | 範囲と内容を再設定 |
| `assign(bits)` / `assign(n,words)` | 0/1 列／整数のビット列から再構築 |
| `universe_size()` | 管理する範囲の長さ |
| `size()` / `empty()` | 要素数／空判定 |
| `contains(p)` | `p` が含まれるか |
| `insert(p)` | 要素を追加。既に含まれる場合は false |
| `erase(p)` | 要素を削除。含まれない場合は false |
| `set(p,present)` | `present=true` なら追加、false なら削除。集合が変化した場合だけ true |
| `next(p)` | `p` 以上の最小要素。なければ `n` |
| `prev(p)` | `p` 以下の最大要素。なければ -1 |
| `min()` / `max()` | 最小／最大要素。空ならそれぞれ `n`／-1 |
| `for_each(l,r,f)` | `[l,r)` の要素について `f(int)` を昇順に呼ぶ |
| `for_each(f)` | 全要素について `f(int)` を昇順に呼ぶ |
| `to_vector()` | 全要素を昇順の `vector<int>` で返す |
| `words()` | 最下層のビット列への読み取り専用 `span<const uint64_t>` |
| `clear()` / `fill()` | 範囲と容量を保ち、空集合／全体集合にする |
| `swap(other)` | 範囲と内容を交換 |
| `memory_usage()` | 確保済みの動的領域のバイト数 |

`n` は `0..INT_MAX`、挿入・削除・存在判定の対象は `[0,n)` としてください。`next`・`prev` の引数は範囲外でも使えます。列挙区間は `0 <= l <= r <= n` とし、コールバック中に同じ集合を変更しないでください。コールバックを保存・コピーしません。

0/1 列には `'0'` と `'1'` だけを渡します。整数のビット列は長さ `ceil(n/64)`、位置 `i` は `words[i/64]` の下から `i%64` bit に対応します。最後の語の範囲外の bit は無視します。入力は保持せず、構築後に破棄できます。`assign(n,set.words())` のように自分自身の領域からの再構築にも対応します。`words()` の内容は更新で変化し、再構築・移動・破棄の後は以前の span を使わないでください。一時オブジェクトには `words()` を呼べません。

x86_64 で AVX2 が使える場合、十分長い 0/1 列を AVX2 でまとめて変換します。実行時に判定するため、AVX2 専用のコンパイルオプションは不要です。ARM の Mac などでは通常の整数演算を使います。コピー・ムーブに対応し、移動元は範囲長 0 の空集合になります。

範囲長を $$n$$ とします。整数・整数のビット列からの構築、`assign(n,...)`、`clear`・`fill` は $$O(1+n/64)$$、0/1 列からの構築・`assign(bits)` は $$O(n)$$ 時間です。`insert`・`erase`・`set`・`next`・`prev`・`min`・`max` は $$O(\log_{64}n)$$、列挙する要素数を $$k$$ として列挙・`to_vector` は $$O((k+1)\log_{64}n)$$、他の問い合わせ・`swap` は $$O(1)$$ です。$$n\leq1$$ では検索・更新を $$O(1)$$ とします。

保持空間は $$O(1+n/64)$$ 語、予約容量を含む確保空間は過去の最大範囲長を $$M$$ として $$O(1+M/64)$$ 語です。`to_vector` の返り値には追加で $$O(k)$$ 空間を使います。コールバックは $$O(1)$$ とします。
