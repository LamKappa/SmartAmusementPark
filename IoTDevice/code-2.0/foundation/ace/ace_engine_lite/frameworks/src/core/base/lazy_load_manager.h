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

#ifndef OHOS_ACELITE_LAZY_LOAD_MANAGER_H
#define OHOS_ACELITE_LAZY_LOAD_MANAGER_H

#include "lazy_load_watcher.h"
#include "non_copyable.h"

namespace OHOS {
namespace ACELite {
/**
 * lazy load state
 */
enum class LazyLoadState : uint8_t {
    INIT,
    READY,
    DONE,
    ABANDON
};

class LazyLoadManager final : public MemoryHeap {
public:
    ACE_DISALLOW_COPY_AND_MOVE(LazyLoadManager);
    LazyLoadManager();

    ~LazyLoadManager();

    void ResetWatchers();

    /**
     * @brief Cache watcher
     */
    void AddLazyLoadWatcher(jerry_value_t nativeElement, jerry_value_t attrName, jerry_value_t getter);

    /**
     * @brief Render watcher at next TE task
     */
    void RenderLazyLoadWatcher();

    const LazyLoadWatcher *GetLazyWatcher() const
    {
        return firstWatcher_;
    }

    void SetState(LazyLoadState state)
    {
        state_ = state;
    }

    LazyLoadState GetState() const
    {
        return state_;
    }
private:
    LazyLoadWatcher *firstWatcher_;
    LazyLoadWatcher *lastWatcher_;
    LazyLoadState state_;
};
} // namespace ACELite
} // namespace OHOS
#endif // OHOS_ACELITE_LAZY_LOAD_MANAGER_H
