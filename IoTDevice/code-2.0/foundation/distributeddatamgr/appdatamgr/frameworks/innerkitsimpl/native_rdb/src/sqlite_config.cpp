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

#include "sqlite_config.h"

#include "sqlite_global_config.h"

namespace OHOS {
namespace NativeRdb {
SqliteConfig::SqliteConfig(const RdbStoreConfig &config)
{
    path = config.GetPath();
    storageMode = config.GetStorageMode();
    readOnly = config.IsReadOnly();
    encryptKey = config.GetEncryptKey();
    encrypted = !encryptKey.empty();
    journalMode = config.GetJournalMode();
    if (journalMode.empty()) {
        journalMode = SqliteGlobalConfig::GetDefaultJournalMode();
    }
}

SqliteConfig::~SqliteConfig()
{
    ClearEncryptKey();
}

std::string SqliteConfig::GetPath() const
{
    return path;
}

StorageMode SqliteConfig::GetStorageMode() const
{
    return storageMode;
}

std::string SqliteConfig::GetJournalMode() const
{
    return journalMode;
}

bool SqliteConfig::IsReadOnly() const
{
    return readOnly;
}

bool SqliteConfig::IsEncrypted() const
{
    return encrypted;
}

std::vector<uint8_t> SqliteConfig::GetEncryptKey() const
{
    return encryptKey;
}

void SqliteConfig::UpdateEncryptKey(const std::vector<uint8_t> &newKey)
{
    std::fill(encryptKey.begin(), encryptKey.end(), 0);
    encryptKey = newKey;
}

void SqliteConfig::ClearEncryptKey()
{
    std::fill(encryptKey.begin(), encryptKey.end(), 0);
    encryptKey.clear();
}
} // namespace NativeRdb
} // namespace OHOS