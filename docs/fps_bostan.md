---
title: Bostan–Mori の NTT 倍長化
documentation_of: //components/fps_bostan.hpp
library_category: components/fps
---

Linux x86_64 / GCC で AVX2（複数の整数をまとめて処理する CPU 命令）を使う内部部品です。通常は公開関数 [Bostan–Mori](https://qandori.github.io/cp-library/fps/bostan_mori.hpp.html) または [線形漸化式](https://qandori.github.io/cp-library/fps/linear_recurrence.hpp.html) を使います。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容 | 時間計算量 |
| --- | --- | --- |
| `fps_detail::bostan_detail::inverse_twisted(plan,in,out,powers,n)` | 長さで割る前の逆 NTT を `in` から `out` へ求め、各要素へ Montgomery 表現の `powers[i]` を掛ける | $$O(n\log n)$$ |

`n` は8以上 $$2^{22}$$ 以下の2冪とし、`plan` はその変換に必要な変換用の定数表を持つものとします。`in`・`out`・`powers` は互いに重ならない各 `n` 要素の配列で、先頭アドレスを 32 byte の倍数にそろえます。各要素は `NTT::U` の生存中のオブジェクトとし、内部の 32 bit 整数値の範囲は $$[0,2p)$$、$$p=998244353$$ です。入力の変更や配列の末尾の余分な領域は要求しません。呼出側で `NTT::vectorized()` を確認します。

逆変換で元の値に戻すための $$n$$ による除算は含みません。必要なら `powers` にその倍率を含めてください。

**計算量**: $$O(n\log n)$$ 時間。入力・出力・係数倍・変換用の定数表を含む空間は $$O(n)$$、局所変数は $$O(1)$$ です。
