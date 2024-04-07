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

#ifndef NATIVE_RDB_SQLITE_CONFIG_H
#define NATIVE_RDB_SQLITE_CONFIG_H

#include <string>
#include <vector>

#include "rdb_store_config.h"

namespace OHOS {
namespace NativeRdb {

class SqliteConfig {
public:
    explicit SqliteConfig(const RdbStoreConfig &config);
    ~SqliteConfig();
    std::string GetPath() const;
    StorageMode GetStorageMode() const;
    std::string GetJournalMode() const;
    bool IsReadOnly() const;
    bool IsEncrypted() const;
    std::vector<uint8_t> GetEncryptKey() const;
    void UpdateEncryptKey(const std::vector<uint8_t> &newKey);
    void ClearEncryptKey();

private:
    std::string path;
    StorageMode storageMode;
    std::string journalMode;
    bool readOnly;
    bool encrypted;
    std::vector<uint8_t> encryptKey;
};

} // namespace NativeRdb
} // namespace OHOS
#endif
