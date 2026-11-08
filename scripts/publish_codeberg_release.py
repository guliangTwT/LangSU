#!/usr/bin/env python3
"""Create or update the Codeberg release for the current version and upload assets.

Reads the token from the Windows credential vault entry written by git
(`git:https://codeberg.org`), so no secret is stored in the repository.

Usage:
    python scripts/publish_codeberg_release.py           # create + upload
    python scripts/publish_codeberg_release.py --dry-run # print what would happen
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import urllib.error
import urllib.parse
import urllib.request
import uuid
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DIST = ROOT / "dist"
OWNER = "GuLiang"
REPO = "LangSU"
API = f"https://codeberg.org/api/v1/repos/{OWNER}/{REPO}"

# See publish_github_release.py: the dead system proxy would break every request.
OPENER = urllib.request.build_opener(urllib.request.ProxyHandler({}))


def read_token() -> str:
    """Token from the environment, ~/.codeberg-token, or the git credential vault."""
    for name in ("CODEBERG_TOKEN", "GITEA_TOKEN"):
        value = os.environ.get(name, "").strip()
        if value:
            return value

    token_file = Path.home() / ".codeberg-token"
    if token_file.is_file():
        value = token_file.read_text(encoding="utf-8").strip()
        if value:
            return value

    script = r"""
Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public class CredX {
  [DllImport("advapi32.dll", SetLastError=true, CharSet=CharSet.Unicode)]
  public static extern bool CredReadW(string target, int type, int flags, out IntPtr cred);
  [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)]
  public struct CREDENTIAL {
    public int Flags; public int Type;
    public string TargetName; public string Comment;
    public System.Runtime.InteropServices.ComTypes.FILETIME LastWritten;
    public int CredentialBlobSize; public IntPtr CredentialBlob;
    public int Persist; public int AttributeCount; public IntPtr Attributes;
    public string TargetAlias; public string UserName;
  }
  public static string Blob(string target) {
    IntPtr p;
    if (!CredReadW(target, 1, 0, out p)) return "";
    CREDENTIAL c = (CREDENTIAL)Marshal.PtrToStructure(p, typeof(CREDENTIAL));
    return Marshal.PtrToStringUni(c.CredentialBlob, c.CredentialBlobSize / 2);
  }
}
"@
[CredX]::Blob("git:https://codeberg.org")
"""
    result = subprocess.run(
        ["powershell", "-NoProfile", "-Command", script],
        capture_output=True,
        text=True,
    )
    token = result.stdout.strip()
    if not token:
        raise SystemExit(
            "no Codeberg token; set CODEBERG_TOKEN or write ~/.codeberg-token"
        )
    return token


def api(method: str, path: str, token: str, payload: dict | None = None) -> dict:
    data = None
    headers = {"Authorization": f"token {token}"}
    if payload is not None:
        data = json.dumps(payload).encode()
        headers["Content-Type"] = "application/json"
    request = urllib.request.Request(API + path, data=data, headers=headers, method=method)
    with OPENER.open(request, timeout=120) as response:
        body = response.read()
    return json.loads(body) if body else {}


def upload(asset_path: Path, release_id: int, token: str) -> None:
    boundary = "----LangSU" + uuid.uuid4().hex
    head = (
        f"--{boundary}\r\n"
        f'Content-Disposition: form-data; name="attachment"; filename="{asset_path.name}"\r\n'
        "Content-Type: application/octet-stream\r\n\r\n"
    ).encode()
    tail = f"\r\n--{boundary}--\r\n".encode()
    body = head + asset_path.read_bytes() + tail
    request = urllib.request.Request(
        f"{API}/releases/{release_id}/assets?name={urllib.parse.quote(asset_path.name)}",
        data=body,
        headers={
            "Authorization": f"token {token}",
            "Content-Type": f"multipart/form-data; boundary={boundary}",
        },
        method="POST",
    )
    with OPENER.open(request, timeout=600) as response:
        print(f"  uploaded {asset_path.name} ({response.status})")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    version = (ROOT / "VERSION.txt").read_text(encoding="utf-8").strip()
    tag = f"v{version}"
    notes_path = DIST / "notes" / f"{version}.md"
    notes = notes_path.read_text(encoding="utf-8").strip() if notes_path.is_file() else ""
    assets = sorted(p for p in DIST.glob("*.apk") if p.is_file())
    if not assets:
        raise SystemExit("no APK in dist/")

    print(f"version : {version}")
    print(f"tag     : {tag}")
    for asset in assets:
        print(f"asset   : {asset.name}  {asset.stat().st_size} bytes")
    if args.dry_run:
        print("dry run, nothing sent")
        return 0

    token = read_token()
    existing = None
    try:
        existing = api("GET", f"/releases/tags/{tag}", token)
    except urllib.error.HTTPError as error:
        if error.code != 404:
            raise

    if existing:
        release_id = existing["id"]
        print(f"release : reusing #{release_id}")
        already = {a["name"] for a in existing.get("assets") or []}
        for asset in assets:
            if asset.name in already:
                print(f"  skip {asset.name} (already attached)")
                continue
            upload(asset, release_id, token)
    else:
        release = api(
            "POST",
            "/releases",
            token,
            {
                "tag_name": tag,
                "name": f"LangSU {version}",
                "body": notes or f"LangSU {version}",
                "draft": False,
                "prerelease": False,
            },
        )
        release_id = release["id"]
        print(f"release : created #{release_id}")
        for asset in assets:
            upload(asset, release_id, token)

    print(f"done    : https://codeberg.org/{OWNER}/{REPO}/releases/tag/{tag}")
    return 0


if __name__ == "__main__":
    sys.exit(main())