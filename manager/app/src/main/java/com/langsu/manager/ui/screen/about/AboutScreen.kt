package com.langsu.manager.ui.screen.about

import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.res.stringResource
import androidx.lifecycle.compose.dropUnlessResumed
import com.langsu.manager.BuildConfig
import com.langsu.manager.R
import com.langsu.manager.ui.LocalUiMode
import com.langsu.manager.ui.UiMode
import com.langsu.manager.ui.navigation3.LocalNavigator

@Composable
fun AboutScreen() {
    val navigator = LocalNavigator.current
    val uriHandler = LocalUriHandler.current
    // Only links that actually resolve for this project belong here. The upstream
    // KernelSU chat/website are intentionally not linked: they are not LangSU support
    // channels and sending users there would be misleading.
    val htmlString = stringResource(
        id = R.string.about_source_code,
        "<b><a href=\"https://github.com/GuLiang-I/LangSU\">GitHub</a></b>",
        "GPL-2.0"
    )
    val state = AboutUiState(
        title = stringResource(R.string.about),
        appName = stringResource(R.string.app_name),
        versionName = BuildConfig.VERSION_NAME,
        links = extractLinks(htmlString),
    )
    val actions = AboutScreenActions(
        onBack = dropUnlessResumed { navigator.pop() },
        onOpenLink = uriHandler::openUri,
    )

    when (LocalUiMode.current) {
        UiMode.Miuix -> AboutScreenMiuix(state, actions)
        UiMode.Material -> AboutScreenMaterial(state, actions)
    }
}
