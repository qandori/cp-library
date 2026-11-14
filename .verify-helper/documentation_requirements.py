import posixpath
import re
from pathlib import Path
from urllib.parse import unquote, urlsplit

import yaml

from affected import LIBRARIES
from size_profiles import split_page


BOUND = re.compile(r'(?<![A-Za-z_])(?:O|\\(?:Theta|Omega)|\\mathcal\{O\})\s*(?:\\(?:left|bigl|Bigl))?\s*\(|定数時間|線形時間|定数回|定数個')
SPACE = re.compile(r'空間|メモリ|記憶|領域')
NO_RUNTIME = re.compile(r'実行時[^。\n]{0,80}(?:ありません|行いません|不要|発生しません)')
REFERENCE = re.compile(r'(?:計算量|時間|空間)[^。\n]*(?:参照|同じ)')


def prose(body):
    body = re.sub(r'^```[^\n]*\n.*?^```[^\n]*(?:\n|$)', '', body, flags=re.M | re.S)
    return re.sub(r'<!--.*?-->', '', body, flags=re.S)


def cells(line):
    line = re.sub(r'(`+)(.*?)\1', lambda m: m[0].replace('|', '\0'), line)
    return [part.strip() for part in re.split(r'(?<!\\)\|', line.strip().strip('|'))]


def table_errors(body):
    columns = []
    result = []
    for line in body.splitlines():
        if not line.lstrip().startswith('|'):
            columns = []
            continue
        row = cells(line)
        if row and row[0].strip('*`') in ('操作', '主な操作', '関数', '関数・操作', '操作・関数', 'API'):
            columns = [i for i, cell in enumerate(row) if '計算量' in cell]
        elif columns and not all(re.fullmatch(r':?-+:?', cell) for cell in row):
            if any(i >= len(row) or row[i] in ('', '-', '—', 'TODO', '未記載') for i in columns):
                result.append('操作表の計算量欄が空です: ' + line.strip())
            if any(i < len(row) and re.fullmatch(r'\$\$\s*[A-Za-z]\s*\$\$', row[i]) for i in columns):
                result.append('操作表の計算量は別記号で置き換えず O(...) を直接記載してください: ' + line.strip())
    return result


def check(root):
    root = Path(root)
    headers = {p.relative_to(root).as_posix() for folder in LIBRARIES for p in (root / folder).rglob('*.hpp')}
    documents, paths, issues = {}, {}, []
    for path in sorted((root / 'docs').rglob('*.md')):
        front, body = split_page(path.read_bytes())
        reference = front.get('documentation_of')
        if not reference:
            continue
        name = path.relative_to(root).as_posix()
        if not isinstance(reference, str):
            issues.append(name + ': documentation_of にはヘッダーへのパスを指定してください')
            continue
        header = reference[2:] if reference.startswith('//') else posixpath.normpath(posixpath.join(posixpath.dirname(name), reference))
        if header not in headers:
            issues.append(name + ': 存在しないライブラリへの documentation_of: ' + header)
            continue
        if header in documents:
            issues.append(name + ': 説明ページが重複しています: ' + header)
        documents[header] = (name, prose(body.decode()))
        paths[name] = header
    issues += [header + ': 説明ページがありません' for header in sorted(headers - documents.keys())]
    config_path = root / '.verify-helper/docs/_config.yml'
    config = (yaml.safe_load(config_path.read_text()) or {}) if config_path.exists() else {}
    host = urlsplit(config.get('url', '')).netloc
    base = config.get('baseurl', '').strip('/')
    variants = config.get('implementation_variants', {})

    def resolve(header, name, link):
        url = urlsplit(link)
        if url.netloc and url.netloc != host:
            return None
        path = unquote(url.path).lstrip('/')
        if base and path.startswith(base + '/'):
            path = path[len(base) + 1:]
        light = path.startswith('lightweight/')
        if light:
            path = path[len('lightweight/'):]
        for candidate in (path, posixpath.normpath(posixpath.join(posixpath.dirname(name), path)),
                          posixpath.normpath(posixpath.join(posixpath.dirname(header), path))):
            target = paths.get(candidate)
            if candidate.endswith('.hpp.html') and candidate[:-5] in documents:
                target = candidate[:-5]
            if target:
                return variants.get(target, target) if light else target
        return None

    def references(header, name, body):
        result = []
        for paragraph in re.split(r'\n\s*\n', body):
            if not REFERENCE.search(paragraph):
                continue
            links = re.findall(r'\[[^\]\n]*\]\(([^)\s]+)\)', paragraph)
            if not links and re.search(r'(?:各|個別|対応する|それぞれの)ページ', paragraph):
                tables = '\n'.join(line for line in body.splitlines() if line.lstrip().startswith('|'))
                links = re.findall(r'\[[^\]\n]*\]\(([^)\s]+)\)', tables)
            result.extend(resolve(header, name, link) for link in links)
        return result

    checked, active = {}, set()

    def visit(header):
        if header in checked:
            return checked[header]
        if header in active:
            return False
        active.add(header)
        name, body = documents[header]
        errors = table_errors(body)
        for formula in re.findall(r'\$\$(.*?)\$\$', body, re.S):
            if re.search(r'\bO\s*\(', formula) and re.search(r'\+\s*[12]\s*(?=[)}])', formula):
                errors.append('計算量の不要な +1・+2 は省き、境界ケースを本文で説明してください: ' + formula)
        has_bound = bool(BOUND.search(body) or NO_RUNTIME.search(body))
        has_space = bool(SPACE.search(body))
        if not has_bound or not has_space:
            targets = references(header, name, body)
            inherited = bool(targets) and all(target in documents and visit(target) for target in targets)
            if not inherited:
                if not has_bound:
                    errors.append('計算量、実行時処理なし、または計算量の明示的な参照先が必要です')
                if not has_space:
                    errors.append('使用空間、追加領域不要、または空間計算量の明示的な参照先が必要です')
        active.remove(header)
        issues.extend(name + ': ' + error for error in errors)
        checked[header] = not errors
        return checked[header]

    for header in sorted(documents):
        visit(header)
    if issues:
        raise AssertionError('\n'.join(issues))
    return len(headers)


if __name__ == '__main__':
    root = Path(__file__).resolve().parents[1]
    from library_categories import check as check_categories
    categories = check_categories(root)
    print(f'Checked explicit categories for {len(categories)} libraries')
    count = check(root)
    print(f'Checked complexity descriptions for {count} libraries')
