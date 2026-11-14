from pathlib import Path
import tempfile
import unittest

from documentation_requirements import check


class DocumentationRequirements(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        (self.root / 'math').mkdir()
        (self.root / 'docs').mkdir()

    def document(self, name, body):
        (self.root / 'math' / (name + '.hpp')).write_text('')
        (self.root / 'docs' / (name + '.md')).write_text(
            '---\ndocumentation_of: //math/' + name + '.hpp\n---\n' + body)

    def test_explicit_time_and_space(self):
        self.document('a', '構築は $$O(n)$$ 時間、使用空間は $$O(n)$$ です。')
        self.assertEqual(check(self.root), 1)

    def test_complexity_table_cannot_defer_to_a_symbol(self):
        self.document('a', '| 操作・型 | 時間計算量 | 使用空間 |\n| --- | --- | --- |\n'
                      '| solve() | $$T$$ | $$O(n)$$ |\n\nT は末尾で定義します。$$T=O(n)$$ です。')
        with self.assertRaisesRegex(AssertionError, '別記号'):
            check(self.root)

    def test_complexity_table_with_explicit_bound(self):
        self.document('a', 'n は頂点数です。\n\n| 操作 | 時間計算量 | 使用空間 |\n'
                      '| --- | --- | --- |\n| solve() | $$O(n^2)$$ | $$O(n)$$ |')
        self.assertEqual(check(self.root), 1)

    def test_asymptotic_padding_is_rejected(self):
        for bound in ('O(n+1)', r'O(n\log(n+2))'):
            self.document('a', f'時間は $${bound}$$、使用空間は $$O(n)$$ です。')
            with self.assertRaisesRegex(AssertionError, '不要な'):
                check(self.root)

    def test_exact_counts_and_nonconstant_terms_are_preserved(self):
        self.document('a', '補助頂点を含む個数は $$n+2$$ です。時間は $$O(n+2^k)$$、使用空間は $$O(n)$$ です。')
        self.assertEqual(check(self.root), 1)

    def test_example_and_comment_are_not_explanations(self):
        self.document('a', '```cpp\n// O(n) 時間・空間\n```\n<!-- O(n) 空間 -->')
        with self.assertRaisesRegex(AssertionError, '計算量'):
            check(self.root)

    def test_space_is_required(self):
        self.document('a', '各操作は $$O(n)$$ 時間です。')
        with self.assertRaisesRegex(AssertionError, '使用空間'):
            check(self.root)

    def test_include_only_page_can_refer_to_library(self):
        self.document('a', '計算量は [本体](b.md) を参照してください。')
        self.document('b', '構築は $$O(n)$$ 時間・空間です。')
        self.assertEqual(check(self.root), 2)

    def test_cyclic_reference_is_not_a_complexity(self):
        self.document('a', '計算量は [B](b.md) を参照。')
        self.document('b', '計算量は [A](a.md) を参照。')
        with self.assertRaisesRegex(AssertionError, '計算量'):
            check(self.root)

    def test_unrelated_link_cannot_replace_broken_reference(self):
        self.document('a', '計算量・使用空間は [本体](missing.md) を参照。\n\n関連: [別](b.md)')
        self.document('b', 'O(n) 時間・空間です。')
        with self.assertRaisesRegex(AssertionError, '計算量'):
            check(self.root)

    def test_umbrella_reference_uses_listed_library_pages(self):
        self.document('a', '| ヘッダー | 内容 |\n| --- | --- |\n| [本体](b.md) | 全操作 |\n\n計算量と空間は各ページを参照。')
        self.document('b', 'O(n) 時間・空間です。')
        self.assertEqual(check(self.root), 2)

    def test_lightweight_reference_uses_implementation_variant(self):
        folder = self.root / '.verify-helper/docs'
        folder.mkdir(parents=True)
        (folder / '_config.yml').write_text('url: https://example.test\nbaseurl: /repo\nimplementation_variants:\n  math/a.hpp: math/a_light.hpp\n')
        self.document('a', 'O(1) 時間・空間です。')
        self.document('a_light', '計算量・空間は [軽量版](https://example.test/repo/lightweight/math/a.hpp.html) を参照。')
        with self.assertRaisesRegex(AssertionError, '計算量'):
            check(self.root)

    def test_no_runtime_processing(self):
        self.document('a', '定数の定義だけで、実行時の処理はありません。追加の記憶領域は不要です。')
        self.assertEqual(check(self.root), 1)

    def test_arbitrary_precision_operation_count(self):
        self.document('a', '有理数演算を定数回行い、定数個の値を保持します。時間と保存空間は値のビット長によります。')
        self.assertEqual(check(self.root), 1)

    def test_empty_operation_cell_is_rejected(self):
        self.document('a', '| 操作 | 計算量 |\n| --- | --- |\n| `a()` | |\n\n全体は O(n) 時間・空間。')
        with self.assertRaisesRegex(AssertionError, '計算量欄'):
            check(self.root)

    def test_pipes_inside_code_are_not_columns(self):
        self.document('a', '| 操作 | 計算量 |\n| --- | --- |\n| `a | b` | O(n) |\n\n使用空間は O(n)。')
        self.assertEqual(check(self.root), 1)

    def test_operation_word_in_body_does_not_reset_table(self):
        self.document('a', '| 操作 | 内容 | 時間計算量 |\n| --- | --- | --- |\n| `a()` | 操作回数を返す | O(1) |\n| `b()` | 取得 | |\n\n空間は O(1)。')
        with self.assertRaisesRegex(AssertionError, '計算量欄'):
            check(self.root)

    def test_header_without_a_page_is_rejected(self):
        (self.root / 'math/a.hpp').write_text('')
        with self.assertRaisesRegex(AssertionError, '説明ページがありません'):
            check(self.root)


if __name__ == '__main__':
    unittest.main()
