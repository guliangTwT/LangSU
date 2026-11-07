# Regenerates the launcher / splash / notification bitmaps from source.png.
#
# Requires ImageMagick 7 (`magick` on PATH, or pass -MagickPath).
# Output goes straight into manager/app/src/main/res/drawable-<density>/.
#
#   powershell -File scripts/icon/make_icons.ps1
#   powershell -File scripts/icon/make_icons.ps1 -MagickPath "C:\Program Files\ImageMagick-7.1.1-Q16\magick.exe"

param(
    [string]$MagickPath = 'magick'
)

$ErrorActionPreference = 'Stop'

$here = $PSScriptRoot
$repo = Split-Path (Split-Path $here -Parent) -Parent
$src = Join-Path $here 'source.png'
$res = Join-Path $repo 'manager\app\src\main\res'
# Scratch space stays inside the repo and is removed at the end.
$work = Join-Path $here '.work'

if (-not (Test-Path $src)) { throw "source image not found: $src" }
if (-not (Test-Path $res)) { throw "resource dir not found: $res" }

$magick = $MagickPath
try { & $magick -version | Out-Null } catch { throw "ImageMagick not found ('$magick'). Pass -MagickPath." }

New-Item -ItemType Directory -Force -Path $work | Out-Null

# Source geometry (the artwork is a 1254x1254 card with a wordmark at the bottom).
$artHeight = 965

# --- 1. Drop the "LangSU" wordmark -----------------------------------------
# It is unreadable at 48dp, so the icon only carries the artwork.
$art = Join-Path $work 'art.png'
& $magick $src -crop "1254x$artHeight+0+0" +repage $art

# --- 2. Transparent version of the artwork ---------------------------------
# The source is a white card. Flood-filling from the four corners removes it;
# a low fuzz preserves the white outlines that separate hair, ribbon and terminal.
$keyed = Join-Path $work 'keyed.png'
& $magick $art -alpha set -fuzz 8% -fill none `
    -floodfill +0+0 white -floodfill +1253+0 white `
    -floodfill "+0+$($artHeight - 1)" white -floodfill "+1253+$($artHeight - 1)" white `
    $keyed
$keyedTrim = Join-Path $work 'keyed_trim.png'
& $magick $keyed -trim +repage $keyedTrim
$keyedSq = Join-Path $work 'keyed_square.png'
& $magick $keyedTrim -background none -gravity center -extent 1024x1024 $keyedSq

# --- 3. Opaque card version (splash + in-app logo) -------------------------
$cardSq = Join-Path $work 'card_square.png'
& $magick $art -background white -alpha remove -gravity center -extent 1254x1254 `
    -resize 1024x1024 $cardSq

# --- 4. Per-density outputs -------------------------------------------------
# canvas = full adaptive-icon size (108dp). The foreground occupies 72% of it:
# large enough to read at 48dp, still within the 66dp safe zone after masking.
$densities = [ordered]@{
    'mdpi'    = 108
    'hdpi'    = 162
    'xhdpi'   = 216
    'xxhdpi'  = 324
    'xxxhdpi' = 432
}
$notificationSizes = @{ 'mdpi' = 24; 'hdpi' = 36; 'xhdpi' = 48; 'xxhdpi' = 72; 'xxxhdpi' = 96 }

foreach ($d in $densities.Keys) {
    $canvas = $densities[$d]
    $inner = [int][Math]::Round($canvas * 0.72)
    $dir = Join-Path $res "drawable-$d"
    New-Item -ItemType Directory -Force -Path $dir | Out-Null

    # Adaptive foreground: transparent artwork padded to the 108dp canvas.
    & $magick $keyedSq -resize "${inner}x${inner}" -background none -gravity center `
        -extent "${canvas}x${canvas}" (Join-Path $dir 'ic_launcher_foreground.png')

    # Splash icon: the complete rounded card.
    & $magick $cardSq -resize "${inner}x${inner}" -background none -gravity center `
        -extent "${canvas}x${canvas}" (Join-Path $dir 'ic_launcher_splash.png')

    # In-app logo (About screen, module shortcut preview).
    $logo = [int][Math]::Round($canvas * 0.92)
    & $magick $cardSq -resize "${logo}x${logo}" (Join-Path $dir 'ic_app_logo.png')

    # Themed icon. A filled silhouette of this illustration is an unreadable blob,
    # so the alpha is built from luminance with stretched contrast: dark line art
    # becomes solid and the white face/ribbon read as negative space. It is then
    # intersected with the artwork alpha. The layer is black on purpose - Android
    # only reads its alpha and applies the system tint.
    $mono = Join-Path $work "mono_$d.png"
    $luma = Join-Path $work "luma_$d.png"
    $alpha = Join-Path $work "alpha_$d.png"
    & $magick $keyedSq -resize "${inner}x${inner}" -colorspace gray -negate `
        -level 0%,55% -alpha off $luma
    & $magick $keyedSq -resize "${inner}x${inner}" -alpha extract `
        -level 35%,60% -alpha off $alpha
    & $magick $luma $alpha -compose multiply -composite -alpha off $mono
    & $magick -size "${inner}x${inner}" xc:black $mono -alpha off `
        -compose CopyOpacity -composite $mono
    & $magick $mono -background none -gravity center -extent "${canvas}x${canvas}" `
        (Join-Path $dir 'ic_launcher_monochrome.png')

    # Notification icon: must be a flat white silhouette.
    $n = $notificationSizes[$d]
    & $magick $mono -resize "${n}x${n}" -fill white -colorize 100 `
        -background none -gravity center -extent "${n}x${n}" `
        (Join-Path $dir 'ic_notification.png')
}

# --- 5. Preview sheet -------------------------------------------------------
$preview = Join-Path $here 'preview.png'
& $magick montage `
    (Join-Path $res 'drawable-xxxhdpi\ic_launcher_foreground.png') `
    (Join-Path $res 'drawable-xxxhdpi\ic_launcher_splash.png') `
    (Join-Path $res 'drawable-xxxhdpi\ic_launcher_monochrome.png') `
    (Join-Path $res 'drawable-xxxhdpi\ic_app_logo.png') `
    (Join-Path $res 'drawable-xxxhdpi\ic_notification.png') `
    -tile 5x1 -geometry 200x200+12+12 -background '#FFE4EE' $preview

Remove-Item $work -Recurse -Force
Write-Output "icon assets written to $res"
Write-Output "preview: $preview"