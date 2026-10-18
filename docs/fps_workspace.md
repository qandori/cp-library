---
title: FPS の作業領域
documentation_of: //components/fps_workspace.hpp
---

FPS 内部で使う、64 byte 以上に整列した `unsigned int` の作業領域です。

```cpp
auto storage = fps_detail::allocate_fps_words(1024);
storage[0] = 1;
```

`FpsWords` は所有権を移動できる配列です。要素の初期値は未規定で、使用前に書き込む必要があります。

Linux では 2 MiB 以上の要求に対し、2 MiB 境界に整列した匿名マッピングと `MADV_HUGEPAGE` を使います。大きなページの利用は OS の設定と資源に依存します。マッピングに失敗した場合と他の環境では、通常の整列した配列確保に戻ります。確保できなければ `std::bad_alloc` を送出します。解放方法は所有する配列が保持します。

大きなページ用のマッピングは要求を 2 MiB 単位に切り上げ、整列のためにさらに 2 MiB の仮想アドレス領域を確保します。

**計算量**: 作業領域は $O(n)$ です。確保と解放の時間は OS とアロケータに依存します。
