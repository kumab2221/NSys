# TASK-002: CoreシステムI/F実装 - RED Phase (失敗するテストの実装)

## TDD RED Phase の目的

この段階では、まず**失敗するテスト**を実装します。これにより：
1. インターフェースの期待される動作を明確に定義
2. 実装すべき機能の具体的な仕様を確定
3. テストが確実に機能をテストしていることを保証

## 実装手順

### 1. テストプロジェクトの作成

まず、NSys プロジェクトにテスト用のプロジェクトを追加します。

#### テストプロジェクト設定ファイル作成

**ファイル**: `NSys/Tests/NSysCore_Tests.vcxproj`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{A1B2C3D4-E5F6-1234-ABCD-EF1234567890}</ProjectGuid>
    <RootNamespace>NSysCoreTests</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
    <WholeProgramOptimization>true</WholeProgramOptimization>
  </PropertyGroup>
  
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <AdditionalIncludeDirectories>
        $(MSBuildProjectDirectory)\..\;
        $(MSBuildProjectDirectory)\..\..\docs\design\nsys\;
        external\googletest\include\;
        %(AdditionalIncludeDirectories)
      </AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalLibraryDirectories>external\googletest\lib\debug\;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <AdditionalDependencies>gtest.lib;gtest_main.lib;gmock.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <AdditionalIncludeDirectories>
        $(MSBuildProjectDirectory)\..\;
        $(MSBuildProjectDirectory)\..\..\docs\design\nsys\;
        external\googletest\include\;
        %(AdditionalIncludeDirectories)
      </AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalLibraryDirectories>external\googletest\lib\release\;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <AdditionalDependencies>gtest.lib;gtest_main.lib;gmock.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  
  <ItemGroup>
    <ClCompile Include="test_apiversion.cpp" />
    <ClCompile Include="test_servicelocator.cpp" />
    <ClCompile Include="test_iplugin.cpp" />
    <ClCompile Include="test_imenuprovider.cpp" />
    <ClCompile Include="test_iwindowprovider.cpp" />
    <ClCompile Include="test_integration.cpp" />
  </ItemGroup>
  
  <ItemGroup>
    <ClInclude Include="test_common.h" />
    <ClInclude Include="mock_services.h" />
  </ItemGroup>
  
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
```

### 2. テスト共通定義

**ファイル**: `NSys/Tests/test_common.h`

```cpp
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
```

**ファイル**: `NSys/Tests/mock_services.h`

```cpp
#pragma once

#include <gmock/gmock.h>
#include "interfaces.h"

namespace NSys {

class MockFileSystemService : public IFileSystemService {
public:
    MOCK_METHOD(bool, ReadFile, (const std::string& path, std::string& content), (override));
    MOCK_METHOD(bool, WriteFile, (const std::string& path, const std::string& content), (override));
    MOCK_METHOD(bool, DeleteFile, (const std::string& path), (override));
    MOCK_METHOD(bool, FileExists, (const std::string& path), (override));
    MOCK_METHOD(bool, CreateDirectory, (const std::string& path), (override));
    MOCK_METHOD(bool, DirectoryExists, (const std::string& path), (override));
    MOCK_METHOD(std::vector<std::string>, ListDirectory, (const std::string& path), (override));
    MOCK_METHOD(std::string, GetWorkingDirectory, (), (override));
    MOCK_METHOD(std::string, GetApplicationDirectory, (), (override));
    MOCK_METHOD(std::string, GetConfigDirectory, (), (override));
    MOCK_METHOD(std::string, JoinPath, (const std::string& path1, const std::string& path2), (override));
    MOCK_METHOD(bool, HasReadPermission, (const std::string& path), (override));
    MOCK_METHOD(bool, HasWritePermission, (const std::string& path), (override));
};

class MockLoggingService : public ILoggingService {
public:
    MOCK_METHOD(void, Log, (LogLevel level, const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Debug, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Info, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Warning, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Error, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Critical, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, SetLogLevel, (LogLevel level), (override));
    MOCK_METHOD(LogLevel, GetLogLevel, (), (const, override));
    MOCK_METHOD(void, EnableFileLogging, (const std::string& filePath), (override));
    MOCK_METHOD(void, DisableFileLogging, (), (override));
};

class MockMemoryService : public IMemoryService {
public:
    MOCK_METHOD(void*, Allocate, (size_t size, const std::string& category), (override));
    MOCK_METHOD(void, Deallocate, (void* ptr), (override));
    MOCK_METHOD(void*, Reallocate, (void* ptr, size_t newSize), (override));
    MOCK_METHOD(MemoryStats, GetGlobalStats, (), (override));
    MOCK_METHOD(MemoryStats, GetCategoryStats, (const std::string& category), (override));
    MOCK_METHOD(bool, CheckMemoryLimits, (), (override));
    MOCK_METHOD(void, SetMemoryLimit, (size_t limit), (override));
    MOCK_METHOD(size_t, GetMemoryLimit, (), (const, override));
};

} // namespace NSys
```

### 3. 失敗するテストの実装

#### APIVersion テスト

**ファイル**: `NSys/Tests/test_apiversion.cpp`

```cpp
#include "test_common.h"

using namespace NSys;

class APIVersionTest : public NSysCoreTestBase {};

TEST_F(APIVersionTest, DefaultConstruction) {
    APIVersion version;
    
    // この時点ではAPIVersionは未実装のため、コンパイルエラーが発生する
    EXPECT_EQ(version.major, 1);
    EXPECT_EQ(version.minor, 0);
    EXPECT_EQ(version.patch, 0);
}

TEST_F(APIVersionTest, CustomConstruction) {
    APIVersion version{2, 1, 3};
    
    // 初期化リスト対応も未実装のため失敗
    EXPECT_EQ(version.major, 2);
    EXPECT_EQ(version.minor, 1);
    EXPECT_EQ(version.patch, 3);
}

TEST_F(APIVersionTest, CompatibilityCheck_SameMajorHigherMinor) {
    APIVersion base{1, 2, 0};
    APIVersion compatible{1, 3, 0};
    
    // IsCompatible メソッドが未実装のため失敗
    EXPECT_TRUE(base.IsCompatible(compatible));
}

TEST_F(APIVersionTest, CompatibilityCheck_DifferentMajor) {
    APIVersion base{1, 2, 0};
    APIVersion incompatible{2, 0, 0};
    
    EXPECT_FALSE(base.IsCompatible(incompatible));
}

TEST_F(APIVersionTest, CurrentAPIVersionDefined) {
    // CURRENT_API_VERSION 定数が未定義のため失敗
    EXPECT_EQ(CURRENT_API_VERSION.major, 1);
    EXPECT_EQ(CURRENT_API_VERSION.minor, 0);
    EXPECT_EQ(CURRENT_API_VERSION.patch, 0);
}
```

#### ServiceLocator テスト

**ファイル**: `NSys/Tests/test_servicelocator.cpp`

```cpp
#include "test_common.h"
#include "mock_services.h"

using namespace NSys;
using ::testing::NotNull;

class ServiceLocatorTest : public NSysCoreTestBase {};

TEST_F(ServiceLocatorTest, InitialState_AllServicesNull) {
    // ServiceLocator クラスが未実装のため、すべてのメソッドでコンパイルエラー
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetLoggingService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetMemoryService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetConfigurationService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetLocalizationService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetEventBus(), nullptr);
    EXPECT_EQ(ServiceLocator::GetPluginManager(), nullptr);
}

TEST_F(ServiceLocatorTest, RegisterAndGetFileSystemService) {
    auto mockService = std::make_unique<MockFileSystemService>();
    auto* servicePtr = mockService.get();
    
    // RegisterFileSystemService メソッドが未実装のため失敗
    ServiceLocator::RegisterFileSystemService(std::move(mockService));
    
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), servicePtr);
}

TEST_F(ServiceLocatorTest, RegisterAndGetLoggingService) {
    auto mockService = std::make_unique<MockLoggingService>();
    auto* servicePtr = mockService.get();
    
    ServiceLocator::RegisterLoggingService(std::move(mockService));
    
    EXPECT_EQ(ServiceLocator::GetLoggingService(), servicePtr);
}

TEST_F(ServiceLocatorTest, DuplicateRegistration_ReplacesPrevious) {
    auto firstService = std::make_unique<MockFileSystemService>();
    auto secondService = std::make_unique<MockFileSystemService>();
    auto* secondPtr = secondService.get();
    
    ServiceLocator::RegisterFileSystemService(std::move(firstService));
    ServiceLocator::RegisterFileSystemService(std::move(secondService));
    
    // 重複登録の処理が未実装のため失敗
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), secondPtr);
}

TEST_F(ServiceLocatorTest, Shutdown_ClearsAllServices) {
    ServiceLocator::RegisterFileSystemService(std::make_unique<MockFileSystemService>());
    ServiceLocator::RegisterLoggingService(std::make_unique<MockLoggingService>());
    
    // Shutdown メソッドが未実装のため失敗
    ServiceLocator::Shutdown();
    
    EXPECT_EQ(ServiceLocator::GetFileSystemService(), nullptr);
    EXPECT_EQ(ServiceLocator::GetLoggingService(), nullptr);
}
```

#### IPlugin テスト

**ファイル**: `NSys/Tests/test_iplugin.cpp`

```cpp
#include "test_common.h"

using namespace NSys;

// テスト用のプラグイン実装（最初は未完成）
class TestPlugin : public IPlugin {
private:
    bool m_initialized = false;
    bool m_enabled = true;
    
public:
    // これらのメソッドは IPlugin インターフェースが未定義のため、最初はコンパイルエラー
    bool Initialize() override {
        if (m_initialized) return false;
        m_initialized = true;
        return true;
    }
    
    void Shutdown() override {
        m_initialized = false;
    }
    
    void Update(float deltaTime) override {
        // テスト用空実装
    }
    
    void Render() override {
        // テスト用空実装
    }
    
    PluginInfo GetPluginInfo() const override {
        // PluginInfo 構造体も未定義のため失敗
        PluginInfo info;
        info.name = "TestPlugin";
        info.version = "1.0.0";
        info.description = "Test plugin";
        return info;
    }
    
    std::string GetName() const override { return "TestPlugin"; }
    std::string GetVersion() const override { return "1.0.0"; }
    bool IsInitialized() const override { return m_initialized; }
    void SetEnabled(bool enabled) override { m_enabled = enabled; }
    bool IsEnabled() const override { return m_enabled; }
};

class IPluginTest : public NSysCoreTestBase {};

TEST_F(IPluginTest, LifecycleManagement) {
    TestPlugin plugin;
    
    // IPlugin インターフェースが未定義のため、全てのメソッド呼び出しが失敗
    EXPECT_FALSE(plugin.IsInitialized());
    EXPECT_TRUE(plugin.IsEnabled());
    
    EXPECT_TRUE(plugin.Initialize());
    EXPECT_TRUE(plugin.IsInitialized());
    
    EXPECT_FALSE(plugin.Initialize()); // 重複初期化防止
    
    plugin.Shutdown();
    EXPECT_FALSE(plugin.IsInitialized());
}

TEST_F(IPluginTest, EnabledStateManagement) {
    TestPlugin plugin;
    
    EXPECT_TRUE(plugin.IsEnabled());
    
    plugin.SetEnabled(false);
    EXPECT_FALSE(plugin.IsEnabled());
    
    plugin.SetEnabled(true);
    EXPECT_TRUE(plugin.IsEnabled());
}

TEST_F(IPluginTest, PluginInfo) {
    TestPlugin plugin;
    
    EXPECT_EQ(plugin.GetName(), "TestPlugin");
    EXPECT_EQ(plugin.GetVersion(), "1.0.0");
    
    auto info = plugin.GetPluginInfo();
    EXPECT_EQ(info.name, "TestPlugin");
    EXPECT_EQ(info.version, "1.0.0");
    EXPECT_EQ(info.description, "Test plugin");
}
```

#### IMenuProvider テスト

**ファイル**: `NSys/Tests/test_imenuprovider.cpp`

```cpp
#include "test_common.h"

using namespace NSys;

// IMenuProvider インターフェースが未定義のため、このクラス定義自体がコンパイルエラー
class TestMenuProvider : public IMenuProvider {
public:
    std::vector<MenuItem> GetMenuItems() override {
        std::vector<MenuItem> items;
        
        MenuItem fileMenu;
        fileMenu.label = "File";
        
        MenuItem newItem;
        newItem.label = "New";
        newItem.shortcut = "Ctrl+N";
        newItem.callback = []() { /* New action */ };
        
        fileMenu.children = {newItem};
        items.push_back(fileMenu);
        
        return items;
    }
    
    void OnMenuItemSelected(const std::string& itemId) override {
        // テスト用実装
    }
};

class IMenuProviderTest : public NSysCoreTestBase {};

TEST_F(IMenuProviderTest, GetMenuItems_ReturnsValidStructure) {
    // IMenuProvider および MenuItem 構造体が未定義のため失敗
    TestMenuProvider provider;
    auto items = provider.GetMenuItems();
    
    EXPECT_EQ(items.size(), 1);
    
    const auto& fileMenu = items[0];
    EXPECT_EQ(fileMenu.label, "File");
    EXPECT_EQ(fileMenu.children.size(), 1);
    
    EXPECT_EQ(fileMenu.children[0].label, "New");
    EXPECT_EQ(fileMenu.children[0].shortcut, "Ctrl+N");
    EXPECT_NE(fileMenu.children[0].callback, nullptr);
}

TEST_F(IMenuProviderTest, MenuItemCallback_IsCallable) {
    TestMenuProvider provider;
    auto items = provider.GetMenuItems();
    
    const auto& newItem = items[0].children[0];
    
    EXPECT_NO_THROW(newItem.callback());
}
```

#### IWindowProvider テスト

**ファイル**: `NSys/Tests/test_iwindowprovider.cpp`

```cpp
#include "test_common.h"

using namespace NSys;

// IWindowProvider インターフェースが未定義のため失敗
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
        windows.push_back(mainWindow);
        
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

class IWindowProviderTest : public NSysCoreTestBase {};

TEST_F(IWindowProviderTest, GetWindows_ReturnsValidConfiguration) {
    // WindowInfo 構造体が未定義のため失敗
    TestWindowProvider provider;
    auto windows = provider.GetWindows();
    
    EXPECT_EQ(windows.size(), 1);
    
    const auto& mainWindow = windows[0];
    EXPECT_EQ(mainWindow.title, "Main Window");
    EXPECT_EQ(mainWindow.defaultSize.x, 1200);
    EXPECT_EQ(mainWindow.defaultSize.y, 800);
}

TEST_F(IWindowProviderTest, WindowStateManagement) {
    TestWindowProvider provider;
    
    EXPECT_TRUE(provider.IsWindowOpen("MainWindow"));
    EXPECT_FALSE(provider.IsWindowOpen("DebugWindow"));
    EXPECT_FALSE(provider.IsWindowOpen("NonexistentWindow"));
    
    provider.SetWindowOpen("DebugWindow", true);
    EXPECT_TRUE(provider.IsWindowOpen("DebugWindow"));
}
```

#### 統合テスト

**ファイル**: `NSys/Tests/test_integration.cpp`

```cpp
#include "test_common.h"
#include "mock_services.h"

using namespace NSys;

class IntegrationTest : public NSysCoreTestBase {};

TEST_F(IntegrationTest, PluginWithServiceLocator) {
    // ServiceLocator と Plugin の統合が未実装のため失敗
    ServiceLocator::RegisterLoggingService(std::make_unique<MockLoggingService>());
    
    TestPlugin plugin; // test_iplugin.cpp から
    
    EXPECT_TRUE(plugin.Initialize());
    
    auto* logger = ServiceLocator::GetLoggingService();
    EXPECT_NE(logger, nullptr);
    
    plugin.Shutdown();
    ServiceLocator::Shutdown();
}
```

### 4. ビルド実行とエラー確認

この段階でテストプロジェクトをビルドすると、以下のようなコンパイルエラーが発生することが期待されます：

```
1. interfaces.h が存在しない
2. APIVersion 構造体が未定義
3. ServiceLocator クラスが未定義
4. IPlugin インターフェースが未定義
5. IMenuProvider インターフェースが未定義
6. IWindowProvider インターフェースが未定義
7. PluginInfo 構造体が未定義
8. MenuItem 構造体が未定義
9. WindowInfo 構造体が未定義
10. CURRENT_API_VERSION 定数が未定義
```

### 5. 期待される失敗の確認

ビルド試行時に以下のエラーメッセージが表示されることを確認：

```cpp
// 期待されるコンパイルエラー:
error C1083: Cannot open include file: 'interfaces.h': No such file or directory
error C2653: 'APIVersion': is not a class or namespace name
error C2065: 'ServiceLocator': undeclared identifier
error C2504: 'IPlugin': base class undefined
error C2079: 'PluginInfo' uses undefined struct
```

これらのエラーは **期待された失敗** であり、次の GREEN フェーズで解決していきます。

## RED Phase 完了チェックリスト

- [x] テストプロジェクトが作成され、ソリューションに追加された
- [x] すべての主要インターフェースのテストケースが実装された
- [x] テストは現在すべてコンパイルエラーで失敗する（期待された動作）
- [x] 失敗の原因が明確に特定できる（未実装のインターフェース）
- [x] テストケースは実装すべき機能を正確に表現している

## 次のステップ

GREEN フェーズでは、これらのテストを通すための **最小限の実装** を行います：

1. `interfaces.h` ファイルの作成
2. 各インターフェースの基本定義
3. `ServiceLocator` の基本実装
4. 必要な構造体とenum定義

各実装は「テストを通す」ことを目的とした最小限に留め、過度な実装を避けます。