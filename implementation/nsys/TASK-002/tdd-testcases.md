# TASK-002: CoreシステムI/F実装 - テストケース設計

## テスト戦略

Core システムインターフェースの品質を保証するため、以下の階層でテストケースを設計する：

1. **単体テスト**: 各インターフェースの基本契約
2. **統合テスト**: インターフェース間の連携
3. **受け入れテスト**: 実際のプラグインでの動作確認

## 1. APIVersion テストケース

### TC-001: APIVersion 基本機能
```cpp
TEST(APIVersionTest, DefaultConstruction) {
    APIVersion version;
    EXPECT_EQ(version.major, 1);
    EXPECT_EQ(version.minor, 0);
    EXPECT_EQ(version.patch, 0);
}

TEST(APIVersionTest, CustomConstruction) {
    APIVersion version{2, 1, 3};
    EXPECT_EQ(version.major, 2);
    EXPECT_EQ(version.minor, 1);
    EXPECT_EQ(version.patch, 3);
}
```

### TC-002: APIVersion 互換性チェック
```cpp
TEST(APIVersionTest, CompatibilityCheck_SameMajorHigherMinor) {
    APIVersion base{1, 2, 0};
    APIVersion compatible{1, 3, 0};
    EXPECT_TRUE(base.IsCompatible(compatible));
}

TEST(APIVersionTest, CompatibilityCheck_SameMajorSameMinor) {
    APIVersion base{1, 2, 0};
    APIVersion compatible{1, 2, 5};
    EXPECT_TRUE(base.IsCompatible(compatible));
}

TEST(APIVersionTest, CompatibilityCheck_DifferentMajor) {
    APIVersion base{1, 2, 0};
    APIVersion incompatible{2, 0, 0};
    EXPECT_FALSE(base.IsCompatible(incompatible));
}

TEST(APIVersionTest, CompatibilityCheck_LowerMinor) {
    APIVersion base{1, 2, 0};
    APIVersion incompatible{1, 1, 0};
    EXPECT_FALSE(base.IsCompatible(incompatible));
}
```

### TC-003: CURRENT_API_VERSION 定数
```cpp
TEST(APIVersionTest, CurrentAPIVersionDefined) {
    EXPECT_EQ(CURRENT_API_VERSION.major, 1);
    EXPECT_EQ(CURRENT_API_VERSION.minor, 0);
    EXPECT_EQ(CURRENT_API_VERSION.patch, 0);
}
```

## 2. ServiceLocator テストケース

### TC-101: ServiceLocator 基本機能
```cpp
TEST(ServiceLocatorTest, InitialState_AllServicesNull) {
    // システム初期状態では全サービスがnullptrであること
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetLoggingService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetMemoryService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetConfigurationService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetLocalizationService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetEventBus(), nullptr);
    EXPECT_EQ(ServiceLocator::GetPluginManager(), nullptr);
}
```

### TC-102: サービス登録・取得
```cpp
class MockFileSystemService : public IFileSystemService {
    // Mock implementation
};

TEST(ServiceLocatorTest, RegisterAndGetFileSystemService) {
    auto mockService = std::make_unique<MockFileSystemService>();
    auto* servicePtr = mockService.get();
    
    ServiceLocator::RegisterFileSystemService(std::move(mockService));
    
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), servicePtr);
}

TEST(ServiceLocatorTest, RegisterAndGetLoggingService) {
    auto mockService = std::make_unique<MockLoggingService>();
    auto* servicePtr = mockService.get();
    
    ServiceLocator::RegisterLoggingService(std::move(mockService));
    
    EXPECT_EQ(ServiceLocator::GetLoggingService(), servicePtr);
}
```

### TC-103: サービス重複登録
```cpp
TEST(ServiceLocatorTest, DuplicateRegistration_ReplacesPrevious) {
    auto firstService = std::make_unique<MockFileSystemService>();
    auto secondService = std::make_unique<MockFileSystemService>();
    auto* secondPtr = secondService.get();
    
    ServiceLocator::RegisterFileSystemService(std::move(firstService));
    ServiceLocator::RegisterFileSystemService(std::move(secondService));
    
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), secondPtr);
}
```

### TC-104: ServiceLocator シャットダウン
```cpp
TEST(ServiceLocatorTest, Shutdown_ClearsAllServices) {
    ServiceLocator::RegisterFileSystemService(std::make_unique<MockFileSystemService>());
    ServiceLocator::RegisterLoggingService(std::make_unique<MockLoggingService>());
    
    ServiceLocator::Shutdown();
    
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetLoggingService(), nullptr);
}
```

## 3. IPlugin インターフェース テストケース

### TC-201: IPlugin 基本契約
```cpp
class TestPlugin : public IPlugin {
private:
    bool m_initialized = false;
    bool m_enabled = true;
    
public:
    bool Initialize() override {
        if (m_initialized) return false;
        m_initialized = true;
        return true;
    }
    
    void Shutdown() override {
        m_initialized = false;
    }
    
    void Update(float deltaTime) override { /* テスト用空実装 */ }
    void Render() override { /* テスト用空実装 */ }
    
    PluginInfo GetPluginInfo() const override {
        return {"TestPlugin", "1.0.0", "Test plugin"};
    }
    
    std::string GetName() const override { return "TestPlugin"; }
    std::string GetVersion() const override { return "1.0.0"; }
    bool IsInitialized() const override { return m_initialized; }
    void SetEnabled(bool enabled) override { m_enabled = enabled; }
    bool IsEnabled() const override { return m_enabled; }
};

TEST(IPluginTest, LifecycleManagement) {
    TestPlugin plugin;
    
    // 初期状態
    EXPECT_FALSE(plugin.IsInitialized());
    EXPECT_TRUE(plugin.IsEnabled());
    
    // 初期化
    EXPECT_TRUE(plugin.Initialize());
    EXPECT_TRUE(plugin.IsInitialized());
    
    // 重複初期化防止
    EXPECT_FALSE(plugin.Initialize());
    
    // 終了処理
    plugin.Shutdown();
    EXPECT_FALSE(plugin.IsInitialized());
}

TEST(IPluginTest, EnabledStateManagement) {
    TestPlugin plugin;
    
    EXPECT_TRUE(plugin.IsEnabled());
    
    plugin.SetEnabled(false);
    EXPECT_FALSE(plugin.IsEnabled());
    
    plugin.SetEnabled(true);
    EXPECT_TRUE(plugin.IsEnabled());
}

TEST(IPluginTest, PluginInfo) {
    TestPlugin plugin;
    
    EXPECT_EQ(plugin.GetName(), "TestPlugin");
    EXPECT_EQ(plugin.GetVersion(), "1.0.0");
    
    auto info = plugin.GetPluginInfo();
    EXPECT_EQ(info.name, "TestPlugin");
    EXPECT_EQ(info.version, "1.0.0");
    EXPECT_EQ(info.description, "Test plugin");
}
```

## 4. IMenuProvider インターフェース テストケース

### TC-301: IMenuProvider 基本機能
```cpp
class TestMenuProvider : public IMenuProvider {
public:
    std::vector<MenuItem> GetMenuItems() override {
        std::vector<MenuItem> items;
        
        // File メニュー
        MenuItem fileMenu;
        fileMenu.label = "File";
        
        MenuItem newItem;
        newItem.label = "New";
        newItem.shortcut = "Ctrl+N";
        newItem.callback = []() { /* New action */ };
        
        MenuItem openItem;
        openItem.label = "Open";
        openItem.shortcut = "Ctrl+O";
        openItem.callback = []() { /* Open action */ };
        
        MenuItem separator;
        separator.separator = true;
        
        MenuItem exitItem;
        exitItem.label = "Exit";
        exitItem.shortcut = "Alt+F4";
        exitItem.callback = []() { /* Exit action */ };
        
        fileMenu.children = {newItem, openItem, separator, exitItem};
        items.push_back(fileMenu);
        
        return items;
    }
    
    void OnMenuItemSelected(const std::string& itemId) override {
        // テスト用実装
    }
};

TEST(IMenuProviderTest, GetMenuItems_ReturnsValidStructure) {
    TestMenuProvider provider;
    auto items = provider.GetMenuItems();
    
    EXPECT_EQ(items.size(), 1);
    
    const auto& fileMenu = items[0];
    EXPECT_EQ(fileMenu.label, "File");
    EXPECT_EQ(fileMenu.children.size(), 4);
    
    EXPECT_EQ(fileMenu.children[0].label, "New");
    EXPECT_EQ(fileMenu.children[0].shortcut, "Ctrl+N");
    EXPECT_NE(fileMenu.children[0].callback, nullptr);
    
    EXPECT_EQ(fileMenu.children[1].label, "Open");
    EXPECT_EQ(fileMenu.children[1].shortcut, "Ctrl+O");
    
    EXPECT_TRUE(fileMenu.children[2].separator);
    
    EXPECT_EQ(fileMenu.children[3].label, "Exit");
    EXPECT_EQ(fileMenu.children[3].shortcut, "Alt+F4");
}

TEST(IMenuProviderTest, MenuItemCallback_IsCallable) {
    TestMenuProvider provider;
    auto items = provider.GetMenuItems();
    
    const auto& newItem = items[0].children[0];
    
    // コールバックが実行可能であることを確認
    EXPECT_NO_THROW(newItem.callback());
}
```

## 5. IWindowProvider インターフェース テストケース

### TC-401: IWindowProvider 基本機能
```cpp
class TestWindowProvider : public IWindowProvider {
private:
    std::map<std::string, bool> m_windowStates;
    
public:
    TestWindowProvider() {
        m_windowStates["MainWindow"] = true;
        m_windowStates["DebugWindow"] = false;
    }
    
    std::vector<WindowInfo> GetWindows() override {
        std::vector<WindowInfo> windows;
        
        WindowInfo mainWindow;
        mainWindow.title = "Main Window";
        mainWindow.defaultSize = {1200, 800};
        mainWindow.minSize = {400, 300};
        mainWindow.dockable = true;
        mainWindow.closeable = true;
        mainWindow.collapsible = true;
        mainWindow.resizable = true;
        windows.push_back(mainWindow);
        
        WindowInfo debugWindow;
        debugWindow.title = "Debug Window";
        debugWindow.defaultSize = {600, 400};
        debugWindow.minSize = {300, 200};
        debugWindow.dockable = true;
        debugWindow.closeable = true;
        debugWindow.collapsible = false;
        debugWindow.resizable = true;
        windows.push_back(debugWindow);
        
        return windows;
    }
    
    void RenderWindow(const std::string& windowId) override {
        // テスト用空実装
    }
    
    bool IsWindowOpen(const std::string& windowId) override {
        auto it = m_windowStates.find(windowId);
        return it != m_windowStates.end() ? it->second : false;
    }
    
    void SetWindowOpen(const std::string& windowId, bool open) override {
        m_windowStates[windowId] = open;
    }
};

TEST(IWindowProviderTest, GetWindows_ReturnsValidConfiguration) {
    TestWindowProvider provider;
    auto windows = provider.GetWindows();
    
    EXPECT_EQ(windows.size(), 2);
    
    const auto& mainWindow = windows[0];
    EXPECT_EQ(mainWindow.title, "Main Window");
    EXPECT_EQ(mainWindow.defaultSize.x, 1200);
    EXPECT_EQ(mainWindow.defaultSize.y, 800);
    EXPECT_EQ(mainWindow.minSize.x, 400);
    EXPECT_EQ(mainWindow.minSize.y, 300);
    EXPECT_TRUE(mainWindow.dockable);
    EXPECT_TRUE(mainWindow.closeable);
    EXPECT_TRUE(mainWindow.collapsible);
    EXPECT_TRUE(mainWindow.resizable);
    
    const auto& debugWindow = windows[1];
    EXPECT_EQ(debugWindow.title, "Debug Window");
    EXPECT_FALSE(debugWindow.collapsible);
}

TEST(IWindowProviderTest, WindowStateManagement) {
    TestWindowProvider provider;
    
    // 初期状態
    EXPECT_TRUE(provider.IsWindowOpen("MainWindow"));
    EXPECT_FALSE(provider.IsWindowOpen("DebugWindow"));
    EXPECT_FALSE(provider.IsWindowOpen("NonexistentWindow"));
    
    // 状態変更
    provider.SetWindowOpen("DebugWindow", true);
    EXPECT_TRUE(provider.IsWindowOpen("DebugWindow"));
    
    provider.SetWindowOpen("MainWindow", false);
    EXPECT_FALSE(provider.IsWindowOpen("MainWindow"));
    
    // 新しいウィンドウの状態設定
    provider.SetWindowOpen("NewWindow", true);
    EXPECT_TRUE(provider.IsWindowOpen("NewWindow"));
}
```

## 6. 統合テストケース

### TC-501: プラグインとServiceLocatorの統合
```cpp
TEST(IntegrationTest, PluginWithServiceLocator) {
    // サービス登録
    ServiceLocator::RegisterLoggingService(std::make_unique<MockLoggingService>());
    
    TestPlugin plugin;
    
    // プラグイン初期化時にサービスが利用可能
    EXPECT_TRUE(plugin.Initialize());
    
    // プラグイン内でサービスにアクセス可能
    auto* logger = ServiceLocator::GetLoggingService();
    EXPECT_NE(logger, nullptr);
    
    plugin.Shutdown();
    ServiceLocator::Shutdown();
}
```

### TC-502: 複数インターフェース実装プラグイン
```cpp
class FullFeaturePlugin : public IPlugin, public IMenuProvider, public IWindowProvider {
    // すべてのインターフェースを実装
};

TEST(IntegrationTest, MultipleInterfacePlugin) {
    FullFeaturePlugin plugin;
    
    // IPlugin として機能
    EXPECT_TRUE(plugin.Initialize());
    EXPECT_TRUE(plugin.IsInitialized());
    
    // IMenuProvider として機能
    auto menuItems = plugin.GetMenuItems();
    EXPECT_FALSE(menuItems.empty());
    
    // IWindowProvider として機能
    auto windows = plugin.GetWindows();
    EXPECT_FALSE(windows.empty());
    
    plugin.Shutdown();
}
```

## 7. エラーケーステスト

### TC-601: API バージョン非互換
```cpp
TEST(ErrorCaseTest, IncompatibleAPIVersion) {
    APIVersion current{1, 0, 0};
    APIVersion incompatible{2, 0, 0};
    
    EXPECT_FALSE(current.IsCompatible(incompatible));
}
```

### TC-602: 未登録サービスアクセス
```cpp
TEST(ErrorCaseTest, UnregisteredServiceAccess) {
    ServiceLocator::Shutdown(); // すべてのサービスをクリア
    
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetLoggingService(), nullptr);
}
```

## 8. パフォーマンステスト

### TC-701: ServiceLocator アクセス性能
```cpp
TEST(PerformanceTest, ServiceLocatorAccess) {
    ServiceLocator::RegisterLoggingService(std::make_unique<MockLoggingService>());
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        auto* service = ServiceLocator::GetLoggingService();
        EXPECT_NE(service, nullptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    // 10000回のアクセスが1ms以内であることを確認（平均100ns以内）
    EXPECT_LT(duration.count(), 1000000);
    
    ServiceLocator::Shutdown();
}
```

## テスト実行環境

### 必要なフレームワーク
- **Google Test**: 単体テスト・統合テスト実行
- **Google Mock**: モックオブジェクト作成

### テスト設定
```cmake
# CMakeLists.txt for tests
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_executable(NSysCore_Tests
    test_apiversion.cpp
    test_servicelocator.cpp
    test_iplugin.cpp
    test_imenuprovider.cpp
    test_iwindowprovider.cpp
    test_integration.cpp
)

target_link_libraries(NSysCore_Tests
    GTest::gtest_main
    GMock::gmock
    NSysCore
)
```

## 継続的テスト

### 自動テスト実行
- ビルド時の自動テスト実行
- GitHub Actions でのテスト自動化
- コードカバレッジ測定（目標: 90%以上）

### テストメトリクス
- テスト実行時間: 5秒以内
- 全テストケース成功率: 100%
- コードカバレッジ: 行カバレッジ 90%以上

このテストケース設計に基づいて、次ステップでは失敗するテストの実装を行います。