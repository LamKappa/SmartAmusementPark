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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_UNITTEST_MOCK_TWEEN_MOCK_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_UNITTEST_MOCK_TWEEN_MOCK_H

#include <utility>

#include "core/components/tween/tween_component.h"

namespace OHOS::Ace {

class MockTweenComponent : public TweenComponent {
public:
    MockTweenComponent(const ComposeId& id, const std::string& name, const RefPtr<Component>& child)
        : TweenComponent(id, name, child)
    {}
    ~MockTweenComponent() override = default;

    using HookTweenElement = std::function<void(const RefPtr<TweenElement>&)>;
    RefPtr<Element> CreateElement() override;

    static void SetMockHook(const HookTweenElement& hook);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_UNITTEST_MOCK_TWEEN_MOCK_H
