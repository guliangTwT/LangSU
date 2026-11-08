plugins {
    alias(libs.plugins.agp.app) apply false
    alias(libs.plugins.kotlin) apply false
    alias(libs.plugins.compose.compiler) apply false
}

// Versioning: the version name comes from the git tag (v1.0.0 -> 1.0.0), the code from the
// commit count. Never fail the build when git is unavailable (release tarballs, CI shallow
// clones, vendored checkouts). Declared before use: script bodies run top to bottom.
val fallbackVersionName = "1.0.0"

/** Runs git and returns its trimmed stdout, or null when git is missing or prints nothing. */
fun gitOutput(vararg args: String): String? = try {
    val process = ProcessBuilder(listOf("git") + args.toList())
        .redirectErrorStream(true)
        .start()
    val text = process.inputStream.bufferedReader().use { it.readText() }.trim()
    val ok = process.waitFor() == 0
    // Only trust real output. On failure git prints messages like
    // "fatal: No names found, cannot describe anything." which must never reach the
    // version name - it ends up in the APK file name and breaks packaging.
    if (ok) text.ifEmpty { null } else null
} catch (_: Throwable) {
    null
}

/** A version name safe to embed in BuildConfig and in the APK file name. */
fun sanitizeVersionName(raw: String): String {
    val cleaned = raw.trim().removePrefix("v").replace(Regex("[^A-Za-z0-9._-]"), "-").trim('-', '.', '_')
    return cleaned.ifEmpty { fallbackVersionName }
}

fun getGitCommitCount(): Int =
    gitOutput("rev-list", "--count", "HEAD")?.toIntOrNull()?.takeIf { it > 0 } ?: 1

/** Prefer a real tag (v1.0.0); fall back to a readable marker instead of a bare hash. */
fun getGitDescribe(): String {
    val exact = gitOutput("describe", "--tags", "--exact-match")
    val described = gitOutput("describe", "--tags", "--always")
    val name = exact ?: described
    if (name == null || name.matches(Regex("^[0-9a-f]{7,40}$"))) return fallbackVersionName
    return sanitizeVersionName(name)
}

/**
 * Version code derived from the version name, `1.0.0` -> 50000, `1.0.1` -> 50100,
 * `1.2.3` -> 50203. The `40000` floor exists because the version name restarted at 1.0.0
 * while earlier builds were already published as `30000 + commit count` (30010 and up):
 * a plain `major*10000 + ...` would give 10000 and such a build could not be installed
 * over the existing one.
 */
fun versionCodeOf(name: String): Int {
    val parts = name.split('.').map { it.takeWhile(Char::isDigit).toIntOrNull() ?: 0 }
    val major = parts.getOrElse(0) { 0 }
    val minor = parts.getOrElse(1) { 0 }
    val patch = parts.getOrElse(2) { 0 }
    return 40000 + major * 10000 + minor * 100 + patch
}

val managerVersionName: String = getGitDescribe()
val managerVersionCode: Int = versionCodeOf(managerVersionName)

extra["androidMinSdkVersion"] = 31
extra["androidTargetSdkVersion"] = 37
extra["androidCompileSdkVersion"] = 37
extra["androidCompileSdkVersionMinor"] = 0
extra["androidBuildToolsVersion"] = "37.0.0"
extra["androidCompileNdkVersion"] = libs.versions.ndk.get()
extra["androidSourceCompatibility"] = JavaVersion.VERSION_21
extra["androidTargetCompatibility"] = JavaVersion.VERSION_21
extra["managerVersionCode"] = managerVersionCode
extra["managerVersionName"] = managerVersionName
