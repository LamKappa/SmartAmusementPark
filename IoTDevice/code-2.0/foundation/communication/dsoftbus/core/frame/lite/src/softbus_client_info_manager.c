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

#include "softbus_client_info_manager.h"

#include "securec.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_mem_interface.h"
#include "softbus_utils.h"

typedef struct {
    ListNode node;
    char name[PKG_NAME_SIZE_MAX]; /* softbus client name */
    unsigned int handle; /* use for small system device */
    unsigned int token; /* use for small system device */
    unsigned int cookie; /* use for small system device */
    void *ipcCtx; /* use for small system device */
} SoftBusClientInfoNode;

static SoftBusList *g_clientInfoList = NULL;

static SoftBusList *SERVER_InitClientInfoManager(void)
{
    if (g_clientInfoList != NULL) {
        LOG_INFO("has inited");
        return g_clientInfoList;
    }

    g_clientInfoList = CreateSoftBusList();
    if (g_clientInfoList == NULL) {
        LOG_ERR("init service info list failed");
        return NULL;
    }

    if (pthread_mutex_init(&g_clientInfoList->lock, NULL) != 0) {
        SoftBusFree(g_clientInfoList);
        g_clientInfoList = NULL;
        return NULL;
    }

    ListInit(&g_clientInfoList->list);
    return g_clientInfoList;
}

SoftBusClientMgr *SERVER_InitClient(void)
{
    SoftBusClientMgr *clientMgr = (SoftBusClientMgr *)SoftBusMalloc(sizeof(SoftBusClientMgr));
    if (clientMgr == NULL) {
        LOG_ERR("malloc client mgr failed");
        return NULL;
    }

    clientMgr->clientInfoList = SERVER_InitClientInfoManager();
    if (clientMgr->clientInfoList == NULL) {
        LOG_ERR("init client mgr failed");
        SoftBusFree(clientMgr);
        return NULL;
    }

    LOG_INFO("init success");
    return clientMgr;
}

int SERVER_RegisterService(const char *name, const struct CommonScvId *svcId)
{
    if (name == NULL || svcId == NULL) {
        LOG_ERR("invalid param");
        return SOFTBUS_ERR;
    }
    LOG_INFO("new client register:%s", name);

    if (g_clientInfoList == NULL) {
        LOG_ERR("not init");
        return SOFTBUS_ERR;
    }

    SoftBusClientInfoNode *clientInfo = (SoftBusClientInfoNode *)SoftBusMalloc(sizeof(SoftBusClientInfoNode));
    if (clientInfo == NULL) {
        LOG_ERR("malloc failed");
        return SOFTBUS_ERR;
    }
    (void)memset_s(clientInfo, sizeof(SoftBusClientInfoNode), 0, sizeof(SoftBusClientInfoNode));

    if (strcpy_s(clientInfo->name, sizeof(clientInfo->name), name) != EOK) {
        LOG_ERR("strcpy failed");
        SoftBusFree(clientInfo);
        return SOFTBUS_ERR;
    }

    clientInfo->handle = svcId->handle;
    clientInfo->token = svcId->token;
    clientInfo->cookie = svcId->cookie;
    clientInfo->ipcCtx = svcId->ipcCtx;
    ListInit(&clientInfo->node);

    if (pthread_mutex_lock(&g_clientInfoList->lock) != 0) {
        LOG_ERR("lock failed");
        SoftBusFree(clientInfo);
        return SOFTBUS_ERR;
    }

    ListAdd(&(g_clientInfoList->list), &(clientInfo->node));
    g_clientInfoList->cnt++;

    (void)pthread_mutex_unlock(&g_clientInfoList->lock);
    return SOFTBUS_OK;
}

int SERVER_GetIdentityByPkgName(const char *name, struct CommonScvId *svcId)
{
    if (name == NULL || svcId == NULL) {
        LOG_ERR("invalid param");
        return SOFTBUS_ERR;
    }

    if (g_clientInfoList == NULL) {
        LOG_ERR("not init");
        return SOFTBUS_ERR;
    }

    if (pthread_mutex_lock(&g_clientInfoList->lock) != 0) {
        LOG_ERR("lock failed");
        return SOFTBUS_ERR;
    }

    SoftBusClientInfoNode *clientInfo = NULL;
    LIST_FOR_EACH_ENTRY(clientInfo, &g_clientInfoList->list, SoftBusClientInfoNode, node) {
        if (strcmp(clientInfo->name, name) == 0) {
            svcId->handle = clientInfo->handle;
            svcId->token = clientInfo->token;
            svcId->cookie = clientInfo->cookie;
            svcId->ipcCtx = clientInfo->ipcCtx;
            (void)pthread_mutex_unlock(&g_clientInfoList->lock);
            return SOFTBUS_OK;
        }
    }

    LOG_ERR("not found");
    (void)pthread_mutex_unlock(&g_clientInfoList->lock);
    return SOFTBUS_ERR;
}

int SERVER_UnregisterService(const char *name)
{
    return SOFTBUS_OK;
}
