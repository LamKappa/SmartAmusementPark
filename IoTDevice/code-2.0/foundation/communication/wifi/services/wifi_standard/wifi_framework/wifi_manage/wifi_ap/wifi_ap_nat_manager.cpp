/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "wifi_ap_nat_manager.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include "network_interface.h"
#include "wifi_log.h"


#undef LOG_TAG
#define LOG_TAG "OHWIFI_AP_WifiApNatManager"

const std::string SYSTEM_COMMAND_IP = "/system/bin/ip";
const std::string SYSTEM_COMMAND_IPTABLES = "/system/bin/iptables";
const std::string SYSTEM_COMMAND_IP6TABLES = "/system/bin/ip6tables";
const std::string IP_V4_FORWARDING_CONFIG_FILE = "/proc/sys/net/ipv4/ip_forward";
const std::string IP_V6_FORWARDING_CONFIG_FILE = "/proc/sys/net/ipv6/conf/all/forwarding";
const int SYSTEM_NOT_EXECUTED = 127;

namespace OHOS {
namespace Wifi {
WifiApNatManager *WifiApNatManager::g_instance = nullptr;

WifiApNatManager &WifiApNatManager::GetInstance()
{
    if (g_instance == nullptr) {
        g_instance = new WifiApNatManager();
    }
    return *g_instance;
}

void WifiApNatManager::DeleteInstance()
{
    if (g_instance != nullptr) {
        delete g_instance;
        g_instance = nullptr;
    }
}

bool WifiApNatManager::EnableInterfaceNat(bool enable, std::string inInterfaceName, std::string outInterfaceName) const
{
    LOGI("EnableInterfaceNat enable [%{public}s], inInterfaceName [%s]  outInterfaceName "
         "[%s]",
        enable ? "true" : "false",
        inInterfaceName.c_str(),
        outInterfaceName.c_str());

    if (!NetworkInterface::IsValidInterfaceName(inInterfaceName) ||
        !NetworkInterface::IsValidInterfaceName(outInterfaceName)) {
        LOGE("Invalid interface name.");
        return false;
    }

    if (inInterfaceName == outInterfaceName) {
        LOGE("Duplicate interface name.");
        return false;
    }

    if (!SetForwarding(enable)) {
        LOGE("SetForwarding failed.");
        return false;
    }

    if (!SetInterfaceRoute(enable)) {
        LOGE("SetInterfaceRoute failed.");
        return false;
    }

    if (!SetInterfaceNat(enable, outInterfaceName)) {
        LOGE("SetInterfaceNat failed.");
        return false;
    }

    return true;
}

bool WifiApNatManager::SetForwarding(bool enable) const
{
    LOGI("SetForwarding enable = %{public}s.", enable ? "true" : "false");

    bool bResult = true;
    const std::string content = enable ? "1" : "0";
    bResult = bResult && WriteDataToFile(IP_V4_FORWARDING_CONFIG_FILE, content);
    bResult = bResult && WriteDataToFile(IP_V6_FORWARDING_CONFIG_FILE, content);
    return bResult;
}

bool WifiApNatManager::SetInterfaceRoute(bool enable) const
{
    const std::string natRouteTable = "10";
    std::vector<std::string> ipRouteCmd;

    ipRouteCmd.push_back(SYSTEM_COMMAND_IP);
    ipRouteCmd.push_back("rule");
    ipRouteCmd.push_back(enable ? "add" : "del");
    ipRouteCmd.push_back("fwmark");
    ipRouteCmd.push_back("0x0/0xffff");
    ipRouteCmd.push_back("lookup");
    ipRouteCmd.push_back("254");
    ipRouteCmd.push_back("prio");
    ipRouteCmd.push_back("18000");
    ExecCommand(ipRouteCmd);

    /* Refresh the cache */
    ipRouteCmd.clear();
    ipRouteCmd.push_back(SYSTEM_COMMAND_IP);
    ipRouteCmd.push_back("route");
    ipRouteCmd.push_back("flush");
    ipRouteCmd.push_back("cache");
    ExecCommand(ipRouteCmd);

    return true;
}

bool WifiApNatManager::SetInterfaceNat(bool enable, const std::string &outInterfaceName) const
{
    std::vector<std::string> iptablesCmd;

    /* Clearing the Firewalls */
    iptablesCmd.push_back(SYSTEM_COMMAND_IPTABLES);
    iptablesCmd.push_back("-F");
    ExecCommand(iptablesCmd);

    /* iptable forward ACCEPT */
    iptablesCmd.clear();
    iptablesCmd.push_back(SYSTEM_COMMAND_IPTABLES);
    iptablesCmd.push_back("-P");
    iptablesCmd.push_back("FORWARD");
    iptablesCmd.push_back(enable ? "ACCEPT" : "DROP");
    ExecCommand(iptablesCmd);

    /* Setting NAT Rules */
    iptablesCmd.clear();
    iptablesCmd.push_back(SYSTEM_COMMAND_IPTABLES);
    iptablesCmd.push_back("-t");
    iptablesCmd.push_back("nat");
    iptablesCmd.push_back(enable ? "-A" : "-D");
    iptablesCmd.push_back("POSTROUTING");
    iptablesCmd.push_back("-o");
    iptablesCmd.push_back(outInterfaceName);
    iptablesCmd.push_back("-j");
    iptablesCmd.push_back("MASQUERADE");
    ExecCommand(iptablesCmd);

    return true;
}

bool WifiApNatManager::WriteDataToFile(const std::string &fileName, const std::string &content) const
{
    std::ofstream outf(fileName, std::ios::out);
    if (!outf) {
        LOGE("write content [%s] to file [%s] failed. error: %{public}s.",
            content.c_str(),
            fileName.c_str(),
            strerror(errno));
        return false;
    }
    outf.write(content.c_str(), content.length());
    outf.close();
    return true;
}

bool WifiApNatManager::ExecCommand(const std::vector<std::string> &vecCommandArg) const
{
    std::string command;
    for (auto iter : vecCommandArg) {
        command += iter;
        command += " ";
    }

    LOGE("exec cmd: [%s]", command.c_str());

    int ret = system(command.c_str());
    if (ret == -1 || ret == SYSTEM_NOT_EXECUTED) {
        LOGE("exec failed. cmd: %s, error:%{public}s", command.c_str(), strerror(errno));
        return false;
    }

    return true;
}
}  // namespace Wifi
}  // namespace OHOS