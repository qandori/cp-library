(() => {
  const original = document.getElementById('code-original');
  const bundled = document.getElementById('code-bundled');
  const bundle = document.getElementById('code-bundle');
  const status = document.getElementById('code-status');
  document.querySelector('.code-toolbar').hidden = false;
  bundle.addEventListener('click', () => {
    document.getElementById('code-copy-fallback')?.remove();
    original.hidden = !original.hidden;
    bundled.hidden = !original.hidden;
    bundle.setAttribute('aria-pressed', String(original.hidden));
    bundle.textContent = original.hidden ? 'Unbundle' : 'Bundle';
    status.textContent = original.hidden ? '依存ヘッダーを展開しました' : '';
  });
  document.getElementById('code-copy').addEventListener('click', async () => {
    document.getElementById('code-copy-fallback')?.remove();
    const code = (original.hidden ? bundled : original).querySelector('code');
    const url = new URL(window.location.href);
    url.search = '';
    url.hash = '';
    const text = `//${url.href}\n${code.textContent}`;
    let copied = false;
    try {
      await navigator.clipboard.writeText(text);
      copied = true;
    } catch {
      const input = document.createElement('textarea');
      input.id = 'code-copy-fallback';
      input.value = text;
      input.readOnly = true;
      input.setAttribute('aria-label', 'コピー用コード');
      input.style.cssText = 'width:100%;min-height:12rem;box-sizing:border-box';
      document.querySelector('.code-toolbar').after(input);
      input.focus();
      input.select();
      try {
        copied = document.execCommand('copy');
      } catch {}
      if (copied) {
        input.remove();
        document.getElementById('code-copy').focus();
      }
    }
    status.textContent = copied ? 'コピーしました' : '選択済みのコードをコピーしてください';
  });
})();
