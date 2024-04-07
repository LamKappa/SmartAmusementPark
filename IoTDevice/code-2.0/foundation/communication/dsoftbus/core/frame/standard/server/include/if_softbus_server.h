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

#ifndef INTERFACES_INNERKITS_SOFTBUS_SERVER_H_
#define INTERFACES_INNERKITS_SOFTBUS_SERVER_H_

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

namespace OHOS {
class ISoftBusServer : public IRemoteBroker {
public:
    virtual ~ISoftBusServer() = default;

    virtual int32_t StartDiscovery(const char *pkgName, const void *info) = 0;
    virtual int32_t StopDiscovery(const char *pkgName, int subscribeId) = 0;
    virtual int32_t PublishService(const char *pkgName, const void *info) = 0;
    virtual int32_t UnPublishService(const char *pkgName, int publishId) = 0;
    virtual int32_t SoftbusRegisterService(const char *clientPkgName, const sptr<IRemoteObject> &object) = 0;

    virtual int32_t CreateSessionServer(const char *pkgName, const char *sessionName) = 0;
    virtual int32_t RemoveSessionServer(const char *pkgName, const char *sessionName) = 0;
    virtual int32_t OpenSession(const char *mySessionName, const char *peerSessionName,
        const char *peerDeviceId, const char *groupId, int32_t flags) = 0;
    virtual int32_t CloseChannel(int32_t channelId) = 0;
    virtual int32_t SendMessage(int32_t channelId, const void *data, uint32_t len, int32_t msgType) = 0;

    virtual int32_t JoinLNN(const char *pkgName, void *addr, uint32_t addrTypeLen) = 0;
    virtual int32_t LeaveLNN(const char *pkgName, const char *networkId) = 0;
    virtual int32_t GetAllOnlineNodeInfo(const char *pkgName, void **info, uint32_t infoTypeLen, int *infoNum) = 0;
    virtual int32_t GetLocalDeviceInfo(const char *pkgName, void *info, uint32_t infoTypeLen) = 0;
    virtual int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId, int key, unsigned char *buf,
        uint32_t len) = 0;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISoftBusServer");
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SOFTBUS_SERVER_H_ )