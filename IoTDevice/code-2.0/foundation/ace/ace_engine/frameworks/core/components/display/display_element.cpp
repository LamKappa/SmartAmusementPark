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

#include "core/components/display/display_element.h"

#include "core/components/display/render_display.h"

namespace OHOS::Ace {

void DisplayElement::PerformBuild()
{
    SoleChildElement::PerformBuild();
    auto displayRender = AceType::DynamicCast<RenderDisplay>(GetRenderNode());
    if (displayRender) {
        displayRender->GetOpacityCallbacks();
        // refresh.
        displayRender->UpdateOpacity(displayRender->GetOpacity());
    }
}

} // namespace OHOS::Ace