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

#ifndef OHOS_AAFWK_ABILITY_MOCK_MANAGER_SERVICE_H
#define OHOS_AAFWK_ABILITY_MOCK_MANAGER_SERVICE_H

#include <memory>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>

#include "ability_manager_stub.h"
#include "iremote_object.h"

#include "gmock/gmock.h"

namespace OHOS {
namespace AAFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class MockAbilityManagerService : public AbilityManagerStub,
                                  public std::enable_shared_from_this<MockAbilityManagerService> {
public:
    MockAbilityManagerService();
    ~MockAbilityManagerService();
    int StartAbility(const Want &want, int requestCode = -1) override;
    int StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode = -1) override;
    int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode = -1, const Want *resultWant = nullptr) override;
    int ConnectAbility(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) override;
    int DisconnectAbility(const sptr<IAbilityConnection> &connect) override;

    sptr<IAbilityScheduler> AcquireDataAbility(const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken);

    ErrCode ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken);

    void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken) override;

    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override;

    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state) override;
    int ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) override;
    int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) override;
    int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) override;

    void DumpState(const std::string &args, std::vector<std::string> &info) override;
    int TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode) override;
    int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId) override;
    int StopServiceAbility(const Want &want) override;

    MOCK_METHOD1(GetAllStackInfo, int(StackInfo &stackInfo));

    MOCK_METHOD1(MoveMissionToTop, int(int32_t missionId));
    MOCK_METHOD1(KillProcess, int(const std::string &bundleName));
    MOCK_METHOD1(UninstallApp, int(const std::string &bundleName));

    int GetRecentMissions(
        const int32_t numMax, const int32_t flags, std::vector<RecentMissionInfo> &recentList) override;

    int GetMissionSnapshot(const int32_t missionId, MissionSnapshotInfo &snapshot) override;

    int RemoveMission(int id) override;

    int RemoveStack(int id) override;

    AbilityLifeCycleState curstate_ = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
    sptr<IAbilityScheduler> abilityScheduler_;  // kit interface used to schedule ability life
    Want want_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_SERVICE_H
