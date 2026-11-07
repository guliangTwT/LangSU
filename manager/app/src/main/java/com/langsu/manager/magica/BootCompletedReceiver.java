package com.langsu.manager.magica;

import static com.langsu.manager.magica.AppZygotePreload.TAG;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.langsu.manager.BuildConfig;
import com.langsu.manager.ui.util.LangSuCliKt;

public class BootCompletedReceiver extends BroadcastReceiver {

    private static final String ACTION_LAUNCH = BuildConfig.APPLICATION_ID + ".magica.LAUNCH";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null) {
            return;
        }
        var action = intent.getAction();
        if (!Intent.ACTION_LOCKED_BOOT_COMPLETED.equals(action)
                && !Intent.ACTION_BOOT_COMPLETED.equals(action)
                && !ACTION_LAUNCH.equals(action)) {
            return;
        }
        if (LangSuCliKt.rootAvailable()) return;
        try {
            context.startService(new Intent(context, MagicaService.class));
            Log.i(TAG, "MagicaService started from boot action: " + action);
        } catch (Throwable e) {

            Log.e(TAG, "Failed to start MagicaService from boot action: " + action, e);
        }
    }
}
