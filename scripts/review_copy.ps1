$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$p = Join-Path $repo 'manager\app\src\main\res\values-zh-rCN\strings.xml'
$t = [IO.File]::ReadAllText($p, [Text.Encoding]::UTF8)
$m = [regex]::Matches($t, '<string name="([^"]+)">([^<]*)</string>')

$tail = [char]0x55B5
$problems = New-Object System.Collections.Generic.List[string]

foreach ($x in $m) {
    $key = $x.Groups[1].Value
    $val = $x.Groups[2].Value
    $hasTail = $val.Contains($tail)

    # Format strings must keep placeholder count; the tail must not sit inside one.
    $phFrom = ([regex]::Matches($val, '%\d*\$?[sd]')).Count
    if ($phFrom -gt 0 -and -not $hasTail) {
        # fine: technical text, placeholders only
    }

    if ($hasTail) {
        # A tail on these keys would hurt readability or break parsing.
        $forbidden = @(
            'home_working_version', 'module_repo_source_commit', 'new_version_available',
            'module_downloading', 'module_start_downloading', 'su_not_allowed',
            'failed_to_update_app_profile', 'failed_to_update_sepolicy',
            'module_failed_to_enable', 'module_failed_to_disable',
            'module_repo_download_verify_failed', 'app_profile_template_id',
            'home_device_model', 'home_kernel', 'home_manager_version', 'home_fingerprint'
        )
        if ($forbidden -contains $key) { $problems.Add("TAIL_ON_TECHNICAL $key = $val") }
        # Keep the tail outside placeholders: no '喵' directly before a %s.
        if ($val -match ('喵.{0,2}%')) { $problems.Add("TAIL_NEAR_PLACEHOLDER $key = $val") }
    }
}

# Sanity: technical labels must not have gained a tail.
foreach ($x in $m) {
    $key = $x.Groups[1].Value
    if ($key -match '^(home_(device_model|kernel|manager_version|fingerprint)|profile_(groups|capabilities|flags|selinux_context|selinux_domain|selinux_rules))$') {
        if ($x.Groups[2].Value.Contains($tail)) { $problems.Add("TECHNICAL_LABEL_CHANGED $key") }
    }
}

if ($problems.Count -eq 0) { Write-Output 'COPY_REVIEW_OK' }
else { foreach ($q in $problems) { Write-Output $q } }