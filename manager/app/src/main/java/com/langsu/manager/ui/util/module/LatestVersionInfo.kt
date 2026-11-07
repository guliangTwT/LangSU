package com.langsu.manager.ui.util.module

data class LatestVersionInfo(
    val versionCode: Int = 0,
    /** Semver of the advertised build, used for the update comparison. */
    val versionName: String = "",
    val downloadUrl: String = "",
    val changelog: String = "",
    /** Lowercase hex sha256 of the APK, empty when the source does not provide one. */
    val sha256: String = "",
)

/**
 * Compares dotted version numbers segment by segment, so "1.0.10" > "1.0.9" while a plain
 * string compare would say the opposite. Missing segments count as 0, and any non-numeric
 * tail (for example the "-2-gabcdef" suffix git describe adds between releases) is ignored.
 *
 * Returns >0 when [left] is newer, 0 when equal, <0 when older.
 */
fun compareVersionNames(left: String, right: String): Int {
    fun parse(value: String): List<Int> =
        value.trim()
            .removePrefix("v")
            .takeWhile { it.isDigit() || it == '.' }
            .split('.')
            .mapNotNull { it.takeIf(String::isNotEmpty)?.toIntOrNull() }

    val a = parse(left)
    val b = parse(right)
    if (a.isEmpty() || b.isEmpty()) return 0

    val length = maxOf(a.size, b.size)
    for (i in 0 until length) {
        val x = a.getOrElse(i) { 0 }
        val y = b.getOrElse(i) { 0 }
        if (x != y) return x.compareTo(y)
    }
    return 0
}