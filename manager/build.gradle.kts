plugins {
    alias(libs.plugins.agp.app) apply false
    alias(libs.plugins.kotlin) apply false
    alias(libs.plugins.compose.compiler) apply false
}

extra["androidMinSdkVersion"] = 31
extra["androidTargetSdkVersion"] = 37
extra["androidCompileSdkVersion"] = 37
extra["androidCompileSdkVersionMinor"] = 0
extra["androidBuildToolsVersion"] = "37.0.0"
extra["androidCompileNdkVersion"] = libs.versions.ndk.get()
extra["androidSourceCompatibility"] = JavaVersion.VERSION_21
extra["androidTargetCompatibility"] = JavaVersion.VERSION_21
extra["managerVersionCode"] = getVersionCode()
extra["managerVersionName"] = getVersionName()

// Versioning: keep the KernelSU scheme (30000 + commit count) so upgrades stay ordered,
// but never fail the build when git is unavailable (release tarballs, CI shallow copies).
val fallbackVersionName = "1.0.0"

fun gitOutput(vararg args: String): String? = runCatching {
    val process = ProcessBuilder(listOf("git") + args.toList())
        .redirectErrorStream(true)
        .start()
    val text = process.inputStream.bufferedReader().use { it.readText() }.trim()
    if (process.waitFor() == 0 && text.isNotEmpty()) text else null
}.getOrNull()

fun getGitCommitCount(): Int =
    gitOutput("rev-list", "--count", "HEAD")?.toIntOrNull() ?: 1

/** Prefer a real tag (v3.3.1); fall back to a readable marker instead of a bare hash. */
fun getGitDescribe(): String {
    val exact = gitOutput("describe", "--tags", "--exact-match")
    val described = gitOutput("describe", "--tags", "--always")
    val name = (exact ?: described)?.removePrefix("v")
    val isHashOnly = name != null && name.matches(Regex("^[0-9a-f]{7,40}$"))
    return if (name == null || isHashOnly) fallbackVersionName else name
}

fun getVersionCode(): Int = 30000 + getGitCommitCount()

fun getVersionName(): String = getGitDescribe()
