package com.langsu.manager.data.repository

import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.content.pm.ApplicationInfo
import android.content.pm.PackageInfo
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.SystemClock
import android.util.Log
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ipc.RootService
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withContext
import com.langsu.manager.ILangSuInterface
import com.langsu.manager.Natives
import com.langsu.manager.data.model.AppInfo
import com.langsu.manager.data.model.WEBVIEW_ZYGOTE_PROFILE_KEY
import com.langsu.manager.data.model.WEBVIEW_ZYGOTE_UID
import com.langsu.manager.langsuApp
import com.langsu.manager.ui.LangSuService
import com.langsu.manager.ui.util.LangSuCli
import kotlin.coroutines.resume

class SuperUserRepositoryImpl : SuperUserRepository {

    companion object {
        private const val TAG = "SuperUserRepository"
    }

    override suspend fun getAppList(): Result<Pair<List<AppInfo>, List<Int>>> = withContext(Dispatchers.IO) {
        runCatching {
            if (!LangSuCli.SHELL.isRoot) {
                return@withContext Result.failure(
                    IllegalStateException("Root access is required")
                )
            }

            val result = connectLangSuService {
                Log.w(TAG, "LangSuService disconnected")
            }

            var currentBinder = result.first
            var currentConnection = result.second

            try {
                suspend fun reconnect(): ILangSuInterface {
                    withContext(Dispatchers.Main) {
                        RootService.unbind(currentConnection)
                    }
                    val retry = connectLangSuService { Log.w(TAG, "LangSuService disconnected") }
                    currentBinder = retry.first
                    currentConnection = retry.second
                    return ILangSuInterface.Stub.asInterface(currentBinder)
                }

                val pm = langsuApp.packageManager
                val start = SystemClock.elapsedRealtime()

                var iface = ILangSuInterface.Stub.asInterface(currentBinder)
                val idsArray = try {
                    iface.userIds
                } catch (_: Exception) {
                    iface = reconnect()
                    iface.userIds
                }

                val slice = try {
                    iface.getPackages(0)
                } catch (_: Exception) {
                    iface = reconnect()
                    iface.getPackages(0)
                }

                val packages = slice.list
                val newApps = packages.filter {
                    val ai = it.applicationInfo ?: return@filter false
                    ai.uid != WEBVIEW_ZYGOTE_UID &&
                            (ai.flags and ApplicationInfo.FLAG_HAS_CODE) != 0
                }.map {
                    val appInfo = it.applicationInfo!!
                    val profile = Natives.getAppProfile(it.packageName, appInfo.uid)
                    AppInfo(
                        label = appInfo.loadLabel(pm).toString(),
                        packageInfo = it,
                        profile = profile,
                    )
                }.toMutableList()

                // WebView Zygote is a single system UID, not a per-user package. Reuse the system icon.
                val systemInfo = ApplicationInfo(pm.getApplicationInfo("android", 0)).apply {
                    uid = WEBVIEW_ZYGOTE_UID
                }
                val placeholder = PackageInfo().apply {
                    packageName = ""
                    applicationInfo = systemInfo
                }
                newApps += AppInfo(
                    label = "WebView Zygote",
                    packageInfo = placeholder,
                    profile = Natives.getAppProfile(WEBVIEW_ZYGOTE_PROFILE_KEY, WEBVIEW_ZYGOTE_UID),
                    profileKey = WEBVIEW_ZYGOTE_PROFILE_KEY,
                    special = true,
                )

                Log.i(TAG, "load cost: ${SystemClock.elapsedRealtime() - start}")
                Pair(newApps, idsArray.toList())
            } finally {
                withContext(Dispatchers.Main) {
                    RootService.unbind(currentConnection)
                }
            }
        }
    }

    override suspend fun refreshProfiles(currentApps: List<AppInfo>): Result<List<AppInfo>> = withContext(Dispatchers.IO) {
        runCatching {
            if (currentApps.isEmpty()) return@runCatching emptyList()

            currentApps.map {
                val profile = Natives.getAppProfile(it.profileKey, it.uid)
                it.copy(profile = profile)
            }
        }
    }

    private suspend inline fun connectLangSuService(
        crossinline onDisconnect: () -> Unit = {}
    ): Pair<IBinder, ServiceConnection> = withContext(Dispatchers.Main) {
        suspendCancellableCoroutine { cont ->
            val connection = object : ServiceConnection {
                override fun onServiceDisconnected(name: ComponentName?) {
                    onDisconnect()
                }

                override fun onServiceConnected(name: ComponentName?, binder: IBinder?) {
                    if (cont.isActive) {
                        cont.resume(binder as IBinder to this)
                    }
                }
            }

            cont.invokeOnCancellation {
                if (Looper.myLooper() == Looper.getMainLooper()) {
                    RootService.unbind(connection)
                } else {
                    Handler(Looper.getMainLooper()).post {
                        RootService.unbind(connection)
                    }
                }
            }

            val intent = Intent(langsuApp, LangSuService::class.java)

            val task = RootService.bindOrTask(
                intent,
                Shell.EXECUTOR,
                connection,
            )
            val shell = LangSuCli.SHELL
            task?.let { shell.execTask(it) }
        }
    }
}
