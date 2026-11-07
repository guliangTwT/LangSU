package com.langsu.manager.ui.component.statustag

import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import com.langsu.manager.ui.LocalUiMode
import com.langsu.manager.ui.UiMode

@Composable
fun StatusTag(
    label: String,
    modifier: Modifier = Modifier,
    backgroundColor: Color,
    contentColor: Color
) {
    when (LocalUiMode.current) {
        UiMode.Miuix -> StatusTagMiuix(label, modifier, backgroundColor, contentColor)
        UiMode.Material -> StatusTagMaterial(label, modifier, backgroundColor, contentColor)
    }
}
