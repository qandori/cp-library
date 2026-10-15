# Li Chao Tree

```cpp
#include "lib/data_structure/li_chao_tree.hpp"

LiChaoTree<> tree({-3, 0, 5});
tree.add_line(2, 1);
tree.add_segment(-3, 5, -1, 4);
auto value = tree.query(0);
```

クエリ座標は構築時に全て登録します。`add_segment(l, r, a, b)` は `[l, r)` に `ax+b` を追加します。`query(x)` は `optional` を返し、対象がなければ `nullopt` です。

`LiChaoTree<T, true>` で最大値を取得できます。既定の `T` は `std::int64_t`、整数の計算・返り値は 32 bit 型なら `std::int64_t`、64 bit 型なら `__int128_t` です。第 3 テンプレート引数で計算型を指定できます。

`index(x)` はソート・重複除去後の添字を返します。`query_index(i)`・`add_segment_index(l, r, a, b)` で添字を直接使えます。`clear()` は座標を残して直線・線分を全て削除します。

登録時の座標数を `n` として、構築は `O(n log n)`（ソート済みなら `O(n)`）、直線追加・問い合わせ・`index` は `O(log n)`、線分追加は `O(log^2 n)`、`clear` は `O(n)` です。空間は `O(n)` です。
