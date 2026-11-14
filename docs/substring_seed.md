---
title: Common Substring Seed Search
documentation_of: //components/substring_seed.hpp
library_category: components/string
---

[最長共通部分文字列](longest_common_substring.md)の内部部品です。通常は `longest_common_substring(s,t)` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `substring_seed_detail::search(s,t,best)` | 既知の共通部分文字列 `best` から最長の一致を探し、確定した場合に `optional<Match>` を返す。専用処理の対象外、または作業上限を超えた場合は `nullopt` | 時間・追加空間 $$O(n+m)$$ |
| `Match::first / second / length` | 各入力の開始位置と共通部分の長さ | $$O(1)$$ |

$$n,m$$ はそれぞれ `s,t` の長さです。`best` は実際に一致する部分文字列を指定します。`s` と `t` の長さの合計は `INT_MAX-1` 以下にしてください。`nullopt` は共通部分がないことを意味せず、呼び出し側で、公開関数が使う接尾辞配列による方法などに切り替える必要があります。候補の一致は実際の要素を比較して確かめるため、誤答確率はありません。
