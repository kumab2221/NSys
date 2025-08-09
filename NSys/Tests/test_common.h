#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "interfaces.h"

// ImGui のモック定義
struct ImVec2 {
    float x = 0.0f;
    float y = 0.0f;
    
    ImVec2() = default;
    ImVec2(float x, float y) : x(x), y(y) {}
    
    bool operator==(const ImVec2& other) const {
        return x == other.x && y == other.y;
    }
};

// テスト用の基本設定
class NSysCoreTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        // 各テスト前にServiceLocatorをリセット
        NSys::ServiceLocator::Shutdown();
    }
    
    void TearDown() override {
        // 各テスト後にクリーンアップ
        NSys::ServiceLocator::Shutdown();
    }
};