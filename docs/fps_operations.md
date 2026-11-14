---
title: "FPS の内部演算"
documentation_of: //components/fps_operations.hpp
library_category: components/fps
---

`fps_detail::Operations<T>` は FPS と独立した多項式関数で共有する内部処理です。通常は `fps/fps.hpp` や各関数のヘッダーから利用します。`F` は `FormalPowerSeries<T>`、`W` は内部係数型の FPS、`base` は `vector<T>`、`Word` は内部の 32 bit 整数です。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `import_work(a)` / `export_work(a)` | 係数型を内部型へ／内部型から変換 | $$O(n)$$ |
| `transform(n)` | 変換用の定数を少なくとも `n` 個保持する共有 NTT オブジェクト | 拡張: $$O(n)$$、準備済み: $$O(1)$$ |
| `words(a)` | 係数配列の内部の 32 bit 整数値へのポインタ | $$O(1)$$ |
| `transform_inplace<inverse>(a)` | 内部順序の NTT。逆変換は未正規化 | $$O(n\log n)$$ |
| `ntt(a)` / `ntt_inverse(a)` / `intt(a)` | 順変換／未正規化の逆変換／正規化した逆変換 | $$O(n\log n)$$ |
| `ntt_half(a)` / `ntt_half(a,n)` | 後半が 0 の係数列を変換 | $$O(n\log n)$$ |
| `scale_words(a,c)` | 全係数を定数倍 | $$O(n)$$ |
| `geometric_scale(a,first,step)` | 第 `i` 係数を `first * step^i` 倍 | $$O(n)$$ |
| `batch_inverse(dst,src)` | 各非零要素の逆元を一括計算 | $$O(n)$$ |
| `pointwise(r,a,b,n)` | 先頭 `n` 個の要素ごとの積 | $$O(n)$$ |
| `accumulate_products<count,first>(dst,a,b,n)` | `count` 組の先頭 `n` 個の要素ごとの積を加算。`first` が真なら上書き、偽なら `dst` へ加算 | $$O(n)$$ |
| `accumulate_products8<count,first>(dst,a,b,n)` | `accumulate_products` と同じ演算を AVX2（複数の整数をまとめて処理する CPU 命令）を有効にして実行 | $$O(n)$$ |
| `join_windows(blocks,length,count)` | 連続する `count` 個の変換済みブロックを接続。前半に直前のブロックを加え、後半から引く | $$O(\mathrm{length}\cdot\mathrm{count})$$ |
| `join_windows_strided(blocks,length,count,pitch)` | 要素数 `pitch` の間隔で並べたブロックを接続 | $$O(\mathrm{length}\cdot\mathrm{count})$$ |
| `inverse_extend(h,a,cache)` | キャッシュした変換を使って逆数の計算済み係数数を 2 倍に増やす | $$O(n\log n)$$ |
| `dot(a,b,n)` | $$\displaystyle\sum_{i=0}^{n-1}a_i b_{n-1-i}$$ を返す | $$O(n)$$ |
| `mod_sqrt(a)` | 法 998244353 の平方根。2 乗で表す値が存在しなければ 0 | $$O(1)$$ |
| `inverse_ntt_roots(n)` / `ntt_roots(n)` | 変換で使う定数の表を逆変換用／順変換用で取得 | 拡張: $$O(n)$$、準備済み: $$O(1)$$ |
| `inverses(n)` | 整数 0～`n` の逆元のキャッシュ。0 の項は 0 | 拡張: $$O(n)$$、準備済み: $$O(1)$$ |
| `trim(a)` | 末尾の零係数を除去 | $$O(n)$$ |

`ntt_inverse` などの未正規化の逆変換は、元の係数列の長さ倍の値を返します。`intt` は長さで割る処理も行うため、元の係数列に戻ります。

`accumulate_products` は $$1\le\mathrm{count}\le7$$ とし、`a`・`b` は各 `count` 個の配列ポインタを渡します。各配列と `dst` に `n` 要素を用意し、内部 Montgomery 表現の整数値として `a[k][i]` は $$[0,p)$$、`b[k][i]` は $$[0,2p)$$ に収めます。ここで $$p=998244353$$ です。`first = false` のときは元の `dst` も $$[0,2p)$$ にします。結果も $$[0,2p)$$ です。`n` が 8 以上なら 8 の倍数にします。`accumulate_products8` は x86 のみで定義され、`n` を 8 の倍数とし、AVX2 対応を確認してから呼び出します。

`join_windows` は `length` を 2 以上の 2 冪、`count` を 1 以上とします。各ブロックは、前半に係数を置き、後半を 0 とした長さ `length` の NTT です。全ブロックで `count * length` 要素を用意します。`join_windows_strided` では、ブロック先頭間の要素数 `pitch` を `length` 以上とし、`count * pitch` 要素を用意します。接続前の直前のブロックを参照するため、処理は後ろから行います。

係数数を $$n$$ として、変換・`inverse_extend` は $$O(n\log n)$$、その他の配列操作は $$O(n)$$ です。根・逆元のキャッシュ拡張は追加要素数に比例し、取得のみなら $$O(1)$$、`words` は $$O(1)$$ です。`mod_sqrt` は固定法に対して $$O(1)$$、追加空間・キャッシュは $$O(n)$$ です。

`accumulate_products`・`accumulate_products8` は時間 $$O(n)$$、`join_windows` は時間 $$O(\mathrm{length}\cdot\mathrm{count})$$ です。これらの追加空間は $$O(1)$$ です。
