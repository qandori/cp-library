# FastInput 整数入力

## 方針

変更前は `token()` が区切りまで走査し、`std::from_chars` が同じ数字をもう一度走査する。整数型だけ現在の読み込み範囲へ直接 `from_chars` を呼び、変換成功かつ直後が空白、または mmap / string_view の終端なら結果を採用する。fread バッファの終端・不正トークン・overflow は従来の `token()` と変換へ戻す。`bool`・文字列・対応型・`FastOutput` は変更しない。

先行変換は一時変数へ行う。バッファ末尾の数字列が一旦有効でも、次のバッファを含めると overflow になる場合があるため、呼び出し元の値を先に書き換えない。`NDEBUG` 時の不正トークン全体の消費・有効な接頭辞だけの変換・失敗時の値保持も従来通りにする。

## 比較対象

2026-10-17 に Library Checker API の `many_aplusb`、`status=AC`、`order=+time`、`limit=8` を取得した。

- [#409103](https://judge.yosupo.jp/submission/409103): Aiyiyi、18 ms、取得時最速 AC。AVX2 とインラインアセンブリで数字列のまま加算し、整数への変換を省く問題特化実装。64 byte 前後の padding と Linux x86-64 を前提とする。任意の整数を返す既存 API の代替ではない。
- [#402646](https://judge.yosupo.jp/submission/402646): nandhagk、23 ms、取得時 4 番目の AC。unsigned 64-bit の既知の桁数・値域・padding を前提に、SWAR / AVX2 で 8 桁単位や 2 個組を変換する。区切り文字の制約、読み越し可能な範囲、overflow を検査しない前提が既存 API と異なる。
- [#193613](https://judge.yosupo.jp/submission/193613): AC、55 ms。128 KiB の fread バッファ、32 byte の余裕を確認してから十進数を 1 文字ずつ蓄積する。二重走査を避ける方針は共通だが、EOF・overflow・不正トークンの扱いをそのまま流用しない。
- [#156493](https://judge.yosupo.jp/submission/156493): 過去の記事で最速と紹介された実装だが、今回取得した最新状態は CE。現在の fastest AC として扱わない。

これらの judge 時間は入出力と加算を含む提出全体の結果であり、本ライブラリの測定時間と直接比較しない。第三者のコードはヘッダーへコピーしない。今回の候補は標準 C++ の範囲で二重走査を除去し、SIMD 専用 API や padding 条件を追加しない。

取得の再現:

```sh
python3 test/benchmark/fast_io_integer_reference.py \
  --cache /tmp/fast-io-integer-reference \
  --output /tmp/fast-io-integer-reference.json
```

## ローカル測定

初回の generic / native 測定では大半のケースで改善したが、ビルド間で逆転する低下もあった。対象を固定した長い標本の再測定では全て改善し、今回の候補を採用した。wall time、paired speedup、process CPU time を区別して記録する。

```sh
g++-14 -std=c++20 -O2 -DNDEBUG -Wall -Wextra -Werror -I . \
  test/benchmark/fast_io_integer.cpp -o /tmp/fast-io-integer-generic
taskset -c 2 /tmp/fast-io-integer-generic 1000000 11

g++-14 -std=c++20 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero \
  -Wall -Wextra -Werror -I . test/benchmark/fast_io_integer.cpp \
  -o /tmp/fast-io-integer-native
taskset -c 2 /tmp/fast-io-integer-native 1000000 11
```

固定 seed `20261017`。各標本で旧版・新版の実行順を交互にし、中央値を比較する。各実行の全要素を期待値と比較し、一致確認は計測外。入力生成・mmap 準備・出力配列確保は計測外。pipe は writer thread と fread を含むため、カーネルだけでなく pipe 転送コストも含む。memory / mmap / pipe で signed64・unsigned64・unsigned32・小さい符号付き整数・1 桁・長い先頭ゼロ列を比較する。

## 検証

`test/unit/fast_io_integer.cpp` は `assert` に依存しない検査を使い、debug / `NDEBUG` の両方で実行できる。標準整数型の最小値・最大値、空白 byte 0..32、EOF、mmap の非ゼロ開始位置、64 KiB 境界、複数バッファをまたぐ先頭ゼロ、bool・対応型との混在を検査する。`NDEBUG` では不正符号・非数字・非 ASCII byte・overflow の出力列を旧版と比較する。Unix / Mac の debug では不正入力が従来通り assert することも別プロセスで確認する。

2026-10-17 の GCC 14.2.0 では、新規テストの C++20 debug / `NDEBUG`、ASan + UBSan の両モード、既存 `test/unit/fast_io.cpp`（BigInt の依存に合わせ C++23）、ヘッダー単独 C++20 検査が成功した。この実行環境は ptrace 下で LeakSanitizer をサポートしないため、sanitizer 実行時のみ `ASAN_OPTIONS=detect_leaks=0` とした。Mac 実機では未実行であり、既存の macOS portability CI で確認する。

初回測定は Linux x86_64、Intel Xeon Platinum 8573C、GCC 14.2.0、CPU 2 固定、他のローカル測定・コンパイルを停止し、上記の各 11 標本で実施した。generic の unsigned64 / memory は 0.89 倍、leading_zeros / memory は 0.80 倍、native の unsigned32 / memory・mmap は 0.91・0.81 倍だった。一方で対応する別ビルドでは逆に改善し、同種入力の絶対時間にも大きな変動があった。これらを都合よく除外せず、対象を固定した追加測定を下記に残す。

引数 3 にケース名、引数 4 に入力種別（0: memory、1: mmap、2: pipe、-1: 全て）を指定すると再測定対象を絞れる。例: `/tmp/fast-io-integer-native 1000000 31 unsigned32 1`。CPU time は writer thread を含むプロセス全体の消費時間であり、wall time とは区別する。

### 追加測定

同じ CPU・コンパイラ・フラグで CPU 2 に固定し、他の測定・コンパイルを停止。各 15 組の実行順を交互にし、1 標本を約 100 ms 以上へ増やした。実装自体は初回から変更せず、計測に process CPU time と paired ratio を追加した。全標本で全要素が期待値と一致した。

| build | 型 / 入力 | 個数 | 旧 wall ms | 新 wall ms | wall 比 | paired 比 | CPU 比 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| generic | unsigned64 / memory | 2,000,000 | 122.874 | 108.859 | 1.129 | 1.106 | 1.128 |
| generic | leading_zeros / memory | 1,000,001 | 155.153 | 145.597 | 1.066 | 1.162 | 1.066 |
| native | unsigned32 / memory | 4,000,000 | 204.868 | 193.146 | 1.061 | 1.213 | 1.065 |
| native | unsigned32 / mmap | 4,000,000 | 251.101 | 205.407 | 1.222 | 1.293 | 1.222 |

wall 比は旧版中央値 / 新版中央値、paired 比は各組の旧版 / 新版の中央値。CPU 比は process CPU time の中央値比であり、同じ値の別名ではない。短い標本に出た低下はこの再測定では再現せず、string_view の経路も残した。初回のその他の通常整数ケースはおおむね 1.03〜1.40 倍、native の unsigned64 / pipe と unsigned32 / pipe はそれぞれ 0.984・0.983 倍で、この程度の差を改善とは扱わない。すべての入力・実機で同じ比率を保証するものではなく、AtCoder 実機・Mac の速度は未測定。

```sh
taskset -c 2 /tmp/fast-io-integer-generic 2000000 15 unsigned64 0
taskset -c 2 /tmp/fast-io-integer-generic 8000000 15 leading_zeros 0
taskset -c 2 /tmp/fast-io-integer-native 4000000 15 unsigned32 0
taskset -c 2 /tmp/fast-io-integer-native 4000000 15 unsigned32 1
```

`leading_zeros` は負荷を揃えるため、指定個数 / 8 + 1 個の 65 桁トークンを作る。上記の `8000000` は実際には 1,000,001 個になる。

統合確認は main `698265c5` 上で依存先を選択した。選択されたヘッダーと 2 つの unit test の再帰的 include を含む 9 ファイルは、上記で実行済みのファイルと byte 単位で一致するため同一テストを再実行していない。影響する 17 個の judge driver は CI の対象であり、ローカルでオンラインジャッジ検証を実行したという意味ではない。
