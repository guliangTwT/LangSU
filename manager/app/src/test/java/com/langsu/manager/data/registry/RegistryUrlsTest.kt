package com.langsu.manager.data.registry

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertThrows
import org.junit.Assert.assertTrue
import org.junit.Test

class RegistryUrlsTest {

    private val good = "https://github.com/owner/repo/releases/download/v1.0.0/module.zip"

    @Test
    fun acceptsGithubReleaseDownloads() {
        assertTrue(RegistryUrls.isAllowedReleaseDownloadUrl(good))
        assertTrue(RegistryUrls.isAllowedReleaseDownloadUrl("https://GitHub.com/o-1/r.e_p/releases/download/v2/a%20b.zip"))
    }

    @Test
    fun rejectsEverythingElse() {
        val bad = listOf(
            "http://github.com/owner/repo/releases/download/v1/module.zip",
            "https://github.com.evil.com/owner/repo/releases/download/v1/module.zip",
            "https://evil.com/owner/repo/releases/download/v1/module.zip",
            "https://objects.githubusercontent.com/owner/repo/releases/download/v1/module.zip",
            "https://github.com/owner/repo/archive/refs/tags/v1.zip",
            "https://github.com/owner/repo/releases/download/module.zip",
            "https://github.com/owner/repo/releases/download/v1/../../../x/module.zip",
            "https://github.com/owner/repo/releases/download/v1/%2e%2e/module.zip",
            "https://github.com/owner/repo/releases/download/v1/a%2Fb.zip",
            "https://user@github.com/owner/repo/releases/download/v1/module.zip",
            "https://github.com:8443/owner/repo/releases/download/v1/module.zip",
            "https://github.com/owner/repo/releases/download/v1/module.zip?x=1",
            "https://github.com/owner/repo/releases/download/v1/module.zip#frag",
            "https://github.com//repo/releases/download/v1/module.zip",
            "javascript:alert(1)",
            "not a url",
            "",
        )
        for (url in bad) assertFalse(url, RegistryUrls.isAllowedReleaseDownloadUrl(url))
    }

    @Test
    fun redirectHops() {
        assertTrue(RegistryUrls.isAllowedDownloadHop(good))
        assertTrue(RegistryUrls.isAllowedDownloadHop("https://objects.githubusercontent.com/github-production-release-asset/1?sig=x"))
        assertTrue(RegistryUrls.isAllowedDownloadHop("https://release-assets.githubusercontent.com/github-production-release-asset/1?sp=r"))
        assertFalse(RegistryUrls.isAllowedDownloadHop("http://objects.githubusercontent.com/x"))
        assertFalse(RegistryUrls.isAllowedDownloadHop("https://raw.githubusercontent.com/x"))
        assertFalse(RegistryUrls.isAllowedDownloadHop("https://evil.com/x"))
    }

    @Test
    fun resolvesApiPathsAgainstSiteRoot() {
        assertEquals(
            "https://modules.langsu.dev/api/v1/modules.json",
            RegistryUrls.resolve(RegistryUrls.DEFAULT_BASE_URL, "api/v1/modules.json")
        )
        assertEquals(
            "https://example.org/mirror/api/v1/index.json",
            RegistryUrls.resolve("https://example.org/mirror", "api/v1/index.json")
        )
        for (bad in listOf("/api/v1/x.json", "../x.json", "api/../x.json", "https://evil.com/x", "a?b", "")) {
            assertThrows(bad, RegistryFormatException::class.java) { RegistryUrls.resolve(RegistryUrls.DEFAULT_BASE_URL, bad) }
        }
        assertThrows(IllegalArgumentException::class.java) { RegistryUrls.normalizeBaseUrl("http://modules.langsu.dev/") }
    }

    @Test
    fun idnAndPunycodeHostsNeverMatch() {
        // Non-ASCII hosts do not parse as a server authority; punycode is compared literally.
        for (url in listOf(
            "https://gíthub.com/owner/repo/releases/download/v1/module.zip",
            "https://xn--gthub-zqa.com/owner/repo/releases/download/v1/module.zip",
            "https://github.com./owner/repo/releases/download/v1/module.zip",
            "https://github.com%2eevil.com/owner/repo/releases/download/v1/module.zip",
        )) {
            assertFalse(url, RegistryUrls.isAllowedReleaseDownloadUrl(url))
            assertFalse(url, RegistryUrls.isAllowedDownloadHop(url))
        }
        val base = RegistryUrls.DEFAULT_BASE_URL
        assertFalse(RegistryUrls.isSameOrigin("https://modules.kérnelsu.org/x.png", base))
        assertFalse(RegistryUrls.isSameOrigin("https://xn--modules-kernelsu-xyz.org/x.png", base))
        assertTrue(RegistryUrls.isSameOrigin("https://xn--mnchen-3ya.de/a", "https://XN--MNCHEN-3YA.de/"))
        assertFalse(RegistryUrls.isSameOrigin("https://xn--mnchen-3ya.de/a", "https://münchen.de/"))
    }

    @Test
    fun sameOriginComparesSchemeHostAndPort() {
        val base = RegistryUrls.DEFAULT_BASE_URL
        assertTrue(RegistryUrls.isSameOrigin("https://modules.langsu.dev/assets/a.png", base))
        assertTrue(RegistryUrls.isSameOrigin("HTTPS://Modules.LangSU.org:443/assets/a.png", base))
        assertFalse(RegistryUrls.isSameOrigin("http://modules.langsu.dev/assets/a.png", base))
        assertFalse(RegistryUrls.isSameOrigin("https://modules.langsu.dev:8443/assets/a.png", base))
        assertFalse(RegistryUrls.isSameOrigin("https://evil.modules.langsu.dev/a.png", base))
        assertFalse(RegistryUrls.isSameOrigin("https://modules.langsu.dev.evil.com/a.png", base))
        assertFalse(RegistryUrls.isSameOrigin("ftp://modules.langsu.dev/a.png", "ftp://modules.langsu.dev/"))
        assertFalse(RegistryUrls.isSameOrigin("not a url", base))
    }

    @Test
    fun restrictedResourcesAndRedirectHops() {
        val base = RegistryUrls.DEFAULT_BASE_URL
        // Every hop of a redirect chain is checked with the same rule.
        val chain = listOf(
            "https://modules.langsu.dev/assets/a.png" to true,
            "https://modules.langsu.dev/assets/b.png?v=2" to true,
            "https://github.com/owner/repo/raw/main/a.png" to false,
            "https://objects.githubusercontent.com/x" to false,
            "http://modules.langsu.dev/assets/a.png" to false,
            "https://modules.langsu.dev:444/assets/a.png" to false,
            "https://user@modules.langsu.dev/assets/a.png" to false,
            "data:image/png;base64,AAAA" to false,
            "file:///data/data/x" to false,
            "javascript:alert(1)" to false,
        )
        for ((url, allowed) in chain) {
            assertEquals(url, allowed, RegistryUrls.isAllowedRestrictedResource(url, base))
        }
    }

    @Test
    fun inPageAnchors() {
        val base = RegistryUrls.DEFAULT_BASE_URL
        assertTrue(RegistryUrls.isInPageAnchor("https://modules.langsu.dev/#install", base))
        assertFalse(RegistryUrls.isInPageAnchor("https://modules.langsu.dev/#", base))
        assertFalse(RegistryUrls.isInPageAnchor("https://modules.langsu.dev/", base))
        assertFalse(RegistryUrls.isInPageAnchor("https://modules.langsu.dev/en/#install", base))
        assertFalse(RegistryUrls.isInPageAnchor("https://evil.com/#https://modules.langsu.dev/", base))
    }

    @Test
    fun linkSchemes() {
        assertTrue(RegistryUrls.isHttpsUrl("https://github.com/o/r"))
        assertFalse(RegistryUrls.isHttpsUrl("http://github.com/o/r"))
        assertFalse(RegistryUrls.isHttpsUrl("https://user:pw@github.com/o/r"))
        assertFalse(RegistryUrls.isHttpsUrl("javascript:alert(1)"))
        assertFalse(RegistryUrls.isHttpsUrl("intent://x#Intent;end"))
        assertTrue(RegistryUrls.isWebUrl("http://example.com/"))
        assertTrue(RegistryUrls.isWebUrl("https://example.com/"))
        assertFalse(RegistryUrls.isWebUrl("ftp://example.com/"))
        assertFalse(RegistryUrls.isWebUrl("mailto:a@b.c"))
    }
}
