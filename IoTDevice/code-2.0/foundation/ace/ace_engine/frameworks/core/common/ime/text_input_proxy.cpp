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

#include "core/common/ime/text_input_proxy.h"

namespace OHOS::Ace {

TextInputProxy::TextInputProxy() = default;

TextInputProxy::~TextInputProxy() = default;

void TextInputProxy::SetDelegate(std::unique_ptr<TextInput>&& delegate)
{
    delegate_ = std::move(delegate);
}

RefPtr<TextInputConnection> TextInputProxy::Attach(const WeakPtr<TextInputClient>& client,
    const TextInputConfiguration& config, const RefPtr<TaskExecutor>& taskExecutor, int32_t instanceId)
{
    if (!delegate_) {
        return nullptr;
    }
    return delegate_->Attach(client, config, taskExecutor, instanceId);
}

} // namespace OHOS::Ace
