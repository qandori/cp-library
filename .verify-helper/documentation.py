from pathlib import Path


def source_paths(plan, excluded):
    paths = set(plan["state"]["targets"]["headers"]) | set(plan["state"]["targets"]["verification"])
    return sorted(path for path in map(Path, paths) if not any(
        path == exclude or exclude in path.parents for exclude in excluded
    ))


def generate(plan):
    from onlinejudge_verify.documentation import build, configure, main
    from onlinejudge_verify.documentation.type import PageRenderJob
    from verification import get_marker

    root = Path.cwd().resolve()
    config = main.load_render_config(basedir=root)
    excluded = [Path(path) for path in config.config_yml.get("exclude", [])]
    paths = source_paths(plan, excluded)
    verification = get_marker()
    depends_on, required_by, verified_with = configure._build_dependency_graph(paths, basedir=root)
    status = configure._build_verification_status(paths, verified_with=verified_with, basedir=root, marker=verification)
    stats = [configure._get_source_code_stat(
        path, depends_on=depends_on, required_by=required_by, verified_with=verified_with,
        verification_status=status, marker=verification, basedir=root,
    ) for path in paths]
    markdown = configure.apply_exclude_list_to_paths(configure.find_markdown_paths(basedir=root), excluded_paths=excluded)
    jobs = configure.convert_to_page_render_jobs(source_code_stats=stats, markdown_paths=markdown, site_render_config=config)
    jobs.append(PageRenderJob(
        path=Path("components/index.md"),
        front_matter={"layout": "toppage", "title": "補助部品", "components": True},
        content=config.index_md.with_name("components.md").read_bytes(),
    ))
    pages = build.render_pages(page_render_jobs=jobs, source_code_stats=stats, site_render_config=config)
    pages.update(build.load_static_files(site_render_config=config))
    for path, content in pages.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
    print(f"Generated documentation for {len(paths)} library and judge sources", flush=True)
