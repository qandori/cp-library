---
title: min-plus / max-plus 畳み込みの整数用共通実装
documentation_of: //components/min_plus_convolution_integral.hpp
library_category: components/convolution
---

[凸・凹 min-plus / max-plus 畳み込み](min_plus_convolution.md)が使う整数型用の内部処理です。通常は公開ヘッダーの関数を使います。

変換した値の範囲と対応命令を確認し、条件を満たす場合は専用処理を選びます。対応しない型・値域・環境では、元の比較による処理を使います。

両入力の長さの和を $$L$$ として、凸 min-plus・凹 max-plus は $$O(L)$$ 時間・空間です。
