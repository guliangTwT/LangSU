# Applies the catgirl tone to the Chinese UI strings.
#
# Deliberately conservative: only headings, status words and short hints get "喵~".
# Everything that carries technical meaning (versions, paths, error details, format
# strings with %s, single-word values shown next to a label) is left untouched so the
# UI stays readable and diagnostics stay copy-pasteable.
#
# Run from the repo root:  powershell -File scripts/catgirl_strings.ps1

$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$repo = Split-Path $PSScriptRoot -Parent
$path = Join-Path $repo 'manager\app\src\main\res\values-zh-rCN\strings.xml'
$enc = New-Object System.Text.UTF8Encoding($false)
$text = [IO.File]::ReadAllText($path, [Text.Encoding]::UTF8)

# key -> new value. Keys not listed keep their current text.
$map = [ordered]@{
    # --- navigation / top level ---
    'home'                        = '主页喵~'
    'module'                      = '模块喵~'
    'superuser'                   = '超级用户喵~'
    'settings'                    = '设置喵~'
    'about'                       = '关于喵~'

    # --- home status ---
    'home_not_installed'          = '还没装喵…'
    'home_working'                = '干活中喵~'
    'home_click_to_install'       = '点这里装上喵~'
    'home_jailbreak'              = '越狱喵~'
    'home_lkm_custom'             = '自己带喵~'
    'home_unsupported'            = '不支持喵…'
    'selinux_status_unknown'      = '不知道喵？'
    'seccomp_status_unknown'      = '不知道喵？'

    # --- modules ---
    'module_empty'                = '还没有装模块喵…'
    'module_repos'                = '模块仓库喵~'
    'module_repo_empty'           = '仓库里空空的喵…'
    'module_repo_no_readme'       = '这个模块没写说明喵…'
    'module_repo_installed'       = '已经装好啦喵~'
    'module_repo_update_available' = '可以更新喵~'
    'module_update'               = '更新喵~'
    'module_changelog'            = '更新日志喵~'
    'recently_installed'          = '刚装好的喵~'

    # --- superuser ---
    'su_not_allowed'              = '没能给 %s 超级用户权限喵…'

    # --- settings groups ---
    'settings_theme'              = '主题喵~'
    'settings_theme_summary'      = '给 LangSU 换身衣服喵~'
    'settings_ui_mode'            = '界面风格喵~'
    'settings_ui_mode_summary'    = '挑一个看着顺眼的样式喵~'
    'settings_key_color'          = '主题色喵~'
    'settings_key_color_default'  = '默认喵~'
    'settings_enable_blur'        = '毛玻璃喵~'
    'settings_enable_blur_summary' = '给顶栏和底栏加一点朦胧感喵~'
    'settings_floating_bottom_bar' = '悬浮底栏喵~'
    'settings_floating_bottom_bar_summary' = '像苹果那样飘起来的底栏喵~'
    'settings_enable_glass'       = '液态玻璃喵~'
    'settings_enable_glass_summary' = '给悬浮底栏再加一层玻璃质感喵~'
    'settings_page_scale'         = '界面缩放喵~'
    'settings_page_scale_summary' = '把整个界面放大或缩小一点喵~'
    'settings_module_description_max_lines' = '模块说明最多几行喵~'

    # --- feature switches ---
    'settings_profile_template'   = 'App Profile 模板喵~'
    'settings_profile_template_summary' = '管理本地和在线模板喵~'
    'settings_selinux_hide'       = '隐藏 SELinux 改动喵~'
    'settings_sulog'              = 'SU 日志喵~'
    'settings_soft_reboot'        = '软重启代替重启喵~'
    'settings_umount_modules_default' = '默认卸载模块喵~'
    'enable_web_debugging'        = 'WebView 调试喵~'
    'settings_auto_jailbreak'     = '自动越狱喵~'
    'settings_hide_launcher_icon' = '隐藏桌面图标喵~'
    'settings_uninstall'          = '卸载喵…'

    # --- prompts / empty states ---
    'processing'                  = '正在努力喵…'
    'refresh_pulling'             = '往下拉可以刷新喵~'
    'refresh_release'             = '松手就刷新喵~'
    'refresh_refresh'             = '刷新中喵…'
    'refresh_complete'            = '刷新好啦喵~'
    'network_offline'             = '网络没连上喵…'
    'network_retry'               = '再试一次喵~'
    'no_such_module'              = '找不到这个模块喵…'
    'undo'                        = '撤销喵~'
    'confirm'                     = '就这个喵~'
    'cancel'                      = '算了喵~'
    'ok'                          = '好喵~'
    'open'                        = '打开喵~'
    'close'                       = '关掉喵~'
    'save'                        = '存好啦喵~'

    # --- secret entry ---
    'settings_hide_launcher_icon_summary' = '把 LangSU 从桌面和应用列表藏起来喵~想找回来就在拨号盘输入 *#*#5264#*#* 喵~'
    'secret_entry_channel_name'   = '暗号入口喵~'
    'secret_entry_title'          = 'LangSU 回来啦喵~'
    'secret_entry_text'           = '点这里打开喵~桌面图标已经恢复了喵~'
}

$applied = 0
$missing = New-Object System.Collections.Generic.List[string]
foreach ($key in $map.Keys) {
    $pattern = '(?s)<string name="' + [regex]::Escape($key) + '"[^>]*>.*?</string>'
    $replacement = '<string name="' + $key + '">' + $map[$key] + '</string>'
    if ([regex]::IsMatch($text, $pattern)) {
        $text = [regex]::Replace($text, $pattern, $replacement)
        $applied++
    } else {
        $missing.Add($key)
    }
}

[IO.File]::WriteAllText($path, $text, $enc)
Write-Output "applied=$applied"
if ($missing.Count -gt 0) {
    Write-Output ("not present in this locale (skipped): " + ($missing -join ', '))
}