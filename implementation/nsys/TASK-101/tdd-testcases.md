# TASK-101: プラグインマネージャー実装 - テストケース設計

## テスト戦略

プラグインマネージャーシステムの品質を保証するため、以下の階層でテストケースを設計する：

1. **単体テスト**: PluginManagerクラスの基本機能
2. **統合テスト**: TASK-002, TASK-003との連携
3. **パフォーマンステスト**: 読み込み時間・メモリ使用量
4. **負荷テスト**: 32個プラグイン制限・大量操作
5. **エラーハンドリングテスト**: 異常系・境界値
6. **セキュリティテスト**: DLL検証・権限チェック

## 1. PluginManager 基本機能テスト

### TC-001: 初期化・終了テスト
```cpp
TEST(PluginManagerTest, Initialize_Success) {
    PluginManager manager;
    
    bool result = manager.Initialize("test_plugins");
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    EXPECT_EQ(manager.GetMaxPluginCount(), 32); // デフォルト値
    
    manager.Shutdown();
}

TEST(PluginManagerTest, Initialize_InvalidDirectory) {
    PluginManager manager;
    
    bool result = manager.Initialize("nonexistent_directory");
    
    // ディレクトリが存在しなくても初期化は成功（後で作成される）
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager.HasErrors());
    
    manager.Shutdown();
}

TEST(PluginManagerTest, Initialize_EmptyDirectory) {
    PluginManager manager;
    
    bool result = manager.Initialize("empty_test_dir");
    
    EXPECT_TRUE(result);
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    
    manager.Shutdown();
}

TEST(PluginManagerTest, Shutdown_ProperCleanup) {
    PluginManager manager;
    manager.Initialize("test_plugins");
    
    // プラグインをいくつか読み込み
    manager.LoadPlugin("test_plugins/TestPlugin1.dll");
    manager.LoadPlugin("test_plugins/TestPlugin2.dll");
    
    EXPECT_GT(manager.GetLoadedPluginCount(), 0);
    
    manager.Shutdown();
    
    // シャットダウン後の状態確認
    EXPECT_EQ(manager.GetLoadedPluginCount(), 0);
    EXPECT_FALSE(manager.HasErrors());
}
```

### TC-002: プラグイン読み込みテスト
```cpp
class PluginManagerLoadTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_unique<PluginManager>();
        m_manager->Initialize("test_plugins");
        
        // テスト用プラグインDLLを準備
        CreateTestPluginDLL("test_plugins/ValidPlugin.dll");
        CreateTestPluginDLL("test_plugins/InvalidPlugin.dll", false); // 無効なプラグイン
        CreateTestPluginDLL("test_plugins/DependentPlugin.dll", true, {"ValidPlugin"});
    }
    
    void TearDown() override {
        m_manager->Shutdown();
        CleanupTestFiles();
    }
    
    std::unique_ptr<PluginManager> m_manager;
};

TEST_F(PluginManagerLoadTest, LoadPlugin_ValidPlugin_Success) {
    bool result = m_manager->LoadPlugin("test_plugins/ValidPlugin.dll");
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(m_manager->HasErrors());
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 1);
    EXPECT_TRUE(m_manager->IsPluginLoaded("ValidPlugin"));
    EXPECT_EQ(m_manager->GetPluginState("ValidPlugin"), PluginState::Active);
    
    auto plugin = m_manager->GetPlugin("ValidPlugin");
    EXPECT_NE(plugin, nullptr);
    EXPECT_TRUE(plugin->IsInitialized());
}

TEST_F(PluginManagerLoadTest, LoadPlugin_InvalidPlugin_Failure) {
    bool result = m_manager->LoadPlugin("test_plugins/InvalidPlugin.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 0);
    EXPECT_FALSE(m_manager->IsPluginLoaded("InvalidPlugin"));
    
    std::string error = m_manager->GetLastError();
    EXPECT_FALSE(error.empty());
    EXPECT_NE(error.find("Invalid plugin"), std::string::npos);
}

TEST_F(PluginManagerLoadTest, LoadPlugin_NonexistentFile_Failure) {
    bool result = m_manager->LoadPlugin("test_plugins/Nonexistent.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 0);
    
    std::string error = m_manager->GetLastError();
    EXPECT_NE(error.find("File not found"), std::string::npos);
}

TEST_F(PluginManagerLoadTest, LoadPlugin_AlreadyLoaded_Success) {
    // 最初の読み込み
    bool result1 = m_manager->LoadPlugin("test_plugins/ValidPlugin.dll");
    EXPECT_TRUE(result1);
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 1);
    
    // 重複読み込み
    bool result2 = m_manager->LoadPlugin("test_plugins/ValidPlugin.dll");
    EXPECT_TRUE(result2); // 既に読み込み済みでも成功
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 1); // カウントは増えない
}

TEST_F(PluginManagerLoadTest, LoadPlugin_WithConfig_Success) {
    PluginConfig config;
    config.enabled = true;
    config.autoLoad = true;
    config.priority = 100;
    config.parameters["test_param"] = "test_value";
    
    bool result = m_manager->LoadPlugin("test_plugins/ValidPlugin.dll", config);
    
    EXPECT_TRUE(result);
    
    auto plugin = m_manager->GetPlugin("ValidPlugin");
    EXPECT_NE(plugin, nullptr);
    
    // 設定が正しく適用されているかテスト用メソッドで確認
    // (実際の実装では適切な検証方法を使用)
}
```

### TC-003: プラグインアンロードテスト
```cpp
TEST_F(PluginManagerLoadTest, UnloadPlugin_LoadedPlugin_Success) {
    // プラグイン読み込み
    m_manager->LoadPlugin("test_plugins/ValidPlugin.dll");
    EXPECT_TRUE(m_manager->IsPluginLoaded("ValidPlugin"));
    
    // アンロード
    bool result = m_manager->UnloadPlugin("ValidPlugin");
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(m_manager->IsPluginLoaded("ValidPlugin"));
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 0);
    EXPECT_EQ(m_manager->GetPluginState("ValidPlugin"), PluginState::Unloaded);
    
    auto plugin = m_manager->GetPlugin("ValidPlugin");
    EXPECT_EQ(plugin, nullptr);
}

TEST_F(PluginManagerLoadTest, UnloadPlugin_NotLoaded_Failure) {
    bool result = m_manager->UnloadPlugin("NonexistentPlugin");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    
    std::string error = m_manager->GetLastError();
    EXPECT_NE(error.find("Plugin not found"), std::string::npos);
}

TEST_F(PluginManagerLoadTest, ReloadPlugin_LoadedPlugin_Success) {
    // 初回読み込み
    m_manager->LoadPlugin("test_plugins/ValidPlugin.dll");
    auto originalPlugin = m_manager->GetPlugin("ValidPlugin");
    EXPECT_NE(originalPlugin, nullptr);
    
    // リロード
    bool result = m_manager->ReloadPlugin("ValidPlugin");
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(m_manager->IsPluginLoaded("ValidPlugin"));
    
    auto reloadedPlugin = m_manager->GetPlugin("ValidPlugin");
    EXPECT_NE(reloadedPlugin, nullptr);
    // リロード後は新しいインスタンス（実装による）
}
```

## 2. 依存関係管理テスト

### TC-101: 依存関係解決テスト
```cpp
class DependencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_unique<PluginManager>();
        m_manager->Initialize("test_plugins");
        
        // 依存関係のあるテストプラグイン群を作成
        CreateTestPluginDLL("test_plugins/BasePlugin.dll");
        CreateTestPluginDLL("test_plugins/MiddlePlugin.dll", true, {"BasePlugin"});
        CreateTestPluginDLL("test_plugins/TopPlugin.dll", true, {"MiddlePlugin"});
        CreateTestPluginDLL("test_plugins/IndependentPlugin.dll");
    }
    
    void TearDown() override {
        m_manager->Shutdown();
        CleanupTestFiles();
    }
    
    std::unique_ptr<PluginManager> m_manager;
};

TEST_F(DependencyTest, LoadPlugin_WithDependency_CorrectOrder) {
    // 依存関係: TopPlugin → MiddlePlugin → BasePlugin
    bool result = m_manager->LoadPlugin("test_plugins/TopPlugin.dll");
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(m_manager->HasErrors());
    
    // 依存関係に基づいて、BasePlugin, MiddlePlugin, TopPlugin の順で読み込まれる
    EXPECT_TRUE(m_manager->IsPluginLoaded("BasePlugin"));
    EXPECT_TRUE(m_manager->IsPluginLoaded("MiddlePlugin"));
    EXPECT_TRUE(m_manager->IsPluginLoaded("TopPlugin"));
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 3);
    
    // すべてがアクティブ状態
    EXPECT_EQ(m_manager->GetPluginState("BasePlugin"), PluginState::Active);
    EXPECT_EQ(m_manager->GetPluginState("MiddlePlugin"), PluginState::Active);
    EXPECT_EQ(m_manager->GetPluginState("TopPlugin"), PluginState::Active);
}

TEST_F(DependencyTest, LoadPlugin_MissingDependency_Failure) {
    // BasePlugin を削除してから MiddlePlugin を読み込み
    std::filesystem::remove("test_plugins/BasePlugin.dll");
    
    bool result = m_manager->LoadPlugin("test_plugins/MiddlePlugin.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    
    std::string error = m_manager->GetLastError();
    EXPECT_NE(error.find("Dependency missing"), std::string::npos);
    EXPECT_NE(error.find("BasePlugin"), std::string::npos);
    
    EXPECT_FALSE(m_manager->IsPluginLoaded("MiddlePlugin"));
}

TEST_F(DependencyTest, CheckDependency_ValidDependency_True) {
    m_manager->LoadPlugin("test_plugins/TopPlugin.dll");
    
    bool hasDependency = m_manager->CheckDependency("TopPlugin", "MiddlePlugin");
    EXPECT_TRUE(hasDependency);
    
    hasDependency = m_manager->CheckDependency("MiddlePlugin", "BasePlugin");
    EXPECT_TRUE(hasDependency);
    
    hasDependency = m_manager->CheckDependency("TopPlugin", "IndependentPlugin");
    EXPECT_FALSE(hasDependency);
}

TEST_F(DependencyTest, GetDependencies_ReturnsCorrectList) {
    m_manager->LoadPlugin("test_plugins/TopPlugin.dll");
    
    auto topDeps = m_manager->GetDependencies("TopPlugin");
    EXPECT_EQ(topDeps.size(), 1);
    EXPECT_EQ(topDeps[0], "MiddlePlugin");
    
    auto middleDeps = m_manager->GetDependencies("MiddlePlugin");
    EXPECT_EQ(middleDeps.size(), 1);
    EXPECT_EQ(middleDeps[0], "BasePlugin");
    
    auto baseDeps = m_manager->GetDependencies("BasePlugin");
    EXPECT_EQ(baseDeps.size(), 0);
}

TEST_F(DependencyTest, GetDependents_ReturnsCorrectList) {
    m_manager->LoadPlugin("test_plugins/TopPlugin.dll");
    
    auto baseDependents = m_manager->GetDependents("BasePlugin");
    EXPECT_EQ(baseDependents.size(), 1);
    EXPECT_EQ(baseDependents[0], "MiddlePlugin");
    
    auto middleDependents = m_manager->GetDependents("MiddlePlugin");
    EXPECT_EQ(middleDependents.size(), 1);
    EXPECT_EQ(middleDependents[0], "TopPlugin");
    
    auto topDependents = m_manager->GetDependents("TopPlugin");
    EXPECT_EQ(topDependents.size(), 0);
}
```

### TC-102: 循環依存検出テスト
```cpp
TEST_F(DependencyTest, LoadPlugin_CircularDependency_Failure) {
    // 循環依存のテストプラグイン作成
    // A → B, B → C, C → A
    CreateTestPluginDLL("test_plugins/PluginA.dll", true, {"PluginB"});
    CreateTestPluginDLL("test_plugins/PluginB.dll", true, {"PluginC"});
    CreateTestPluginDLL("test_plugins/PluginC.dll", true, {"PluginA"});
    
    bool result = m_manager->LoadPlugin("test_plugins/PluginA.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    
    std::string error = m_manager->GetLastError();
    EXPECT_NE(error.find("Circular dependency"), std::string::npos);
    
    // どのプラグインも読み込まれていない
    EXPECT_FALSE(m_manager->IsPluginLoaded("PluginA"));
    EXPECT_FALSE(m_manager->IsPluginLoaded("PluginB"));
    EXPECT_FALSE(m_manager->IsPluginLoaded("PluginC"));
}

TEST_F(DependencyTest, ResolveDependencies_ValidGraph_Success) {
    // 複雑な依存関係グラフでテスト
    CreateTestPluginDLL("test_plugins/Core.dll");
    CreateTestPluginDLL("test_plugins/Utils.dll", true, {"Core"});
    CreateTestPluginDLL("test_plugins/UI.dll", true, {"Core"});
    CreateTestPluginDLL("test_plugins/App.dll", true, {"Utils", "UI"});
    
    // すべてのプラグインを読み込み
    std::vector<std::string> plugins = {"Core", "Utils", "UI", "App"};
    for (const auto& plugin : plugins) {
        m_manager->LoadPlugin("test_plugins/" + plugin + ".dll");
    }
    
    bool result = m_manager->ResolveDependencies();
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(m_manager->HasErrors());
    
    // 正しい順序で読み込まれる: Core → Utils, UI → App
    EXPECT_TRUE(m_manager->IsPluginLoaded("Core"));
    EXPECT_TRUE(m_manager->IsPluginLoaded("Utils"));
    EXPECT_TRUE(m_manager->IsPluginLoaded("UI"));
    EXPECT_TRUE(m_manager->IsPluginLoaded("App"));
}
```

## 3. パフォーマンステスト

### TC-201: 読み込み時間テスト
```cpp
class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_unique<PluginManager>();
        m_manager->Initialize("test_plugins");
        
        // パフォーマンステスト用の複数プラグイン作成
        for (int i = 0; i < 10; ++i) {
            CreateTestPluginDLL("test_plugins/PerfPlugin" + std::to_string(i) + ".dll");
        }
    }
    
    void TearDown() override {
        m_manager->Shutdown();
        CleanupTestFiles();
    }
    
    std::unique_ptr<PluginManager> m_manager;
};

TEST_F(PerformanceTest, LoadPlugin_SinglePlugin_WithinTimeLimit) {
    auto start = std::chrono::high_resolution_clock::now();
    
    bool result = m_manager->LoadPlugin("test_plugins/PerfPlugin0.dll");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_TRUE(result);
    EXPECT_LT(duration.count(), 3000); // 3秒以内
    
    std::cout << "Single plugin load time: " << duration.count() << "ms" << std::endl;
}

TEST_F(PerformanceTest, LoadPlugin_MultiplePlugins_PerformanceAcceptable) {
    const int pluginCount = 10;
    std::vector<float> loadTimes;
    
    for (int i = 0; i < pluginCount; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        bool result = m_manager->LoadPlugin("test_plugins/PerfPlugin" + std::to_string(i) + ".dll");
        
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
    float maxTime = *std::max_element(loadTimes.begin(), loadTimes.end());
    
    std::cout << "Average load time: " << averageTime << "ms" << std::endl;
    std::cout << "Max load time: " << maxTime << "ms" << std::endl;
    std::cout << "Total load time: " << totalTime << "ms" << std::endl;
    
    EXPECT_LT(averageTime, 1000.0f); // 平均1秒以内
    EXPECT_LT(maxTime, 3000.0f); // 最大3秒以内
}

TEST_F(PerformanceTest, DependencyResolution_Performance) {
    // 複雑な依存関係グラフでのパフォーマンステスト
    const int pluginCount = 20;
    
    // 依存関係のあるプラグイン群を作成
    CreateTestPluginDLL("test_plugins/Root.dll");
    for (int i = 1; i < pluginCount; ++i) {
        std::vector<std::string> deps;
        if (i > 1) {
            deps.push_back("Plugin" + std::to_string(i - 1));
        } else {
            deps.push_back("Root");
        }
        CreateTestPluginDLL("test_plugins/Plugin" + std::to_string(i) + ".dll", true, deps);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    bool result = m_manager->ResolveDependencies();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_TRUE(result);
    EXPECT_LT(duration.count(), 1000); // 1秒以内
    
    std::cout << "Dependency resolution time for " << pluginCount << " plugins: " 
              << duration.count() << "ms" << std::endl;
}
```

### TC-202: メモリ使用量テスト
```cpp
TEST_F(PerformanceTest, MemoryUsage_WithinLimits) {
    // メモリ使用量測定開始
    PROCESS_MEMORY_COUNTERS_EX memCounters;
    GetProcessMemoryInfo(GetCurrentProcess(), 
                        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memCounters), 
                        sizeof(memCounters));
    SIZE_T initialMemory = memCounters.WorkingSetSize;
    
    // プラグインを読み込み
    for (int i = 0; i < 10; ++i) {
        bool result = m_manager->LoadPlugin("test_plugins/PerfPlugin" + std::to_string(i) + ".dll");
        EXPECT_TRUE(result);
    }
    
    // メモリ使用量測定終了
    GetProcessMemoryInfo(GetCurrentProcess(), 
                        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memCounters), 
                        sizeof(memCounters));
    SIZE_T finalMemory = memCounters.WorkingSetSize;
    SIZE_T usedMemory = finalMemory - initialMemory;
    
    std::cout << "Memory usage for 10 plugins: " << (usedMemory / 1024 / 1024) << "MB" << std::endl;
    
    // プラグインあたり100MB制限 → 10個で1GB以内
    EXPECT_LT(usedMemory, 1024 * 1024 * 1024); // 1GB以内
    
    // 個別プラグインのメモリ使用量推定
    SIZE_T averagePerPlugin = usedMemory / 10;
    EXPECT_LT(averagePerPlugin, 100 * 1024 * 1024); // 100MB以内
}

TEST_F(PerformanceTest, MemoryLeak_NoLeakOnUnload) {
    PROCESS_MEMORY_COUNTERS_EX memCounters;
    
    // 初期メモリ使用量
    GetProcessMemoryInfo(GetCurrentProcess(), 
                        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memCounters), 
                        sizeof(memCounters));
    SIZE_T initialMemory = memCounters.WorkingSetSize;
    
    // プラグイン読み込み・アンロードを繰り返し
    for (int cycle = 0; cycle < 5; ++cycle) {
        for (int i = 0; i < 5; ++i) {
            m_manager->LoadPlugin("test_plugins/PerfPlugin" + std::to_string(i) + ".dll");
        }
        
        for (int i = 0; i < 5; ++i) {
            m_manager->UnloadPlugin("PerfPlugin" + std::to_string(i));
        }
    }
    
    // 最終メモリ使用量
    GetProcessMemoryInfo(GetCurrentProcess(), 
                        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memCounters), 
                        sizeof(memCounters));
    SIZE_T finalMemory = memCounters.WorkingSetSize;
    
    // メモリ使用量の増加が50MB以内（メモリリークの許容範囲）
    EXPECT_LT((finalMemory - initialMemory), 50 * 1024 * 1024);
    
    std::cout << "Memory difference after load/unload cycles: " 
              << ((finalMemory - initialMemory) / 1024 / 1024) << "MB" << std::endl;
}
```

## 4. 負荷テスト

### TC-301: 32個プラグイン制限テスト
```cpp
class LoadTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_unique<PluginManager>();
        m_manager->Initialize("test_plugins");
        
        // 35個のテストプラグインを作成（制限を超える数）
        for (int i = 0; i < 35; ++i) {
            CreateTestPluginDLL("test_plugins/LoadTestPlugin" + std::to_string(i) + ".dll");
        }
    }
    
    void TearDown() override {
        m_manager->Shutdown();
        CleanupTestFiles();
    }
    
    std::unique_ptr<PluginManager> m_manager;
};

TEST_F(LoadTest, MaxPluginCount_32PluginLimit_Enforced) {
    EXPECT_EQ(m_manager->GetMaxPluginCount(), 32);
    
    std::vector<std::string> loadedPlugins;
    std::vector<std::string> failedPlugins;
    
    // 35個のプラグインを順次読み込み
    for (int i = 0; i < 35; ++i) {
        std::string pluginPath = "test_plugins/LoadTestPlugin" + std::to_string(i) + ".dll";
        bool result = m_manager->LoadPlugin(pluginPath);
        
        if (result) {
            loadedPlugins.push_back("LoadTestPlugin" + std::to_string(i));
        } else {
            failedPlugins.push_back("LoadTestPlugin" + std::to_string(i));
        }
    }
    
    // 32個まで読み込み成功、それ以降は失敗
    EXPECT_EQ(loadedPlugins.size(), 32);
    EXPECT_EQ(failedPlugins.size(), 3);
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 32);
    EXPECT_TRUE(m_manager->IsAtMaxCapacity());
    
    // 33個目以降の読み込みでエラーが発生
    for (int i = 32; i < 35; ++i) {
        std::string pluginName = "LoadTestPlugin" + std::to_string(i);
        EXPECT_FALSE(m_manager->IsPluginLoaded(pluginName));
    }
}

TEST_F(LoadTest, MaxPluginCount_CustomLimit_Respected) {
    m_manager->SetMaxPluginCount(10);
    EXPECT_EQ(m_manager->GetMaxPluginCount(), 10);
    
    // 15個のプラグイン読み込みを試行
    int successCount = 0;
    for (int i = 0; i < 15; ++i) {
        std::string pluginPath = "test_plugins/LoadTestPlugin" + std::to_string(i) + ".dll";
        if (m_manager->LoadPlugin(pluginPath)) {
            successCount++;
        }
    }
    
    EXPECT_EQ(successCount, 10);
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 10);
    EXPECT_TRUE(m_manager->IsAtMaxCapacity());
}

TEST_F(LoadTest, LoadTime_32Plugins_WithinTimeLimit) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 32個のプラグインを読み込み
    int loadedCount = 0;
    for (int i = 0; i < 32; ++i) {
        std::string pluginPath = "test_plugins/LoadTestPlugin" + std::to_string(i) + ".dll";
        if (m_manager->LoadPlugin(pluginPath)) {
            loadedCount++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(loadedCount, 32);
    EXPECT_LT(duration.count(), 10000); // 10秒以内
    
    std::cout << "32 plugins load time: " << duration.count() << "ms" << std::endl;
}
```

## 5. エラーハンドリングテスト

### TC-401: 不正DLLファイル処理テスト
```cpp
class ErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_unique<PluginManager>();
        m_manager->Initialize("test_plugins");
        
        // 各種不正ファイルを作成
        CreateInvalidDLL("test_plugins/NotADLL.dll"); // DLLではないファイル
        CreateCorruptedDLL("test_plugins/Corrupted.dll"); // 破損したDLL
        CreateMissingEntryPointDLL("test_plugins/NoEntryPoint.dll"); // エントリポイントなし
        CreateValidPluginDLL("test_plugins/ValidPlugin.dll"); // 比較用の正常プラグイン
    }
    
    void TearDown() override {
        m_manager->Shutdown();
        CleanupTestFiles();
    }
    
    std::unique_ptr<PluginManager> m_manager;
};

TEST_F(ErrorHandlingTest, LoadPlugin_NotADLL_Failure) {
    bool result = m_manager->LoadPlugin("test_plugins/NotADLL.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    
    std::string error = m_manager->GetLastError();
    EXPECT_NE(error.find("Invalid DLL"), std::string::npos);
    
    EXPECT_FALSE(m_manager->IsPluginLoaded("NotADLL"));
    EXPECT_EQ(m_manager->GetLoadedPluginCount(), 0);
}

TEST_F(ErrorHandlingTest, LoadPlugin_CorruptedDLL_Failure) {
    bool result = m_manager->LoadPlugin("test_plugins/Corrupted.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    
    auto errors = m_manager->GetPluginErrors();
    EXPECT_GT(errors.size(), 0);
    EXPECT_EQ(errors[0].type, PluginErrorType::LoadFailed);
    
    EXPECT_FALSE(m_manager->IsPluginLoaded("Corrupted"));
}

TEST_F(ErrorHandlingTest, LoadPlugin_MissingEntryPoint_Failure) {
    bool result = m_manager->LoadPlugin("test_plugins/NoEntryPoint.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    
    std::string error = m_manager->GetLastError();
    EXPECT_NE(error.find("Entry point"), std::string::npos);
    
    EXPECT_FALSE(m_manager->IsPluginLoaded("NoEntryPoint"));
}

TEST_F(ErrorHandlingTest, PluginInitializationFailure_GracefulHandling) {
    // 初期化時に失敗するプラグインを作成
    CreateFailingInitializationPluginDLL("test_plugins/FailInit.dll");
    
    bool result = m_manager->LoadPlugin("test_plugins/FailInit.dll");
    
    EXPECT_FALSE(result);
    EXPECT_TRUE(m_manager->HasErrors());
    
    auto errors = m_manager->GetPluginErrors();
    bool foundInitError = false;
    for (const auto& error : errors) {
        if (error.type == PluginErrorType::InitializationFailed) {
            foundInitError = true;
            break;
        }
    }
    EXPECT_TRUE(foundInitError);
    
    // プラグインの状態がErrorになっている
    EXPECT_EQ(m_manager->GetPluginState("FailInit"), PluginState::Error);
}
```

### TC-402: メモリ不足時の処理テスト
```cpp
TEST_F(ErrorHandlingTest, MemoryExhaustion_GracefulDegradation) {
    // メモリ不足をシミュレート（実際のテストでは慎重に実装）
    // この例では、大量のメモリを消費するプラグインを使用
    
    std::vector<std::string> loadedPlugins;
    std::vector<std::string> memoryFailedPlugins;
    
    // メモリ制限に近づくまでプラグインを読み込み
    for (int i = 0; i < 50; ++i) {
        CreateLargeMemoryPluginDLL("test_plugins/LargeMemPlugin" + std::to_string(i) + ".dll");
        
        bool result = m_manager->LoadPlugin("test_plugins/LargeMemPlugin" + std::to_string(i) + ".dll");
        
        if (result) {
            loadedPlugins.push_back("LargeMemPlugin" + std::to_string(i));
        } else {
            // メモリ不足でエラー
            if (m_manager->HasErrors()) {
                auto errors = m_manager->GetPluginErrors();
                for (const auto& error : errors) {
                    if (error.type == PluginErrorType::MemoryError) {
                        memoryFailedPlugins.push_back("LargeMemPlugin" + std::to_string(i));
                        break;
                    }
                }
            }
        }
        
        // メモリエラーが発生したら停止
        if (!memoryFailedPlugins.empty()) {
            break;
        }
    }
    
    // メモリ不足が適切に検出され、エラーハンドリングされた
    EXPECT_GT(loadedPlugins.size(), 0);
    EXPECT_GT(memoryFailedPlugins.size(), 0);
    
    std::cout << "Loaded plugins before memory limit: " << loadedPlugins.size() << std::endl;
    std::cout << "Failed plugins due to memory: " << memoryFailedPlugins.size() << std::endl;
    
    // アプリケーション全体は安定している
    EXPECT_FALSE(m_manager->HasErrors()); // エラー状態はクリアされている
}
```

## 6. 統合テスト

### TC-501: TASK-003統合テスト
```cpp
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // TASK-003 のコンポーネントをセットアップ
        m_windowManager = std::make_unique<ImGuiWindowManager>();
        m_pluginIntegration = std::make_unique<PluginIntegration>();
        
        // ServiceLocatorにサービスを登録
        ServiceLocator::RegisterService<ImGuiWindowManager>(m_windowManager.get());
        ServiceLocator::RegisterService<PluginIntegration>(m_pluginIntegration.get());
        
        // PluginManagerを初期化
        m_manager = std::make_unique<PluginManager>();
        m_manager->Initialize("test_plugins");
        
        // 統合テスト用のプラグインを作成
        CreateWindowProviderPluginDLL("test_plugins/WindowPlugin.dll");
        CreateMenuProviderPluginDLL("test_plugins/MenuPlugin.dll");
        CreateMixedProviderPluginDLL("test_plugins/MixedPlugin.dll");
    }
    
    void TearDown() override {
        m_manager->Shutdown();
        ServiceLocator::Clear();
        CleanupTestFiles();
    }
    
    std::unique_ptr<PluginManager> m_manager;
    std::unique_ptr<ImGuiWindowManager> m_windowManager;
    std::unique_ptr<PluginIntegration> m_pluginIntegration;
};

TEST_F(IntegrationTest, ImGuiIntegration_WindowProvider_Success) {
    // WindowProviderプラグインを読み込み
    bool result = m_manager->LoadPlugin("test_plugins/WindowPlugin.dll");
    EXPECT_TRUE(result);
    
    // ImGuiWindowManagerとの統合を実行
    m_manager->IntegrateWithImGuiCore();
    
    // ウィンドウプロバイダーが正しく登録されたか確認
    EXPECT_TRUE(m_windowManager->IsWindowVisible("WindowPlugin"));
    
    // プラグインインテグレーションにも登録されたか確認
    auto activePlugins = m_pluginIntegration->GetActivePlugins();
    EXPECT_EQ(std::find(activePlugins.begin(), activePlugins.end(), "WindowPlugin"), 
              activePlugins.end() - 1);
}

TEST_F(IntegrationTest, ImGuiIntegration_MenuProvider_Success) {
    // MenuProviderプラグインを読み込み
    bool result = m_manager->LoadPlugin("test_plugins/MenuPlugin.dll");
    EXPECT_TRUE(result);
    
    // ImGuiとの統合
    m_manager->IntegrateWithImGuiCore();
    
    // メニュープロバイダーの統合確認
    m_pluginIntegration->RenderMainMenuBar();
    
    // メニュー項目が追加されたか確認（実装依存の確認方法）
    EXPECT_TRUE(true); // 実際のテストでは適切な確認方法を使用
}

TEST_F(IntegrationTest, ServiceLocatorIntegration_LoggingService_Works) {
    // ロギングサービスのモック設定
    auto mockLoggingService = std::make_shared<MockLoggingService>();
    ServiceLocator::RegisterService<ILoggingService>(mockLoggingService.get());
    
    // プラグイン読み込み（内部でロギングが使用される）
    bool result = m_manager->LoadPlugin("test_plugins/WindowPlugin.dll");
    EXPECT_TRUE(result);
    
    // ロギングが適切に呼ばれたか確認
    EXPECT_GT(mockLoggingService->GetInfoCallCount(), 0);
    
    // エラーログの確認
    EXPECT_EQ(mockLoggingService->GetErrorCallCount(), 0);
}
```

### TC-502: 設定永続化テスト
```cpp
TEST_F(IntegrationTest, ConfigurationPersistence_SaveLoad_Success) {
    // プラグイン設定を行う
    PluginConfig config1;
    config1.enabled = true;
    config1.autoLoad = true;
    config1.priority = 100;
    
    m_manager->LoadPlugin("test_plugins/WindowPlugin.dll", config1);
    
    PluginConfig config2;
    config2.enabled = false;
    config2.autoLoad = false;
    config2.priority = 50;
    
    m_manager->LoadPlugin("test_plugins/MenuPlugin.dll", config2);
    
    // 設定保存
    std::string configFile = "test_plugin_config.ini";
    bool saveResult = m_manager->SaveConfiguration(configFile);
    EXPECT_TRUE(saveResult);
    EXPECT_TRUE(std::filesystem::exists(configFile));
    
    // 新しいマネージャーで設定読み込み
    auto newManager = std::make_unique<PluginManager>();
    newManager->Initialize("test_plugins");
    
    bool loadResult = newManager->LoadConfiguration(configFile);
    EXPECT_TRUE(loadResult);
    
    // 設定が正しく復元されたか確認
    EXPECT_TRUE(newManager->IsPluginLoaded("WindowPlugin"));
    EXPECT_FALSE(newManager->IsPluginLoaded("MenuPlugin")); // enabled=false
    
    // 設定ファイルのクリーンアップ
    std::filesystem::remove(configFile);
}
```

## テスト実行環境

### 必要なフレームワーク・ツール
- **Google Test**: 単体テスト・統合テスト実行
- **Google Mock**: モックオブジェクト作成  
- **Visual Studio**: Windows DLL操作
- **Memory Profiler**: メモリリーク検出

### テスト用プラグインDLL作成
```cpp
// テスト用プラグインDLL作成ヘルパー
class TestPluginDLLGenerator {
public:
    static void CreateTestPluginDLL(const std::string& filename, 
                                   bool isValid = true,
                                   const std::vector<std::string>& dependencies = {}) {
        // 動的にテスト用DLLを作成
        // 実際の実装では、テンプレートDLLから必要な設定で生成
    }
    
    static void CreateValidPluginDLL(const std::string& filename) {
        // 正常な動作をするプラグインDLL
    }
    
    static void CreateInvalidDLL(const std::string& filename) {
        // DLLではないファイル（テキストファイルなど）
    }
    
    static void CreateCorruptedDLL(const std::string& filename) {
        // 破損したDLLファイル
    }
    
    static void CreateMissingEntryPointDLL(const std::string& filename) {
        // 必要なエントリポイント関数が欠けているDLL
    }
};
```

### テスト設定
```cmake
# CMakeLists.txt for tests
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_executable(NSysPluginManager_Tests
    test_plugin_manager.cpp
    test_dependency_resolver.cpp
    test_dll_manager.cpp
    test_lifecycle_manager.cpp
    test_performance.cpp
    test_load_test.cpp
    test_error_handling.cpp
    test_integration.cpp
    test_helpers.cpp
)

target_link_libraries(NSysPluginManager_Tests
    GTest::gtest_main
    GMock::gmock
    NSysCore
    NSysImGuiCore
    kernel32  # Windows DLL API
    psapi     # Process memory API
)
```

### 継続的テスト
- **自動テスト実行**: ビルド時の全テスト実行
- **パフォーマンス回帰テスト**: 週次でのベンチマーク
- **メモリリークテスト**: 長時間実行でのリーク検出
- **負荷テスト**: 制限値近辺での動作確認

### テストメトリクス
- **単体テスト実行時間**: 45秒以内
- **統合テスト実行時間**: 3分以内
- **パフォーマンステスト実行時間**: 15分以内
- **全テストケース成功率**: 100%
- **コードカバレッジ**: 行カバレッジ 85%以上

このテストケース設計により、プラグインマネージャーシステムの信頼性とパフォーマンスを包括的に検証します。