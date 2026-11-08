package com.langsu.manager.ui.util

import android.net.Uri
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.withContext
import com.langsu.manager.data.registry.ExpectedFile
import com.langsu.manager.langsuApp
import com.langsu.manager.ui.util.module.LatestVersionInfo
import okhttp3.Request

/**
 * @author weishu
 * @date 2023/6/22.
 */
suspend fun download(
    url: String,
    fileName: String,
    onDownloaded: (Uri) -> Unit = {},
    onDownloading: () -> Unit = {},
    onProgress: (Int) -> Unit = {},
    expected: ExpectedFile? = null,
    onFailed: (DownloadManager.DownloadState) -> Unit = {},
) {
    onDownloading()

    val downloadId = DownloadManager.enqueue(
        context = langsuApp,
        url = url,
        fileName = fileName,
        onCompleted = onDownloaded,
        expected = expected,
    )

    val finalState = DownloadManager.downloads
        .onEach { map -> map[downloadId]?.let { onProgress(it.progress) } }
        .first { map ->
            val status = map[downloadId]?.status
            status == DownloadManager.Status.COMPLETED ||
                status == DownloadManager.Status.FAILED
        }[downloadId]
    if (finalState?.status == DownloadManager.Status.FAILED) {
        withContext(Dispatchers.Main) { onFailed(finalState) }
    }
}

internal suspend fun isDownloadAvailable(uri: Uri): Boolean = withContext(Dispatchers.IO) {
    runCatching {
        langsuApp.contentResolver.openFileDescriptor(uri, "r").use { it != null }
    }.getOrDefault(false)
}

/**
 * Update sources, tried in order. GitHub is the primary channel; Codeberg mirrors the same
 * releases and is used when GitHub is unreachable (rate limited, blocked, account issue).
 * The self-hosted manifest is the last resort so a simultaneous outage of both forges still
 * leaves the app able to advertise an update.
 */
private const val GITHUB_LATEST_URL = "https://api.github.com/repos/GuLiang-I/LangSU/releases/latest"
private const val CODEBERG_LATEST_URL = "https://codeberg.org/api/v1/repos/GuLiang/LangSU/releases/latest"
private const val UPDATE_MANIFEST_URL = "https://guliang.me/langsu/updates/latest.json"

private fun fetchText(url: String): String? = runCatching {
    langsuApp.okhttpClient.newCall(Request.Builder().url(url).build()).execute().use { response ->
        if (response.isSuccessful) response.body.string() else null
    }
}.getOrNull()

/**
 * Reads the self-hosted manifest. Returns null when it is unreachable or malformed so the
 * caller can try the next source.
 */
private fun checkSelfHostedManifest(): LatestVersionInfo? {
    // The timestamp defeats any intermediate cache; the page and client both use it.
    val body = fetchText("$UPDATE_MANIFEST_URL?t=${System.currentTimeMillis()}") ?: return null
    return runCatching {
        val json = org.json.JSONObject(body)
        val versionName = json.optString("version").trim().removePrefix("v")
        if (versionName.isEmpty()) return null

        val assets = json.optJSONArray("assets") ?: return null
        for (i in 0 until assets.length()) {
            val asset = assets.getJSONObject(i)
            val name = asset.optString("name")
            if (!name.endsWith(".apk", ignoreCase = true)) continue

            val urls = asset.optJSONArray("urls")
            val primary = urls?.optString(0)?.takeIf { it.isNotBlank() } ?: continue

            return LatestVersionInfo(
                versionCode = 0,
                versionName = versionName,
                downloadUrl = primary,
                changelog = json.optString("notes"),
                sha256 = asset.optString("sha256").lowercase(),
            )
        }
        null
    }.getOrNull()
}

/**
 * Reads a forge release. GitHub and Codeberg (Forgejo) return the same fields for the parts
 * used here, so one parser covers both.
 */
private fun checkForgeRelease(endpoint: String): LatestVersionInfo? {
    val body = fetchText(endpoint) ?: return null
    return runCatching {
        val json = org.json.JSONObject(body)
        val versionName = json.optString("tag_name").trim().removePrefix("v")
        if (versionName.isEmpty()) return null

        val assets = json.optJSONArray("assets") ?: return null
        for (i in 0 until assets.length()) {
            val asset = assets.getJSONObject(i)
            val name = asset.optString("name")
            if (!name.endsWith(".apk", ignoreCase = true)) continue
            val url = asset.optString("browser_download_url").takeIf { it.isNotBlank() } ?: continue
            return LatestVersionInfo(
                versionCode = 0,
                versionName = versionName,
                downloadUrl = url,
                changelog = json.optString("body"),
            )
        }
        null
    }.getOrNull()
}

fun checkNewVersion(): LatestVersionInfo {
    if (!isNetworkAvailable(langsuApp)) return LatestVersionInfo()
    checkForgeRelease(GITHUB_LATEST_URL)?.let { return it }
    checkForgeRelease(CODEBERG_LATEST_URL)?.let { return it }
    checkSelfHostedManifest()?.let { return it }
    return LatestVersionInfo()
}
