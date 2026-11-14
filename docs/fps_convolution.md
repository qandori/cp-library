---
title: FPS の融合畳み込み
documentation_of: //components/fps_convolution.hpp
library_category: components/fps
---

Linux x86_64 / GCC の `fps::multiply` と `convolution(vector<mint>,vector<mint>)` が使う内部部品です。公開 API から利用し、呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::multiply_detail::multiply(a,b)` | 内部 Montgomery 係数の二つの非空配列の積を返す | $$O(n\log n)$$ |

通常は [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `f*g` を使います。ここで `n` は入出力の最大係数数です。

`fps_detail::multiply_detail::multiply(a,b)` は、非空の Montgomery 係数列から積を返します。出力長は $$2^{23}$$ 以下とし、入力を変更しません。同一配列の二乗にも対応します。入力に先頭アドレスの配置条件や末尾の余分な領域は要求しません。

内部の `Product(n,m,borrowed_a)` と `ntt::Convolution(n,m,borrowed_a)` は、第 1 入力用の作業配列 A として `borrowed_a` を使います。第 2 入力用の作業配列 B と変換用の定数表は内部で確保します。`borrowed_a` は先頭アドレスを 32 byte の倍数にし、構築済みの 4 byte 係数を `length()+16` 個以上持つ領域を指定します。呼び出し側で入力を法未満の値に直してコピーし、末尾の余分な領域を 0 にします。この変換オブジェクトの使用終了まで領域を破棄しないでください。`borrowed_a` を省略すると A も内部で確保します。

出力長を $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。
