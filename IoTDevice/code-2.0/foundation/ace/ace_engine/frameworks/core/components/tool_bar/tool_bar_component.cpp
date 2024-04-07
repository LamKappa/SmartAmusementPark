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

#include "core/components/tool_bar/tool_bar_component.h"

#include "base/log/log.h"
#include "core/components/tool_bar/render_tool_bar.h"
#include "core/components/tool_bar/tool_bar_element.h"

namespace OHOS::Ace {

RefPtr<RenderNode> ToolBarComponent::CreateRenderNode()
{
    return RenderToolBar::Create();
}

RefPtr<Element> ToolBarComponent::CreateElement()
{
    return AceType::MakeRefPtr<ToolBarElement>();
}

} // namespace OHOS::Ace
