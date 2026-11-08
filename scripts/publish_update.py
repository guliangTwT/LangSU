#!/usr/bin/env python3
"""Build the self-hosted update channel layout into `dist/`.

Reads VERSION.txt and the matching CHANGELOG.md section, hashes every APK in
`dist/`, and writes:

    dist/latest.json                 manifest read by the manager
    dist/versions/<version>.json     snapshot of this version
    dist/notes/<version>.md          changelog for this version
    dist/index.html                  download page

Uploading is a separate step: copy the tree to the web host's /langsu/updates/.
"""

from __future__ import annotations

import hashlib
import html
import json
import shutil
import sys
from datetime import datetime, timezone
from pathlib import Path
from string import Template

ROOT = Path(__file__).resolve().parent.parent
DIST = ROOT / "dist"
BASE_URL = "https://guliang.me/langsu/updates"
APP = "langsu"


def read_version() -> str:
    version = (ROOT / "VERSION.txt").read_text(encoding="utf-8").strip()
    if not version:
        raise SystemExit("VERSION.txt is empty")
    return version


def read_notes(version: str) -> str:
    """Return the CHANGELOG.md section body for `version`, without the heading."""
    lines = (ROOT / "CHANGELOG.md").read_text(encoding="utf-8").splitlines()
    body: list[str] = []
    inside = False
    for line in lines:
        if line.startswith("## "):
            if inside:
                break
            inside = line[3:].strip().startswith(version)
            continue
        if inside:
            body.append(line)
    text = "\n".join(body).strip()
    return text or f"LangSU {version}"


def sha256_of(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def collect_assets(version: str) -> list[dict]:
    apks = sorted(p for p in DIST.glob("*.apk") if p.is_file())
    if not apks:
        raise SystemExit("no APK in dist/ - build and repack first")
    assets = []
    for apk in apks:
        assets.append(
            {
                "name": apk.name,
                "size": apk.stat().st_size,
                "sha256": sha256_of(apk),
                "urls": [f"{BASE_URL}/files/{version}/{apk.name}"],
            }
        )
    return assets


PAGE = Template("""<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>LangSU $version 下载</title>
<style>
  :root { color-scheme: light dark; }
  * { box-sizing: border-box; }
  body {
    margin: 0; padding: 32px 20px 48px;
    font: 16px/1.7 -apple-system, "Segoe UI", "PingFang SC", "Microsoft YaHei", sans-serif;
    background: radial-gradient(1200px 600px at 50% -10%, #ffe4f0 0%, #fff8fb 45%, #ffffff 100%);
    color: #3a2b33;
  }
  main { max-width: 720px; margin: 0 auto; }
  h1 { font-size: 30px; margin: 0 0 4px; color: #c2185b; letter-spacing: .5px; }
  .sub { color: #8a6b78; margin: 0 0 24px; font-size: 15px; }
  .card {
    background: #fff; border: 1px solid #ffd7e6; border-radius: 18px;
    padding: 20px 22px; margin: 0 0 18px;
    box-shadow: 0 10px 30px rgba(194, 24, 91, .08);
  }
  .card h2 { font-size: 17px; margin: 0 0 12px; color: #a31550; }
  .ver { font-size: 26px; font-weight: 700; color: #d81b60; }
  a.dl {
    display: inline-block; margin: 6px 0 14px; padding: 13px 26px;
    background: #ec407a; color: #fff; border-radius: 999px;
    text-decoration: none; font-weight: 600;
  }
  a.dl:hover { background: #d81b60; }
  code {
    display: block; word-break: break-all; padding: 10px 12px; border-radius: 10px;
    background: #fff1f6; border: 1px solid #ffd7e6; font-size: 13px; color: #6d3348;
  }
  ol { padding-left: 22px; margin: 0; }
  li { margin: 6px 0; }
  .muted { color: #8a6b78; font-size: 14px; }
  .links a { color: #c2185b; }
</style>
</head>
<body>
<main>
  <h1>LangSU</h1>
  <p class="sub">KernelSU 3.3.1 的非官方分支：换名、换签名、带隐藏入口。</p>

  <div class="card">
    <h2>最新版本</h2>
    <div class="ver">$version</div>
    <p class="muted">发布于 $published</p>
    $downloads
  </div>

  <div class="card">
    <h2>校验</h2>
    <p class="muted">下载后可以核对 SHA-256：</p>
    $hashes
  </div>

  <div class="card">
    <h2>安装</h2>
    <ol>
      <li>先装这个管理器 APK。</li>
      <li>打开管理器，允许它拿到 root 权限。</li>
      <li>在管理器里安装内核模块，重启生效。模块会按设备的 KMI 自动挑，8 个版本已内置在 APK 里。</li>
      <li>只有一个内核能被接管；已经装了 KernelSU 的设备需要先卸载它。</li>
    </ol>
  </div>

  <div class="card links">
    <h2>链接</h2>
    <p>源码：<a href="https://codeberg.org/GuLiang/LangSU">codeberg.org/GuLiang/LangSU</a></p>
    <p>更新日志：<a href="$notes_url">notes/$version.md</a></p>
    <p class="muted">基于 KernelSU（GPL-2.0）修改，与上游作者没有隶属关系。</p>
  </div>
</main>
</body>
</html>
""")


def main() -> int:
    version = read_version()
    notes = read_notes(version)
    assets = collect_assets(version)
    published = datetime.now(timezone.utc).strftime("%Y-%m-%d")

    DIST.mkdir(parents=True, exist_ok=True)
    (DIST / "notes").mkdir(exist_ok=True)
    (DIST / "versions").mkdir(exist_ok=True)
    files_dir = DIST / "files" / version
    files_dir.mkdir(parents=True, exist_ok=True)
    for asset in assets:
        source = DIST / asset["name"]
        target = files_dir / asset["name"]
        if not target.exists() or target.stat().st_size != source.stat().st_size:
            shutil.copy2(source, target)

    manifest = {
        "schema": 1,
        "app": APP,
        "version": version,
        "published": published,
        "notes": notes,
        "assets": assets,
    }
    text = json.dumps(manifest, ensure_ascii=False, indent=2) + "\n"
    (DIST / "latest.json").write_text(text, encoding="utf-8")
    (DIST / "versions" / f"{version}.json").write_text(text, encoding="utf-8")
    (DIST / "notes" / f"{version}.md").write_text(notes + "\n", encoding="utf-8")

    downloads = "\n    ".join(
        f'<a class="dl" href="files/{version}/{html.escape(a["name"])}">'
        f'下载 {html.escape(a["name"])} · {a["size"] / 1048576:.1f} MB</a><br>'
        for a in assets
    )
    hashes = "\n    ".join(
        f'<code>{html.escape(a["name"])}<br>{a["sha256"]}</code>' for a in assets
    )
    page = PAGE.substitute(
        version=html.escape(version),
        published=html.escape(published),
        downloads=downloads,
        hashes=hashes,
        notes_url=f"notes/{version}.md",
    )
    (DIST / "index.html").write_text(page, encoding="utf-8")

    print(f"version  : {version}")
    for a in assets:
        print(f"asset    : {a['name']}  {a['size']} bytes  {a['sha256']}")
    print(f"manifest : {DIST / 'latest.json'}")
    print(f"page     : {DIST / 'index.html'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())