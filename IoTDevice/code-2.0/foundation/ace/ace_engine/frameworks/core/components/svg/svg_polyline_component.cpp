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

#include "frameworks/core/components/svg/svg_polyline_component.h"

#include "frameworks/core/components/svg/render_svg_polyline.h"
#include "frameworks/core/components/svg/svg_polyline_element.h"

namespace OHOS::Ace {

RefPtr<Element> SvgPolylineComponent::CreateElement()
{
    return AceType::MakeRefPtr<SvgPolylineElement>();
}

RefPtr<RenderNode> SvgPolylineComponent::CreateRenderNode()
{
    return RenderSvgPolyline::Create();
}

} // namespace OHOS::Ace
