# TASK-003: ImGuiコア統合 - RED Phase (失敗するテストの実装)

## TDD RED Phase の目的

この段階では、まず**失敗するテスト**を実装します。これにより：
1. ImGuiコア統合システムの期待される動作を明確に定義
2. 実装すべき機能の具体的な仕様を確定
3. テストが確実に機能をテストしていることを保証

## 実装手順

### 1. テストプロジェクトの拡張

既存のNSys Testsプロジェクトを拡張し、ImGui統合テストを追加します。

#### テスト共通定義の拡張

**ファイル**: `NSys/Tests/test_imgui_common.h`

```cpp
#pragma once

#include "test_common.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <chrono>
#include <memory>
#include <filesystem>

// Windows関連のモック/ヘルパー
class TestWindowHelper {
public:
    static HWND CreateTestWindow(const std::string& title = "Test", int width = 800, int height = 600) {
        // 実装時にテスト用ウィンドウを作成
        // この時点では未実装のため、nullptrを返して失敗させる
        return nullptr;
    }
    
    static void DestroyTestWindow(HWND hwnd) {
        // ウィンドウ破棄（未実装）
    }
};

// DirectX 12モック
class MockD3D12Device {
public:
    ID3D12Device* GetDevice() {
        // この時点では未実装
        return nullptr;
    }
    
    ID3D12CommandQueue* GetCommandQueue() {
        // この時点では未実装
        return nullptr;
    }
};

// パフォーマンス測定ヘルパー
class PerformanceTimer {
public:
    void Start() {
        m_startTime = std::chrono::high_resolution_clock::now();
    }
    
    void End() {
        m_endTime = std::chrono::high_resolution_clock::now();
    }
    
    float GetDurationMs() const {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_endTime - m_startTime);
        return duration.count() / 1000.0f;
    }
    
private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_endTime;
};

// テスト用の基本設定（ImGui統合テスト用）
class ImGuiTestBase : public NSysCoreTestBase {
protected:
    void SetUp() override {
        NSysCoreTestBase::SetUp();
        // ImGui関連の初期化（後で実装）
    }
    
    void TearDown() override {
        // ImGui関連のクリーンアップ（後で実装）
        NSysCoreTestBase::TearDown();
    }
};
```

### 2. 失敗するテストの実装

#### DX12Renderer テスト (失敗版)

**ファイル**: `NSys/Tests/test_dx12renderer.cpp`

```cpp
#include "test_imgui_common.h"

// この時点では DX12Renderer クラスは存在しないため、
// すべてのテストがコンパイルエラーで失敗する

class DX12RendererTest : public ImGuiTestBase {};

TEST_F(DX12RendererTest, Initialize_ValidDevice) {
    // DX12Renderer クラスが未実装のため、コンパイルエラー
    DX12Renderer renderer;
    HWND hwnd = TestWindowHelper::CreateTestWindow();
    
    bool result = renderer.Initialize(hwnd);
    
    EXPECT_TRUE(result);
    EXPECT_NE(renderer.GetDevice(), nullptr);
    EXPECT_NE(renderer.GetCommandQueue(), nullptr);
    EXPECT_NE(renderer.GetCommandList(), nullptr);
    
    renderer.Shutdown();
    TestWindowHelper::DestroyTestWindow(hwnd);
}

TEST_F(DX12RendererTest, Initialize_InvalidWindow) {
    DX12Renderer renderer;  // コンパイルエラー: 未定義クラス
    HWND invalidHwnd = nullptr;
    
    bool result = renderer.Initialize(invalidHwnd);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(renderer.GetDevice(), nullptr);
}

TEST_F(DX12RendererTest, FrameManagement_BasicCycle) {
    DX12Renderer renderer;  // コンパイルエラー: 未定義クラス
    HWND hwnd = TestWindowHelper::CreateTestWindow();
    renderer.Initialize(hwnd);
    
    renderer.BeginFrame();  // コンパイルエラー: 未定義メソッド
    renderer.EndFrame();    // コンパイルエラー: 未定義メソッド
    renderer.Present();     // コンパイルエラー: 未定義メソッド
    
    float frameTime = renderer.GetFrameTime();  // コンパイルエラー
    EXPECT_GT(frameTime, 0.0f);
    
    renderer.Shutdown();
    TestWindowHelper::DestroyTestWindow(hwnd);
}

TEST_F(DX12RendererTest, PerformanceMonitoring_60FPS) {
    DX12Renderer renderer;  // コンパイルエラー: 未定義クラス
    HWND hwnd = TestWindowHelper::CreateTestWindow();
    renderer.Initialize(hwnd);
    
    // フレームレート測定
    for (int i = 0; i < 60; ++i) {
        renderer.BeginFrame();
        renderer.EndFrame();
        renderer.Present();
    }
    
    int fps = renderer.GetCurrentFPS();  // コンパイルエラー: 未定義メソッド
    EXPECT_GE(fps, 55);  // 60FPS近似
    
    bool maintaining60FPS = renderer.IsMaintaining60FPS();  // コンパイルエラー
    EXPECT_TRUE(maintaining60FPS);
    
    renderer.Shutdown();
    TestWindowHelper::DestroyTestWindow(hwnd);
}
```

#### Win32Platform テスト (失敗版)

**ファイル**: `NSys/Tests/test_win32platform.cpp`

```cpp
#include "test_imgui_common.h"

// Win32Platform クラスは未実装のため、全テストがコンパイルエラー
class Win32PlatformTest : public ImGuiTestBase {};

TEST_F(Win32PlatformTest, Initialize_ValidParameters) {
    Win32Platform platform;  // コンパイルエラー: 未定義クラス
    
    bool result = platform.Initialize("Test Window", 800, 600);
    
    EXPECT_TRUE(result);
    EXPECT_NE(platform.GetHWND(), nullptr);
    EXPECT_FALSE(platform.ShouldClose());
    
    platform.Shutdown();
}

TEST_F(Win32PlatformTest, WindowOperations_Resize) {
    Win32Platform platform;  // コンパイルエラー: 未定義クラス
    platform.Initialize("Test", 800, 600);
    
    platform.SetSize(1200, 900);  // コンパイルエラー: 未定義メソッド
    
    int width, height;
    platform.GetSize(width, height);  // コンパイルエラー: 未定義メソッド
    EXPECT_EQ(width, 1200);
    EXPECT_EQ(height, 900);
    
    platform.Shutdown();
}

TEST_F(Win32PlatformTest, DPISupport_EnableAwareness) {
    Win32Platform platform;  // コンパイルエラー: 未定義クラス
    platform.Initialize("Test", 800, 600);
    platform.EnableDPIAwareness();  // コンパイルエラー: 未定義メソッド
    
    float dpiScale = platform.GetDPIScale();  // コンパイルエラー: 未定義メソッド
    
    EXPECT_GT(dpiScale, 0.5f);
    EXPECT_LT(dpiScale, 4.0f);
    
    platform.Shutdown();
}

TEST_F(Win32PlatformTest, MessageLoop_ProcessMessages) {
    Win32Platform platform;  // コンパイルエラー: 未定義クラス
    platform.Initialize("Test", 800, 600);
    
    bool hasMessages = platform.ProcessMessages();  // コンパイルエラー
    
    EXPECT_FALSE(platform.ShouldClose());
    
    platform.Shutdown();
}

TEST_F(Win32PlatformTest, EventCallbacks_OnResize) {
    Win32Platform platform;  // コンパイルエラー: 未定義クラス
    platform.Initialize("Test", 800, 600);
    
    bool resizeCalled = false;
    platform.OnResize = [&resizeCalled](int w, int h) {  // コンパイルエラー
        resizeCalled = true;
    };
    
    // リサイズイベントのシミュレーション
    platform.SetSize(1024, 768);
    
    EXPECT_TRUE(resizeCalled);
    
    platform.Shutdown();
}
```

#### FrameRateController テスト (失敗版)

**ファイル**: `NSys/Tests/test_frameratecontroller.cpp`

```cpp
#include "test_imgui_common.h"

// FrameRateController クラスは未実装のため、全テストがコンパイルエラー
class FrameRateControllerTest : public ImGuiTestBase {};

TEST_F(FrameRateControllerTest, SetTargetFPS_60FPS) {
    FrameRateController controller;  // コンパイルエラー: 未定義クラス
    
    controller.SetTargetFPS(60);  // コンパイルエラー: 未定義メソッド
    
    // フレーム実行のシミュレーション
    for (int i = 0; i < 10; ++i) {
        controller.BeginFrame();  // コンパイルエラー: 未定義メソッド
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        controller.EndFrame();    // コンパイルエラー: 未定義メソッド
        controller.WaitForNextFrame();  // コンパイルエラー: 未定義メソッド
    }
    
    int currentFPS = controller.GetCurrentFPS();  // コンパイルエラー: 未定義メソッド
    EXPECT_NEAR(currentFPS, 60, 10);
    
    EXPECT_TRUE(controller.IsMaintainingTargetFPS());  // コンパイルエラー: 未定義メソッド
}

TEST_F(FrameRateControllerTest, FrameTimeAccuracy) {
    FrameRateController controller;  // コンパイルエラー: 未定義クラス
    controller.SetTargetFPS(60);
    
    controller.BeginFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    controller.EndFrame();
    
    float frameTime = controller.GetLastFrameTime();  // コンパイルエラー: 未定義メソッド
    EXPECT_NEAR(frameTime, 16.0f, 5.0f);
}

TEST_F(FrameRateControllerTest, PerformanceStatistics) {
    FrameRateController controller;  // コンパイルエラー: 未定義クラス
    controller.SetTargetFPS(60);
    
    // データ蓄積
    for (int i = 0; i < 60; ++i) {
        controller.BeginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        controller.EndFrame();
    }
    
    float averageFrameTime = controller.GetAverageFrameTime();  // コンパイルエラー
    EXPECT_NEAR(averageFrameTime, 16.0f, 2.0f);
    
    auto history = controller.GetFrameTimeHistory();  // コンパイルエラー: 未定義メソッド
    EXPECT_GT(history.size(), 0);
    
    controller.ResetStatistics();  // コンパイルエラー: 未定義メソッド
    EXPECT_EQ(controller.GetCurrentFPS(), 0);
}
```

#### ImGuiWindowManager テスト (失敗版)

**ファイル**: `NSys/Tests/test_imguiwindowmanager.cpp`

```cpp
#include "test_imgui_common.h"
#include "mock_services.h"

// ImGuiWindowManager クラスは未実装のため、全テストがコンパイルエラー
class ImGuiWindowManagerTest : public ImGuiTestBase {};

class MockWindowProvider : public NSys::IWindowProvider {
public:
    std::vector<NSys::WindowInfo> GetWindows() override {
        NSys::WindowInfo info;
        info.id = "test_window";
        info.title = "Test Window";
        info.defaultSize = {800, 600};
        return {info};
    }
    
    void RenderWindow(const std::string& windowId) override {
        m_renderCalled = true;
        m_lastRenderedId = windowId;
    }
    
    bool IsWindowOpen(const std::string& windowId) override {
        return m_windowOpen;
    }
    
    void SetWindowOpen(const std::string& windowId, bool open) override {
        m_windowOpen = open;
    }
    
    bool m_renderCalled = false;
    std::string m_lastRenderedId;
    bool m_windowOpen = true;
};

TEST_F(ImGuiWindowManagerTest, Initialize_Success) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    MockD3D12Device mockDevice;
    HWND hwnd = TestWindowHelper::CreateTestWindow();
    
    bool result = manager.Initialize(hwnd, mockDevice.GetDevice(), mockDevice.GetCommandQueue());
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());  // コンパイルエラー: 未定義メソッド
    
    manager.Shutdown();  // コンパイルエラー: 未定義メソッド
    TestWindowHelper::DestroyTestWindow(hwnd);
}

TEST_F(ImGuiWindowManagerTest, WindowRegistration_Success) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    auto mockProvider = std::make_shared<MockWindowProvider>();
    
    manager.RegisterWindow("test", mockProvider);  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(manager.IsWindowVisible("test"));  // コンパイルエラー: 未定義メソッド
}

TEST_F(ImGuiWindowManagerTest, RenderingCycle_Complete) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    // 初期化は省略（コンパイルエラーのため）
    
    manager.NewFrame();   // コンパイルエラー: 未定義メソッド
    manager.Render();     // コンパイルエラー: 未定義メソッド
    manager.EndFrame();   // コンパイルエラー: 未定義メソッド
    
    EXPECT_FALSE(manager.HasErrors());
}

TEST_F(ImGuiWindowManagerTest, FPSMonitoring_TargetFPS) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    
    manager.SetTargetFPS(60);  // コンパイルエラー: 未定義メソッド
    
    // フレーム実行
    for (int i = 0; i < 10; ++i) {
        manager.NewFrame();
        manager.Render();
        manager.EndFrame();
    }
    
    int fps = manager.GetCurrentFPS();  // コンパイルエラー: 未定義メソッド
    EXPECT_GT(fps, 0);
    
    float frameTime = manager.GetFrameTime();  // コンパイルエラー: 未定義メソッド
    EXPECT_GT(frameTime, 0.0f);
}

TEST_F(ImGuiWindowManagerTest, DockingSupport_LayoutManagement) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    
    manager.SetupDockLayout();  // コンパイルエラー: 未定義メソッド
    
    std::string layoutFile = "test_layout.ini";
    manager.SaveDockLayout(layoutFile);  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(std::filesystem::exists(layoutFile));
    
    bool loadResult = manager.LoadDockLayout(layoutFile);  // コンパイルエラー
    EXPECT_TRUE(loadResult);
    
    std::filesystem::remove(layoutFile);
}
```

#### PluginIntegration テスト (失敗版)

**ファイル**: `NSys/Tests/test_pluginintegration.cpp`

```cpp
#include "test_imgui_common.h"
#include "mock_services.h"

// PluginIntegration クラスは未実装のため、全テストがコンパイルエラー
class PluginIntegrationTest : public ImGuiTestBase {};

class MockPlugin : public NSys::IPlugin {
public:
    bool Initialize() override { 
        m_initialized = true; 
        return true; 
    }
    void Shutdown() override { 
        m_initialized = false; 
    }
    void Update(float deltaTime) override { 
        m_lastDeltaTime = deltaTime; 
        m_updateCalled = true;
    }
    void Render() override { 
        m_renderCalled = true; 
    }
    
    NSys::PluginInfo GetPluginInfo() const override {
        NSys::PluginInfo info;
        info.name = "MockPlugin";
        info.version = "1.0.0";
        return info;
    }
    
    std::string GetName() const override { return "MockPlugin"; }
    std::string GetVersion() const override { return "1.0.0"; }
    bool IsInitialized() const override { return m_initialized; }
    void SetEnabled(bool enabled) override { m_enabled = enabled; }
    bool IsEnabled() const override { return m_enabled; }
    
    // テスト用メソッド
    float GetLastDeltaTime() const { return m_lastDeltaTime; }
    bool WasRenderCalled() const { return m_renderCalled; }
    bool WasUpdateCalled() const { return m_updateCalled; }
    
private:
    bool m_initialized = false;
    bool m_enabled = true;
    float m_lastDeltaTime = 0.0f;
    bool m_renderCalled = false;
    bool m_updateCalled = false;
};

class MockMenuProvider : public NSys::IMenuProvider {
public:
    std::vector<NSys::MenuItem> GetMenuItems() override {
        NSys::MenuItem item;
        item.id = "test_menu";
        item.label = "Test Menu";
        item.callback = [this]() { m_menuClicked = true; };
        return {item};
    }
    
    void OnMenuItemSelected(const std::string& itemId) override {
        m_selectedItemId = itemId;
    }
    
    bool m_menuClicked = false;
    std::string m_selectedItemId;
};

TEST_F(PluginIntegrationTest, RegisterPlugin_Success) {
    PluginIntegration integration;  // コンパイルエラー: 未定義クラス
    auto mockPlugin = std::make_shared<MockPlugin>();
    
    integration.RegisterPlugin("test_plugin", mockPlugin);  // コンパイルエラー
    
    auto activePlugins = integration.GetActivePlugins();  // コンパイルエラー
    EXPECT_EQ(activePlugins.size(), 1);
    EXPECT_EQ(activePlugins[0], "test_plugin");
    EXPECT_TRUE(integration.IsPluginActive("test_plugin"));  // コンパイルエラー
}

TEST_F(PluginIntegrationTest, UpdatePlugins_CallsUpdate) {
    PluginIntegration integration;  // コンパイルエラー: 未定義クラス
    auto mockPlugin = std::make_shared<MockPlugin>();
    
    integration.RegisterPlugin("test_plugin", mockPlugin);
    integration.UpdatePlugins(16.67f);  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(mockPlugin->WasUpdateCalled());
    EXPECT_EQ(mockPlugin->GetLastDeltaTime(), 16.67f);
}

TEST_F(PluginIntegrationTest, RenderPlugins_CallsRender) {
    PluginIntegration integration;  // コンパイルエラー: 未定義クラス
    auto mockPlugin = std::make_shared<MockPlugin>();
    
    integration.RegisterPlugin("test_plugin", mockPlugin);
    integration.RenderPlugins();  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(mockPlugin->WasRenderCalled());
}

TEST_F(PluginIntegrationTest, MenuIntegration_AddProvider) {
    PluginIntegration integration;  // コンパイルエラー: 未定義クラス
    auto mockMenuProvider = std::make_shared<MockMenuProvider>();
    
    integration.AddMenuProvider("test_menu", mockMenuProvider);  // コンパイルエラー
    
    // メニューバー描画のシミュレーション
    integration.RenderMainMenuBar();  // コンパイルエラー: 未定義メソッド
    
    // メニュー項目の存在確認は実装時に詳細化
    EXPECT_TRUE(true);
}

TEST_F(PluginIntegrationTest, WindowIntegration_AddWindowProvider) {
    PluginIntegration integration;  // コンパイルエラー: 未定義クラス
    auto mockWindowProvider = std::make_shared<MockWindowProvider>();
    
    integration.AddWindowProvider("test_window", mockWindowProvider);  // コンパイルエラー
    integration.RenderPluginWindows();  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(mockWindowProvider->m_renderCalled);
}
```

#### パフォーマンステスト (失敗版)

**ファイル**: `NSys/Tests/test_performance.cpp`

```cpp
#include "test_imgui_common.h"

// パフォーマンステストも各クラスの未実装により失敗
class PerformanceTest : public ImGuiTestBase {};

TEST_F(PerformanceTest, Maintain60FPS_UnderLoad) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    FrameRateController controller;  // コンパイルエラー: 未定義クラス
    
    controller.SetTargetFPS(60);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    const int testFrames = 600; // 10秒分
    int framesAbove60 = 0;
    
    for (int frame = 0; frame < testFrames; ++frame) {
        controller.BeginFrame();  // コンパイルエラー: 未定義メソッド
        
        manager.NewFrame();  // コンパイルエラー: 未定義メソッド
        
        // 複数のプラグインウィンドウをシミュレート
        for (int i = 0; i < 10; ++i) {
            // ウィンドウ描画のシミュレーション
        }
        
        manager.Render();    // コンパイルエラー: 未定義メソッド
        manager.EndFrame();  // コンパイルエラー: 未定義メソッド
        
        controller.EndFrame();  // コンパイルエラー: 未定義メソッド
        
        if (controller.GetCurrentFPS() >= 60) {  // コンパイルエラー
            framesAbove60++;
        }
        
        controller.WaitForNextFrame();  // コンパイルエラー: 未定義メソッド
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 60FPS維持率が95%以上であることを確認
    float maintainRate = static_cast<float>(framesAbove60) / testFrames;
    EXPECT_GE(maintainRate, 0.95f);
    
    EXPECT_NEAR(duration.count(), 10000, 1000);
}

TEST_F(PerformanceTest, MemoryUsage_WithinLimits) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    
    // メモリ使用量測定
    PROCESS_MEMORY_COUNTERS_EX memCounters;
    GetProcessMemoryInfo(GetCurrentProcess(), 
                        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memCounters), 
                        sizeof(memCounters));
    SIZE_T initialMemory = memCounters.WorkingSetSize;
    
    // ウィンドウマネージャーの実行
    for (int i = 0; i < 100; ++i) {
        manager.NewFrame();  // コンパイルエラー: 未定義メソッド
        
        // 複数ウィンドウの描画シミュレーション
        for (int j = 0; j < 20; ++j) {
            // 描画処理
        }
        
        manager.Render();    // コンパイルエラー: 未定義メソッド
        manager.EndFrame();  // コンパイルエラー: 未定義メソッド
    }
    
    GetProcessMemoryInfo(GetCurrentProcess(), 
                        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memCounters), 
                        sizeof(memCounters));
    SIZE_T finalMemory = memCounters.WorkingSetSize;
    SIZE_T usedMemory = finalMemory - initialMemory;
    
    // メモリ使用量が256MB以下であることを確認
    EXPECT_LT(usedMemory, 256 * 1024 * 1024);
}

TEST_F(PerformanceTest, WindowResize_Responsiveness) {
    Win32Platform platform;      // コンパイルエラー: 未定義クラス
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    platform.Initialize("Test", 800, 600);
    
    std::vector<float> resizeTimes;
    
    for (int i = 0; i < 10; ++i) {
        PerformanceTimer timer;
        timer.Start();
        
        // ウィンドウリサイズのシミュレーション
        int newWidth = 800 + i * 100;
        int newHeight = 600 + i * 75;
        platform.SetSize(newWidth, newHeight);  // コンパイルエラー: 未定義メソッド
        
        // レンダリング更新
        manager.NewFrame();   // コンパイルエラー: 未定義メソッド
        manager.Render();     // コンパイルエラー: 未定義メソッド
        manager.EndFrame();   // コンパイルエラー: 未定義メソッド
        
        timer.End();
        resizeTimes.push_back(timer.GetDurationMs());
    }
    
    // すべてのリサイズが100ms以内に完了することを確認
    for (float time : resizeTimes) {
        EXPECT_LT(time, 100.0f);
    }
    
    float averageTime = std::accumulate(resizeTimes.begin(), resizeTimes.end(), 0.0f) / resizeTimes.size();
    EXPECT_LT(averageTime, 50.0f);
    
    platform.Shutdown();  // コンパイルエラー: 未定義メソッド
}
```

#### 統合テスト (失敗版)

**ファイル**: `NSys/Tests/test_imgui_integration.cpp`

```cpp
#include "test_imgui_common.h"
#include "mock_services.h"

// 統合テストも各クラスの未実装により失敗
class ImGuiIntegrationTest : public ImGuiTestBase {};

TEST_F(ImGuiIntegrationTest, FullSystem_PluginWithWindows) {
    // システム全体の統合テスト（すべて未実装によりコンパイルエラー）
    Win32Platform platform;         // コンパイルエラー: 未定義クラス
    DX12Renderer renderer;          // コンパイルエラー: 未定義クラス
    ImGuiWindowManager manager;     // コンパイルエラー: 未定義クラス
    PluginIntegration integration;  // コンパイルエラー: 未定義クラス
    
    // 初期化
    platform.Initialize("NSys Integration Test", 1280, 720);
    renderer.Initialize(platform.GetHWND());
    manager.Initialize(platform.GetHWND(), renderer.GetDevice(), renderer.GetCommandQueue());
    
    // プラグインとプロバイダーの登録
    auto mockPlugin = std::make_shared<MockPlugin>();
    auto mockWindowProvider = std::make_shared<MockWindowProvider>();
    auto mockMenuProvider = std::make_shared<MockMenuProvider>();
    
    integration.RegisterPlugin("test_plugin", mockPlugin);
    integration.AddWindowProvider("test_window", mockWindowProvider);
    integration.AddMenuProvider("test_menu", mockMenuProvider);
    manager.RegisterWindow("test_window", mockWindowProvider);
    
    // フルレンダリングサイクルの実行
    for (int frame = 0; frame < 60; ++frame) {
        platform.ProcessMessages();
        if (platform.ShouldClose()) break;
        
        renderer.BeginFrame();
        manager.NewFrame();
        
        integration.RenderMainMenuBar();
        integration.UpdatePlugins(16.67f);
        integration.RenderPlugins();
        integration.RenderPluginWindows();
        
        manager.Render();
        manager.EndFrame();
        renderer.EndFrame();
        renderer.Present();
    }
    
    // 統合動作の確認
    EXPECT_TRUE(mockPlugin->IsInitialized());
    EXPECT_TRUE(integration.IsPluginActive("test_plugin"));
    EXPECT_TRUE(manager.IsWindowVisible("test_window"));
    EXPECT_FALSE(manager.HasErrors());
    EXPECT_FALSE(renderer.GetDevice() == nullptr);
    
    // クリーンアップ
    integration.UnregisterPlugin("test_plugin");
    manager.Shutdown();
    renderer.Shutdown();
    platform.Shutdown();
}

TEST_F(ImGuiIntegrationTest, ErrorHandling_GracefulDegradation) {
    ImGuiWindowManager manager;  // コンパイルエラー: 未定義クラス
    
    // 無効な初期化パラメータでテスト
    bool result = manager.Initialize(nullptr, nullptr, nullptr);
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());  // コンパイルエラー: 未定義メソッド
    EXPECT_FALSE(manager.GetLastError().empty());  // コンパイルエラー
    
    // エラー状態でも安全に呼び出せることを確認
    EXPECT_NO_THROW(manager.NewFrame());   // コンパイルエラー: 未定義メソッド
    EXPECT_NO_THROW(manager.Render());     // コンパイルエラー: 未定義メソッド
    EXPECT_NO_THROW(manager.EndFrame());   // コンパイルエラー: 未定義メソッド
    EXPECT_NO_THROW(manager.Shutdown());   // コンパイルエラー: 未定義メソッド
}
```

### 3. プロジェクト設定の更新

**NSys.vcxproj に追加するファイル**:
```xml
<ItemGroup>
  <ClCompile Include="Tests\test_dx12renderer.cpp" />
  <ClCompile Include="Tests\test_win32platform.cpp" />
  <ClCompile Include="Tests\test_frameratecontroller.cpp" />
  <ClCompile Include="Tests\test_imguiwindowmanager.cpp" />
  <ClCompile Include="Tests\test_pluginintegration.cpp" />
  <ClCompile Include="Tests\test_performance.cpp" />
  <ClCompile Include="Tests\test_imgui_integration.cpp" />
</ItemGroup>

<ItemGroup>
  <ClInclude Include="Tests\test_imgui_common.h" />
</ItemGroup>
```

### 4. ビルド実行とエラー確認

この段階でテストプロジェクトをビルドすると、以下のようなコンパイルエラーが発生することが期待されます：

```cpp
// 期待されるコンパイルエラー:

// DX12Renderer関連
error C2065: 'DX12Renderer': undeclared identifier
error C2228: left of '.Initialize' must have class/struct/union type
error C2228: left of '.GetDevice' must have class/struct/union type

// Win32Platform関連  
error C2065: 'Win32Platform': undeclared identifier
error C2228: left of '.SetSize' must have class/struct/union type
error C2228: left of '.GetDPIScale' must have class/struct/union type

// FrameRateController関連
error C2065: 'FrameRateController': undeclared identifier
error C2228: left of '.SetTargetFPS' must have class/struct/union type
error C2228: left of '.GetCurrentFPS' must have class/struct/union type

// ImGuiWindowManager関連
error C2065: 'ImGuiWindowManager': undeclared identifier
error C2228: left of '.RegisterWindow' must have class/struct/union type
error C2228: left of '.NewFrame' must have class/struct/union type

// PluginIntegration関連
error C2065: 'PluginIntegration': undeclared identifier
error C2228: left of '.RegisterPlugin' must have class/struct/union type
error C2228: left of '.UpdatePlugins' must have class/struct/union type
```

### 5. 期待される失敗の確認

#### コンパイルエラー一覧
1. **DX12Renderer** クラス未定義 - 全メソッド未実装
2. **Win32Platform** クラス未定義 - 全メソッド未実装  
3. **FrameRateController** クラス未定義 - 全メソッド未実装
4. **ImGuiWindowManager** クラス未定義 - 全メソッド未実装
5. **PluginIntegration** クラス未定義 - 全メソッド未実装

#### ヘルパークラスの不完全実装
- **TestWindowHelper** - ウィンドウ作成メソッドが `nullptr` を返す
- **MockD3D12Device** - モックデバイスが `nullptr` を返す

これらのエラーは **期待された失敗** であり、次の GREEN フェーズで解決していきます。

## RED Phase 完了チェックリスト

- [x] ImGui統合テストプロジェクトが作成された
- [x] すべての主要クラスのテストケースが実装された
- [x] テストは現在すべてコンパイルエラーで失敗する（期待された動作）
- [x] 失敗の原因が明確に特定できる（未実装のクラスとメソッド）
- [x] テストケースは実装すべき機能を正確に表現している
- [x] パフォーマンステストと統合テストも含まれている

## 次のステップ

GREEN フェーズでは、これらのテストを通すための **最小限の実装** を行います：

1. **DX12Renderer** クラスの基本実装
2. **Win32Platform** クラスの基本実装  
3. **FrameRateController** クラスの基本実装
4. **ImGuiWindowManager** クラスの基本実装
5. **PluginIntegration** クラスの基本実装
6. **テストヘルパー** の実装

各実装は「テストを通す」ことを目的とした最小限に留め、過度な実装を避けます。実際の機能実装は次のREFACTORフェーズで行います。