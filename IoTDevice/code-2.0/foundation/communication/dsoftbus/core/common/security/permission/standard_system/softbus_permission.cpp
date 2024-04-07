/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "softbus_permission.h"

#include <sys/types.h>
#include <unistd.h>

#include "bundle_mgr_interface.h"
#include "ipc_skeleton.h"
#include "permission/permission.h"
#include "permission/permission_kit.h"
#include "permission_entry.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_mem_interface.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::Security::Permission;

namespace {
    const std::string SYSTEM_APP_PERMISSION = "com.huawei.permission.MANAGE_DISTRIBUTED_PERMISSION";
    const std::string DANGER_APP_PERMISSION = "ohos.permission.DISTRIBUTED_DATASYNC";
    const std::string BIND_DISCOVER_SERVICE = "com.huawei.hwddmp.permission.BIND_DISCOVER_SERVICE";
    const int32_t SYSTEM_UID = 1000;
    const int32_t SHELL_UID = 2000;
    const int32_t INVALID_UID = -1;
    const int32_t FIRST_APPLICATION_UID = 10000;
    const int32_t MULTE_USER_RADIX = 100000;

#ifdef PERMISSION_TEST
    const int32_t TEST_UID = 1000;
    const int32_t TEST_PID = 1;
#endif

    int32_t CheckSystemPermission(const std::string &pkgName, const std::string &permission)
    {
        auto bundleObj =
        OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (bundleObj == nullptr) {
            LOG_ERR("failed to get bundle manager service");
            return SOFTBUS_ERR;
        }
        OHOS::sptr<IBundleMgr> bmgr = OHOS::iface_cast<IBundleMgr>(bundleObj);
        return bmgr->CheckPermission(std::string(pkgName), std::string(permission));
    }

    int32_t GetPermType(int32_t callingUid, int32_t callingPid, const char *pkgName)
    {
        std::string pkg;
        if (pkgName != nullptr) {
            pkg = std::string(pkgName);
        }
        if (callingUid == SHELL_UID) {
            return SOFTBUS_PERMISSION_DENIED;
        }
        if (callingUid == static_cast<int32_t>(getuid()) &&
            callingPid == static_cast<int32_t>(getpid())) {
            return SELF_APP;
        }
        int32_t ret = CheckSystemPermission(pkg, SYSTEM_APP_PERMISSION);
        if (ret == PERMISSION_GRANTED) {
            LOG_INFO("system app");
            return SYSTEM_APP;
        }
        ret = CheckSystemPermission(pkg, DANGER_APP_PERMISSION);
        if (ret == PERMISSION_GRANTED) {
            LOG_INFO("normal app");
            return NORMAL_APP;
        }
        if (callingUid > INVALID_UID && callingUid < FIRST_APPLICATION_UID && callingUid != SHELL_UID) {
            LOG_INFO("native app");
            return NATIVE_APP;
        }
        return SOFTBUS_PERMISSION_DENIED;
    }
}

int32_t TransPermissionInit(const char *fileName)
{
    if (fileName == nullptr) {
        return SOFTBUS_PERMISSION_DENIED;
    }
    return LoadPermissionJson(fileName);
}

void TransPermissionDeinit(void)
{
    DeinitPermissionJson();
}

int32_t CheckTransPermission(const char *sessionName, const char *pkgName, uint32_t actions)
{
    int32_t ret;
#ifdef PERMISSION_TEST
    int32_t callingUid = TEST_UID;
    int32_t callingPid = TEST_PID;
#else
    int32_t callingUid = OHOS::IPCSkeleton::GetCallingUid();
    int32_t callingPid = OHOS::IPCSkeleton::GetCallingPid();
#endif
    int32_t permType = GetPermType(callingUid, callingPid, pkgName);
    if (permType < 0) {
        return permType;
    }
    if (sessionName == nullptr && pkgName == nullptr) {
        return permType;
    }
    if (sessionName == nullptr && pkgName != nullptr) {
        if (IsValidPkgName(callingUid, pkgName) == SOFTBUS_OK) {
            return permType;
        }
        return SOFTBUS_PERMISSION_DENIED;
    }
    SoftBusPermissionItem *pItem = CreatePermissionItem(permType, callingUid, callingPid, pkgName, actions);
    if (pItem == nullptr) {
        return SOFTBUS_MALLOC_ERR;
    }
    ret = CheckPermissionEntry(sessionName, pItem);
    SoftBusFree(pItem);
    return ret;
}

bool CheckDiscPermission(const char *pkgName)
{
    std::string pkg;
    if (pkgName != nullptr) {
        pkg = std::string(pkgName);
    } else {
        return false;
    }
#ifdef PERMISSION_TEST
    int32_t callingUid = TEST_UID;
#else
    int32_t callingUid = OHOS::IPCSkeleton::GetCallingUid();
#endif
    if (callingUid == SYSTEM_UID || callingUid % MULTE_USER_RADIX == SYSTEM_UID) {
        return true;
    }
    if (CheckSystemPermission(pkg, BIND_DISCOVER_SERVICE) == PERMISSION_GRANTED) {
        return true;
    }
    return false;
}

bool CheckBusCenterPermission(const char *pkgName)
{
    std::string pkg;
    if (pkgName != nullptr) {
        pkg = std::string(pkgName);
    } else {
        return false;
    }

#ifdef PERMISSION_TEST
    int32_t callingUid = TEST_UID;
#else
    int32_t callingUid = OHOS::IPCSkeleton::GetCallingUid();
#endif
    if (callingUid == SYSTEM_UID || callingUid % MULTE_USER_RADIX == SYSTEM_UID) {
        return true;
    }
    if (CheckSystemPermission(pkg, SYSTEM_APP_PERMISSION) == PERMISSION_GRANTED) {
        return true;
    }
    if (CheckSystemPermission(pkg, DANGER_APP_PERMISSION) == PERMISSION_GRANTED) {
        return true;
    }
    return false;
}