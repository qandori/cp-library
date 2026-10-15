(() => {
  const original = document.getElementById('code-original');
  const bundled = document.getElementById('code-bundled');
  const bundle = document.getElementById('code-bundle');
  const status = document.getElementById('code-status');
  document.querySelector('.code-toolbar').hidden = false;
  bundle.addEventListener('click', () => {
    original.hidden = !original.hidden;
    bundled.hidden = !original.hidden;
    bundle.setAttribute('aria-pressed', String(original.hidden));
    bundle.textContent = original.hidden ? 'Unbundle' : 'Bundle';
    status.textContent = original.hidden ? '依存ヘッダーを展開しました' : '';
  });
  document.getElementById('code-copy').addEventListener('click', async () => {
    const code = (original.hidden ? bundled : original).querySelector('code');
    let copied = false;
    try {
      await navigator.clipboard.writeText(code.textContent);
      copied = true;
    } catch {
      const selection = window.getSelection();
      const range = document.createRange();
      range.selectNodeContents(code);
      selection.removeAllRanges();
      selection.addRange(range);
      try {
        copied = document.execCommand('copy');
      } catch {}
      if (copied) selection.removeAllRanges();
    }
    status.textContent = copied ? 'コピーしました' : 'コードを選択してコピーしてください';
  });
})();
