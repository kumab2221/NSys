# TASK-101: プラグインマネージャー実装 - RED Phase (失敗するテストの実装)

## TDD RED Phase の目的

この段階では、まず**失敗するテスト**を実装します。これにより：
1. プラグインマネージャーシステムの期待される動作を明確に定義
2. 実装すべき機能の具体的な仕様を確定
3. テストが確実に機能をテストしていることを保証

## 実装手順

### 1. テストプロジェクトの拡張

既存のNSys Testsプロジェクトを拡張し、プラグインマネージャーテストを追加します。

#### プラグインマネージャーテスト共通定義の追加

**ファイル**: `NSys/Tests/test_plugin_manager_common.h`

```cpp
#pragma once

#include "test_common.h"
#include "test_imgui_common.h"
#include <windows.h>
#include <memory>
#include <vector>
#include <filesystem>
#include <chrono>

// この時点では PluginManager クラスは存在しないため、
// すべてのテストがコンパイルエラーで失敗する

// テスト用DLL生成ヘルパー（実装時に詳細化）
class TestPluginDLLGenerator {
public:
    static void CreateTestPluginDLL(const std::string& filename, 
                                   bool isValid = true,
                                   const std::vector<std::string>& dependencies = {}) {
        // この時点では未実装のため、空のファイルを作成してテストを失敗させる
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            file << "DUMMY"; // DLLではない無効なコンテンツ
            file.close();
        }
    }
    
    static void CreateValidPluginDLL(const std::string& filename) {
        CreateTestPluginDLL(filename, true);
    }
    
    static void CreateInvalidDLL(const std::string& filename) {
        // テキストファイルとして作成（DLLではない）
        std::ofstream file(filename);
        if (file.is_open()) {
            file << "This is not a DLL file";
            file.close();
        }
    }
    
    static void CreateCorruptedDLL(const std::string& filename) {
        // 破損したDLLとして作成
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            file << "CORRUPTED_DLL_DATA";
            file.close();
        }
    }
    
    static void CreateMissingEntryPointDLL(const std::string& filename) {
        // エントリポイントが欠けているDLL（実際には無効なファイル）
        CreateCorruptedDLL(filename);
    }
    
    static void CleanupTestFiles() {
        // テストファイルのクリーンアップ
        std::filesystem::remove_all("test_plugins");
        std::filesystem::remove("test_plugin_config.ini");
    }
};

// テスト用の基本設定
class PluginManagerTestBase : public ImGuiTestBase {
protected:
    void SetUp() override {
        ImGuiTestBase::SetUp();
        
        // テストディレクトリの準備
        std::filesystem::create_directories("test_plugins");
    }
    
    void TearDown() override {
        // テストファイルのクリーンアップ
        TestPluginDLLGenerator::CleanupTestFiles();
        
        ImGuiTestBase::TearDown();
    }
};
```

### 2. 失敗するテストの実装

#### PluginManager基本機能テスト (失敗版)

**ファイル**: `NSys/Tests/test_plugin_manager.cpp`

```cpp
#include "test_plugin_manager_common.h"

// この時点では PluginManager クラスは存在しないため、
// すべてのテストがコンパイルエラーで失敗する

class PluginManagerTest : public PluginManagerTestBase {};

TEST_F(PluginManagerTest, Initialize_Success) {
    // PluginManager クラスが未実装のため、コンパイルエラー
    PluginManager manager;
    
    bool result = manager.Initialize("test_plugins");
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    EXPECT_EQ(manager.GetMaxPluginCount(), 32); // デフォルト値
    
    manager.Shutdown();
}

TEST_F(PluginManagerTest, Initialize_InvalidDirectory) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    
    bool result = manager.Initialize("nonexistent_directory");
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());
    
    manager.Shutdown();
}

TEST_F(PluginManagerTest, Shutdown_ProperCleanup) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // プラグインをいくつか読み込み
    manager.LoadPlugin("test_plugins/TestPlugin1.dll");  // コンパイルエラー: 未定義メソッド
    manager.LoadPlugin("test_plugins/TestPlugin2.dll");  // コンパイルエラー: 未定義メソッド
    
    EXPECT_GT(manager.GetLoadedPluginCount(), 0);  // コンパイルエラー: 未定義メソッド
    
    manager.Shutdown();  // コンパイルエラー: 未定義メソッド
    
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    EXPECT_FALSE(manager.HasErrors());  // コンパイルエラー: 未定義メソッド
}

TEST_F(PluginManagerTest, GetMaxPluginCount_DefaultValue) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    
    uint32_t maxCount = manager.GetMaxPluginCount();  // コンパイルエラー: 未定義メソッド
    
    EXPECT_EQ(maxCount, 32);
}

TEST_F(PluginManagerTest, SetMaxPluginCount_CustomValue) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    
    manager.SetMaxPluginCount(10);  // コンパイルエラー: 未定義メソッド
    
    uint32_t maxCount = manager.GetMaxPluginCount();
    EXPECT_EQ(maxCount, 10);
}
```

#### プラグイン読み込みテスト (失敗版)

**ファイル**: `NSys/Tests/test_plugin_load.cpp`

```cpp
#include "test_plugin_manager_common.h"

// プラグイン読み込み機能のテスト（すべて未実装によりコンパイルエラー）
class PluginManagerLoadTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        
        // テスト用プラグインDLLを準備
        TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/ValidPlugin.dll");
        TestPluginDLLGenerator::CreateInvalidDLL("test_plugins/InvalidPlugin.dll");
        TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/DependentPlugin.dll", 
                                                     true, {"ValidPlugin"});
    }
};

TEST_F(PluginManagerLoadTest, LoadPlugin_ValidPlugin_Success) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.LoadPlugin("test_plugins/ValidPlugin.dll");  // コンパイルエラー
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());
    EXPECT_EQ(manager.GetLoadedPluginCount(), 1);  // コンパイルエラー: 未定義メソッド
    EXPECT_TRUE(manager.IsPluginLoaded("ValidPlugin"));  // コンパイルエラー: 未定義メソッド
    EXPECT_EQ(manager.GetPluginState("ValidPlugin"), PluginState::Active);  // コンパイルエラー
    
    auto plugin = manager.GetPlugin("ValidPlugin");  // コンパイルエラー: 未定義メソッド
    EXPECT_NE(plugin, nullptr);
    EXPECT_TRUE(plugin->IsInitialized());
    
    manager.Shutdown();
}

TEST_F(PluginManagerLoadTest, LoadPlugin_InvalidPlugin_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.LoadPlugin("test_plugins/InvalidPlugin.dll");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());  // コンパイルエラー: 未定義メソッド
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    EXPECT_FALSE(manager.IsPluginLoaded("InvalidPlugin"));
    
    std::string error = manager.GetLastError();  // コンパイルエラー: 未定義メソッド
    EXPECT_FALSE(error.empty());
    EXPECT_NE(error.find("Invalid plugin"), std::string::npos);
    
    manager.Shutdown();
}

TEST_F(PluginManagerLoadTest, LoadPlugin_NonexistentFile_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.LoadPlugin("test_plugins/Nonexistent.dll");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    
    std::string error = manager.GetLastError();
    EXPECT_NE(error.find("File not found"), std::string::npos);
    
    manager.Shutdown();
}

TEST_F(PluginManagerLoadTest, LoadPlugin_WithConfig_Success) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    
    PluginConfig config;  // コンパイルエラー: 未定義構造体
    config.enabled = true;
    config.autoLoad = true;
    config.priority = 100;
    config.parameters["test_param"] = "test_value";
    
    manager.Initialize("test_plugins");
    bool result = manager.LoadPlugin("test_plugins/ValidPlugin.dll", config);  // コンパイルエラー
    
    EXPECT_TRUE(result);
    
    auto plugin = manager.GetPlugin("ValidPlugin");
    EXPECT_NE(plugin, nullptr);
    
    manager.Shutdown();
}
```

#### プラグインアンロードテスト (失敗版)

**ファイル**: `NSys/Tests/test_plugin_unload.cpp`

```cpp
#include "test_plugin_manager_common.h"

// プラグインアンロード機能のテスト（すべて未実装によりコンパイルエラー）
class PluginManagerUnloadTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/ValidPlugin.dll");
    }
};

TEST_F(PluginManagerUnloadTest, UnloadPlugin_LoadedPlugin_Success) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // プラグイン読み込み
    manager.LoadPlugin("test_plugins/ValidPlugin.dll");  // コンパイルエラー
    EXPECT_TRUE(manager.IsPluginLoaded("ValidPlugin"));  // コンパイルエラー
    
    // アンロード
    bool result = manager.UnloadPlugin("ValidPlugin");  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.IsPluginLoaded("ValidPlugin"));
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    EXPECT_EQ(manager.GetPluginState("ValidPlugin"), PluginState::Unloaded);  // コンパイルエラー
    
    auto plugin = manager.GetPlugin("ValidPlugin");
    EXPECT_EQ(plugin, nullptr);
    
    manager.Shutdown();
}

TEST_F(PluginManagerUnloadTest, UnloadPlugin_NotLoaded_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.UnloadPlugin("NonexistentPlugin");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());
    
    std::string error = manager.GetLastError();
    EXPECT_NE(error.find("Plugin not found"), std::string::npos);
    
    manager.Shutdown();
}

TEST_F(PluginManagerUnloadTest, ReloadPlugin_LoadedPlugin_Success) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // 初回読み込み
    manager.LoadPlugin("test_plugins/ValidPlugin.dll");
    auto originalPlugin = manager.GetPlugin("ValidPlugin");
    EXPECT_NE(originalPlugin, nullptr);
    
    // リロード
    bool result = manager.ReloadPlugin("ValidPlugin");  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(manager.IsPluginLoaded("ValidPlugin"));
    
    auto reloadedPlugin = manager.GetPlugin("ValidPlugin");
    EXPECT_NE(reloadedPlugin, nullptr);
    
    manager.Shutdown();
}
```

#### 依存関係管理テスト (失敗版)

**ファイル**: `NSys/Tests/test_dependency_management.cpp`

```cpp
#include "test_plugin_manager_common.h"

// 依存関係管理のテスト（すべて未実装によりコンパイルエラー）
class DependencyTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        
        // 依存関係のあるテストプラグイン群を作成
        TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/BasePlugin.dll");
        TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/MiddlePlugin.dll", 
                                                     true, {"BasePlugin"});
        TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/TopPlugin.dll", 
                                                     true, {"MiddlePlugin"});
        TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/IndependentPlugin.dll");
    }
};

TEST_F(DependencyTest, LoadPlugin_WithDependency_CorrectOrder) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // 依存関係: TopPlugin → MiddlePlugin → BasePlugin
    bool result = manager.LoadPlugin("test_plugins/TopPlugin.dll");  // コンパイルエラー
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());
    
    // 依存関係に基づいて、BasePlugin, MiddlePlugin, TopPlugin の順で読み込まれる
    EXPECT_TRUE(manager.IsPluginLoaded("BasePlugin"));  // コンパイルエラー: 未定義メソッド
    EXPECT_TRUE(manager.IsPluginLoaded("MiddlePlugin"));
    EXPECT_TRUE(manager.IsPluginLoaded("TopPlugin"));
    EXPECT_EQ(manager.GetLoadedPluginCount(), 3);  // コンパイルエラー: 未定義メソッド
    
    // すべてがアクティブ状態
    EXPECT_EQ(manager.GetPluginState("BasePlugin"), PluginState::Active);  // コンパイルエラー
    EXPECT_EQ(manager.GetPluginState("MiddlePlugin"), PluginState::Active);
    EXPECT_EQ(manager.GetPluginState("TopPlugin"), PluginState::Active);
    
    manager.Shutdown();
}

TEST_F(DependencyTest, LoadPlugin_MissingDependency_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // BasePlugin を削除してから MiddlePlugin を読み込み
    std::filesystem::remove("test_plugins/BasePlugin.dll");
    
    bool result = manager.LoadPlugin("test_plugins/MiddlePlugin.dll");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());  // コンパイルエラー: 未定義メソッド
    
    std::string error = manager.GetLastError();  // コンパイルエラー: 未定義メソッド
    EXPECT_NE(error.find("Dependency missing"), std::string::npos);
    EXPECT_NE(error.find("BasePlugin"), std::string::npos);
    
    EXPECT_FALSE(manager.IsPluginLoaded("MiddlePlugin"));
    
    manager.Shutdown();
}

TEST_F(DependencyTest, CheckDependency_ValidDependency_True) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    manager.LoadPlugin("test_plugins/TopPlugin.dll");
    
    bool hasDependency = manager.CheckDependency("TopPlugin", "MiddlePlugin");  // コンパイルエラー
    EXPECT_TRUE(hasDependency);
    
    hasDependency = manager.CheckDependency("MiddlePlugin", "BasePlugin");
    EXPECT_TRUE(hasDependency);
    
    hasDependency = manager.CheckDependency("TopPlugin", "IndependentPlugin");
    EXPECT_FALSE(hasDependency);
    
    manager.Shutdown();
}

TEST_F(DependencyTest, GetDependencies_ReturnsCorrectList) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    manager.LoadPlugin("test_plugins/TopPlugin.dll");
    
    auto topDeps = manager.GetDependencies("TopPlugin");  // コンパイルエラー: 未定義メソッド
    EXPECT_EQ(topDeps.size(), 1);
    EXPECT_EQ(topDeps[0], "MiddlePlugin");
    
    auto middleDeps = manager.GetDependencies("MiddlePlugin");
    EXPECT_EQ(middleDeps.size(), 1);
    EXPECT_EQ(middleDeps[0], "BasePlugin");
    
    auto baseDeps = manager.GetDependencies("BasePlugin");
    EXPECT_EQ(baseDeps.size(), 0);
    
    manager.Shutdown();
}

TEST_F(DependencyTest, ResolveDependencies_ValidGraph_Success) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // 複雑な依存関係グラフでテスト
    TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/Core.dll");
    TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/Utils.dll", true, {"Core"});
    TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/UI.dll", true, {"Core"});
    TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/App.dll", true, {"Utils", "UI"});
    
    bool result = manager.ResolveDependencies();  // コンパイルエラー: 未定義メソッド
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());
    
    manager.Shutdown();
}
```

#### 循環依存検出テスト (失敗版)

**ファイル**: `NSys/Tests/test_circular_dependency.cpp`

```cpp
#include "test_plugin_manager_common.h"

class CircularDependencyTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        
        // 循環依存のテストプラグイン作成
        // A → B, B → C, C → A
        TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/PluginA.dll", 
                                                     true, {"PluginB"});
        TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/PluginB.dll", 
                                                     true, {"PluginC"});
        TestPluginDLLGenerator::CreateTestPluginDLL("test_plugins/PluginC.dll", 
                                                     true, {"PluginA"});
    }
};

TEST_F(CircularDependencyTest, LoadPlugin_CircularDependency_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.LoadPlugin("test_plugins/PluginA.dll");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());  // コンパイルエラー: 未定義メソッド
    
    std::string error = manager.GetLastError();  // コンパイルエラー: 未定義メソッド
    EXPECT_NE(error.find("Circular dependency"), std::string::npos);
    
    // どのプラグインも読み込まれていない
    EXPECT_FALSE(manager.IsPluginLoaded("PluginA"));  // コンパイルエラー: 未定義メソッド
    EXPECT_FALSE(manager.IsPluginLoaded("PluginB"));
    EXPECT_FALSE(manager.IsPluginLoaded("PluginC"));
    
    manager.Shutdown();
}
```

#### パフォーマンステスト (失敗版)

**ファイル**: `NSys/Tests/test_plugin_performance.cpp`

```cpp
#include "test_plugin_manager_common.h"

class PerformanceTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        
        // パフォーマンステスト用の複数プラグイン作成
        for (int i = 0; i < 10; ++i) {
            TestPluginDLLGenerator::CreateValidPluginDLL(
                "test_plugins/PerfPlugin" + std::to_string(i) + ".dll");
        }
    }
};

TEST_F(PerformanceTest, LoadPlugin_SinglePlugin_WithinTimeLimit) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    bool result = manager.LoadPlugin("test_plugins/PerfPlugin0.dll");  // コンパイルエラー
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_TRUE(result);
    EXPECT_LT(duration.count(), 3000); // 3秒以内
    
    std::cout << "Single plugin load time: " << duration.count() << "ms" << std::endl;
    
    manager.Shutdown();
}

TEST_F(PerformanceTest, LoadPlugin_MultiplePlugins_PerformanceAcceptable) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    const int pluginCount = 10;
    std::vector<float> loadTimes;
    
    for (int i = 0; i < pluginCount; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        bool result = manager.LoadPlugin("test_plugins/PerfPlugin" + 
                                        std::to_string(i) + ".dll");  // コンパイルエラー
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        float durationMs = duration.count() / 1000.0f;
        
        EXPECT_TRUE(result);
        EXPECT_LT(durationMs, 3000.0f); // 各プラグイン3秒以内
        
        loadTimes.push_back(durationMs);
    }
    
    // 統計計算
    float totalTime = std::accumulate(loadTimes.begin(), loadTimes.end(), 0.0f);
    float averageTime = totalTime / pluginCount;
    
    std::cout << "Average load time: " << averageTime << "ms" << std::endl;
    
    EXPECT_LT(averageTime, 1000.0f); // 平均1秒以内
    
    manager.Shutdown();
}
```

#### 負荷テスト (失敗版)

**ファイル**: `NSys/Tests/test_plugin_load_limits.cpp`

```cpp
#include "test_plugin_manager_common.h"

class LoadTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        
        // 35個のテストプラグインを作成（制限を超える数）
        for (int i = 0; i < 35; ++i) {
            TestPluginDLLGenerator::CreateValidPluginDLL(
                "test_plugins/LoadTestPlugin" + std::to_string(i) + ".dll");
        }
    }
};

TEST_F(LoadTest, MaxPluginCount_32PluginLimit_Enforced) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    EXPECT_EQ(manager.GetMaxPluginCount(), 32);  // コンパイルエラー: 未定義メソッド
    
    std::vector<std::string> loadedPlugins;
    std::vector<std::string> failedPlugins;
    
    // 35個のプラグインを順次読み込み
    for (int i = 0; i < 35; ++i) {
        std::string pluginPath = "test_plugins/LoadTestPlugin" + std::to_string(i) + ".dll";
        bool result = manager.LoadPlugin(pluginPath);  // コンパイルエラー: 未定義メソッド
        
        if (result) {
            loadedPlugins.push_back("LoadTestPlugin" + std::to_string(i));
        } else {
            failedPlugins.push_back("LoadTestPlugin" + std::to_string(i));
        }
    }
    
    // 32個まで読み込み成功、それ以降は失敗
    EXPECT_EQ(loadedPlugins.size(), 32);
    EXPECT_EQ(failedPlugins.size(), 3);
    EXPECT_EQ(manager.GetLoadedPluginCount(), 32);  // コンパイルエラー: 未定義メソッド
    EXPECT_TRUE(manager.IsAtMaxCapacity());  // コンパイルエラー: 未定義メソッド
    
    manager.Shutdown();
}

TEST_F(LoadTest, LoadTime_32Plugins_WithinTimeLimit) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 32個のプラグインを読み込み
    int loadedCount = 0;
    for (int i = 0; i < 32; ++i) {
        std::string pluginPath = "test_plugins/LoadTestPlugin" + std::to_string(i) + ".dll";
        if (manager.LoadPlugin(pluginPath)) {  // コンパイルエラー: 未定義メソッド
            loadedCount++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(loadedCount, 32);
    EXPECT_LT(duration.count(), 10000); // 10秒以内
    
    std::cout << "32 plugins load time: " << duration.count() << "ms" << std::endl;
    
    manager.Shutdown();
}
```

#### エラーハンドリングテスト (失敗版)

**ファイル**: `NSys/Tests/test_plugin_error_handling.cpp`

```cpp
#include "test_plugin_manager_common.h"

class ErrorHandlingTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        
        // 各種不正ファイルを作成
        TestPluginDLLGenerator::CreateInvalidDLL("test_plugins/NotADLL.dll");
        TestPluginDLLGenerator::CreateCorruptedDLL("test_plugins/Corrupted.dll");
        TestPluginDLLGenerator::CreateMissingEntryPointDLL("test_plugins/NoEntryPoint.dll");
        TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/ValidPlugin.dll");
    }
};

TEST_F(ErrorHandlingTest, LoadPlugin_NotADLL_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.LoadPlugin("test_plugins/NotADLL.dll");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());  // コンパイルエラー: 未定義メソッド
    
    std::string error = manager.GetLastError();  // コンパイルエラー: 未定義メソッド
    EXPECT_NE(error.find("Invalid DLL"), std::string::npos);
    
    EXPECT_FALSE(manager.IsPluginLoaded("NotADLL"));  // コンパイルエラー: 未定義メソッド
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);  // コンパイルエラー: 未定義メソッド
    
    manager.Shutdown();
}

TEST_F(ErrorHandlingTest, LoadPlugin_CorruptedDLL_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.LoadPlugin("test_plugins/Corrupted.dll");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());
    
    auto errors = manager.GetPluginErrors();  // コンパイルエラー: 未定義メソッド
    EXPECT_GT(errors.size(), 0);
    EXPECT_EQ(errors[0].type, PluginErrorType::LoadFailed);  // コンパイルエラー: 未定義型
    
    EXPECT_FALSE(manager.IsPluginLoaded("Corrupted"));
    
    manager.Shutdown();
}

TEST_F(ErrorHandlingTest, LoadPlugin_MissingEntryPoint_Failure) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    bool result = manager.LoadPlugin("test_plugins/NoEntryPoint.dll");  // コンパイルエラー
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(manager.HasErrors());
    
    std::string error = manager.GetLastError();
    EXPECT_NE(error.find("Entry point"), std::string::npos);
    
    EXPECT_FALSE(manager.IsPluginLoaded("NoEntryPoint"));
    
    manager.Shutdown();
}
```

#### 統合テスト (失敗版)

**ファイル**: `NSys/Tests/test_plugin_integration.cpp`

```cpp
#include "test_plugin_manager_common.h"
#include "mock_services.h"

class IntegrationTest : public PluginManagerTestBase {
protected:
    void SetUp() override {
        PluginManagerTestBase::SetUp();
        
        // TASK-003 のコンポーネントをセットアップしようとするが、未実装でエラー
        // この段階では統合対象のクラスも存在しない
        
        // 統合テスト用のプラグインを作成
        TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/WindowPlugin.dll");
        TestPluginDLLGenerator::CreateValidPluginDLL("test_plugins/MenuPlugin.dll");
    }
    
    void TearDown() override {
        PluginManagerTestBase::TearDown();
    }
};

TEST_F(IntegrationTest, ImGuiIntegration_WindowProvider_Success) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // WindowProviderプラグインを読み込み
    bool result = manager.LoadPlugin("test_plugins/WindowPlugin.dll");  // コンパイルエラー
    EXPECT_TRUE(result);
    
    // ImGuiWindowManagerとの統合を実行（未実装）
    manager.IntegrateWithImGuiCore();  // コンパイルエラー: 未定義メソッド
    
    // 実際の統合確認は実装後に詳細化
    EXPECT_TRUE(true); // プレースホルダー
    
    manager.Shutdown();
}

TEST_F(IntegrationTest, ServiceLocatorIntegration_LoggingService_Works) {
    // ServiceLocatorとの統合テスト（未実装によりコンパイルエラー）
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    manager.Initialize("test_plugins");
    
    // プラグイン読み込み（内部でロギングが使用される）
    bool result = manager.LoadPlugin("test_plugins/WindowPlugin.dll");  // コンパイルエラー
    EXPECT_TRUE(result);
    
    // ロギングサービスとの連携確認は実装後に詳細化
    EXPECT_TRUE(true); // プレースホルダー
    
    manager.Shutdown();
}

TEST_F(IntegrationTest, ConfigurationPersistence_SaveLoad_Success) {
    PluginManager manager;  // コンパイルエラー: 未定義クラス
    
    PluginConfig config1;  // コンパイルエラー: 未定義構造体
    config1.enabled = true;
    config1.autoLoad = true;
    config1.priority = 100;
    
    manager.Initialize("test_plugins");
    manager.LoadPlugin("test_plugins/WindowPlugin.dll", config1);  // コンパイルエラー
    
    // 設定保存
    std::string configFile = "test_plugin_config.ini";
    bool saveResult = manager.SaveConfiguration(configFile);  // コンパイルエラー: 未定義メソッド
    EXPECT_TRUE(saveResult);
    EXPECT_TRUE(std::filesystem::exists(configFile));
    
    manager.Shutdown();
    
    // 新しいマネージャーで設定読み込み
    auto newManager = std::make_unique<PluginManager>();
    newManager->Initialize("test_plugins");
    
    bool loadResult = newManager->LoadConfiguration(configFile);  // コンパイルエラー
    EXPECT_TRUE(loadResult);
    
    newManager->Shutdown();
    
    // 設定ファイルのクリーンアップ
    std::filesystem::remove(configFile);
}
```

### 3. プロジェクト設定の更新

**NSys.vcxproj に追加するファイル**:
```xml
<ItemGroup>
  <ClCompile Include="Tests\test_plugin_manager.cpp" />
  <ClCompile Include="Tests\test_plugin_load.cpp" />
  <ClCompile Include="Tests\test_plugin_unload.cpp" />
  <ClCompile Include="Tests\test_dependency_management.cpp" />
  <ClCompile Include="Tests\test_circular_dependency.cpp" />
  <ClCompile Include="Tests\test_plugin_performance.cpp" />
  <ClCompile Include="Tests\test_plugin_load_limits.cpp" />
  <ClCompile Include="Tests\test_plugin_error_handling.cpp" />
  <ClCompile Include="Tests\test_plugin_integration.cpp" />
</ItemGroup>

<ItemGroup>
  <ClInclude Include="Tests\test_plugin_manager_common.h" />
</ItemGroup>
```

### 4. ビルド実行とエラー確認

この段階でテストプロジェクトをビルドすると、以下のようなコンパイルエラーが発生することが期待されます：

```cpp
// 期待されるコンパイルエラー:

// PluginManager関連
error C2065: 'PluginManager': undeclared identifier
error C2228: left of '.Initialize' must have class/struct/union type
error C2228: left of '.LoadPlugin' must have class/struct/union type
error C2228: left of '.UnloadPlugin' must have class/struct/union type
error C2228: left of '.GetLoadedPluginCount' must have class/struct/union type

// 構造体・列挙型関連
error C2065: 'PluginConfig': undeclared identifier
error C2065: 'PluginState': undeclared identifier
error C2065: 'PluginErrorType': undeclared identifier

// メソッド関連
error C2228: left of '.HasErrors' must have class/struct/union type
error C2228: left of '.GetLastError' must have class/struct/union type
error C2228: left of '.IsPluginLoaded' must have class/struct/union type
error C2228: left of '.GetPlugin' must have class/struct/union type
error C2228: left of '.CheckDependency' must have class/struct/union type
error C2228: left of '.ResolveDependencies' must have class/struct/union type
error C2228: left of '.GetMaxPluginCount' must have class/struct/union type
error C2228: left of '.IsAtMaxCapacity' must have class/struct/union type
error C2228: left of '.GetPluginErrors' must have class/struct/union type
error C2228: left of '.SaveConfiguration' must have class/struct/union type
error C2228: left of '.LoadConfiguration' must have class/struct/union type
error C2228: left of '.IntegrateWithImGuiCore' must have class/struct/union type
```

### 5. 期待される失敗の確認

#### コンパイルエラー一覧
1. **PluginManager** クラス未定義 - 全メソッド未実装
2. **PluginConfig** 構造体未定義 - 設定管理未実装
3. **PluginState** 列挙型未定義 - 状態管理未実装
4. **PluginErrorType** 列挙型未定義 - エラー管理未実装
5. **依存関係管理** 機能未実装
6. **パフォーマンス測定** 機能未実装
7. **統合機能** 未実装

#### テストヘルパーの不完全実装
- **TestPluginDLLGenerator** - 実際のDLL生成ではなく、ダミーファイル作成のみ
- **統合テスト環境** - TASK-003統合部分が未準備

これらのエラーは **期待された失敗** であり、次の GREEN フェーズで解決していきます。

## RED Phase 完了チェックリスト

- [x] プラグインマネージャーテストプロジェクトが作成された
- [x] すべての主要機能のテストケースが実装された
- [x] テストは現在すべてコンパイルエラーで失敗する（期待された動作）
- [x] 失敗の原因が明確に特定できる（未実装のクラス・メソッド・構造体）
- [x] テストケースは実装すべき機能を正確に表現している
- [x] パフォーマンステスト、負荷テスト、エラーハンドリングテストも含まれている
- [x] TASK-003統合テストのフレームワークが準備された

## 次のステップ

GREEN フェーズでは、これらのテストを通すための **最小限の実装** を行います：

1. **PluginManager** クラスの基本実装
2. **PluginConfig, PluginState, PluginErrorType** の定義
3. **DLLManager** クラスの基本実装
4. **DependencyResolver** クラスの基本実装
5. **基本的なエラーハンドリング** の実装
6. **テストヘルパー** の実装改良
7. **統合テスト基盤** の準備

各実装は「テストを通す」ことを目的とした最小限に留め、過度な実装を避けます。実際の完全機能実装は次のREFACTORフェーズで行います。

RED フェーズは「期待される動作を明確に定義する失敗テスト」の作成であり、実装の方向性を確実に定めるための重要なステップです。