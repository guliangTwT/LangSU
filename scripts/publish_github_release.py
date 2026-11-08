#!/usr/bin/env python3
"""Create the GitHub release for the current version and upload the APK.

Uses the token that gh already stored, talks to api.github.com / uploads.github.com
directly, and retries the upload on transient DNS or connection errors (the upload host
occasionally fails to resolve on this machine).

Releases always start as drafts: the tag only becomes public once the asset is there.
"""

from __future__ import annotations

import argparse
import json
import mimetypes
import os
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DIST = ROOT / "dist"
OWNER = "guliangTwT"
REPO = "LangSU"
API = f"https://api.github.com/repos/{OWNER}/{REPO}"

# The dead system proxy (127.0.0.1:7890) is picked up from the registry by urllib's default
# opener and turns every request into a connection-refused error, so build one that never
# proxies.
OPENER = urllib.request.build_opener(urllib.request.ProxyHandler({}))


def read_token() -> str:
    """Read the token gh already stored, without a proxy (direct API works here)."""
    env = dict(os.environ)
    for key in ("HTTPS_PROXY", "HTTP_PROXY", "https_proxy", "http_proxy", "ALL_PROXY", "all_proxy"):
        env.pop(key, None)
    result = subprocess.run(
        [r"C:\Program Files\GitHub CLI\gh.exe", "auth", "token"],
        capture_output=True, text=True, env=env,
    )
    token = result.stdout.strip()
    if not token:
        raise SystemExit("gh auth token returned nothing: " + result.stderr.strip())
    return token


def request(method: str, url: str, token: str, payload: dict | None = None) -> dict:
    data = json.dumps(payload).encode() if payload is not None else None
    headers = {
        "Authorization": f"Bearer {token}",
        "Accept": "application/vnd.github+json",
        "User-Agent": "langsu-release",
    }
    if data is not None:
        headers["Content-Type"] = "application/json"
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    with OPENER.open(req, timeout=120) as response:
        body = response.read()
    return json.loads(body) if body else {}


def upload_asset(asset: Path, release: dict, token: str, attempts: int = 6) -> dict:
    """POST the binary to uploads.github.com, retrying transient DNS/connection failures."""
    name = asset.name
    url = f"{release['upload_url'].split('{')[0]}?name={urllib.parse.quote(name)}"
    content_type = mimetypes.guess_type(name)[0] or "application/octet-stream"
    data = asset.read_bytes()

    last_error: Exception | None = None
    for attempt in range(1, attempts + 1):
        req = urllib.request.Request(
            url,
            data=data,
            headers={
                "Authorization": f"Bearer {token}",
                "Accept": "application/vnd.github+json",
                "Content-Type": content_type,
                "Content-Length": str(len(data)),
                "User-Agent": "langsu-release",
            },
            method="POST",
        )
        try:
            with OPENER.open(req, timeout=600) as response:
                return json.loads(response.read())
        except (urllib.error.URLError, OSError) as error:
            last_error = error
            print(f"  attempt {attempt}/{attempts} failed: {error}")
            time.sleep(5 * attempt)
    raise SystemExit(f"upload failed after {attempts} attempts: {last_error}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    version = (ROOT / "VERSION.txt").read_text(encoding="utf-8").strip()
    tag = f"v{version}"
    notes = (DIST / "notes" / f"{version}.md").read_text(encoding="utf-8").strip()
    apk = DIST / f"langsu-{version}-android-universal.apk"
    if not apk.is_file():
        raise SystemExit(f"missing {apk}")

    print(f"version: {version}  tag: {tag}")
    print(f"asset  : {apk.name}  {apk.stat().st_size} bytes")
    if args.dry_run:
        return 0

    token = read_token()

    existing = None
    try:
        existing = request("GET", f"{API}/releases/tags/{tag}", token)
    except urllib.error.HTTPError as error:
        if error.code != 404:
            raise

    if existing:
        release = existing
        print(f"reusing release {release['id']} (draft={release['draft']})")
    else:
        release = request(
            "POST",
            f"{API}/releases",
            token,
            {
                "tag_name": tag,
                "name": f"LangSU {version}",
                "body": notes,
                "draft": True,
                "prerelease": False,
            },
        )
        print(f"created draft release {release['id']}")

    have = {a["name"] for a in release.get("assets") or []}
    if apk.name in have:
        print("asset already attached")
    else:
        asset = upload_asset(apk, release, token)
        print(f"uploaded {asset['name']} ({asset['size']} bytes)")

    if release["draft"]:
        request("PATCH", f"{API}/releases/{release['id']}", token, {"draft": False})
        print("published (draft=false)")

    print(f"done: https://github.com/{OWNER}/{REPO}/releases/tag/{tag}")
    return 0


if __name__ == "__main__":
    sys.exit(main())