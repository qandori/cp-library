---
title: "軽量 FPS の基本演算"
documentation_of: //components/fps_light_core.hpp
library_category: components/fps
---

[軽量 FPS](fps_light.md) の型 `FormalPowerSeries<T>` と別名 `fps` を提供する内部部品です。軽量版を明示する名前 `LightFormalPowerSeries<T>`・`light_fps`・`LightFPS` も使えます。係数型・各演算の引数・返り値・計算量は公開ページを参照してください。

四則演算・逆数・対数・指数・平方根・累乗を含みます。NTT は [共有部品](fps_light_ntt.md) を使います。多点評価・補間・Bostan–Mori などの追加演算と積木は読み込みません。通常は `fps/fps_light.hpp` を include してください。

入力・出力の最大係数数を $$n$$ として、加減算・微分・積分・シフト・一点評価は $$O(n)$$、乗算・除算・逆数・対数・指数・平方根は $$O(n\log n)$$、累乗は指数を $$k$$ として $$O(n\log n+\log k)$$ 時間です。返す係数列を含めた追加空間は $$O(n)$$ 以内です。NTT の根と整数逆元の共有表は、これまでに必要とした最大長に比例する領域を保持します。

指数の変換結果を再利用する手法は、[NyaanNyaan/library](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/fps/ntt-friendly-fps.hpp) と [suisen-cp/cp-library-cpp](https://github.com/suisen-cp/cp-library-cpp/blob/c49e847ae29ed11510ad9cd7713f5ec77ddf20b1/library/polynomial/formal_power_series.hpp) を参考にしています。
