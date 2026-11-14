---
title: 流量の上下限と容量の変換
documentation_of: //components/flow_bounds.hpp
library_category: components/graph
---

`flow_detail::CapacityBounds<Cap>` は、下限付きフローで使う整数の変換部品です。符号付きの流量と、上下限の差を表す符号なしの容量を扱います。通常は [MaxFlowLowerBound](max_flow_lower_bound.md) または [MinCostBFlow](min_cost_b_flow.md) を使います。

`Cap` は 64 bit 以下の標準の符号付き整数型です。`Wide` は `__int128_t`、`Residual` は `Cap` と同じ幅の符号なし整数型 `std::make_unsigned_t<Cap>` の別名です。符号付き整数の最小値から最大値までの差も `Residual` で表せます。

```cpp
#include "lib/components/flow_bounds.hpp"
#include <cassert>
#include <limits>

int main() {
	using Bounds = flow_detail::CapacityBounds<std::int64_t>;
	auto lower = std::numeric_limits<std::int64_t>::min();
	auto upper = std::numeric_limits<std::int64_t>::max();
	auto width = Bounds::capacity(lower, upper);
	assert(width == std::numeric_limits<Bounds::Residual>::max());
	assert(Bounds::value(-5) == -5);
	assert(Bounds::amount(7) == 7);
}
```

| 操作・型 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `CapacityBounds<Cap>` | 型引数を指定する。保持するデータはない | —（型） | —（型） |
| `Wide` | `__int128_t` の別名 | —（型） | —（型） |
| `Residual` | `std::make_unsigned_t<Cap>` の別名 | —（型） | —（型） |
| `value(x)` | `Wide` 型の `x` を `Cap` に変換して返す。`Cap` の範囲内であることが必要 | $$O(1)$$ | $$O(1)$$ |
| `amount(x)` | `Wide` 型の `x` を `Residual` に変換して返す。0 以上で `Residual` の範囲内であることが必要 | $$O(1)$$ | $$O(1)$$ |
| `capacity(lower,upper)` | `Cap` 型の下限・上限の差 `upper - lower` を `Residual` で返す。`lower <= upper` が必要 | $$O(1)$$ | $$O(1)$$ |

全て静的な関数で、オブジェクトを構築する必要はありません。`capacity` は差を 128 bit 整数で計算するため、`Cap` で引き算した場合にあふれる組合せも扱えます。範囲外の引数を丸めたり、エラーを返したりする機能はありません。各関数の前提を満たして呼び出してください。

固定幅整数の演算を定数時間として、全ての関数は最悪 $$O(1)$$ 時間・追加空間です。配列・前計算・共有表を保持せず、動的な領域確保も行いません。
