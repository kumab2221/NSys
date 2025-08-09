# TASK-002: CoreシステムI/F実装 - GREEN Phase (最小実装)

## TDD GREEN Phase の目的

RED Phase で失敗したテストを**最小限の実装**で成功させます。この段階では：
1. テストを通すことだけを目的とする
2. 過度に複雑な実装は避ける
3. 実装の品質向上は次のREFACTOR フェーズで行う

## 実装戦略

### 1. interfaces.h ファイルの作成

まず、NSys プロジェクトに基本的な `interfaces.h` を実装します。

**ファイル**: `NSys/interfaces.h`

```cpp
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <cstdint>
#include <type_traits>

// ImGui forward declarations (最小限のモック)
struct ImVec2 {
    float x = 0.0f;
    float y = 0.0f;
    
    ImVec2() = default;
    ImVec2(float x, float y) : x(x), y(y) {}
};

namespace NSys {

// ==================== API Version Management ====================

/**
 * @brief API バージョン情報
 */
struct APIVersion {
    uint32_t major = 1;
    uint32_t minor = 0;
    uint32_t patch = 0;
    
    bool IsCompatible(const APIVersion& other) const {
        // 最小実装: 基本的な互換性チェック
        return major == other.major && minor >= other.minor;
    }
};

// テストで要求される定数定義
constexpr APIVersion CURRENT_API_VERSION = {1, 0, 0};

/**
 * @brief プラグイン情報構造体
 */
struct PluginInfo {
    APIVersion apiVersion = CURRENT_API_VERSION;
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::vector<std::string> dependencies;
};

// ==================== Core Service Interfaces (最小定義) ====================

/**
 * @brief ファイルシステムサービスインターフェース（最小版）
 */
class IFileSystemService {
public:
    virtual ~IFileSystemService() = default;
    
    // テストで必要な最小限のメソッドのみ定義
    virtual bool ReadFile(const std::string& path, std::string& content) = 0;
    virtual bool WriteFile(const std::string& path, const std::string& content) = 0;
    virtual bool DeleteFile(const std::string& path) = 0;
    virtual bool FileExists(const std::string& path) = 0;
    virtual bool CreateDirectory(const std::string& path) = 0;
    virtual bool DirectoryExists(const std::string& path) = 0;
    virtual std::vector<std::string> ListDirectory(const std::string& path) = 0;
    virtual std::string GetWorkingDirectory() = 0;
    virtual std::string GetApplicationDirectory() = 0;
    virtual std::string GetConfigDirectory() = 0;
    virtual std::string JoinPath(const std::string& path1, const std::string& path2) = 0;
    virtual bool HasReadPermission(const std::string& path) = 0;
    virtual bool HasWritePermission(const std::string& path) = 0;
};

/**
 * @brief ログレベル列挙型
 */
enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
};

/**
 * @brief ログサービスインターフェース（最小版）
 */
class ILoggingService {
public:
    virtual ~ILoggingService() = default;
    
    virtual void Log(LogLevel level, const std::string& message, const std::string& category = "") = 0;
    virtual void Debug(const std::string& message, const std::string& category = "") = 0;
    virtual void Info(const std::string& message, const std::string& category = "") = 0;
    virtual void Warning(const std::string& message, const std::string& category = "") = 0;
    virtual void Error(const std::string& message, const std::string& category = "") = 0;
    virtual void Critical(const std::string& message, const std::string& category = "") = 0;
    virtual void SetLogLevel(LogLevel level) = 0;
    virtual LogLevel GetLogLevel() const = 0;
    virtual void EnableFileLogging(const std::string& filePath) = 0;
    virtual void DisableFileLogging() = 0;
};

/**
 * @brief メモリ使用量統計（最小版）
 */
struct MemoryStats {
    size_t totalAllocated = 0;
    size_t totalFreed = 0;
    size_t currentUsage = 0;
    size_t peakUsage = 0;
    uint32_t allocationCount = 0;
};

/**
 * @brief メモリ管理サービスインターフェース（最小版）
 */
class IMemoryService {
public:
    virtual ~IMemoryService() = default;
    
    virtual void* Allocate(size_t size, const std::string& category = "") = 0;
    virtual void Deallocate(void* ptr) = 0;
    virtual void* Reallocate(void* ptr, size_t newSize) = 0;
    virtual MemoryStats GetGlobalStats() = 0;
    virtual MemoryStats GetCategoryStats(const std::string& category) = 0;
    virtual bool CheckMemoryLimits() = 0;
    virtual void SetMemoryLimit(size_t limit) = 0;
    virtual size_t GetMemoryLimit() const = 0;
};

// 他のサービスインターフェースも最小定義（テストをパスするため）
class IConfigurationService {
public:
    virtual ~IConfigurationService() = default;
    // 最小実装のため空のインターフェース
};

class ILocalizationService {
public:
    virtual ~ILocalizationService() = default;
    // 最小実装のため空のインターフェース
};

class IEventBus {
public:
    virtual ~IEventBus() = default;
    // 最小実装のため空のインターフェース
};

class IPluginManager {
public:
    virtual ~IPluginManager() = default;
    // 最小実装のため空のインターフェース
};

// ==================== Plugin Interfaces ====================

/**
 * @brief プラグインの基底インターフェース
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // ライフサイクル
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    
    // プラグイン情報
    virtual PluginInfo GetPluginInfo() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetVersion() const = 0;
    
    // 状態管理
    virtual bool IsInitialized() const = 0;
    virtual void SetEnabled(bool enabled) = 0;
    virtual bool IsEnabled() const = 0;
};

/**
 * @brief メニュー項目構造体
 */
struct MenuItem {
    std::string label;
    std::function<void()> callback;
    std::string shortcut;
    bool separator = false;
    std::vector<MenuItem> children;
};

/**
 * @brief メニュープロバイダーインターフェース
 */
class IMenuProvider {
public:
    virtual ~IMenuProvider() = default;
    
    virtual std::vector<MenuItem> GetMenuItems() = 0;
    virtual void OnMenuItemSelected(const std::string& itemId) = 0;
};

/**
 * @brief ウィンドウ情報構造体
 */
struct WindowInfo {
    std::string title;
    ImVec2 defaultSize = {800, 600};
    ImVec2 minSize = {200, 150};
    bool dockable = true;
    bool closeable = true;
    bool collapsible = true;
    bool resizable = true;
};

/**
 * @brief ウィンドウプロバイダーインターフェース
 */
class IWindowProvider {
public:
    virtual ~IWindowProvider() = default;
    
    virtual std::vector<WindowInfo> GetWindows() = 0;
    virtual void RenderWindow(const std::string& windowId) = 0;
    virtual bool IsWindowOpen(const std::string& windowId) = 0;
    virtual void SetWindowOpen(const std::string& windowId, bool open) = 0;
};

// ==================== Service Locator (最小実装) ====================

/**
 * @brief サービスロケーター（最小実装版）
 */
class ServiceLocator {
private:
    // 最小実装: 静的メンバ変数でサービスを保持
    static std::unique_ptr<IFileSystemService> s_fileSystemService;
    static std::unique_ptr<ILoggingService> s_loggingService;
    static std::unique_ptr<IMemoryService> s_memoryService;
    static std::unique_ptr<IConfigurationService> s_configurationService;
    static std::unique_ptr<ILocalizationService> s_localizationService;
    static std::unique_ptr<IEventBus> s_eventBus;
    static std::unique_ptr<IPluginManager> s_pluginManager;
    
public:
    // Service getters
    static IFileSystemService* GetFileSystemService() {
        return s_fileSystemService.get();
    }
    
    static ILoggingService* GetLoggingService() {
        return s_loggingService.get();
    }
    
    static IMemoryService* GetMemoryService() {
        return s_memoryService.get();
    }
    
    static IConfigurationService* GetConfigurationService() {
        return s_configurationService.get();
    }
    
    static ILocalizationService* GetLocalizationService() {
        return s_localizationService.get();
    }
    
    static IEventBus* GetEventBus() {
        return s_eventBus.get();
    }
    
    static IPluginManager* GetPluginManager() {
        return s_pluginManager.get();
    }
    
    // Service registration
    static void RegisterFileSystemService(std::unique_ptr<IFileSystemService> service) {
        s_fileSystemService = std::move(service);
    }
    
    static void RegisterLoggingService(std::unique_ptr<ILoggingService> service) {
        s_loggingService = std::move(service);
    }
    
    static void RegisterMemoryService(std::unique_ptr<IMemoryService> service) {
        s_memoryService = std::move(service);
    }
    
    static void RegisterConfigurationService(std::unique_ptr<IConfigurationService> service) {
        s_configurationService = std::move(service);
    }
    
    static void RegisterLocalizationService(std::unique_ptr<ILocalizationService> service) {
        s_localizationService = std::move(service);
    }
    
    static void RegisterEventBus(std::unique_ptr<IEventBus> service) {
        s_eventBus = std::move(service);
    }
    
    static void RegisterPluginManager(std::unique_ptr<IPluginManager> service) {
        s_pluginManager = std::move(service);
    }
    
    // Service cleanup
    static void Shutdown() {
        s_fileSystemService.reset();
        s_loggingService.reset();
        s_memoryService.reset();
        s_configurationService.reset();
        s_localizationService.reset();
        s_eventBus.reset();
        s_pluginManager.reset();
    }
};

} // namespace NSys
```

### 2. ServiceLocator の静的メンバ定義

**ファイル**: `NSys/ServiceLocator.cpp`

```cpp
#include "interfaces.h"

namespace NSys {

// ServiceLocator の静的メンバ変数定義（最小実装）
std::unique_ptr<IFileSystemService> ServiceLocator::s_fileSystemService;
std::unique_ptr<ILoggingService> ServiceLocator::s_loggingService;
std::unique_ptr<IMemoryService> ServiceLocator::s_memoryService;
std::unique_ptr<IConfigurationService> ServiceLocator::s_configurationService;
std::unique_ptr<ILocalizationService> ServiceLocator::s_localizationService;
std::unique_ptr<IEventBus> ServiceLocator::s_eventBus;
std::unique_ptr<IPluginManager> ServiceLocator::s_pluginManager;

} // namespace NSys
```

### 3. プロジェクト設定の更新

既存の NSys.vcxproj に ServiceLocator.cpp を追加：

```xml
<ItemGroup>
  <ClCompile Include="external\imgui\backends\imgui_impl_dx12.cpp" />
  <!-- 既存のファイル... -->
  <ClCompile Include="ServiceLocator.cpp" />  <!-- 追加 -->
</ItemGroup>

<ItemGroup>
  <ClInclude Include="json.hpp" />
  <!-- 既存のファイル... -->
  <ClInclude Include="interfaces.h" />  <!-- 追加 -->
</ItemGroup>
```

### 4. テストコンパイルの確認

実装後、テストファイルが正しくコンパイルできることを確認します。

### 5. 最小実装の検証

実装した interfaces.h でテストが通ることを確認：

```bash
# 基本的なコンパイルチェック
cl /EHsc /I. NSys/Tests/test_compile_check.cpp
```

## 実装のポイント

### 最小実装の原則

1. **テストを通すことが唯一の目的**
   - 不要な機能は実装しない
   - 複雑なロジックは避ける

2. **単純な実装を選択**
   - ServiceLocator は静的メンバ変数で実装
   - 複雑なファクトリーパターンは使用しない

3. **エラーハンドリングは最小限**
   - nullptr チェックなどは REFACTOR フェーズで追加

4. **完全性より動作性を優先**
   - 一部のサービスインターフェースは空実装
   - 必要最小限のメソッドのみ定義

### 実装の制約

1. **コンパイルエラーの解消が最優先**
   - すべてのテストがコンパイル可能になること
   - 実行時エラーは REFACTOR フェーズで対応

2. **依存関係の最小化**
   - 外部ライブラリへの依存を避ける
   - ImGui は最小限のモックで対応

3. **スレッドセーフティは考慮しない**
   - 単純な実装を優先
   - 複雑な同期処理は次フェーズで検討

## 期待される結果

GREEN フェーズ完了後、以下の状態になることを期待：

### コンパイル成功
```bash
✅ test_apiversion.cpp - コンパイル成功
✅ test_servicelocator.cpp - コンパイル成功  
✅ test_iplugin.cpp - コンパイル成功
✅ test_imenuprovider.cpp - コンパイル成功
✅ test_iwindowprovider.cpp - コンパイル成功
✅ test_integration.cpp - コンパイル成功
```

### テスト実行結果（期待値）
```
[==========] Running X tests from Y test suites.
[----------] Global test environment set-up.
[----------] Tests from APIVersionTest
[ RUN      ] APIVersionTest.DefaultConstruction
[       OK ] APIVersionTest.DefaultConstruction
[ RUN      ] APIVersionTest.CustomConstruction  
[       OK ] APIVersionTest.CustomConstruction
[----------] Tests from ServiceLocatorTest
[ RUN      ] ServiceLocatorTest.InitialState_AllServicesNull
[       OK ] ServiceLocatorTest.InitialState_AllServicesNull
[----------] Tests passed
[==========] X tests passed
```

## 次のステップ

GREEN フェーズでテストが通ったら、REFACTOR フェーズで以下を改善：

1. **コードの品質向上**
   - エラーハンドリングの追加
   - nullptrチェック
   - 例外安全性の確保

2. **実装の完全性**
   - 空実装だったサービスの基本機能追加
   - APIの一貫性確保

3. **パフォーマンスの最適化**
   - ServiceLocator のアクセス効率化
   - 不要なメモリ確保の最適化

4. **設計の改善**
   - インターフェースの分離度向上
   - 依存関係の整理

GREEN フェーズは「動作する最小限の実装」の作成であり、品質やパフォーマンスは次のフェーズで改善していきます。