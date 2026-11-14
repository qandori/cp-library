---
title: Kinetic Tournament Core
documentation_of: //components/kinetic_tournament_monoid.hpp
library_category: components/data_structure
---

区間更新版 `KineticTournament` が `SegtreeBeats` に渡す内部処理です。通常は `KineticTournament` を使い、この部品を直接呼ぶ必要はありません。

```cpp
#include "lib/components/kinetic_tournament_monoid.hpp"

int main() {
	using M = kinetic_tournament_detail::Monoid<std::int64_t, true>;
	M::Node left{10, 1, M::never, 0};
	M::Node right{4, 3, M::never, 1};
	auto node = M::op(left, right);
	assert(node.index == 0 && node.remaining == 2);
	assert(M::mapping(M::Tag{2, 0}, node));
	assert(node.value == 12 && node.remaining == 0);
	assert(!M::mapping(M::Tag{1, 0}, node));
}
```

各区間で、最大値版なら最大値、最小値版なら最小値を与える要素を選びます。以下ではこの要素を「選択した要素」と呼びます。`remaining` は、選択が変わらない範囲で `Tag.steps` の累積をあといくつ増やせるかを表します。

`Monoid<T,maximize>` の `T` は符号付き 64 bit 以下の整数型です。`maximize=true` は最大値、`false` は最小値を選びます。係数は添字順に昇順、重み付き加算の増分は非負であることが必要です。

| 型・値 | 内容 |
| --- | --- |
| `Node` | 選択した要素の現在値 `value`、固定係数 `slope`、添字 `index`、安全に進められる残りステップ数 `remaining` |
| `Tag` | 非負の増分 `steps` と一様加算 `add`。`value += steps*slope+add` を表す |
| `W` / `U` | 内部評価型 `__int128_t` / 残り増分の型 `uint64_t` |
| `never` | 選択した要素が変わらないことを表す `UINT64_MAX` |

`index == -1` は有効な要素がない節点です。葉は `{value,slope,never,index}` で作ります。`op` には、左の区間と右の区間をこの順で渡します。選択した要素の値に加え、両方の子の `remaining` と、左右で選択した要素が入れ替わるまでの増分の上限を保存します。`remaining` は木の構造に依存する内部情報で、区間の値そのものではありません。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `identity()` | 要素のない `Node` | $$O(1)$$ |
| `tag_identity()` | 何もしない `Tag` | $$O(1)$$ |
| `better(a,b)` | `a` の選択した要素を優先するか。空要素は優先しない | $$O(1)$$ |
| `op(a,b)` | 左右の節点から選択した要素と `remaining` を求める | $$O(1)$$ |
| `mapping(tag,node)` | 増分が `remaining` 以下なら更新して true、子へ降りる必要があれば false | $$O(1)$$ |
| `composition(f,g)` | `g` の後に `f` を行う合成更新 | $$O(1)$$ |

同値の場合、最大値版は添字が大きい方、最小値版は添字が小さい方を選びます。`mapping` が false を返した場合は節点を変更しません。`SegtreeBeats` 側で更新を子に伝え、その結果から親を作り直します。空要素への `mapping` は true です。

現在値と更新後の値は `T` に、増分・加算の累積と評価途中は `W` に収めてください。残りステップ数の計算は符号なしの差と除算を使い、符号付き 64 bit 全域の係数・値の差を扱います。各操作の時間・追加空間は $$O(1)$$ です。木全体での更新回数の評価は `KineticTournament` の説明を参照してください。
