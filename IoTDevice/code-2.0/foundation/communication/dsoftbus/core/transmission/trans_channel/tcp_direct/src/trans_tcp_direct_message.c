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

#include "trans_tcp_direct_message.h"

#include <securec.h>
#include <string.h>

#include "auth_interface.h"
#include "bus_center_manager.h"
#include "cJSON.h"
#include "softbus_crypto.h"
#include "softbus_errcode.h"
#include "softbus_interface.h"
#include "softbus_log.h"
#include "softbus_mem_interface.h"
#include "softbus_message_open_channel.h"
#include "softbus_tcp_socket.h"
#include "trans_session_manager.h"
#include "trans_tcp_direct_manager.h"

#define MAX_PACKET_SIZE (64 * 1024)

static int32_t PackBytes(int32_t channelId, const uint8_t *data, TdcPacketHead *packetHead, uint8_t *buffer,
    uint32_t bufLen)
{
    SessionConn *conn = GetTdcInfoByChannelId(channelId);
    if (conn == NULL || data == NULL || buffer == NULL || packetHead == NULL || bufLen == 0) {
        LOG_ERR("Invalid para.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (memcpy_s(buffer, bufLen, packetHead, sizeof(TdcPacketHead)) != EOK) {
        LOG_ERR("memcpy packetHead error.");
        return SOFTBUS_ERR;
    }

    ConnectOption option = {0};
    option.type = CONNECT_TCP;
    if (strcpy_s(option.info.ipOption.ip, IP_LEN, conn->appInfo.peerData.ip) != 0) {
        LOG_ERR("strcpy_s peer ip err.");
        return SOFTBUS_ERR;
    }
    option.info.ipOption.port = conn->appInfo.peerData.port;

    AuthSideFlag side;
    uint32_t len = packetHead->dataLen - SESSION_KEY_INDEX_SIZE - OVERHEAD_LEN;
    OutBuf outbuf = {0};
    outbuf.buf = buffer + DC_MSG_PACKET_HEAD_SIZE;
    outbuf.bufLen = packetHead->dataLen;

    int32_t ret = AuthEncrypt(&option, &side, (uint8_t*)data, len, &outbuf);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("AuthDecrypt err.");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t TransTdcPostBytes(int32_t channelId, TdcPacketHead *packetHead, const char *data)
{
    SessionConn *conn = GetTdcInfoByChannelId(channelId);
    if (conn == NULL || data == NULL || packetHead == NULL) {
        LOG_ERR("Invalid para.");
        return SOFTBUS_INVALID_PARAM;
    }
    uint32_t bufferLen = packetHead->dataLen + DC_MSG_PACKET_HEAD_SIZE;
    if (bufferLen <= OVERHEAD_LEN + MESSAGE_INDEX_SIZE + DC_MSG_PACKET_HEAD_SIZE) {
        LOG_ERR("Invalid bufferLen.");
        return SOFTBUS_INVALID_PARAM;
    }
    char *buffer = (char *)SoftBusMalloc(bufferLen);
    if (buffer == NULL) {
        LOG_ERR("buffer malloc error.");
        return SOFTBUS_MALLOC_ERR;
    }
    if (PackBytes(channelId, (uint8_t*)data, packetHead, (uint8_t*)buffer, bufferLen) != SOFTBUS_OK) {
        LOG_ERR("Pack Bytes error.");
        SoftBusFree(buffer);
        return SOFTBUS_ENCRYPT_ERR;
    }
    if (SendTcpData(conn->appInfo.fd, buffer, bufferLen, 0) != (int)bufferLen) {
        SoftBusFree(buffer);
        return SOFTBUS_ERR;
    }
    SoftBusFree(buffer);
    return SOFTBUS_OK;
}

static int32_t RecvPacket(int32_t channelId, uint32_t len)
{
    SessionConn *conn = GetTdcInfoByChannelId(channelId);
    if (conn == NULL || len >= MAX_BUF_LENGTH) {
        LOG_ERR("Invalid para, len=%u", len);
        return SOFTBUS_INVALID_PARAM;
    }
    while (conn->dataBuffer.offset < len) {
        int32_t rc = RecvTcpData(conn->appInfo.fd, (char*)(conn->dataBuffer.data + conn->dataBuffer.offset),
            len - conn->dataBuffer.offset, 0);
        if (rc < 0) {
            return rc;
        } else if (rc == 0) {
            break;
        } else {
            conn->dataBuffer.offset += (uint32_t)rc;
        }
    }
    return conn->dataBuffer.offset;
}

static int32_t DecryptMessage(int32_t channelId, const TdcPacketHead *pktHead, uint8_t *out, uint32_t *outLen)
{
    if (channelId < 0 || pktHead == NULL || out == NULL || outLen == NULL) {
        LOG_ERR("param is invalid.");
        return SOFTBUS_ERR;
    }
    SessionConn *conn = GetTdcInfoByChannelId(channelId);
    if (conn == NULL) {
        LOG_ERR("channelId[%d] is not exist.", channelId);
        return SOFTBUS_ERR;
    }

    ConnectOption option = {0};
    option.type = CONNECT_TCP;
    if (strcpy_s(option.info.ipOption.ip, IP_LEN, conn->appInfo.peerData.ip) != 0) {
        LOG_ERR("strcpy_s peer ip err.");
        return SOFTBUS_ERR;
    }
    option.info.ipOption.port = conn->appInfo.peerData.port;

    AuthSideFlag side = CLIENT_SIDE_FLAG;
    uint8_t *data = conn->dataBuffer.data + DC_MSG_PACKET_HEAD_SIZE;
    uint32_t len = pktHead->dataLen;

    OutBuf outbuf = {0};
    outbuf.bufLen = pktHead->dataLen - SESSION_KEY_INDEX_SIZE - OVERHEAD_LEN + 1;
    outbuf.buf = out;
    int32_t ret = AuthDecrypt(&option, side, data, len, &outbuf);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("AuthDecrypt err.");
        return SOFTBUS_ERR;
    }
    *outLen = outbuf.outLen;
    return ret;
}

static int32_t NotifyChannelOpened(int32_t channelId)
{
    SessionConn *tdcInfo = GetTdcInfoByChannelId(channelId);
    if (tdcInfo == NULL) {
        LOG_ERR("notify channel open failed, get tdcInfo is null");
        return SOFTBUS_ERR;
    }
    ChannelInfo info = {0};
    info.channelId = channelId;
    info.channelType = CHANNEL_TYPE_TCP_DIRECT;
    info.isServer = tdcInfo->serverSide;
    info.isEnabled = true;
    info.fd = channelId;
    info.sessionKey = tdcInfo->appInfo.sessionKey;

    info.peerSessionName = tdcInfo->appInfo.peerData.sessionName;
    info.groupId = tdcInfo->appInfo.groupId;
    info.keyLen = SESSION_KEY_LENGTH;

    char buf[NETWORK_ID_BUF_LEN] = {0};
    int32_t ret = LnnGetNetworkIdByUuid(tdcInfo->appInfo.peerData.deviceId, buf, NETWORK_ID_BUF_LEN);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("get info networkId fail.");
        return SOFTBUS_ERR;
    }
    info.peerDeviceId = buf;

    char pkgName[PKG_NAME_SIZE_MAX] = {0};
    if (TransGetPkgNameBySessionName(info.peerSessionName, pkgName, PKG_NAME_SIZE_MAX) != SOFTBUS_OK) {
        LOG_ERR("get pkg name fail.");
        return SOFTBUS_ERR;
    }
    ret = GetClientProvideInterface()->onChannelOpened(pkgName, tdcInfo->appInfo.myData.sessionName, &info);
    tdcInfo->status = TCP_DIRECT_CHANNEL_STATUS_CONNECTED;
    return ret;
}

int32_t NotifyChannelOpenFailed(int32_t channelId)
{
    SessionConn *tdcInfo = GetTdcInfoByChannelId(channelId);
    if (tdcInfo == NULL) {
        LOG_ERR("notify channel open failed, get tdcInfo is null");
        return SOFTBUS_ERR;
    }

    char pkgName[PKG_NAME_SIZE_MAX] = {0};
    if (TransGetPkgNameBySessionName(tdcInfo->appInfo.peerData.sessionName, pkgName, PKG_NAME_SIZE_MAX) != SOFTBUS_OK) {
        LOG_ERR("get pkg name fail.");
        return SOFTBUS_ERR;
    }

    int ret = GetClientProvideInterface()->onChannelOpenFailed(pkgName, channelId);
    LOG_INFO("TCP direct channel failed, channelId = %d, ret = %d", channelId, ret);
    return ret;
}

int32_t NotifyChannelClosed(int32_t channelId)
{
    SessionConn *tdcInfo = GetTdcInfoByChannelId(channelId);
    if (tdcInfo == NULL) {
        LOG_ERR("notify channel closed failed(%d), get tdcInfo is null", channelId);
        return SOFTBUS_ERR;
    }

    if (tdcInfo->serverSide == false) {
        int ret = GetClientProvideInterface()->onChannelClosed(tdcInfo->appInfo.myData.pkgName, channelId);
        LOG_INFO("TCP direct channel close, channelId = %d, ret = %d", channelId, ret);
        return ret;
    }
    return SOFTBUS_OK;
}

static int32_t OpenDataBusReply(int32_t channelId, uint64_t seq, const cJSON *reply)
{
    SessionConn *tdcInfo = GetTdcInfoByChannelId(channelId);
    if (tdcInfo == NULL) {
        LOG_ERR("notify channel open failed, get tdcInfo is null");
        return SOFTBUS_ERR;
    }
    if (UnpackReply(reply, &tdcInfo->appInfo) != SOFTBUS_OK) {
        LOG_ERR("UnpackReply failed");
        return SOFTBUS_ERR;
    }

    TransTdcStopSessionConn(tdcInfo->channelId);
    int32_t ret = NotifyChannelOpened(tdcInfo->channelId);
    pthread_mutex_lock(&(GetTdcInfoList()->lock));
    tdcInfo->openChannelFinished = true;
    ListDelete(&tdcInfo->node);
    GetTdcInfoList()->cnt--;
    pthread_mutex_unlock(&(GetTdcInfoList()->lock));

    if (tdcInfo->serverSide == true || tdcInfo->openChannelFinished == true) {
        SoftBusFree(tdcInfo);
    }
    return ret;
}

static int32_t OpenDataBusRequest(int32_t channelId, uint32_t flags, uint64_t seq, const cJSON *request)
{
    SessionConn *conn = GetTdcInfoByChannelId(channelId);
    if (conn == NULL || request == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (UnpackRequest(request, &conn->appInfo) != SOFTBUS_OK) {
        LOG_ERR("UnpackRequest error");
        return SOFTBUS_ERR;
    }
    TransTdcStopSessionConn(channelId);
    int32_t ret = NotifyChannelOpened(channelId);
    TdcPacketHead packetHead = {
        .magicNumber = MAGIC_NUMBER,
        .module = MODULE_SESSION,
        .seq = seq,
        .flags = flags | FLAG_REPLY,
        .dataLen = 0,
    };

    if (ret != SOFTBUS_OK) {
        LOG_ERR("NotifyChannelOpened err");
        char *errDesc = "notifyChannelOpened";
        char *errReply = PackError(ret, errDesc);
        if (errReply == NULL) {
            LOG_ERR("Failed to send notify channel opened");
            return SOFTBUS_ERR;
        }

        packetHead.dataLen = strlen(errReply) + OVERHEAD_LEN + MESSAGE_INDEX_SIZE;
        ret = TransTdcPostBytes(channelId, &packetHead, errReply);
        if (ret != SOFTBUS_OK) {
            LOG_ERR("TransTdc post bytes failed");
            SoftBusFree(errReply);
            return SOFTBUS_ERR;
        }
        SoftBusFree(errReply);
        return SOFTBUS_ERR;
    }
    char *reply = PackReply(&conn->appInfo);
    if (reply == NULL) {
        LOG_ERR("PackReply failed");
        return SOFTBUS_ERR;
    }

    packetHead.dataLen = strlen(reply) + OVERHEAD_LEN + MESSAGE_INDEX_SIZE;
    ret = TransTdcPostBytes(channelId, &packetHead, reply);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("TransTdc post bytes failed");
        SoftBusFree(reply);
        return SOFTBUS_ERR;
    }
    SoftBusFree(reply);
    pthread_mutex_lock(&(GetTdcInfoList()->lock));
    ListDelete(&conn->node);
    GetTdcInfoList()->cnt--;
    pthread_mutex_unlock(&(GetTdcInfoList()->lock));
    if (conn->serverSide == true || conn->openChannelFinished == true) {
        SoftBusFree(conn);
    }
    return SOFTBUS_OK;
}

static int32_t MessageReceived(int32_t channelId, uint32_t flags, uint64_t seq, const cJSON *packet)
{
    if (flags & FLAG_REPLY) {
        return OpenDataBusReply(channelId, seq, packet);
    }
    return OpenDataBusRequest(channelId, FLAG_REPLY, seq, packet);
}

static int32_t DataReceived(int32_t channelId, const TdcPacketHead *pktHead)
{
    switch (pktHead->module) {
        case MODULE_SESSION: {
            uint8_t *out = (uint8_t*)SoftBusCalloc(pktHead->dataLen - SESSION_KEY_INDEX_SIZE - OVERHEAD_LEN + 1);
            if (out == NULL) {
                LOG_INFO("malloc fail.");
                return SOFTBUS_ERR;
            }
            uint32_t outLen;
            if (DecryptMessage(channelId, pktHead, out, &outLen) != SOFTBUS_OK) {
                LOG_ERR("Decrypt Message err.");
                SoftBusFree(out);
                return SOFTBUS_ERR;
            }
            out[outLen] = 0;
            cJSON *packet = cJSON_Parse((char*)out);
            if (packet == NULL) {
                SoftBusFree(out);
                LOG_ERR("json parse failed.");
                return SOFTBUS_ERR;
            }
            int ret = MessageReceived(channelId, pktHead->flags, pktHead->seq, packet);
            SoftBusFree(out);
            cJSON_Delete(packet);
            return ret;
        }
        default: {
            NotifyChannelClosed(channelId);
            TransTdcCloseSessionConn(channelId);
            SessionConn *conn = GetTdcInfoByChannelId(channelId);
            if (conn == NULL) {
                LOG_ERR("Invalid para");
                return SOFTBUS_INVALID_PARAM;
            }
            SoftBusFree(conn);
            return -1;
        }
    }
}

int32_t TransTdcProcessPacket(int32_t channelId)
{
    SessionConn *conn = GetTdcInfoByChannelId(channelId);
    if (conn == NULL) {
        LOG_ERR("Invalid para");
        return SOFTBUS_INVALID_PARAM;
    }
    conn->dataBuffer.offset = 0;
    int rc = RecvPacket(channelId, DC_MSG_PACKET_HEAD_SIZE);
    if (rc <= 0) {
        LOG_ERR("recv failed.");
        return SOFTBUS_ERR;
    }
    if (rc < DC_MSG_PACKET_HEAD_SIZE) {
        LOG_ERR("MsgPacketHeadRecv failed, recv next time.");
        return SOFTBUS_OK;
    }

    TdcPacketHead *pktHead = (TdcPacketHead *)(conn->dataBuffer.data);
    if (pktHead->magicNumber != MAGIC_NUMBER) {
        LOG_ERR("Invalid packetHead");
        return SOFTBUS_ERR;
    }
    uint32_t dataLen = pktHead->dataLen;
    if (dataLen > MAX_PACKET_SIZE - DC_MSG_PACKET_HEAD_SIZE) {
        LOG_ERR("Invalid packet size, %u", dataLen);
        return SOFTBUS_ERR;
    }

    rc = RecvPacket(channelId, DC_MSG_PACKET_HEAD_SIZE + dataLen);
    if (rc < 0) {
        LOG_ERR("Connection break");
        return SOFTBUS_ERR;
    }
    if (rc < (int)(DC_MSG_PACKET_HEAD_SIZE + dataLen)) {
        LOG_ERR("DataPacketRecv failed");
        return SOFTBUS_OK;
    }
    if (DataReceived(channelId, pktHead) != SOFTBUS_OK) {
        LOG_ERR("Data Received failed");
        return SOFTBUS_ERR;
    }

    return SOFTBUS_OK;
}

