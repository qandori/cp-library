(() => {
  const input = document.getElementById('library-search');
  const normalize = text => text.normalize('NFKC').toLowerCase().replace(/[\s_-]+/g, '');
  const entries = Array.from(document.querySelectorAll('.library-entry'), element => ({
    element,
    text: normalize(element.dataset.search),
  }));
  const categories = document.querySelectorAll('.library-category');
  const update = () => {
    const words = input.value.trim().split(/\s+/).map(normalize).filter(Boolean);
    let count = 0;
    for (const entry of entries) {
      entry.element.hidden = !words.every(word => entry.text.includes(word));
      count += !entry.element.hidden;
    }
    for (const category of categories) {
      category.hidden = !category.querySelector('.library-entry:not([hidden])');
    }
    document.getElementById('search-status').textContent = `${count} / ${entries.length} 件`;
    document.getElementById('search-empty').hidden = count !== 0;
    const url = new URL(location.href);
    if (input.value) url.searchParams.set('q', input.value);
    else url.searchParams.delete('q');
    history.replaceState(null, '', url);
  };
  input.value = new URL(location.href).searchParams.get('q') || '';
  input.addEventListener('input', update);
  update();
})();
