package com.langsu.manager.magica;

import android.app.ZygotePreload;
import android.content.pm.ApplicationInfo;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.File;

public class AppZygotePreload implements ZygotePreload {
    public static final String TAG = "LangSUMagica";

    private static native void forkDontCareAndExecLangsud(String langsudPath, String packageName);

    @Override
    public void doPreload(@NonNull ApplicationInfo appInfo) {
        File f = new File(appInfo.nativeLibraryDir, "liblangsud.so");
        try {
            System.loadLibrary("langsu");
            Log.d(TAG, "executing magica ...");
            forkDontCareAndExecLangsud(f.getAbsolutePath(), appInfo.packageName);
        } catch (Throwable t) {
            Log.e(TAG, "failed to late load", t);
        }
    }
}
