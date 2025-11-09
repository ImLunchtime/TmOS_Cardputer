#pragma once
#include <lvgl.h>
#include <mooncake.h>

class TestApp : public mooncake::AppAbility {
public:
    TestApp();
    ~TestApp() override;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    lv_obj_t* page_ = nullptr;
    lv_obj_t* label_ = nullptr;
    lv_obj_t* ta_ = nullptr;
};