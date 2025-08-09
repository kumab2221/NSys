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