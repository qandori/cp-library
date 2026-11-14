---
title: 区間加算・最小値のノード演算
documentation_of: //components/range_add_min_node.hpp
library_category: components/data_structure
---

[区間加算・区間最小値](range_add_range_min.md)の 8 子ノードを処理する内部部品です。通常は `data_structure/range_add_range_min.hpp` を include します。

`range_add_min_detail::Node<T>` の `T` は 64 bit 以下の標準の符号付き整数型、`U` は同じ幅の符号なし型です。配列には各値とノードの最小値の差を `U` で格納します。差分の最小値が 0 になるように揃え直す処理を、以下では「正規化」と呼びます。差分は `U` の全範囲を取り得るため、通常の経路は符号なし整数として比較します。`build` への入力は、差分ではなく元の符号付き値のビット表現です。

通常の `values` は、初期化済みの連続 8 要素の `U` 配列です。この 8 要素の区画をブロックと呼びます。有効要素は先頭 `count` 個で、`1 <= count <= 8` とします。後ろの無効要素は、要素数を 8 に揃えるための詰め物（パディング）です。更新には 8 要素とも読み書き可能な配列を渡します。`build` の count 0 と `materialize` の領域条件は後述します。

複数の値を同時に計算する CPU 命令を SIMD と呼び、その命令がまとめて扱う値の一つをレーンと呼びます。64 bit の SIMD 経路では、`build`・`materialize` を除き、配列の先頭アドレスを 32 の倍数に揃えます。`build`・`materialize` と 32 bit 以下の操作には、通常の `T`・`U` 配列より厳しいアドレスの配置条件はありません。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `build(values,count)` | `U*` の先頭 `unsigned count` 個を元の `T` のビット表現として解釈し、符号付き最小値を `T` で返す。有効要素を最小値からの unsigned 差分へ置き換え、残りは変更しない | $$O(1)$$ |
| `minimum<Narrow>(values,l,r)` | `const U*` と `unsigned l,r` を渡し、`0 <= l <= r <= count` の最小差分を `U` で返す。空区間の返り値は本文参照 | $$O(1)$$ |
| `normalize<Narrow>(values,count,negative,first,last,delta,x=0,dx=0,y=0,dy=0)` | `U*` の `[first,last)` に `U delta`、添字 `x,y` に `U dx,dy` を加え、有効要素を再び最小差分 0 に揃える。元の最小値に加える変化量を `U` で返す | $$O(1)$$ |
| `normalize_boundaries<Narrow>(left,right,right_count,negative,first,last,delta,left_change,right_change)` | 左の添字 `first` に `U left_change`、`[first+1,8)` に `U delta` を加える。右の `[0,last)` に `delta`、添字 `last` に `U right_change` を加える。両ブロックを正規化し、左右の最小値の変化量を `std::pair<U,U>` で返す | $$O(1)$$ |
| `change_one<Narrow>(values,count,index,delta,negative)` | `U*` の添字 `index` に `U delta` を加えて正規化する。元の最小値に加える変化量を `U` で返す。`delta = 0` なら何もしない | $$O(1)$$ |
| `full_minimum<Narrow>(values)` | `const U*` の 8 要素すべての最小差分を `U` で返す。パディングも参照する | $$O(1)$$ |
| `subtract(values,count,change)` | `U*` の先頭 `count` 要素から `U change` を引く。パディングは変更しない。返り値は `void` | $$O(1)$$ |
| `materialize(src,dst,count,base)` | `const U* src` の先頭 `unsigned count` 個に `U base` を巡回加算し、そのビット表現を `T* dst` へ出力。返り値は `void` | $$O(1)$$ |
| `min_signed(x,y)` | AVX2 用の補助関数。`__m256i` の各 signed 64 bit レーンの最小値を `__m256i` で返す | $$O(1)$$ |
| `broadcast_min(value)` | AVX2 用の補助関数。`__m256i` の 4 レーンの signed 64 bit 最小値を、全レーンに複製した `__m256i` で返す | $$O(1)$$ |
| `reduce_min(x)` | AVX2 用の補助関数。`__m256i` の 4 レーンを signed 64 bit として比較し、最小値のビット表現を `U` で返す | $$O(1)$$ |
| `broadcast_min32(value)` | AVX2 用の補助関数。`__m256i` の 8 レーンの unsigned 32 bit 最小値を、全レーンに複製した `__m256i` で返す | $$O(1)$$ |
| `reduce_min32(value)` | AVX2 用の補助関数。`__m256i` の 8 レーンを unsigned 32 bit として比較し、最小値を `U` で返す | $$O(1)$$ |
| `minimum_vector<Narrow>(values,l,r,neutral)` | AVX2 用の補助関数。`const U*` の `[l,r)` 以外を `U neutral` で置き換え、前半と後半の対応レーンを比較した 4 レーンの `__m256i` を返す。変換と中立値の条件は本文参照 | $$O(1)$$ |

`Narrow` は省略できない `bool` のテンプレート引数で、`true` は 64 bit 整数用です。32 bit 以下では `false` を使います。添字と `count,right_count,first,last,x,y,index` は `unsigned`、`negative` は `bool` です。`normalize` は `0 <= first <= last <= count`、`change_one` は `0 <= index < count` とします。`x,y` は `dx,dy` が 0 でも常に 8 未満にし、非ゼロの差分を渡す添字は `count` 未満にします。`normalize` では範囲と一点の重複や `x == y` も指定できます。重なる位置には変化量を合計して加え、更新後の実際の値と `Narrow` の条件を満たすようにします。空の `[first,last)` も指定でき、`dx,dy` のみを適用できます。表に示したもの以外に既定値はありません。

`build` は `0 <= count <= 8` を指定します。先頭 `count` 個を `std::bit_cast<T>` で符号付き値として解釈し、その最小値を求めます。有効要素から最小値のビット表現を unsigned 演算で引き、差分へ正規化します。`T` の最小値と最大値を同時に含められ、`Narrow` の値域条件は不要です。

`build` は count が正なら、無効要素を含む初期化済みの読み書き可能な 8 要素領域を必要とします。無効要素は符号付き最小値の比較から除外し、元の値を保持します。通常は無効要素を `std::numeric_limits<U>::max()` で埋めておきます。これは raw の `T` では負の値ですが、最小値として選ばれません。count 0 は配列を参照せず `std::numeric_limits<T>::max()` を返し、`values=nullptr` も指定できます。

`materialize` も `0 <= count <= 8` を指定します。各出力は `std::bit_cast<T>(U(src[i] + base))` です。`src` と `base` は任意の `U` の値を渡せ、加算は `U` の幅で巡回します。入力は変更せず、出力は先頭 `count` 個だけを書き換えます。count が正なら読み取り可能な `src` の `count` 要素と書き込み可能な `dst` の `count` 要素を用意し、両領域を重ねないでください。count 0 は両方のポインタに `nullptr` を指定できます。count が 8 未満なら、8 要素分の領域を確保する必要はありません。

`normalize_boundaries` の `left,right` は重ならない別の 8 要素ブロックを指す `U*` です。左の有効要素数は 8、右は `1 <= right_count <= 8` とし、`0 <= first < 8`、`0 <= last < right_count` を満たします。左右をそれぞれ正規化しておき、返された変化量をそれぞれの親へ反映します。左右の境界位置には共通の `delta` を重ねず、`left_change,right_change` だけを加えます。

差分への加減算は `U` の巡回演算で行います。これは `U` が $$k$$ bit なら、結果を $$2^k$$ で割った余りとして扱う計算です。負の変化量も `U` に変換して渡し、実際の増減方向を `negative` で別に指定します。1 回の呼び出しで渡す非ゼロの変化量はすべて同じ増減方向にし、減少なら `negative=true`、増加なら `false` とします。`normalize_boundaries` の左右の変化量にも同じ条件が必要です。差分の最上位ビットから増減方向を決めないでください。呼び出し元は、各子が表す更新後の値が `T` の範囲に収まることを保証します。更新前の有効要素は正規化しておき、返された変化量を親ノードの最小値に加えます。

`Narrow=false` では、パディングを `std::numeric_limits<U>::max()` にします。負の更新で差分が 0 を下回り、`U` の最大値付近へ回り込む場合も扱い、`U` 全域の差分を比較します。

32 bit の AVX2 経路は unsigned 32 bit の最小値を使い、符号付き整数の全範囲に対応します。`negative=true` では、更新後の unsigned 差分が元より大きくなるレーンを、0 を下回った差分として区別します。このレーンが存在すればその中の最小値を選び、なければ全有効レーンの最小値を選びます。`normalize_boundaries` では左右それぞれでこの判定を行ってから、両ブロックの最小値を同時に求めます。右のパディングには正規化量を引かず、`U` の最大値を維持します。

`minimum` の空区間は、32 bit 以下では `std::numeric_limits<U>::max()` を返します。64 bit も `Narrow=false` なら同じです。64 bit の `Narrow=true` では、SIMD 経路は `std::numeric_limits<T>::max()`、スカラー経路は `std::numeric_limits<U>::max()` を `U` で返します。空区間から通常の差分が得られるとみなして加算しないでください。

`min_signed`・`broadcast_min`・`reduce_min`・`minimum_vector` は AVX2 有効時だけ定義される、64 bit 整数用の補助関数です。`broadcast_min32`・`reduce_min32` は同じ条件で定義される unsigned 32 bit 用です。

`minimum_vector` は `0 <= l <= r <= 8` とし、指定区間が有効要素だけを含むようにします。配列の先頭アドレスは前述のとおり 32 の倍数に揃えます。レーン `j` は添字 `j` と `j+4` の小さい方を表します。`l == r` なら全レーンが `neutral` になります。レーン間の最小値はまだ取らず、`reduce_min` または `broadcast_min` と組み合わせます。

`minimum_vector<true>` は元の差分を signed 64 bit として比較します。`minimum_vector<false>` は読み込んだ各差分 `value` を `key = value ^ sign_bit` に変換し、`key` を signed 64 bit として比較します。`sign_bit` は最上位 bit だけが 1 の整数です。これにより元の unsigned 順序と一致します。`neutral` はこの変換後の表現で渡し、signed の順序で全候補以上になる値を選びます。このように最小値として選ばれないための値を、表では中立値と呼んでいます。通常経路では `U(std::numeric_limits<T>::max())` です。`false` の返り値に `reduce_min` を適用した後は、さらに `sign_bit` を XOR して元の差分に戻します。配列に保存する差分自体は変換しません。

`Narrow=true` は 64 bit 整数の内部用経路です。`M = std::numeric_limits<T>::max()`、初期列の最大値と最小値の差を $$S_0$$、全区間加算を除く更新の絶対変化量累計を $$A$$ とし、$$\displaystyle L=\left\lfloor\frac{M}{3}\right\rfloor$$ とします。一点代入では新旧の値の差の絶対値を累計に加えます。呼び出し元が $$S_0+A<L$$ を保証し、構築後のパディングを $$2L$$ にします。SIMD の `normalize<true>` はパディングも含めて signed の最小値を計算し、8 レーンすべてからその値を引きます。

有効要素の正規化後の差分は $$[0,S_0+A]$$ に収まります。各ノードの最小値の変化量の絶対値は、その更新で加えた変化量の絶対値以下です。したがって、パディングから差し引かれる正規化量の累計の絶対値も $$A$$ 以下で、パディングの差分は $$[2L-A,2L+A]$$ に収まります。$$A<L$$ なのでパディングは $$L$$ より大きく、有効要素は $$L$$ 未満です。一時的な負の差分も $$-L$$ より大きく、パディングを含むすべての比較対象が signed 64 bit の範囲内です。これにより、パディングを含む signed の最小値計算と全レーンの正規化が成立します。

範囲条件を満たさなくなる更新の前に、各段の末尾のパディングを `std::numeric_limits<U>::max()` に戻し、通常の経路に切り替えます。この管理は `RangeAddRangeMin` が行い、利用者の値域制限は増えません。

子の数は 8 に固定されているため、各操作は $$O(1)$$ 時間・追加空間です。
