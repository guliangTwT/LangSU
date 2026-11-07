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
 * Base URL of the self-hosted update channel. Releases are published as a static
 * `latest.json` plus the APK (see the selfhost-release notes in REFACTOR_NOTES.md).
 */
private const val UPDATE_MANIFEST_URL = "https://guliang.me/langsu/updates/latest.json"

/** Codeberg is only a fallback: the manifest is the authoritative source. */
private const val CODEBERG_LATEST_URL = "https://codeberg.org/api/v1/repos/GuLiang/LangSU/releases/latest"

private fun fetchText(url: String): String? = runCatching {
    langsuApp.okhttpClient.newCall(Request.Builder().url(url).build()).execute().use { response ->
        if (response.isSuccessful) response.body.string() else null
    }
}.getOrNull()

/**
 * Reads the self-hosted manifest. Returns null when it is unreachable or malformed so the
 * caller can fall back to the repository releases on Codeberg.
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

/** Fallback source: the repository's own releases on Codeberg. */
private fun checkRepositoryReleases(): LatestVersionInfo? {
    val body = fetchText(CODEBERG_LATEST_URL) ?: return null
    return runCatching {
        val json = org.json.JSONObject(body)
        val versionName = json.optString("tag_name").trim().removePrefix("v")
        if (versionName.isEmpty()) return null

        val assets = json.optJSONArray("assets") ?: return null
        for (i in 0 until assets.length()) {
            val asset = assets.getJSONObject(i)
            if (!asset.optString("name").endsWith(".apk", ignoreCase = true)) continue
            return LatestVersionInfo(
                versionCode = 0,
                versionName = versionName,
                downloadUrl = asset.optString("browser_download_url"),
                changelog = json.optString("body"),
            )
        }
        null
    }.getOrNull()
}

fun checkNewVersion(): LatestVersionInfo {
    if (!isNetworkAvailable(langsuApp)) return LatestVersionInfo()
    // Manifest first (own host, no rate limits, works where the repository host is slow or
    // blocked); the repository releases stay as a fallback so an outage of the host does not
    // kill update checks.
    checkSelfHostedManifest()?.let { return it }
    checkRepositoryReleases()?.let { return it }
    return LatestVersionInfo()
}
