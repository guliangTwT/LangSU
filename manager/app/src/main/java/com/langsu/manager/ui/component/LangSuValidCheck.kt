package com.langsu.manager.ui.component

import androidx.compose.runtime.Composable
import com.langsu.manager.Natives

@Composable
fun LangSuIsValid(
    content: @Composable () -> Unit
) {
    val isManager = Natives.isManager
    val langsuVersion = if (isManager) Natives.version else null

    if (langsuVersion != null) {
        content()
    }
}
