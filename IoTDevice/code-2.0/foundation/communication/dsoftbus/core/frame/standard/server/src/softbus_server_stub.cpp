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

#include "softbus_server_stub.h"

#include "discovery_service.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "softbus_errcode.h"
#include "softbus_interface.h"
#include "softbus_log.h"
#include "softbus_mem_interface.h"
#include "softbus_server.h"

namespace OHOS {
SoftBusServerStub::SoftBusServerStub()
{
    memberFuncMap_[SERVER_START_DISCOVERY] =
        &SoftBusServerStub::StartDiscoveryInner;
    memberFuncMap_[SERVER_STOP_DISCOVERY] =
        &SoftBusServerStub::StopDiscoveryInner;
    memberFuncMap_[SERVER_PUBLISH_SERVICE] =
        &SoftBusServerStub::PublishServiceInner;
    memberFuncMap_[SERVER_UNPUBLISH_SERVICE] =
        &SoftBusServerStub::UnPublishServiceInner;
    memberFuncMap_[MANAGE_REGISTER_SERVICE] =
        &SoftBusServerStub::SoftbusRegisterServiceInner;
    memberFuncMap_[SERVER_CREATE_SESSION_SERVER] =
        &SoftBusServerStub::CreateSessionServerInner;
    memberFuncMap_[SERVER_REMOVE_SESSION_SERVER] =
        &SoftBusServerStub::RemoveSessionServerInner;
    memberFuncMap_[SERVER_OPEN_SESSION] =
        &SoftBusServerStub::OpenSessionInner;
    memberFuncMap_[SERVER_CLOSE_CHANNEL] =
        &SoftBusServerStub::CloseChannelInner;
    memberFuncMap_[SERVER_SESSION_SENDMSG] =
        &SoftBusServerStub::SendMessageInner;
    memberFuncMap_[SERVER_JOIN_LNN] =
        &SoftBusServerStub::JoinLNNInner;
    memberFuncMap_[SERVER_LEAVE_LNN] =
        &SoftBusServerStub::LeaveLNNInner;
    memberFuncMap_[SERVER_GET_ALL_ONLINE_NODE_INFO] =
        &SoftBusServerStub::GetAllOnlineNodeInfoInner;
    memberFuncMap_[SERVER_GET_LOCAL_DEVICE_INFO] =
        &SoftBusServerStub::GetLocalDeviceInfoInner;
    memberFuncMap_[SERVER_GET_NODE_KEY_INFO] =
        &SoftBusServerStub::GetNodeKeyInfoInner;
}

int32_t SoftBusServerStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    LOG_INFO("SoftBusServerStub::OnReceived, code = %u", code);
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    LOG_INFO("SoftBusServerStub:: default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t SoftBusServerStub::StartDiscoveryInner(MessageParcel &data, MessageParcel &reply)
{
    SubscribeInfo subInfo;
    const char *pkgName = data.ReadCString();
    subInfo.subscribeId = data.ReadInt32();
    subInfo.mode = (DiscoverMode)data.ReadInt32();
    subInfo.medium = (ExchanageMedium)data.ReadInt32();
    subInfo.freq = (ExchangeFreq)data.ReadInt32();
    subInfo.isSameAccount = data.ReadBool();
    subInfo.isWakeRemote = data.ReadBool();
    subInfo.capability = data.ReadCString();
    subInfo.capabilityData = (unsigned char *)data.ReadCString();
    subInfo.dataLen = data.ReadUint32();
    int32_t retReply = StartDiscovery(pkgName, &subInfo);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("StartDiscoveryInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::StopDiscoveryInner(MessageParcel &data, MessageParcel &reply)
{
    const char *pkgName = data.ReadCString();
    int32_t subscribeId = data.ReadInt32();
    LOG_INFO("StopDiscoveryInner %s, %d!\n", pkgName, subscribeId);
    int32_t retReply = StopDiscovery(pkgName, subscribeId);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("StopDiscoveryInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::PublishServiceInner(MessageParcel &data, MessageParcel &reply)
{
    PublishInfo pubInfo;
    const char *pkgName = data.ReadCString();
    pubInfo.publishId = data.ReadInt32();
    pubInfo.mode = (DiscoverMode)data.ReadInt32();
    pubInfo.medium = (ExchanageMedium)data.ReadInt32();
    pubInfo.freq = (ExchangeFreq)data.ReadInt32();
    pubInfo.capability = data.ReadCString();
    pubInfo.capabilityData = (unsigned char *)data.ReadCString();
    pubInfo.dataLen = data.ReadUint32();
    int32_t retReply = PublishService(pkgName, &pubInfo);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("PublishServiceInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::UnPublishServiceInner(MessageParcel &data, MessageParcel &reply)
{
    const char *pkgName = data.ReadCString();
    int32_t publishId = data.ReadInt32();
    int32_t retReply = UnPublishService(pkgName, publishId);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("UnPublishServiceInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::SoftbusRegisterServiceInner(MessageParcel &data, MessageParcel &reply)
{
    auto remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        LOG_ERR("SoftbusRegisterServiceInner read systemAbilityId failed!");
        return SOFTBUS_ERR;
    }
    const char *pkgName = data.ReadCString();
    if (pkgName == nullptr) {
        LOG_ERR("SoftbusRegisterServiceInner read pkgName failed!");
        return SOFTBUS_ERR;
    }
    int32_t retReply = SoftbusRegisterService(pkgName, remote);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("SoftbusRegisterServiceInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::CreateSessionServerInner(MessageParcel &data, MessageParcel &reply)
{
    const char *pkgName = data.ReadCString();
    if (pkgName == nullptr) {
        LOG_ERR("CreateSessionServerInner read pkgName failed!");
        return SOFTBUS_ERR;
    }
    const char *sessionName = data.ReadCString();
    if (sessionName == nullptr) {
        LOG_ERR("CreateSessionServerInner read sessionName failed!");
        return SOFTBUS_ERR;
    }
    int32_t retReply = CreateSessionServer(pkgName, sessionName);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("CreateSessionServerInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::RemoveSessionServerInner(MessageParcel &data, MessageParcel &reply)
{
    const char *pkgName = data.ReadCString();
    if (pkgName == nullptr) {
        LOG_ERR("RemoveSessionServerInner read pkgName failed!");
        return SOFTBUS_ERR;
    }
    const char *sessionName = data.ReadCString();
    if (sessionName == nullptr) {
        LOG_ERR("RemoveSessionServerInner read sessionName failed!");
        return SOFTBUS_ERR;
    }
    int32_t retReply = RemoveSessionServer(pkgName, sessionName);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("RemoveSessionServerInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::OpenSessionInner(MessageParcel &data, MessageParcel &reply)
{
    const char *mySessionName = data.ReadCString();
    if (mySessionName == nullptr) {
        LOG_ERR("OpenSessionInner read my session name failed!");
        return SOFTBUS_ERR;
    }
    const char *peerSessionName = data.ReadCString();
    if (peerSessionName == nullptr) {
        LOG_ERR("OpenSessionInner read peer session name failed!");
        return SOFTBUS_ERR;
    }
    const char *peerDeviceId = data.ReadCString();
    if (peerDeviceId == nullptr) {
        LOG_ERR("OpenSessionInner read peeer deviceid failed!");
        return SOFTBUS_ERR;
    }
    const char *groupId = data.ReadCString();
    if (groupId == nullptr) {
        LOG_ERR("OpenSessionInner read group id failed!");
        return SOFTBUS_ERR;
    }
    int32_t flags;
    if (!data.ReadInt32(flags)) {
        LOG_ERR("OpenSessionInner read flags failed!");
        return SOFTBUS_ERR;
    }
    int32_t retReply = OpenSession(mySessionName, peerSessionName, peerDeviceId, groupId, flags);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("OpenSessionInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::CloseChannelInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t channelId;
    if (!data.ReadInt32(channelId)) {
        LOG_ERR("CloseChannelInner read channel Id failed!");
        return SOFTBUS_ERR;
    }
    int32_t retReply = CloseChannel(channelId);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("CloseChannelInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::SendMessageInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t channelId;
    if (!data.ReadInt32(channelId)) {
        LOG_ERR("SendMessage read channel Id failed!");
        return SOFTBUS_ERR;
    }
    uint32_t len;
    if (!data.ReadUint32(len)) {
        LOG_ERR("SendMessage dataInfo len failed!");
        return SOFTBUS_ERR;
    }
    void *dataInfo = (void *)data.ReadRawData(len);
    if (dataInfo == nullptr) {
        LOG_ERR("SendMessage read dataInfo failed!");
        return SOFTBUS_ERR;
    }
    int32_t msgType;
    if (!data.ReadInt32(msgType)) {
        LOG_ERR("SendMessage message type failed!");
        return SOFTBUS_ERR;
    }

    int32_t retReply = SendMessage(channelId, dataInfo, len, msgType);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("SendMessage write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::JoinLNNInner(MessageParcel &data, MessageParcel &reply)
{
    const char *clientName = data.ReadCString();
    if (clientName == nullptr) {
        LOG_ERR("SoftbusJoinLNNInner read clientName failed!");
        return SOFTBUS_ERR;
    }
    uint32_t addrTypeLen;
    if (!data.ReadUint32(addrTypeLen)) {
        LOG_ERR("SoftbusJoinLNNInner read addr type length failed!");
        return SOFTBUS_ERR;
    }
    void *addr = (void *)data.ReadRawData(addrTypeLen);
    if (addr == nullptr) {
        LOG_ERR("SoftbusJoinLNNInner read addr failed!");
        return SOFTBUS_ERR;
    }
    int32_t retReply = JoinLNN(clientName, addr, addrTypeLen);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("SoftbusJoinLNNInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::LeaveLNNInner(MessageParcel &data, MessageParcel &reply)
{
    const char *clientName = data.ReadCString();
    if (clientName == nullptr) {
        LOG_ERR("SoftbusLeaveLNNInner read clientName failed!");
        return SOFTBUS_ERR;
    }
    const char *networkId = data.ReadCString();
    if (networkId == nullptr) {
        LOG_ERR("SoftbusLeaveLNNInner read networkId failed!");
        return SOFTBUS_ERR;
    }
    int32_t retReply = LeaveLNN(clientName, networkId);
    if (!reply.WriteInt32(retReply)) {
        LOG_ERR("SoftbusJoinLNNInner write reply failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::GetAllOnlineNodeInfoInner(MessageParcel &data, MessageParcel &reply)
{
    void *nodeInfo = nullptr;
    int32_t infoNum;
    uint32_t infoTypeLen;
    const char *clientName = data.ReadCString();
    if (clientName == nullptr) {
        LOG_ERR("GetAllOnlineNodeInfoInner read clientName failed!");
        return SOFTBUS_ERR;
    }
    if (!data.ReadUint32(infoTypeLen)) {
        LOG_ERR("GetAllOnlineNodeInfoInner read info type length failed");
        return SOFTBUS_ERR;
    }
    if (GetAllOnlineNodeInfo(clientName, &nodeInfo, infoTypeLen, &infoNum) != SOFTBUS_OK) {
        LOG_ERR("GetAllOnlineNodeInfoInner get info failed");
        return SOFTBUS_ERR;
    }
    if (infoNum < 0 || (infoNum > 0 && nodeInfo == nullptr)) {
        LOG_ERR("GetAllOnlineNodeInfoInner node info is invalid");
        return SOFTBUS_ERR;
    }
    if (!reply.WriteInt32(infoNum)) {
        LOG_ERR("GetAllOnlineNodeInfoInner write infoNum failed!");
        if (nodeInfo != nullptr) {
            SoftBusFree(nodeInfo);
        }
        return SOFTBUS_ERR;
    }
    int32_t ret = SOFTBUS_OK;
    if (infoNum > 0) {
        if (!reply.WriteRawData(nodeInfo, infoTypeLen * infoNum)) {
            LOG_ERR("GetAllOnlineNodeInfoInner write node info failed!");
            ret = SOFTBUS_ERR;
        }
        SoftBusFree(nodeInfo);
    }
    return ret;
}

int32_t SoftBusServerStub::GetLocalDeviceInfoInner(MessageParcel &data, MessageParcel &reply)
{
    void *nodeInfo = nullptr;
    uint32_t infoTypeLen;
    const char *clientName = data.ReadCString();
    if (clientName == nullptr) {
        LOG_ERR("GetLocalDeviceInfoInner read clientName failed!");
        return SOFTBUS_ERR;
    }
    if (!data.ReadUint32(infoTypeLen)) {
        LOG_ERR("GetLocalDeviceInfoInner read info type length failed");
        return SOFTBUS_ERR;
    }
    nodeInfo = SoftBusCalloc(infoTypeLen);
    if (nodeInfo == nullptr) {
        LOG_ERR("GetLocalDeviceInfoInner malloc info type length failed");
        return SOFTBUS_ERR;
    }
    if (GetLocalDeviceInfo(clientName, nodeInfo, infoTypeLen) != SOFTBUS_OK) {
        LOG_ERR("GetLocalDeviceInfoInner get local info failed");
        SoftBusFree(nodeInfo);
        return SOFTBUS_ERR;
    }
    if (!reply.WriteRawData(nodeInfo, infoTypeLen)) {
        LOG_ERR("GetLocalDeviceInfoInner write node info failed!");
        SoftBusFree(nodeInfo);
        return SOFTBUS_ERR;
    }
    SoftBusFree(nodeInfo);
    return SOFTBUS_OK;
}

int32_t SoftBusServerStub::GetNodeKeyInfoInner(MessageParcel &data, MessageParcel &reply)
{
    const char *clientName = data.ReadCString();
    if (clientName == nullptr) {
        LOG_ERR("GetNodeKeyInfoInner read clientName failed!");
        return SOFTBUS_ERR;
    }
    const char *networkId = data.ReadCString();
    if (networkId == nullptr) {
        LOG_ERR("GetNodeKeyInfoInner read networkId failed!");
        return SOFTBUS_ERR;
    }
    int32_t key;
    if (!data.ReadInt32(key)) {
        LOG_ERR("GetNodeKeyInfoInner read key failed!");
        return SOFTBUS_ERR;
    }
    int32_t len;
    if (!data.ReadInt32(len)) {
        LOG_ERR("GetNodeKeyInfoInner read len failed!");
        return SOFTBUS_ERR;
    }
    void *buf = SoftBusMalloc(len);
    if (buf == nullptr) {
        LOG_ERR("GetNodeKeyInfoInner malloc buffer failed!");
        return SOFTBUS_ERR;
    }
    if (GetNodeKeyInfo(clientName, networkId, key, (unsigned char *)buf, len) != SOFTBUS_OK) {
        LOG_ERR("GetNodeKeyInfoInner get key info failed!");
        SoftBusFree(buf);
        return SOFTBUS_ERR;
    }
    if (!reply.WriteRawData(buf, len)) {
        LOG_ERR("GetNodeKeyInfoInner write key info failed!");
        SoftBusFree(buf);
        return SOFTBUS_ERR;
    }
    SoftBusFree(buf);
    return SOFTBUS_OK;
}
} // namespace OHOS