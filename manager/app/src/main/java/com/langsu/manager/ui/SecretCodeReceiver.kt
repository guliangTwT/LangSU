package com.langsu.manager.ui

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent

/**
 * Secret launcher entry used when the launcher icon has been hidden.
 *
 * Dial `*#*#5264#*#*` on the dial pad to open LangSU again. The receiver only
 * reacts to its own secret code and never exposes any privileged operation.
 */
class SecretCodeReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action != "android.provider.Telephony.SECRET_CODE") return
        if (intent.data?.host != SECRET_CODE) return

        val launchIntent = Intent(context, MainActivity::class.java).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
        }
        runCatching { context.startActivity(launchIntent) }
    }

    private companion object {
        const val SECRET_CODE = "5264"
    }
}