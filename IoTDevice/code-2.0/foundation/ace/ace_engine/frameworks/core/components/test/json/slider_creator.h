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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_JSON_SLIDER_CREATOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_JSON_SLIDER_CREATOR_H

#include "base/json/json_util.h"
#include "core/components/slider/slider_component.h"
#include "core/components/test/json/component_factory.h"
#include "core/pipeline/base/component.h"

namespace OHOS::Ace {

class SliderCreator {
public:
    static RefPtr<Component> CreateFromJson(const JsonValue& json, const JsonComponentFactory& factory);
    static void CreateSliderBar(RefPtr<SliderComponent>& slider, const JsonValue& componentJson);
    static void CreateSliderBlock(RefPtr<SliderComponent>& slider, const JsonValue& componentJson);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_JSON_SLIDER_CREATOR_H
