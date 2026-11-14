---
title: SIMD 命令のコンパイル可否
documentation_of: //components/simd_capabilities.hpp
library_category: components/util
---

SIMD を使う内部部品が共有する設定です。通常は各ライブラリから自動的に読み込まれます。

`CP_LIBRARY_AVX512` はコンパイラが AVX-512F の有効化を許可する場合に `1`、それ以外では `0` です。実行 CPU が対応しているかは、利用するライブラリが別に判定します。このマクロは利用者側で定義・変更しないでください。

コンパイル時の定義だけを提供し、実行時の処理はありません。実行時の追加の記憶領域も不要です。
