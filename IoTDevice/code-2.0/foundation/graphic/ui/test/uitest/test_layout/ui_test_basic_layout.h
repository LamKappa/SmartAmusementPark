/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#ifndef UI_TEST_BASIC_LAYOUT_H
#define UI_TEST_BASIC_LAYOUT_H

#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "layout/flex_layout.h"
#include "layout/grid_layout.h"
#include "ui_test.h"

namespace OHOS {
class UITestBasicLayout : public UITest, public UIView::OnClickListener {
public:
    UITestBasicLayout() {}
    ~UITestBasicLayout() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    /**
     * @brief Test basic layout function.
     */
    void UIKit_Layout_Test_Basic_Layout_001();

    bool OnClick(UIView& view, const ClickEvent& event) override;
    void OnClickButton(UIView& view);
    void InitSetUpButton();
private:
    UIScrollView* container_ = nullptr;
    GridLayout* layout_ = nullptr;
    UIViewGroup* uiViewGroupFrame_ = nullptr;
    UIViewGroup* testTarget_ = nullptr;
    UIView* testTargetSmall1_ = nullptr;
    UIView* testTargetSmall2_ = nullptr;
    UIView* refTarget_ = nullptr;
    UILabelButton* resetBtn_ = nullptr;

    UILabelButton* lCenterOfParentBtn_ = nullptr;
    UILabelButton* lLeftOfParentBtn_ = nullptr;
    UILabelButton* lRightOfParentBtn_ = nullptr;
    UILabelButton* lTopOfParentBtn_ = nullptr;
    UILabelButton* lBottomOfParentBtn_ = nullptr;

    UILabelButton* aLeftToSiblingBtn_ = nullptr;
    UILabelButton* aRightToSiblingBtn_ = nullptr;
    UILabelButton* aTopToSiblingBtn_ = nullptr;
    UILabelButton* aBottomToSiblingBtn_ = nullptr;
    UILabelButton* aHorCenterToSiblingBtn_ = nullptr;
    UILabelButton* aVerCenterToSiblingBtn_ = nullptr;

    UILabelButton* lLeftToSiblingBtn_ = nullptr;
    UILabelButton* lRightToSiblingBtn_ = nullptr;
    UILabelButton* lTopToSiblingBtn_ = nullptr;
    UILabelButton* lBottomToSiblingBtn_ = nullptr;
    UILabelButton* lInvalidLayoutBtn_ = nullptr;
    UILabelButton* marginBtn_ = nullptr;
    UILabelButton* padBtn_ = nullptr;
    UILabelButton* setStyle_ = nullptr;

    void SetUpButton(UILabelButton* btn, const char* title)
    {
        if (btn == nullptr) {
            return;
        }
        layout_->Add(btn);
        btn->Resize(BUTTON_WIDHT2, BUTTON_HEIGHT2);
        btn->SetText(title);
        btn->SetFont(DEFAULT_VECTOR_FONT_FILENAME, BUTTON_LABEL_SIZE);
        btn->SetOnClickListener(this);
        btn->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::RELEASED);
        btn->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::PRESSED);
        btn->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::INACTIVE);
        btn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::RELEASED);
        btn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::PRESSED);
        btn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::INACTIVE);
    }
};
} // namespace OHOS
#endif // UI_TEST_INPUT_EVENT_H
