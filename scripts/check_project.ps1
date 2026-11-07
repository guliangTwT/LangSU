$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$repo = Split-Path $PSScriptRoot -Parent
$problems = New-Object System.Collections.Generic.List[string]
function Add-Problem([string]$m) { $script:problems.Add($m) }

# 1) Every XML resource must parse.
Get-ChildItem -LiteralPath $repo -Recurse -File -Filter *.xml | Where-Object {
    $_.FullName -notmatch '\\(build|\.gradle|\.cxx|node_modules|\.git)\\'
} | ForEach-Object {
    try { [xml](Get-Content -LiteralPath $_.FullName -Raw -Encoding UTF8) | Out-Null }
    catch { Add-Problem ("XML_BAD " + $_.FullName.Substring($repo.Length + 1)) }
}

# 2) Text files must still be valid UTF-8 with no mojibake markers.
Get-ChildItem -LiteralPath $repo -Recurse -File -Force | Where-Object {
    $_.FullName -notmatch '\\(build|\.gradle|\.cxx|node_modules|\.git)\\' -and
    $_.Length -gt 0 -and $_.Extension -notin '.png', '.jpg', '.jpeg', '.webp', '.ico'
} | ForEach-Object {
    $bytes = [IO.File]::ReadAllBytes($_.FullName)
    $probe = [Math]::Min($bytes.Length, 4096)
    for ($i = 0; $i -lt $probe; $i++) { if ($bytes[$i] -eq 0) { return } }
    $strict = New-Object System.Text.UTF8Encoding($false, $true)
    try { $strict.GetString($bytes) | Out-Null }
    catch { Add-Problem ("UTF8_BAD " + $_.FullName.Substring($repo.Length + 1)) }
}

# 3) Package declarations must match the directory tree.
foreach ($root in @('manager\app\src\main\java', 'manager\app\src\test\java')) {
    $base = Join-Path $repo $root
    if (-not (Test-Path $base)) { continue }
    Get-ChildItem -LiteralPath $base -Recurse -File -Include *.kt, *.java | ForEach-Object {
        $dir = Split-Path $_.FullName.Substring($base.Length + 1) -Parent
        if (-not $dir) { return }
        $expected = $dir -replace '\\', '.'
        $text = [IO.File]::ReadAllText($_.FullName)
        $m = [regex]::Match($text, '(?m)^\s*package\s+([A-Za-z0-9_.]+)')
        if (-not $m.Success) { Add-Problem ("NO_PACKAGE " + $_.Name) }
        elseif ($m.Groups[1].Value -ne $expected) {
            Add-Problem ("PKG_MISMATCH " + $_.Name + " file=" + $m.Groups[1].Value + " dir=" + $expected)
        }
    }
}

# 4) Manifest components that are real classes must exist; activity-alias is exempt.
$manifest = [IO.File]::ReadAllText((Join-Path $repo 'manager\app\src\main\AndroidManifest.xml'))
$aliases = @{}
foreach ($m in [regex]::Matches($manifest, '<activity-alias[^>]*?android:name="([^"]+)"')) {
    $aliases[$m.Groups[1].Value] = $true
}
$pkgRoot = Join-Path $repo 'manager\app\src\main\java'
foreach ($m in [regex]::Matches($manifest, 'android:(?:name|targetActivity)="(com\.langsu\.manager\.[A-Za-z0-9_.]+)"')) {
    $cls = $m.Groups[1].Value
    if ($aliases.ContainsKey($cls)) { continue }
    $p = Join-Path $pkgRoot ($cls -replace '\.', '\')
    if (-not ((Test-Path ($p + '.kt')) -or (Test-Path ($p + '.java')))) {
        Add-Problem ("MANIFEST_CLASS_MISSING " + $cls)
    }
}

# 5) Every R.string.foo used from code must exist in the default resources.
$defaultStrings = Get-Content (Join-Path $repo 'manager\app\src\main\res\values\strings.xml') -Raw
$known = @{}
foreach ($m in [regex]::Matches($defaultStrings, '<string name="([^"]+)"')) { $known[$m.Groups[1].Value] = $true }
Get-ChildItem (Join-Path $repo 'manager\app\src\main\java') -Recurse -File -Include *.kt, *.java | ForEach-Object {
    $t = [IO.File]::ReadAllText($_.FullName)
    foreach ($m in [regex]::Matches($t, '(?<!android\.)R\.string\.([A-Za-z0-9_]+)')) {
        $n = $m.Groups[1].Value
        # app_name comes from resValue() in the build script.
        if ($n -ne 'app_name' -and -not $known.ContainsKey($n)) {
            Add-Problem ("STRING_MISSING " + $n + " in " + $_.Name)
        }
    }
}

# 6) No placeholders to non-existent hosts should remain in shipped code.
Get-ChildItem (Join-Path $repo 'manager\app\src\main\java') -Recurse -File -Include *.kt, *.java | ForEach-Object {
    $t = [IO.File]::ReadAllText($_.FullName)
    if ($t -match 'langsu-dev|modules\.github\.com|mui\.github\.com') {
        Add-Problem ("DEAD_HOST " + $_.Name)
    }
}

# 7) Load-time sanity: no localised strings.xml may define a duplicate key.
Get-ChildItem (Join-Path $repo 'manager\app\src\main\res') -Directory | Where-Object { $_.Name -like 'values*' } | ForEach-Object {
    $f = Join-Path $_.FullName 'strings.xml'
    if (-not (Test-Path $f)) { return }
    $keys = [regex]::Matches([IO.File]::ReadAllText($f), '<string name="([^"]+)"') |
        ForEach-Object { $_.Groups[1].Value }
    $dupes = $keys | Group-Object | Where-Object { $_.Count -gt 1 }
    foreach ($d in $dupes) { Add-Problem ("DUP_STRING " + $_.Name + " " + $d.Name) }
}

if ($problems.Count -eq 0) { Write-Output 'ALL_CHECKS_PASSED' }
else { $problems | Sort-Object -Unique | ForEach-Object { Write-Output $_ } }