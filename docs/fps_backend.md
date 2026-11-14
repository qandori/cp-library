---
title: "FPS の互換ヘッダー"
documentation_of: //components/fps_backend.hpp
library_category: components/fps
---

[FPS 本体](https://qandori.github.io/cp-library/fps/fps.hpp.html)を読み込む互換ヘッダーです。新しいコードでは `fps/fps.hpp` を直接 include してください。合成・補間・GCD などの追加演算は、それぞれの公開ヘッダーを include します。

利用時は通常の `#include`・`oj-bundle` を使います。[提出コードの生成とサイズ上限](bundle.md)も参照してください。

このヘッダー自身は実行時の前計算や領域確保を行いません。公開する操作の引数・返り値・利用条件と、時間・空間計算量は [FPS 本体](fps.md) と同じです。
