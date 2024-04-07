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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_LAYOUT_POSITION_PARAM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_LAYOUT_POSITION_PARAM_H

#include "base/geometry/dimension.h"
#include "core/components/common/layout/constants.h"

namespace OHOS::Ace {

struct PositionParam {
    std::pair<Dimension, bool> left = { 0.0_px, false };
    std::pair<Dimension, bool> right = { 0.0_px, false };
    std::pair<Dimension, bool> top = { 0.0_px, false };
    std::pair<Dimension, bool> bottom = { 0.0_px, false };
    PositionType type = PositionType::RELATIVE;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_LAYOUT_POSITION_PARAM_H
