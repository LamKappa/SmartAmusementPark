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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_LIBEVENTHANDLER_INCLUDE_DUMPER_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_LIBEVENTHANDLER_INCLUDE_DUMPER_H

namespace OHOS {
namespace AppExecFwk {
class Dumper {
public:
    /**
     * Processes the content of a specified string.
     * @param message the content of a specified string.
     */
    virtual void Dump(const std::string &message) = 0;

    /**
     * Obtains the tag information.
     * which is a prefix added to each string before the string content is processed.
     * @return the tag information.
     */
    virtual std::string GetTag() = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_LIBEVENTHANDLER_INCLUDE_DUMPER_H