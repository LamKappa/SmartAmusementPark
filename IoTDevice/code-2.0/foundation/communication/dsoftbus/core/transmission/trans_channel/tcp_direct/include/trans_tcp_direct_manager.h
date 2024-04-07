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

#ifndef SOFTBUS_DIRECT_CHANNEL_INTERFACE_H
#define SOFTBUS_DIRECT_CHANNEL_INTERFACE_H

#include <stdbool.h>

#include "softbus_app_info.h"
#include "softbus_base_listener.h"
#include "softbus_conn_interface.h"
#include "softbus_property.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DC_MSG_PACKET_HEAD_SIZE 24
#define SESSION_KEY_INDEX_SIZE 4
#define MESSAGE_INDEX_SIZE 4

#define MAGIC_NUMBER 0xBABEFACE
#define MODULE_SESSION 6
#define FLAG_REPLY 1

#define SKEY_LENGTH 16

typedef enum {
    TCP_DIRECT_CHANNEL_STATUS_CONNECTED,
    TCP_DIRECT_CHANNEL_STATUS_CONNECTING,
    TCP_DIRECT_CHANNEL_STATUS_HANDSHAKING,
    TCP_DIRECT_CHANNEL_STATUS_HANDSHAKE_TIMEOUT,
} TcpDirectChannelStatus;

typedef struct {
    char sessionKey[SKEY_LENGTH];
    int sessionIndex;
} IAuthConnection;

typedef struct {
    uint32_t offset;
    uint8_t data[MAX_BUF_LENGTH];
} DcDataBuffer;

typedef struct {
    ListNode node;
    DcDataBuffer dataBuffer;
    bool serverSide;
    bool authStarted;
    bool openChannelFinished;
    long channelId;
    TriggerType triggerType;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    AppInfo appInfo;
    IAuthConnection authConn;
    uint32_t status;
    uint32_t timeout;
} SessionConn;

typedef struct {
    uint32_t magicNumber;
    uint32_t module;
    uint64_t seq;
    uint32_t flags;
    uint32_t dataLen;
} TdcPacketHead;

int32_t TransOpenTcpDirectChannel(AppInfo *appInfo, const ConnectOption *connInfo, int *fd);
int32_t TransCloseDirectChannel(int32_t channelId);
uint64_t TransTdcGetNewSeqId(bool serverSide);
SessionConn *GetTdcInfoByChannelId(int32_t channelId);
SessionConn *GetTdcInfoByFd(int fd);

int32_t TransTdcAddSessionConn(SessionConn *conn, const TriggerType triggerType);
void TransTdcStopSessionConn(int32_t channelId);
void TransTdcCloseSessionConn(int32_t channelId);

SoftBusList *GetTdcInfoList(void);
void SetTdcInfoList(SoftBusList *sessionConnList);
int32_t TransTcpDirectInit(void);
void TransTcpDirectDeinit(void);

#ifdef __cplusplus
}
#endif

#endif
