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


#ifndef INTERFACES_INNERKITS_SOFTBUS_CLIENT_PROXY_H_
#define INTERFACES_INNERKITS_SOFTBUS_CLIENT_PROXY_H_

#include "if_softbus_client.h"

namespace OHOS {
class SoftBusClientProxy : public IRemoteProxy<ISoftBusClient> {
public:
    explicit SoftBusClientProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<ISoftBusClient>(impl) {}
    virtual ~SoftBusClientProxy() = default;

    void OnDeviceFound(const void *device) override;
    void OnDiscoverFailed(int subscribeId, int failReason) override;
    void OnDiscoverySuccess(int subscribeId) override;
    void OnPublishSuccess(int publishId) override;
    void OnPublishFail(int publishId, int reason) override;
    int32_t OnChannelOpened(const char *pkgName, const char *sessionName, const void *channel) override;
    int32_t OnChannelOpenFailed(const char *pkgName, int32_t channelId) override;
    int32_t OnChannelClosed(const char *pkgName, int32_t channelId) override;
    int32_t OnChannelMsgReceived(const char *pkgName, int32_t channelId, const void *data, uint32_t len,
        int32_t type) override;

    int32_t OnJoinLNNResult(void *addr, uint32_t addrTypeLen, const char *networkId, int retCode) override;
    int32_t OnLeaveLNNResult(const char *networkId, int retCode) override;
    int32_t OnNodeOnlineStateChanged(bool isOnline, void *info, uint32_t infoTypeLen) override;
    int32_t OnNodeBasicInfoChanged(void *info, uint32_t infoTypeLen, int32_t type) override;

private:
    static inline BrokerDelegator<SoftBusClientProxy> delegator_;
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SOFTBUS_CLIENT_PROXY_H_)
