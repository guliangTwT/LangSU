package com.langsu.manager.ui

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import com.langsu.manager.R

/**
 * Secret launcher entry used when the launcher icon has been hidden.
 *
 * Dial `*#*#5264#*#*` on the dial pad to bring LangSU back. The receiver only reacts
 * to its own secret code and never exposes any privileged operation.
 *
 * A broadcast receiver cannot simply call `startActivity`: since Android 10 that is a
 * blocked background activity launch (`BAL_BLOCK`, reproducible with
 * `am broadcast -a android.provider.Telephony.SECRET_CODE -d android_secret_code://5264`).
 * So this does three things, cheapest first:
 *
 *  1. try `startActivity` anyway - it succeeds when the app holds a BAL exemption
 *     (e.g. "display over other apps" granted),
 *  2. re-enable the launcher alias, so the icon is back and the app is reachable,
 *  3. post a notification whose tap is an allow-listed user gesture.
 *
 * Step 2 is the guaranteed path; it only happens on an explicit user action.
 */
class SecretCodeReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action != ACTION_SECRET_CODE) return
        if (intent.data?.host != SECRET_CODE) return

        runCatching { startMainActivity(context) }
        runCatching { restoreLauncherIcon(context) }
        runCatching { postUnlockNotification(context) }
    }

    private fun startMainActivity(context: Context) {
        val launch = Intent(context, MainActivity::class.java).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
        }
        context.startActivity(launch)
    }

    private fun restoreLauncherIcon(context: Context) {
        context.packageManager.setComponentEnabledSetting(
            ComponentName(context.packageName, LAUNCHER_ALIAS_CLASS),
            PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
            PackageManager.DONT_KILL_APP
        )
    }

    private fun postUnlockNotification(context: Context) {
        val manager = context.getSystemService(NotificationManager::class.java) ?: return
        manager.createNotificationChannel(
            NotificationChannel(
                CHANNEL_ID,
                context.getString(R.string.secret_entry_channel_name),
                NotificationManager.IMPORTANCE_HIGH
            )
        )

        val launch = Intent(context, MainActivity::class.java).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
        }
        val pending = PendingIntent.getActivity(
            context,
            REQUEST_CODE,
            launch,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val notification = NotificationCompat.Builder(context, CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_notification)
            .setContentTitle(context.getString(R.string.secret_entry_title))
            .setContentText(context.getString(R.string.secret_entry_text))
            .setContentIntent(pending)
            .setAutoCancel(true)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .build()

        NotificationManagerCompat.from(context).notify(NOTIFICATION_ID, notification)
    }

    private companion object {
        const val ACTION_SECRET_CODE = "android.provider.Telephony.SECRET_CODE"
        const val SECRET_CODE = "5264"
        const val CHANNEL_ID = "secret_entry"
        const val NOTIFICATION_ID = 5264
        const val REQUEST_CODE = 5264

        // Must match the alias declared in AndroidManifest.xml.
        const val LAUNCHER_ALIAS_CLASS = "com.langsu.manager.ui.LauncherActivity"
    }
}