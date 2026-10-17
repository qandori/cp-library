# XOR 基底

2026-10-17 に Linux x86_64 / GCC 15.2 で実装・測定した。[公開 API](../../docs/xor_basis.md) に全関数をまとめた。

## 参照した AC 提出

Library Checker の `intersection_of_f2_vector_spaces` について、公式 API の `status=AC&order=%2Btime&limit=3` で上位を取得した。取得日時・一覧・ソース SHA256 は [xor_basis_reference.json](results/xor_basis_reference.json) に保存した。ソース全文は一時キャッシュに保存し、リポジトリには含めていない。

| 提出 | ユーザー | judge 表示時間 |
| --- | --- | ---: |
| [407091](https://judge.yosupo.jp/submission/407091) | Anonymous | 152 ms |
| [406229](https://judge.yosupo.jp/submission/406229) | Rohan_Kapri | 230 ms |
| [206339](https://judge.yosupo.jp/submission/206339) | maspy | 240 ms |

407091 は値と復元情報を 64 bit に詰め、非零行を連続領域に追加しながら `min(x,x^row)` で消去する。406229・206339 も拡大空間を使う消去で、汎用のベクトル空間の最小最大・結合・正規化などの実装も確認した。

## 採用した構成

- 固定配列の先頭に独立な基底だけを詰め、挿入・所属・最小最大を次元に比例する処理にした。通常の操作は領域確保を行わない。
- 各最上位ビットに対応する格納位置も保持する。順序統計を呼ぶたびに簡約基底へ作り直さず、上位ビットから直接処理する。
- 全次元を張る場合は下位ビットが全て自由になるため、追加・所属・最小最大・kth・個数クエリを定数時間で処理する。
- 32 bit 以下の共通部分は、上位提出と同じ 64 bit の拡大空間による方式を採用した。それより広い場合は値と復元情報を別々の整数に保持する。
- 挿入時に既存の行を書き換えないため、ロールバックは追加した格納位置だけを履歴に残す。従属な挿入も 1 回として巻き戻せる。
- 復元付き基底は選んだ元の入力を高々64個保持する。prefix 用は新しい添字を優先してピボットを入れ替え、左端による絞り込みができる。
- 個数と添字の境界は `__uint128_t` を使い、64次元の `2^64` 個と上端を表せるようにした。演算は通常の整数処理で、x86 専用命令は要求しない。

## 通常演算の測定

Intel Core i7-13700F / WSL2 Linux x86_64 / GCC 15.2、CPU 2 固定、seed 713。`-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。native では `-march=native -ftrivial-auto-var-init=zero` を加えた。各7回、実行順を固定 seed でシャッフルした中央値。入力生成と出力は計測外。

`build` は同じ96個の値から基底を作り直す処理を1万回行う。`query` は構築済み基底に対して20万個の値の最小・最大 XOR を両方求める。各回、3実装の checksum を照合した。`scanning` は全ビットを走査する三角基底、`compact` は非零行だけを詰める最小構成。いずれも比較用コードを [xor_basis.cpp](xor_basis.cpp) に含めた。

単位は ms。

| 処理 | 次元 | scanning | compact | 採用版 | native 採用版 |
| --- | ---: | ---: | ---: | ---: | ---: |
| build | 8 | 20.622 | 2.660 | 2.979 | 2.941 |
| build | 32 | 18.815 | 11.406 | 12.086 | 12.231 |
| build | 63 | 19.311 | 22.031 | 22.687 | 22.418 |
| build | 64 | 19.223 | 22.142 | 11.371 | 11.133 |
| query | 8 | 13.089 | 0.887 | 0.894 | 0.890 |
| query | 32 | 13.014 | 5.214 | 5.279 | 5.292 |
| query | 63 | 12.975 | 12.787 | 13.439 | 12.778 |
| query | 64 | 13.038 | 13.013 | 0.020 | 0.020 |

低次元では全ビット走査を省く効果があった。一方、63次元の構築など走査版の方が速い条件もある。64次元の最小・最大値は入力によらない定数となり、コンパイラによるループの簡略化も含むため、この値を一般のクエリの速度とは扱わない。

全標本・環境・ソース SHA256 は [generic](results/xor_basis_generic/)・[native](results/xor_basis_native/) に保存した。初期の最上位ビットによる挿入と全ビットのクエリは [exploration](results/xor_basis_exploration/)、詰めた基底は [compact_exploration](results/xor_basis_compact_exploration/)、共通部分の比較は [indexed_intersection](results/xor_basis_indexed_intersection/)・[packed_intersection](results/xor_basis_packed_intersection/) にある。探索は各3回で、最終測定とはソースが異なる。

## 最速提出とのローカル比較

`815bc8b` の整数入力などの更新を取り込み、併用テスト・公式問題を再検証した。下表は新しい入力処理で測り直した値。入力処理の更新前の測定も [judge_generic_before_fast_io](results/xor_basis_judge_generic_before_fast_io/)・[judge_native_before_fast_io](results/xor_basis_judge_native_before_fast_io/) に保存した。

407091 の原文を同じコンパイルコマンドでビルドした。提出内の `O3,unroll-loops` pragma と入出力は変更していない。プログラム起動・入出力・通常基底の構築を含む経過時間であり、上の処理本体だけの値とは別の測定。

1入力10万ケース、各7回の中央値。空・全次元・29次元・ランダム次元・低次元・同じ空間の6種類を混ぜた。独立な共通の座標系から二つの空間を作り、それぞれ基底変換した。全ての出力について、次元と簡約した基底を既知の共通部分に照合した。

| コンパイル条件 | 採用版 | 407091 の原文 |
| --- | ---: | ---: |
| generic | 82.248 ms | 46.978 ms |
| native | 77.418 ms | 46.251 ms |

参照提出は入力が線形独立なことを利用し、入力を直接拡大空間へ流し込む。公開 API では二つの入力空間を通常の基底として構築し、共通部分も通常の基底として返すため、その構築・消去も含む。入出力の実装も異なる。この結果から judge 上での最速や同じ速度比は断定しない。

[judge_generic](results/xor_basis_judge_generic/)・[judge_native](results/xor_basis_judge_native/) に全標本・入力と期待する空間の SHA256 を保存した。

## 検証

- 4 bit の全67空間と4489組の共通部分・和空間、直交補空間を検査。
- 小さい入力は部分集合 XOR を全列挙し、最小最大・順序統計・係数復元・個数・境界検索を照合。
- 8・16・32・64 bit の全幅、最高ビット、`2^64` 個とその上端、0・重複・空の区間を検査。
- 32・64 bit の共通部分は独立な座標系から作った既知の答えとも照合。
- 元の添字による復元を代入し、ロールバック・従属な挿入の undo・prefix の各左端を素朴な再構築と比較。
- GCC 15.2 / Clang 21.1.8、Clang ASan / UBSan、GCC 13.4、ARM64 GCC の静的バイナリを QEMU 上で実行して成功。
- Library Checker の全17ケースを GCC / Clang の両方で実行し、合計34ケースが成功。
- 新規ヘッダーの単独コンパイル・展開コードのコンパイル・他ライブラリと併用する standalone テストも成功。Apple Clang は macOS portability CI の対象にした。

[unit](results/xor_basis_unit_check.log)・[sanitize](results/xor_basis_sanitize_check.log)・[judge](results/xor_basis_judge_check.log)・[validation](results/xor_basis_validation.json) に記録した。変更箇所と依存先のテストのみ実行した。一時ブランチは作成していない。

## 再現

Linux のリポジトリルートで実行する。

```sh
python3 test/benchmark/xor_basis_reference.py --cache /tmp/xor-basis-references --output /tmp/xor-basis-reference.json
python3 test/benchmark/xor_basis.py --output /tmp/xor-basis-generic --trials 7
python3 test/benchmark/xor_basis.py --output /tmp/xor-basis-native --trials 7 --native
python3 test/benchmark/xor_basis_judge.py --cache /tmp/xor-basis-references --references /tmp/xor-basis-reference.json --output /tmp/xor-basis-judge --trials 7 --cases 100000
python3 test/benchmark/xor_basis_judge.py --cache /tmp/xor-basis-references --references /tmp/xor-basis-reference.json --output /tmp/xor-basis-judge-native --trials 7 --cases 100000 --native
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/xor_basis_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/xor_basis_selection.json
oj-verify run --timeout 1800 --tle 10 -j 2 test/xor_basis_intersection.test.cpp
```
