---
title: "疎 FPS の共通演算"
documentation_of: //components/fps_sparse.hpp
library_category: components/fps
---

`fps_detail::SparseSeries` は、非零係数が少ない FPS の逆数・対数・指数・冪を計算する通常版 FPS の内部部品です。係数型は `mint` です。軽量版では [小さな疎演算の部品](fps_sparse_scalar.md) を使います。

`Terms` は次数と係数の組を次数の昇順に並べた `vector<pair<int,mint>>` です。定数項は含めず、各次数は正・重複なし、係数は非零とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `collect(f,size,n,limit=12)` | 配列 `f` の次数 1 以上 `min(size,n)` 未満の非零係数を `Terms` で返す。`limit` を超えた場合は `limit+1` 個まで返す | $$O(m)$$ |
| `get<Kind::inverse>(terms,n,initial,0,inverses)` | $$\mathrm{initial}/(1+h(x))$$ の先頭 `n` 係数を `vector<mint>` で返す | $$O(nk)$$ |
| `get<Kind::logarithm>(terms,n,0,0,inverses)` | $$\log(1+h(x))$$ の先頭 `n` 係数を `vector<mint>` で返す | $$O(nk)$$ |
| `get<Kind::exponential>(terms,n,initial,0,inverses)` | $$\mathrm{initial}\exp(h(x))$$ の先頭 `n` 係数を `vector<mint>` で返す | $$O(nk)$$ |
| `get<Kind::power>(terms,n,initial,exponent,inverses)` | $$\mathrm{initial}(1+h(x))^{\mathrm{exponent}}$$ の先頭 `n` 係数を `vector<mint>` で返す | $$O(nk)$$ |

ここで $$h(x)=\displaystyle\sum_{(j,a)\in\mathrm{terms}} ax^j$$ です。`get` に渡す項数は 64 以下、各次数は `n` 未満とします。`n=0` は空配列を返します。逆数以外は $$n<998244353$$ とします。

`inverses` は `const mint *(int)` 型の関数ポインタです。引数 `m` に対し、整数 0 以上 `m` 未満の逆元を格納した配列を返します。0 の項は 0 とし、配列は呼び出し中有効とします。逆数の計算では使わないため `nullptr` も渡せます。`exponent` は法 998244353 上の値として扱います。

参考: [疎な逆数の実装](https://judge.yosupo.jp/submission/409581)、[疎な冪の実装](https://judge.yosupo.jp/submission/409706)。

非零係数数を $$k$$、求める係数数を $$n$$ として、時間 $$O(nk)$$、追加空間 $$O(n+k)$$ です。逆元表の準備時間・空間は別途必要です。`collect` は走査した係数数を $$m$$ として $$O(m)$$ 時間、返す項数に比例する追加空間を使います。項が空の場合、`get` は $$O(n)$$ 時間です。
