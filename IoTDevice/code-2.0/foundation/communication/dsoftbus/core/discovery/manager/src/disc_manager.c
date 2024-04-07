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

#include "disc_manager.h"
#include "common_list.h"
#include "disc_coap.h"
#include "securec.h"
#include "softbus.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_mem_interface.h"
#include "softbus_utils.h"

#define DISCOVERY_CB_LEN (sizeof(IServerDiscoveryCallback))
#define PUBLISH_CB_LEN   (sizeof(IServerPublishCallback))

static bool g_isInited = false;
static SoftBusList *g_publishInfoList = NULL;
static SoftBusList *g_discoveryInfoList = NULL;
static DiscInnerCallback g_discMgrMediumCb = { DiscOnDeviceFound };
static DiscoveryFuncInterface *g_discCoapInterface = NULL;
static DiscoveryFuncInterface *g_discBleInterface = NULL;
static ListNode g_capabilityList[CAPABILITY_MAX_BITNUM];
static const char *g_discModuleMap[] = {
    "MODULE_LNN",
    "MODULE_CONN",
};

typedef enum {
    MIN_SERVICE = 0,
    PUBLISH_SERVICE = MIN_SERVICE,
    PUBLISH_INNER_SERVICE = 1,
    SUBSCRIBE_SERVICE = 2,
    SUBSCRIBE_INNER_SERVICE = 3,
    MAX_SERVICE = SUBSCRIBE_INNER_SERVICE,
} ServiceType;

typedef enum {
    PUBLISH_FUNC = 0,
    UNPUBLISH_FUNC = 1,
    STARTDISCOVERTY_FUNC = 2,
    STOPDISCOVERY_FUNC = 3
} InterfaceFuncType;

typedef union {
    PublishOption publishOption;
    SubscribeOption subscribeOption;
} InnerOption;

typedef union  {
    IServerPublishCallback publishCb;
    IServerDiscoveryCallback subscribeCb;
    DiscInnerCallback innerCb;
} InnerCallback;

typedef struct {
    ListNode node;
    char packageName[PKG_NAME_SIZE_MAX];
    InnerCallback callback;
    uint32_t infoNum;
    ListNode InfoList;
} DiscItem;

typedef struct {
    ListNode node;
    int32_t id;
    DiscoverMode mode;
    ExchanageMedium medium;
    InnerOption option;
    ListNode capNode;
    DiscItem *item;
} DiscInfo;

void __attribute__ ((weak)) DiscBleDeinit(void)
{
    return;
}

DiscoveryFuncInterface __attribute__ ((weak)) *DiscBleInit(DiscInnerCallback *bleCb)
{
    (void)bleCb;
    return NULL;
}

static void BitmapSet(uint32_t *bitMap, const uint32_t pos)
{
    if (bitMap == NULL || pos > CAPABILITY_MAX_BITNUM) {
        return;
    }
    *bitMap |= 1U << pos;
}

static bool IsBitmapSet(uint32_t *bitMap, const uint32_t pos)
{
    if (bitMap == NULL || pos > CAPABILITY_MAX_BITNUM) {
        return false;
    }
    bool flag = ((1U << pos) & (*bitMap)) ? true : false;
    return flag;
}

static int32_t DiscInterfaceProcess(const InnerOption *option, const DiscoveryFuncInterface *interface,
    const DiscoverMode mode, InterfaceFuncType type)
{
    if (interface == NULL) {
        return SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL;
    }

    switch (type) {
        case PUBLISH_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->Publish(&(option->publishOption))) :
                (interface->StartScan(&(option->publishOption))));
        case UNPUBLISH_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->Unpublish(&(option->publishOption))) :
                (interface->StopScan(&(option->publishOption))));
        case STARTDISCOVERTY_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->StartAdvertise(&(option->subscribeOption))) :
                (interface->Subscribe(&(option->subscribeOption))));
        case STOPDISCOVERY_FUNC:
            return ((mode == DISCOVER_MODE_ACTIVE) ? (interface->StopAdvertise(&(option->subscribeOption))) :
                (interface->Unsubscribe(&(option->subscribeOption))));
        default:
            return SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL;
    }
}

static int32_t DiscInterfaceByMedium(const DiscInfo *info, const InterfaceFuncType type)
{
    switch (info->medium) {
        case BLE:
            return DiscInterfaceProcess(&(info->option), g_discBleInterface, info->mode, type);
        case COAP:
            return DiscInterfaceProcess(&(info->option), g_discCoapInterface, info->mode, type);
        case AUTO: {
            int32_t ret1 = DiscInterfaceProcess(&(info->option), g_discBleInterface, info->mode, type);
            int32_t ret2 = DiscInterfaceProcess(&(info->option), g_discCoapInterface, info->mode, type);
            if ((ret1 == SOFTBUS_OK) || (ret2 == SOFTBUS_OK)) {
                return SOFTBUS_OK;
            }
            return SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL;
        }
        default:
            return SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL;
    }
}

static int32_t CapabilityStringToBimap(const char *capability)
{
    for (int32_t i = 0; i < CAPABILITY_MAX_BITNUM; i++) {
        if (strcmp(capability, g_capabilityMap[i].capability) == 0) {
            return g_capabilityMap[i].bitmap;
        }
    }
    return SOFTBUS_DISCOVER_MANAGER_CAPABILITY_INVALID;
}

static void AddInfoToCapability(DiscInfo *info, const ServiceType type)
{
    if ((type != SUBSCRIBE_SERVICE) && (type != SUBSCRIBE_INNER_SERVICE)) {
        return;
    }

    uint32_t tmp;
    for (tmp = 0; tmp < CAPABILITY_MAX_BITNUM; tmp++) {
        if (IsBitmapSet(&(info->option.subscribeOption.capabilityBitmap[0]), tmp) == true) {
            break;
        }
    }

    if (type == SUBSCRIBE_INNER_SERVICE) {
        ListNodeInsert(&(g_capabilityList[tmp]), &(info->capNode));
    }

    if (type == SUBSCRIBE_SERVICE) {
        ListTailInsert(&(g_capabilityList[tmp]), &(info->capNode));
    }

    return;
}

static void DeleteInfoFromCapability(DiscInfo *info, const ServiceType type)
{
    if ((type != SUBSCRIBE_SERVICE) && (type != SUBSCRIBE_INNER_SERVICE)) {
        return;
    }
    ListDelete(&(info->capNode));
    return;
}

static void ReleaseInfoNodeMem(DiscInfo *info, const ServiceType type)
{
    if ((type < MIN_SERVICE) || (type > MAX_SERVICE)) {
        LOG_ERR("type erro");
        return;
    }
    if ((type == PUBLISH_SERVICE) || (type == PUBLISH_INNER_SERVICE)) {
        SoftBusFree(info->option.publishOption.capabilityData);
    }

    if ((type == SUBSCRIBE_SERVICE) || (type == SUBSCRIBE_INNER_SERVICE)) {
        SoftBusFree(info->option.subscribeOption.capabilityData);
    }
    SoftBusFree(info);
    return;
}

static void InnerDeviceFound(const DiscInfo *infoNode, const DeviceInfo *device)
{
    uint32_t tmp;
    bool isInnerInfo = false;
    for (tmp = 0; tmp < MODULE_MAX; tmp++) {
        if (strcmp(infoNode->item->packageName, g_discModuleMap[tmp]) != 0) {
            continue;
        }
        isInnerInfo = true;
    }
    if (isInnerInfo == false) {
        infoNode->item->callback.subscribeCb.OnServerDeviceFound(infoNode->item->packageName, device);
        return;
    }
    if (infoNode->item->callback.innerCb.OnDeviceFound == NULL) {
        LOG_ERR("OnDeviceFound not regist");
        return;
    }
    infoNode->item->callback.innerCb.OnDeviceFound(device);
}

void DiscOnDeviceFound(const DeviceInfo *device)
{
    uint32_t tmp;
    DiscInfo *infoNode = NULL;
    for (tmp = 0; tmp < CAPABILITY_MAX_BITNUM; tmp++) {
        if (IsBitmapSet((uint32_t *)&(device->capabilityBitmap[0]), tmp) == false) {
            continue;
        }
        LIST_FOR_EACH_ENTRY(infoNode, &(g_capabilityList[tmp]), DiscInfo, capNode) {
            LOG_INFO("find callback:id = %d", infoNode->id);
            InnerDeviceFound(infoNode, device);
        }
    }
    return;
}

static int32_t UpdateItemCallback(DiscItem *item, const InnerCallback *cb, const ServiceType type)
{
    if ((type != PUBLISH_SERVICE) && (type != SUBSCRIBE_SERVICE)) {
        return SOFTBUS_OK;
    }

    if ((type == PUBLISH_SERVICE) &&
        (memcpy_s(&(item->callback.publishCb), PUBLISH_CB_LEN, cb, PUBLISH_CB_LEN) != EOK)) {
        LOG_ERR("memcpy_s failed");
        return SOFTBUS_MEM_ERR;
    }
    if ((type == SUBSCRIBE_SERVICE) &&
        (memcpy_s(&(item->callback.subscribeCb), DISCOVERY_CB_LEN, cb, DISCOVERY_CB_LEN) != EOK)) {
        LOG_ERR("memcpy_s failed");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t PublishInfoCheck(const char *packageName, const PublishInfo *info)
{
    if (strlen(packageName) >= PKG_NAME_SIZE_MAX) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->mode != DISCOVER_MODE_PASSIVE) && (info->mode != DISCOVER_MODE_ACTIVE)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->medium < AUTO) || (info->medium > COAP)) {
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM;
    }

    if ((info->freq < LOW) || (info->freq > SUPER_HIGH)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->capabilityData == NULL) && (info->dataLen != 0)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (info->dataLen == 0) {
        return SOFTBUS_OK;
    }

    if ((info->dataLen > MAX_CAPABILITYDATA_LEN) ||
        (strlen((char *)(info->capabilityData)) >= MAX_CAPABILITYDATA_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }

    return SOFTBUS_OK;
}

static int32_t PublishInnerInfoCheck(const PublishInnerInfo *info)
{
    if ((info->medium < AUTO) || (info->medium > COAP)) {
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM;
    }

    if ((info->freq < LOW) || (info->freq > SUPER_HIGH)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->capabilityData == NULL) && (info->dataLen != 0)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (info->dataLen == 0) {
        return SOFTBUS_OK;
    }

    if ((info->dataLen > MAX_CAPABILITYDATA_LEN) ||
        (strlen((char *)(info->capabilityData)) >= MAX_CAPABILITYDATA_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }

    return SOFTBUS_OK;
}

static int32_t SubscribeInfoCheck(const char *packageName, const SubscribeInfo *info)
{
    if (strlen(packageName) >= PKG_NAME_SIZE_MAX) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->mode != DISCOVER_MODE_PASSIVE) && (info->mode != DISCOVER_MODE_ACTIVE)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->medium < AUTO) || (info->medium > COAP)) {
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM;
    }

    if ((info->freq < LOW) || (info->freq > SUPER_HIGH)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->capabilityData == NULL) && (info->dataLen != 0)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (info->dataLen == 0) {
        return SOFTBUS_OK;
    }

    if ((info->dataLen > MAX_CAPABILITYDATA_LEN) ||
        (strlen((char *)(info->capabilityData)) >= MAX_CAPABILITYDATA_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }

    return SOFTBUS_OK;
}

static int32_t SubscribeInnerInfoCheck(const SubscribeInnerInfo *info)
{
    if ((info->medium < AUTO) || (info->medium > COAP)) {
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM;
    }

    if ((info->freq < LOW) || (info->freq > SUPER_HIGH)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->capabilityData == NULL) && (info->dataLen != 0)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (info->dataLen == 0) {
        return SOFTBUS_OK;
    }

    if ((info->dataLen > MAX_CAPABILITYDATA_LEN) ||
        (strlen((char *)(info->capabilityData)) >= MAX_CAPABILITYDATA_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }

    return SOFTBUS_OK;
}

static DiscItem *CreateNewItem(SoftBusList *serviceList, const char *packageName, const InnerCallback *cb,
    const ServiceType type)
{
    if ((type < MIN_SERVICE) || (type > MAX_SERVICE)) {
        LOG_ERR("type erro");
        return NULL;
    }
    DiscItem *itemNode = (DiscItem *)SoftBusCalloc(sizeof(DiscItem));
    if (itemNode == NULL) {
        LOG_ERR("calloc itemNode failed");
        return NULL;
    }
    itemNode->infoNum = 0;
    if ((type == PUBLISH_INNER_SERVICE) || (type == SUBSCRIBE_INNER_SERVICE)) {
        ListNodeInsert(&(serviceList->list), &(itemNode->node));
    }

    if ((type == PUBLISH_SERVICE) || (type == SUBSCRIBE_SERVICE)) {
        ListTailInsert(&(serviceList->list), &(itemNode->node));
    }
    if ((type == PUBLISH_SERVICE) &&
        ((memcpy_s(&(itemNode->callback.publishCb), PUBLISH_CB_LEN, cb, PUBLISH_CB_LEN) != EOK) ||
        (memcpy_s(itemNode->packageName, PKG_NAME_SIZE_MAX, packageName, PKG_NAME_SIZE_MAX) != EOK))) {
        LOG_ERR("memcpy_s failed");
        SoftBusFree(itemNode);
        return NULL;
    }
    if ((type == SUBSCRIBE_SERVICE) &&
        ((memcpy_s(&(itemNode->callback.subscribeCb), DISCOVERY_CB_LEN, cb, DISCOVERY_CB_LEN) != EOK) ||
        (memcpy_s(itemNode->packageName, PKG_NAME_SIZE_MAX, packageName, PKG_NAME_SIZE_MAX) != EOK))) {
        LOG_ERR("memcpy_s failed");
        SoftBusFree(itemNode);
        return NULL;
    }
    if (((type == PUBLISH_INNER_SERVICE) || (type == SUBSCRIBE_INNER_SERVICE)) &&
        (memcpy_s(itemNode->packageName, PKG_NAME_SIZE_MAX, packageName, PKG_NAME_SIZE_MAX) != EOK)) {
        LOG_ERR("memcpy_s failed");
        SoftBusFree(itemNode);
        return NULL;
    }
    if (type == SUBSCRIBE_INNER_SERVICE) {
        itemNode->callback.innerCb.OnDeviceFound = cb->innerCb.OnDeviceFound;
    }
    serviceList->cnt++;
    ListInit(&(itemNode->InfoList));
    return itemNode;
}

static DiscInfo *CreateNewPublishInfoNode(const PublishInfo *info)
{
    int32_t ret;
    DiscInfo *infoNode = (DiscInfo *)SoftBusCalloc(sizeof(DiscInfo));
    if (infoNode == NULL) {
        LOG_ERR("calloc infoNode failed");
        return NULL;
    }
    ListInit(&(infoNode->node));
    ListInit(&(infoNode->capNode));
    infoNode->item = NULL;
    infoNode->id = info->publishId;
    infoNode->medium = info->medium;
    infoNode->mode = info->mode;
    infoNode->option.publishOption.freq = info->freq;
    infoNode->option.publishOption.dataLen = info->dataLen;
    infoNode->option.publishOption.capabilityData =
        (unsigned char *)SoftBusCalloc(MAX_CAPABILITYDATA_LEN * sizeof(unsigned char));
    if (infoNode->option.publishOption.capabilityData == NULL) {
        LOG_ERR("calloc capabilityData failed");
        SoftBusFree(infoNode);
        return NULL;
    }
    if ((info->dataLen != 0) && (memcpy_s(infoNode->option.publishOption.capabilityData, MAX_CAPABILITYDATA_LEN,
        info->capabilityData, info->dataLen) != EOK)) {
        LOG_ERR("memcpy_s fail");
        ReleaseInfoNodeMem(infoNode, PUBLISH_SERVICE);
        return NULL;
    }
    ret = CapabilityStringToBimap(info->capability);
    if (ret < 0) {
        LOG_ERR("capability not found");
        ReleaseInfoNodeMem(infoNode, PUBLISH_SERVICE);
        return NULL;
    }
    BitmapSet(&(infoNode->option.publishOption.capabilityBitmap[0]), ret);
    return infoNode;
}

static DiscInfo *CreateNewSubscribeInfoNode(const SubscribeInfo *info)
{
    int32_t ret;
    DiscInfo *infoNode = (DiscInfo *)SoftBusCalloc(sizeof(DiscInfo));
    if (infoNode == NULL) {
        LOG_ERR("calloc infoNode failed");
        return NULL;
    }
    ListInit(&(infoNode->node));
    ListInit(&(infoNode->capNode));
    infoNode->item = NULL;
    infoNode->id = info->subscribeId;
    infoNode->medium = info->medium;
    infoNode->mode = info->mode;
    infoNode->option.subscribeOption.freq = info->freq;
    infoNode->option.subscribeOption.dataLen = info->dataLen;
    infoNode->option.subscribeOption.isSameAccount = info->isSameAccount;
    infoNode->option.subscribeOption.isWakeRemote = info->isWakeRemote;
    infoNode->option.subscribeOption.capabilityData =
        (unsigned char *)SoftBusCalloc(MAX_CAPABILITYDATA_LEN * sizeof(unsigned char));
    if (infoNode->option.subscribeOption.capabilityData == NULL) {
        LOG_ERR("calloc capabilityData failed");
        SoftBusFree(infoNode);
        return NULL;
    }
    if ((info->dataLen != 0) && (memcpy_s(infoNode->option.subscribeOption.capabilityData, MAX_CAPABILITYDATA_LEN,
        info->capabilityData, info->dataLen) != EOK)) {
        LOG_ERR("memcpy_s fail");
        ReleaseInfoNodeMem(infoNode, SUBSCRIBE_SERVICE);
        return NULL;
    }
    ret = CapabilityStringToBimap(info->capability);
    if (ret < 0) {
        LOG_ERR("capability not found");
        ReleaseInfoNodeMem(infoNode, SUBSCRIBE_SERVICE);
        return NULL;
    }
    BitmapSet(&(infoNode->option.subscribeOption.capabilityBitmap[0]), ret);
    return infoNode;
}

static DiscInfo *CreateNewPublishInnerInfoNode(const PublishInnerInfo *info, const DiscoverMode mode)
{
    int32_t ret;
    DiscInfo *infoNode = (DiscInfo *)SoftBusCalloc(sizeof(DiscInfo));
    if (infoNode == NULL) {
        LOG_ERR("calloc infoNode failed");
        return NULL;
    }
    ListInit(&(infoNode->node));
    ListInit(&(infoNode->capNode));
    infoNode->item = NULL;
    infoNode->id = info->publishId;
    infoNode->medium = info->medium;
    infoNode->mode = mode;
    infoNode->option.publishOption.freq = info->freq;
    infoNode->option.publishOption.dataLen = info->dataLen;
    infoNode->option.publishOption.capabilityData =
        (unsigned char *)SoftBusCalloc(MAX_CAPABILITYDATA_LEN * sizeof(unsigned char));
    if (infoNode->option.publishOption.capabilityData == NULL) {
        LOG_ERR("calloc capabilityData failed");
        SoftBusFree(infoNode);
        return NULL;
    }
    if ((info->dataLen != 0) && (memcpy_s(infoNode->option.publishOption.capabilityData, MAX_CAPABILITYDATA_LEN,
        info->capabilityData, info->dataLen) != EOK)) {
        LOG_ERR("memcpy_s fail");
        ReleaseInfoNodeMem(infoNode, PUBLISH_SERVICE);
        return NULL;
    }
    ret = CapabilityStringToBimap(info->capability);
    if (ret < 0) {
        LOG_ERR("capability not found");
        ReleaseInfoNodeMem(infoNode, PUBLISH_SERVICE);
        return NULL;
    }
    BitmapSet(&(infoNode->option.publishOption.capabilityBitmap[0]), ret);
    return infoNode;
}

static DiscInfo *CreateNewSubscribeInnerInfoNode(const SubscribeInnerInfo *info, const DiscoverMode mode)
{
    int32_t ret;
    DiscInfo *infoNode = (DiscInfo *)SoftBusCalloc(sizeof(DiscInfo));
    if (infoNode == NULL) {
        LOG_ERR("calloc infoNode failed");
        return NULL;
    }
    ListInit(&(infoNode->node));
    ListInit(&(infoNode->capNode));
    infoNode->item = NULL;
    infoNode->id = info->subscribeId;
    infoNode->medium = info->medium;
    infoNode->mode = mode;
    infoNode->option.subscribeOption.freq = info->freq;
    infoNode->option.subscribeOption.dataLen = info->dataLen;
    infoNode->option.subscribeOption.isSameAccount = info->isSameAccount;
    infoNode->option.subscribeOption.isWakeRemote = info->isWakeRemote;
    infoNode->option.subscribeOption.capabilityData =
        (unsigned char *)SoftBusCalloc(MAX_CAPABILITYDATA_LEN * sizeof(unsigned char));
    if (infoNode->option.subscribeOption.capabilityData == NULL) {
        LOG_ERR("calloc capabilityData failed");
        SoftBusFree(infoNode);
        return NULL;
    }
    if ((info->dataLen != 0) && (memcpy_s(infoNode->option.subscribeOption.capabilityData, MAX_CAPABILITYDATA_LEN,
        info->capabilityData, info->dataLen) != EOK)) {
        LOG_ERR("memcpy_s fail");
        ReleaseInfoNodeMem(infoNode, SUBSCRIBE_SERVICE);
        return NULL;
    }
    ret = CapabilityStringToBimap(info->capability);
    if (ret < 0) {
        LOG_ERR("capability not found");
        ReleaseInfoNodeMem(infoNode, SUBSCRIBE_SERVICE);
        return NULL;
    }
    BitmapSet(&(infoNode->option.subscribeOption.capabilityBitmap[0]), ret);
    return infoNode;
}

static int32_t AddInfoToList(SoftBusList *serviceList, const char *packageName, const InnerCallback *cb,
    DiscInfo *info, const ServiceType type)
{
    bool isPackageNameExist = false;
    DiscItem *itemNode = NULL;
    DiscInfo *infoNode = NULL;

    if (pthread_mutex_lock(&(serviceList->lock)) != 0) {
        LOG_ERR("lock failed");
        return SOFTBUS_LOCK_ERR;
    }

    LIST_FOR_EACH_ENTRY(itemNode, &(serviceList->list), DiscItem, node) {
        if (strcmp(itemNode->packageName, packageName) != 0) {
            continue;
        }
        LIST_FOR_EACH_ENTRY(infoNode, &(itemNode->InfoList), DiscInfo, node) {
            if (infoNode->id == info->id) {
                LOG_INFO("id already exsisted");
                (void)pthread_mutex_unlock(&(serviceList->lock));
                return SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM;
            }
        }
        if (UpdateItemCallback(itemNode, cb, type) != SOFTBUS_OK) {
            LOG_ERR("update item fail");
            (void)pthread_mutex_unlock(&(serviceList->lock));
            return SOFTBUS_MEM_ERR;
        }
        isPackageNameExist = true;
        itemNode->infoNum++;
        info->item = itemNode;
        ListTailInsert(&(itemNode->InfoList), &(info->node));
        AddInfoToCapability(info, type);
        break;
    }

    if (isPackageNameExist == false) {
        itemNode = CreateNewItem(serviceList, packageName, cb, type);
        if (itemNode == NULL) {
            LOG_ERR("itemNode create failed");
            (void)pthread_mutex_unlock(&(serviceList->lock));
            return SOFTBUS_DISCOVER_MANAGER_ITEM_NOT_CREATE;
        }
        itemNode->infoNum++;
        info->item = itemNode;
        ListTailInsert(&(itemNode->InfoList), &(info->node));
        AddInfoToCapability(info, type);
    }
    (void)pthread_mutex_unlock(&(serviceList->lock));
    return SOFTBUS_OK;
}

static DiscInfo *DeleteInfoFromList(SoftBusList *serviceList, const char *packageName, const int32_t id,
    const ServiceType type)
{
    if (pthread_mutex_lock(&(serviceList->lock)) != 0) {
        LOG_ERR("lock failed");
        return NULL;
    }

    bool isIdExist = false;
    DiscItem *itemNode = NULL;
    DiscInfo *infoNode = NULL;

    LIST_FOR_EACH_ENTRY(itemNode, &(serviceList->list), DiscItem, node) {
        if (strcmp(itemNode->packageName, packageName) != 0) {
            continue;
        }
        LIST_FOR_EACH_ENTRY(infoNode, &(itemNode->InfoList), DiscInfo, node) {
            if (infoNode->id != id) {
                continue;
            }
            isIdExist = true;
            itemNode->infoNum--;
            DeleteInfoFromCapability(infoNode, type);
            ListDelete(&(infoNode->node));
            if (itemNode->infoNum == 0) {
                serviceList->cnt--;
                ListDelete(&(itemNode->node));
                SoftBusFree(itemNode);
            }
            break;
        }
        break;
    }
    (void)pthread_mutex_unlock(&(serviceList->lock));
    if (isIdExist == false) {
        LOG_INFO("can not find publishId");
        return NULL;
    }
    return infoNode;
}

static int32_t InnerPublishService(const char *packageName, DiscInfo *info, const IServerPublishCallback *cb,
    const ServiceType type)
{
    int32_t ret;
    InnerCallback callback;

    if ((cb != NULL) &&
        (memcpy_s(&(callback.publishCb), PUBLISH_CB_LEN, cb, PUBLISH_CB_LEN) != EOK)) {
        LOG_ERR("memcpy_s erro");
        return SOFTBUS_MEM_ERR;
    }

    ret = AddInfoToList(g_publishInfoList, packageName, &callback, info, type);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("add list fail");
        return ret;
    }
    ret = DiscInterfaceByMedium(info, PUBLISH_FUNC);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("interface fail");
        return ret;
    }
    return SOFTBUS_OK;
}

static int32_t InnerUnPublishService(const char *packageName, int32_t publishId, const ServiceType type)
{
    DiscInfo *infoNode = DeleteInfoFromList(g_publishInfoList, packageName, publishId, type);
    if (infoNode == NULL) {
        LOG_ERR("delete list fail");
        return SOFTBUS_DISCOVER_MANAGER_INFO_NOT_DELETE;
    }

    int32_t ret = DiscInterfaceByMedium(infoNode, UNPUBLISH_FUNC);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("interface fail");
        return ret;
    }
    ReleaseInfoNodeMem(infoNode, type);
    return SOFTBUS_OK;
}

static int32_t InnerStartDiscovery(const char *packageName, DiscInfo *info, const IServerDiscoveryCallback *cb,
    const ServiceType type)
{
    int32_t ret;
    InnerCallback callback;

    if ((cb != NULL) &&
        (memcpy_s(&(callback.subscribeCb), DISCOVERY_CB_LEN, cb, DISCOVERY_CB_LEN) != EOK)) {
        LOG_ERR("memcpy_s erro");
        return SOFTBUS_MEM_ERR;
    }

    if (type == SUBSCRIBE_INNER_SERVICE) {
        callback.innerCb.OnDeviceFound = NULL;
    }

    ret = AddInfoToList(g_discoveryInfoList, packageName, &callback, info, type);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("add list fail");
        return ret;
    }

    ret = DiscInterfaceByMedium(info, STARTDISCOVERTY_FUNC);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("interface fail");
        return ret;
    }
    return SOFTBUS_OK;
}

static int32_t InnerStopDiscovery(const char *packageName, int32_t subscribeId, const ServiceType type)
{
    DiscInfo *infoNode = DeleteInfoFromList(g_discoveryInfoList, packageName, subscribeId, type);
    if (infoNode == NULL) {
        LOG_ERR("delete list fail");
        return SOFTBUS_DISCOVER_MANAGER_INFO_NOT_DELETE;
    }

    int32_t ret = DiscInterfaceByMedium(infoNode, STOPDISCOVERY_FUNC);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("interface erro");
        return ret;
    }
    ReleaseInfoNodeMem(infoNode, type);
    return SOFTBUS_OK;
}

static char *ModuleIdToPackageName(DiscModule moduleId)
{
    char *packageName = (char *)SoftBusCalloc(PKG_NAME_SIZE_MAX * sizeof(char));
    if (packageName == NULL) {
        LOG_ERR("calloc fail");
        return NULL;
    }
    int32_t ret = memcpy_s(packageName, PKG_NAME_SIZE_MAX, g_discModuleMap[(moduleId - 1)], PKG_NAME_SIZE_MAX);
    if (ret != EOK) {
        SoftBusFree(packageName);
        LOG_ERR("memcpy_s fail");
        return NULL;
    }
    return packageName;
}

static int32_t InnerSetDiscoverCallback(const char *packageName, const DiscInnerCallback *cb)
{
    if (pthread_mutex_lock(&(g_discoveryInfoList->lock)) != 0) {
        LOG_ERR("lock failed");
        return SOFTBUS_LOCK_ERR;
    }

    bool isIdExist = false;
    DiscItem *itemNode = NULL;
    InnerCallback callback;
    LIST_FOR_EACH_ENTRY(itemNode, &(g_discoveryInfoList->list), DiscItem, node) {
        if (strcmp(itemNode->packageName, packageName) != 0) {
            continue;
        }
        itemNode->callback.innerCb.OnDeviceFound = cb->OnDeviceFound;
        isIdExist = true;
        break;
    }
    if (isIdExist == false) {
        callback.innerCb.OnDeviceFound = cb->OnDeviceFound;
        itemNode = CreateNewItem(g_discoveryInfoList, packageName, &callback, SUBSCRIBE_INNER_SERVICE);
        if (itemNode == NULL) {
            LOG_ERR("itemNode create failed");
            (void)pthread_mutex_unlock(&(g_discoveryInfoList->lock));
            return SOFTBUS_DISCOVER_MANAGER_ITEM_NOT_CREATE;
        }
    }
    (void)pthread_mutex_unlock(&(g_discoveryInfoList->lock));
    return SOFTBUS_OK;
}

int32_t DiscSetDiscoverCallback(DiscModule moduleId, const DiscInnerCallback *cb)
{
    if ((moduleId < MODULE_MIN) || (moduleId > MODULE_MAX) || (cb == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    char *packageName = ModuleIdToPackageName(moduleId);
    if (packageName == NULL) {
        LOG_ERR("packageName get fail");
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MODULE;
    }

    int32_t ret = InnerSetDiscoverCallback(packageName, cb);
    if (ret != SOFTBUS_OK) {
        SoftBusFree(packageName);
        return ret;
    }
    SoftBusFree(packageName);
    return SOFTBUS_OK;
}

int32_t DiscPublish(DiscModule moduleId, const PublishInnerInfo *info)
{
    if ((moduleId < MODULE_MIN) || (moduleId > MODULE_MAX) || (info == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (PublishInnerInfoCheck(info) != SOFTBUS_OK) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    char *packageName = ModuleIdToPackageName(moduleId);
    if (packageName == NULL) {
        LOG_ERR("packageName get fail");
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MODULE;
    }

    DiscInfo *infoNode = CreateNewPublishInnerInfoNode(info, DISCOVER_MODE_ACTIVE);
    if (infoNode == NULL) {
        LOG_ERR("infoNode create failed");
        SoftBusFree(packageName);
        return SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE;
    }

    int32_t ret = InnerPublishService(packageName, infoNode, NULL, PUBLISH_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        SoftBusFree(packageName);
        ReleaseInfoNodeMem(infoNode, PUBLISH_INNER_SERVICE);
        return ret;
    }
    SoftBusFree(packageName);
    return SOFTBUS_OK;
}

int32_t DiscStartScan(DiscModule moduleId, const PublishInnerInfo *info)
{
    if ((moduleId < MODULE_MIN) || (moduleId > MODULE_MAX) || (info == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (PublishInnerInfoCheck(info) != SOFTBUS_OK) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    char *packageName = ModuleIdToPackageName(moduleId);
    if (packageName == NULL) {
        LOG_ERR("packageName get fail");
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MODULE;
    }

    DiscInfo *infoNode = CreateNewPublishInnerInfoNode(info, DISCOVER_MODE_PASSIVE);
    if (infoNode == NULL) {
        LOG_ERR("infoNode create failed");
        SoftBusFree(packageName);
        return SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE;
    }

    int32_t ret = InnerPublishService(packageName, infoNode, NULL, PUBLISH_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        ReleaseInfoNodeMem(infoNode, PUBLISH_INNER_SERVICE);
        SoftBusFree(packageName);
        return ret;
    }
    SoftBusFree(packageName);
    return SOFTBUS_OK;
}

int32_t DiscUnpublish(DiscModule moduleId, int32_t publishId)
{
    if ((moduleId < MODULE_MIN) || (moduleId > MODULE_MAX)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    char *packageName = ModuleIdToPackageName(moduleId);
    if (packageName == NULL) {
        LOG_ERR("packageName get fail");
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MODULE;
    }

    int32_t ret = InnerUnPublishService(packageName, publishId, PUBLISH_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        SoftBusFree(packageName);
        return ret;
    }
    SoftBusFree(packageName);
    return SOFTBUS_OK;
}

int32_t DiscStartAdvertise(DiscModule moduleId, const SubscribeInnerInfo *info)
{
    if ((moduleId < MODULE_MIN) || (moduleId > MODULE_MAX) || (info == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (SubscribeInnerInfoCheck(info) != SOFTBUS_OK) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    char *packageName = ModuleIdToPackageName(moduleId);
    if (packageName == NULL) {
        LOG_ERR("packageName get fail");
        return SOFTBUS_DISCOVER_MANAGER_INVALID_MODULE;
    }

    DiscInfo *infoNode = CreateNewSubscribeInnerInfoNode(info, DISCOVER_MODE_ACTIVE);
    if (infoNode == NULL) {
        LOG_ERR("infoNode create failed");
        SoftBusFree(packageName);
        return SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE;
    }

    int32_t ret = InnerStartDiscovery(packageName, infoNode, NULL, SUBSCRIBE_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        ReleaseInfoNodeMem(infoNode, SUBSCRIBE_INNER_SERVICE);
        SoftBusFree(packageName);
        return ret;
    }
    SoftBusFree(packageName);
    return SOFTBUS_OK;
}

int32_t DiscSubscribe(DiscModule moduleId, const SubscribeInnerInfo *info)
{
    if ((moduleId < MODULE_MIN) || (moduleId > MODULE_MAX) || (info == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (SubscribeInnerInfoCheck(info) != SOFTBUS_OK) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    char *packageName = ModuleIdToPackageName(moduleId);
    if (packageName == NULL) {
        LOG_ERR("packageName get fail");
        return SOFTBUS_DISCOVER_MANAGER_INVALID_PKGNAME;
    }

    DiscInfo *infoNode = CreateNewSubscribeInnerInfoNode(info, DISCOVER_MODE_PASSIVE);
    if (infoNode == NULL) {
        LOG_ERR("infoNode create failed");
        SoftBusFree(packageName);
        return SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE;
    }

    int32_t ret = InnerStartDiscovery(packageName, infoNode, NULL, SUBSCRIBE_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        ReleaseInfoNodeMem(infoNode, SUBSCRIBE_INNER_SERVICE);
        SoftBusFree(packageName);
        return ret;
    }
    SoftBusFree(packageName);
    return SOFTBUS_OK;
}

int32_t DiscStopAdvertise(DiscModule moduleId, int32_t subscribeId)
{
    if ((moduleId < MODULE_MIN) || (moduleId > MODULE_MAX)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    char *packageName = ModuleIdToPackageName(moduleId);
    if (packageName == NULL) {
        LOG_ERR("packageName get fail");
        return SOFTBUS_DISCOVER_MANAGER_INVALID_PKGNAME;
    }
    int32_t ret = InnerStopDiscovery(packageName, subscribeId, SUBSCRIBE_INNER_SERVICE);
    if (ret != SOFTBUS_OK) {
        SoftBusFree(packageName);
        return ret;
    }
    SoftBusFree(packageName);
    return SOFTBUS_OK;
}

int32_t DiscPublishService(const char *packageName, const PublishInfo *info, const IServerPublishCallback *cb)
{
    int32_t ret;

    if ((packageName == NULL) || (info == NULL) || (cb == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    ret = PublishInfoCheck(packageName, info);
    if (ret != SOFTBUS_OK) {
        goto EXIT;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        ret = SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
        goto EXIT;
    }

    DiscInfo *infoNode = CreateNewPublishInfoNode(info);
    if (infoNode == NULL) {
        LOG_ERR("infoNode create failed");
        ret = SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE;
        goto EXIT;
    }

    ret = InnerPublishService(packageName, infoNode, cb, PUBLISH_SERVICE);
    if (ret != SOFTBUS_OK) {
        ReleaseInfoNodeMem(infoNode, PUBLISH_SERVICE);
        goto EXIT;
    }
    cb->OnServerPublishSuccess(packageName, info->publishId);
    return SOFTBUS_OK;
EXIT:
    if (ret == SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM) {
        cb->OnServerPublishFail(packageName, info->publishId, PUBLISH_FAIL_REASON_NOT_SUPPORT_MEDIUM);
        return ret;
    }
    cb->OnServerPublishFail(packageName, info->publishId, PUBLISH_FAIL_REASON_INTERNAL);
    return ret;
}

int32_t DiscUnPublishService(const char *packageName, int32_t publishId)
{
    if ((packageName == NULL) || (strlen(packageName) >= PKG_NAME_SIZE_MAX)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    int32_t ret = InnerUnPublishService(packageName, publishId, PUBLISH_SERVICE);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    return SOFTBUS_OK;
}

int32_t DiscStartDiscovery(const char *packageName, const SubscribeInfo *info, const IServerDiscoveryCallback *cb)
{
    int32_t ret;

    if ((packageName == NULL) || (info == NULL) || (cb == NULL)) {
        return SOFTBUS_INVALID_PARAM;
    }

    ret = SubscribeInfoCheck(packageName, info);
    if (ret != SOFTBUS_OK) {
        goto EXIT;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        ret = SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
        goto EXIT;
    }

    DiscInfo *infoNode = CreateNewSubscribeInfoNode(info);
    if (infoNode == NULL) {
        LOG_ERR("infoNode create failed");
        ret = SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE;
        goto EXIT;
    }

    ret = InnerStartDiscovery(packageName, infoNode, cb, SUBSCRIBE_SERVICE);
    if (ret != SOFTBUS_OK) {
        ReleaseInfoNodeMem(infoNode, SUBSCRIBE_SERVICE);
        goto EXIT;
    }
    cb->OnServerDiscoverySuccess(packageName, info->subscribeId);
    return SOFTBUS_OK;
EXIT:
    if (ret == SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM) {
        cb->OnServerDiscoverFailed(packageName, info->subscribeId, DISCOVERY_FAIL_REASON_NOT_SUPPORT_MEDIUM);
        return ret;
    }
    cb->OnServerDiscoverFailed(packageName, info->subscribeId, DISCOVERY_FAIL_REASON_INTERNAL);
    return ret;
}

int32_t DiscStopDiscovery(const char *packageName, int32_t subscribeId)
{
    if ((packageName == NULL) || (strlen(packageName) >= PKG_NAME_SIZE_MAX)) {
        return SOFTBUS_INVALID_PARAM;
    }

    if (g_isInited == false) {
        LOG_ERR("not init");
        return SOFTBUS_DISCOVER_MANAGER_NOT_INIT;
    }

    int32_t ret = InnerStopDiscovery(packageName, subscribeId, SUBSCRIBE_SERVICE);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    return SOFTBUS_OK;
}

int32_t DiscMgrInit(void)
{
    if (g_isInited) {
        return SOFTBUS_OK;
    }

    g_discCoapInterface = DiscCoapInit(&g_discMgrMediumCb);
    g_discBleInterface = DiscBleInit(&g_discMgrMediumCb);
    if ((g_discCoapInterface == NULL) && (g_discBleInterface == NULL)) {
        LOG_ERR("medium init all fail");
        return SOFTBUS_ERR;
    }

    g_publishInfoList = CreateSoftBusList();
    g_discoveryInfoList = CreateSoftBusList();
    if ((g_publishInfoList == NULL) || (g_discoveryInfoList == NULL)) {
        LOG_ERR("init service info list fail");
        return SOFTBUS_ERR;
    }

    for (int32_t i = 0; i < CAPABILITY_MAX_BITNUM; i++) {
        ListInit(&g_capabilityList[i]);
    }

    g_isInited = true;
    LOG_INFO("init success");
    return SOFTBUS_OK;
}

static void DiscMgrInfoListDeinit(SoftBusList *itemList, const ServiceType type)
{
    DiscItem *itemNode = NULL;
    DiscItem *itemNodeNext = NULL;
    DiscInfo *infoNode = NULL;
    DiscInfo *infoNodeNext = NULL;

    if (pthread_mutex_lock(&(itemList->lock)) != 0) {
        LOG_ERR("lock failed");
        return;
    }

    LIST_FOR_EACH_ENTRY_SAFE(itemNode, itemNodeNext, &(itemList->list), DiscItem, node) {
        LIST_FOR_EACH_ENTRY_SAFE(infoNode, infoNodeNext, &(itemNode->InfoList), DiscInfo, node) {
            ListDelete(&(infoNode->node));
            DeleteInfoFromCapability(infoNode, type);
            ReleaseInfoNodeMem(infoNode, type);
        }
        itemList->cnt--;
        ListDelete(&(itemNode->node));
        SoftBusFree(itemNode);
    }
    (void)pthread_mutex_unlock(&(itemList->lock));
    DestroySoftBusList(itemList);
}

void DiscMgrDeinit(void)
{
    if (g_isInited == false) {
        return;
    }
    DiscMgrInfoListDeinit(g_publishInfoList, PUBLISH_SERVICE);
    DiscMgrInfoListDeinit(g_discoveryInfoList, SUBSCRIBE_SERVICE);
    g_publishInfoList = NULL;
    g_discoveryInfoList = NULL;
    g_discCoapInterface = NULL;
    g_discBleInterface = NULL;
    DiscCoapDeinit();
    DiscBleDeinit();
    g_isInited = false;
}
