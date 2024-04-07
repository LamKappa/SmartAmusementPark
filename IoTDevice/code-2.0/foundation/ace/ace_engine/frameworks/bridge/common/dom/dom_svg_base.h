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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_SVG_BASE_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_SVG_BASE_H

#include "base/geometry/rect.h"
#include "frameworks/bridge/common/dom/dom_node.h"
#include "frameworks/bridge/common/dom/dom_type.h"
#include "frameworks/core/components/common/properties/svg_paint_state.h"
#include "frameworks/core/components/svg/svg_sharp.h"


namespace OHOS::Ace::Framework {
class DOMSvgBase : public DOMNode {
    DECLARE_ACE_TYPE(DOMSvgBase, DOMNode);

public:
    DOMSvgBase(NodeId nodeId, const std::string& nodeName);
    ~DOMSvgBase() override = default;

    bool SetPresentationAttr(const std::pair<std::string, std::string>& attr);

    const FillState& GetFillState() const
    {
        return fillState_;
    }

    const StrokeState& GetStrokeState() const
    {
        return strokeState_;
    }

    const SvgTextStyle& GetTextStyle() const
    {
        return textStyle_;
    }

protected:
    void InheritCommonAttrs(const RefPtr<SvgSharp>& svgSharp, const RefPtr<DOMNode>& parentNode);
    void PrepareCommonAttrs(const RefPtr<SvgSharp>& svgSharp);
    FillState fillState_;
    StrokeState strokeState_;
    SvgTextStyle textStyle_;
};

} // namespace OHOS::Ace::Framework


#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_SVG_BASE_H
