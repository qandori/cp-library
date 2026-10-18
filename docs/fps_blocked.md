---
title: FPS のブロック変換
documentation_of: //components/fps_blocked.hpp
---

`fps::log()` と `fps::pow()` が使う内部部品です。公開 API から利用します。Linux x86_64 / GCC 用に定義され、呼び出し側が AVX2 対応を実行時に確認します。他の環境では既存の FPS 実装を使います。

`fps_detail::blocked_poly::native_log(in,input,out,n,integrate)` は、`in` の `input` 個の内部 Montgomery 係数から、対数微分の最初の `n-1` 項を求めます。`integrate=true` では積分した対数を、偽では定数項 0 の後に対数微分を格納します。結果は空の `vector<Mint>` である `out` に追加し、長さを `n` にします。`in` は変更せず、`out` と重ねません。公開 FPS 側では `256 <= n <= 2^24`、入力長 64 以上の条件で使います。

入力の範囲外は 0 とし、出力次数より後ろは読みません。係数の内部ワードは $[0,2p)$、定数項は非零です。入力全体の共通因子 $R=2^{32}$ は $f'/f$ で相殺されるので、入力の Montgomery 解除は行いません。積分の倍率に $R$ を組み込み、出力を直接 Montgomery 表現に戻します。ここで $p=998244353$ です。

`blocked_poly::Transform` は root と Shoup quotient を隣接配置した、次数 8 の環を葉とする部分変換です。葉の積と最下段を融合し、最上段で倍率と必要な半分の復元を行います。`Arena` は既存の FPS 作業領域を使い、64 byte の間隔を空けて配列を配置します。確保失敗時は通常の整列配列へフォールバックし、それも失敗した場合は例外を送出します。公開配列に padding やアラインメントは要求しません。

入力と出力の最大係数数を $n$ として、時間は $O(n\log n)$、追加空間は $O(n)$ です。
