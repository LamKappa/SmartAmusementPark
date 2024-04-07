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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_RENDER_SVG_POLYLINE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_RENDER_SVG_POLYLINE_H

#include "frameworks/core/components/svg/render_svg_base.h"

namespace OHOS::Ace {

class RenderSvgPolyline : public RenderSvgBase {
    DECLARE_ACE_TYPE(RenderSvgPolyline, RenderSvgBase);

public:
    static RefPtr<RenderNode> Create();

    void Update(const RefPtr<Component>& component) override;
    void PerformLayout() override;
    bool PrepareSelfAnimation(const RefPtr<SvgAnimate>& svgAnimate) override;

    void SetWeight(float weight)
    {
        weight_ = weight;
    }

    const std::string& GetPoints() const
    {
        return points_;
    }

protected:
    std::string points_;
    float weight_ = 0.0f;
    std::vector<std::string> pointsVector_;
    bool isBy_ = false;

private:
    void PrepareAnimations(const RefPtr<Component>& component);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_RENDER_SVG_POLYLINE_H
