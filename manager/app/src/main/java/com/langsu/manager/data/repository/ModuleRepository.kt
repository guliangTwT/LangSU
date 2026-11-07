package com.langsu.manager.data.repository

import com.langsu.manager.data.model.Module
import com.langsu.manager.data.model.ModuleUpdateInfo

interface ModuleRepository {
    suspend fun getModules(): Result<List<Module>>
    suspend fun checkUpdate(module: Module): Result<ModuleUpdateInfo>
}
