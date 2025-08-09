#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <cstdint>
#include <type_traits>

// ImGui forward declarations
struct ImGuiContext;
struct ImVec2;
struct ImVec4;

namespace NSys {

// ==================== Core System Interfaces ====================

/**
 * @brief API バージョン情報
 */
struct APIVersion {
    uint32_t major = 1;
    uint32_t minor = 0;
    uint32_t patch = 0;
    
    bool IsCompatible(const APIVersion& other) const {
        return major == other.major && minor >= other.minor;
    }
};

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

// ==================== Service Interfaces ====================

/**
 * @brief ファイルシステムサービスインターフェース
 */
class IFileSystemService {
public:
    virtual ~IFileSystemService() = default;
    
    // ファイル操作
    virtual bool ReadFile(const std::string& path, std::string& content) = 0;
    virtual bool WriteFile(const std::string& path, const std::string& content) = 0;
    virtual bool DeleteFile(const std::string& path) = 0;
    virtual bool FileExists(const std::string& path) = 0;
    
    // ディレクトリ操作
    virtual bool CreateDirectory(const std::string& path) = 0;
    virtual bool DirectoryExists(const std::string& path) = 0;
    virtual std::vector<std::string> ListDirectory(const std::string& path) = 0;
    
    // パス操作
    virtual std::string GetWorkingDirectory() = 0;
    virtual std::string GetApplicationDirectory() = 0;
    virtual std::string GetConfigDirectory() = 0;
    virtual std::string JoinPath(const std::string& path1, const std::string& path2) = 0;
    
    // 権限チェック
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
 * @brief ログサービスインターフェース
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
 * @brief メモリ使用量統計
 */
struct MemoryStats {
    size_t totalAllocated = 0;
    size_t totalFreed = 0;
    size_t currentUsage = 0;
    size_t peakUsage = 0;
    uint32_t allocationCount = 0;
};

/**
 * @brief メモリ管理サービスインターフェース
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

/**
 * @brief 設定値変換ヘルパー
 */
class ConfigValue {
private:
    std::string m_value;
    
public:
    explicit ConfigValue(const std::string& value) : m_value(value) {}
    
    template<typename T>
    T As() const;
    
    std::string AsString() const { return m_value; }
    int AsInt() const;
    float AsFloat() const;
    bool AsBool() const;
    
    template<typename T>
    operator T() const { return As<T>(); }
};

/**
 * @brief 設定管理サービスインターフェース
 */
class IConfigurationService {
public:
    virtual ~IConfigurationService() = default;
    
    // 設定値の取得・設定
    virtual ConfigValue GetValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") = 0;
    virtual void SetValue(const std::string& section, const std::string& key, const std::string& value) = 0;
    
    // 型安全なアクセス
    template<typename T>
    T GetValue(const std::string& section, const std::string& key, const T& defaultValue = T{});
    
    template<typename T>
    void SetValue(const std::string& section, const std::string& key, const T& value);
    
    // セクション管理
    virtual bool HasSection(const std::string& section) = 0;
    virtual bool HasKey(const std::string& section, const std::string& key) = 0;
    virtual void RemoveKey(const std::string& section, const std::string& key) = 0;
    virtual void RemoveSection(const std::string& section) = 0;
    virtual std::vector<std::string> GetSections() = 0;
    virtual std::vector<std::string> GetKeys(const std::string& section) = 0;
    
    // ファイル操作
    virtual bool LoadFromFile(const std::string& filePath) = 0;
    virtual bool SaveToFile(const std::string& filePath) = 0;
    virtual void SetAutoSave(bool enabled, int intervalSeconds = 60) = 0;
};

/**
 * @brief ローカライゼーションサービスインターフェース
 */
class ILocalizationService {
public:
    virtual ~ILocalizationService() = default;
    
    virtual bool LoadLanguage(const std::string& languageCode) = 0;
    virtual std::string GetText(const std::string& key, const std::string& defaultText = "") = 0;
    virtual std::string GetCurrentLanguage() const = 0;
    virtual std::vector<std::string> GetAvailableLanguages() = 0;
    virtual bool AddTranslation(const std::string& key, const std::string& text) = 0;
    virtual void SetFallbackLanguage(const std::string& languageCode) = 0;
};

// ローカライゼーション用マクロ
#define _(key) NSys::ServiceLocator::GetLocalizationService()->GetText(key, key)
#define _T(key, defaultText) NSys::ServiceLocator::GetLocalizationService()->GetText(key, defaultText)

// ==================== Event System ====================

/**
 * @brief イベントベース
 */
class IEvent {
public:
    virtual ~IEvent() = default;
    virtual std::type_info const& GetType() const = 0;
};

/**
 * @brief 型付きイベントベース
 */
template<typename T>
class Event : public IEvent {
public:
    std::type_info const& GetType() const override {
        return typeid(T);
    }
};

/**
 * @brief イベントハンドラー
 */
template<typename T>
using EventHandler = std::function<void(const T&)>;

/**
 * @brief イベントバスインターフェース
 */
class IEventBus {
public:
    virtual ~IEventBus() = default;
    
    template<typename T>
    void Subscribe(const EventHandler<T>& handler, const std::string& subscriberId = "");
    
    template<typename T>
    void Unsubscribe(const std::string& subscriberId);
    
    template<typename T>
    void Publish(const T& event);
    
    virtual void UnsubscribeAll(const std::string& subscriberId) = 0;
    
protected:
    virtual void SubscribeImpl(const std::type_info& type, std::function<void(const IEvent&)> handler, const std::string& subscriberId) = 0;
    virtual void UnsubscribeImpl(const std::type_info& type, const std::string& subscriberId) = 0;
    virtual void PublishImpl(const IEvent& event) = 0;
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
 * @brief メニュープロバイダーインターフェース
 */
class IMenuProvider {
public:
    virtual ~IMenuProvider() = default;
    
    struct MenuItem {
        std::string label;
        std::function<void()> callback;
        std::string shortcut;
        bool separator = false;
        std::vector<MenuItem> children;
    };
    
    virtual std::vector<MenuItem> GetMenuItems() = 0;
    virtual void OnMenuItemSelected(const std::string& itemId) = 0;
};

/**
 * @brief ウィンドウプロバイダーインターフェース
 */
class IWindowProvider {
public:
    virtual ~IWindowProvider() = default;
    
    struct WindowInfo {
        std::string title;
        ImVec2 defaultSize = {800, 600};
        ImVec2 minSize = {200, 150};
        bool dockable = true;
        bool closeable = true;
        bool collapsible = true;
        bool resizable = true;
    };
    
    virtual std::vector<WindowInfo> GetWindows() = 0;
    virtual void RenderWindow(const std::string& windowId) = 0;
    virtual bool IsWindowOpen(const std::string& windowId) = 0;
    virtual void SetWindowOpen(const std::string& windowId, bool open) = 0;
};

/**
 * @brief データプロバイダーインターフェース
 */
class IDataProvider {
public:
    virtual ~IDataProvider() = default;
    
    template<typename T>
    void SetData(const std::string& key, const T& data);
    
    template<typename T>
    T GetData(const std::string& key, const T& defaultValue = T{}) const;
    
    virtual bool HasData(const std::string& key) const = 0;
    virtual void RemoveData(const std::string& key) = 0;
    virtual std::vector<std::string> GetDataKeys() const = 0;
    
protected:
    virtual void SetDataImpl(const std::string& key, std::shared_ptr<void> data, const std::type_info& type) = 0;
    virtual std::shared_ptr<void> GetDataImpl(const std::string& key, const std::type_info& type) const = 0;
};

// ==================== Core System Events ====================

/**
 * @brief アプリケーション開始イベント
 */
struct ApplicationStartedEvent : public Event<ApplicationStartedEvent> {
    std::string applicationName;
    std::string version;
};

/**
 * @brief アプリケーション終了イベント
 */
struct ApplicationShutdownEvent : public Event<ApplicationShutdownEvent> {
    bool gracefulShutdown = true;
};

/**
 * @brief プラグイン読み込みイベント
 */
struct PluginLoadedEvent : public Event<PluginLoadedEvent> {
    std::string pluginName;
    std::string pluginVersion;
    IPlugin* plugin = nullptr;
};

/**
 * @brief プラグインアンロードイベント
 */
struct PluginUnloadedEvent : public Event<PluginUnloadedEvent> {
    std::string pluginName;
    std::string reason;
};

/**
 * @brief 設定変更イベント
 */
struct ConfigurationChangedEvent : public Event<ConfigurationChangedEvent> {
    std::string section;
    std::string key;
    std::string oldValue;
    std::string newValue;
};

/**
 * @brief 言語変更イベント
 */
struct LanguageChangedEvent : public Event<LanguageChangedEvent> {
    std::string oldLanguage;
    std::string newLanguage;
};

// ==================== Plugin Management ====================

/**
 * @brief プラグイン状態
 */
enum class PluginState {
    Unloaded = 0,
    Loaded = 1,
    Initialized = 2,
    Error = 3
};

/**
 * @brief プラグインマネージャーインターフェース
 */
class IPluginManager {
public:
    virtual ~IPluginManager() = default;
    
    // プラグイン読み込み・アンロード
    virtual bool LoadPlugin(const std::string& dllPath) = 0;
    virtual bool UnloadPlugin(const std::string& pluginName) = 0;
    virtual bool ReloadPlugin(const std::string& pluginName) = 0;
    
    // プラグイン検索・取得
    virtual IPlugin* FindPlugin(const std::string& name) = 0;
    virtual std::vector<IPlugin*> GetAllPlugins() = 0;
    virtual std::vector<std::string> GetPluginNames() = 0;
    
    // プラグイン状態
    virtual PluginState GetPluginState(const std::string& pluginName) = 0;
    virtual std::string GetPluginError(const std::string& pluginName) = 0;
    
    // プラグイン依存関係
    virtual bool CheckDependencies(const std::string& pluginName) = 0;
    virtual std::vector<std::string> GetDependencies(const std::string& pluginName) = 0;
    virtual std::vector<std::string> GetDependents(const std::string& pluginName) = 0;
    
    // プラグインディレクトリ
    virtual void AddPluginDirectory(const std::string& directory) = 0;
    virtual void RemovePluginDirectory(const std::string& directory) = 0;
    virtual std::vector<std::string> GetPluginDirectories() = 0;
    virtual void ScanPluginDirectories() = 0;
};

// ==================== Service Locator ====================

/**
 * @brief サービスロケーター
 */
class ServiceLocator {
public:
    // Core services
    static IFileSystemService* GetFileSystemService();
    static ILoggingService* GetLoggingService();
    static IMemoryService* GetMemoryService();
    static IConfigurationService* GetConfigurationService();
    static ILocalizationService* GetLocalizationService();
    static IEventBus* GetEventBus();
    static IPluginManager* GetPluginManager();
    
    // Service registration (for internal use)
    static void RegisterFileSystemService(std::unique_ptr<IFileSystemService> service);
    static void RegisterLoggingService(std::unique_ptr<ILoggingService> service);
    static void RegisterMemoryService(std::unique_ptr<IMemoryService> service);
    static void RegisterConfigurationService(std::unique_ptr<IConfigurationService> service);
    static void RegisterLocalizationService(std::unique_ptr<ILocalizationService> service);
    static void RegisterEventBus(std::unique_ptr<IEventBus> service);
    static void RegisterPluginManager(std::unique_ptr<IPluginManager> service);
    
    // Service cleanup
    static void Shutdown();
    
private:
    ServiceLocator() = default;
    static ServiceLocator& Instance();
    
    struct ServiceRegistry;
    std::unique_ptr<ServiceRegistry> m_registry;
};

// ==================== Plugin Factory ====================

/**
 * @brief プラグインファクトリー関数型
 */
using CreatePluginFunc = IPlugin*();
using DestroyPluginFunc = void(IPlugin*);

/**
 * @brief プラグインエクスポート用マクロ
 */
#define NSYS_PLUGIN_EXPORT extern "C" __declspec(dllexport)

#define NSYS_DECLARE_PLUGIN(PluginClass) \
    NSYS_PLUGIN_EXPORT NSys::IPlugin* CreatePlugin() { \
        return new PluginClass(); \
    } \
    NSYS_PLUGIN_EXPORT void DestroyPlugin(NSys::IPlugin* plugin) { \
        delete plugin; \
    } \
    NSYS_PLUGIN_EXPORT NSys::PluginInfo GetPluginInfo() { \
        PluginClass temp; \
        return temp.GetPluginInfo(); \
    }

// ==================== Utility Classes ====================

/**
 * @brief RAII スタイルのプラグイン基底クラス
 */
class PluginBase : public IPlugin {
protected:
    PluginInfo m_info;
    bool m_initialized = false;
    bool m_enabled = true;
    
    // Protected services access
    IFileSystemService* GetFileSystem() const { return ServiceLocator::GetFileSystemService(); }
    ILoggingService* GetLogger() const { return ServiceLocator::GetLoggingService(); }
    IConfigurationService* GetConfig() const { return ServiceLocator::GetConfigurationService(); }
    ILocalizationService* GetLocalization() const { return ServiceLocator::GetLocalizationService(); }
    IEventBus* GetEventBus() const { return ServiceLocator::GetEventBus(); }
    
public:
    explicit PluginBase(const PluginInfo& info) : m_info(info) {}
    virtual ~PluginBase() = default;
    
    // IPlugin implementation
    bool Initialize() override final {
        if (m_initialized) return true;
        
        bool result = OnInitialize();
        if (result) {
            m_initialized = true;
            GetEventBus()->Publish(PluginLoadedEvent{m_info.name, m_info.version, this});
        }
        return result;
    }
    
    void Shutdown() override final {
        if (!m_initialized) return;
        
        OnShutdown();
        m_initialized = false;
        GetEventBus()->Publish(PluginUnloadedEvent{m_info.name, "Normal shutdown"});
    }
    
    PluginInfo GetPluginInfo() const override { return m_info; }
    std::string GetName() const override { return m_info.name; }
    std::string GetVersion() const override { return m_info.version; }
    bool IsInitialized() const override { return m_initialized; }
    void SetEnabled(bool enabled) override { m_enabled = enabled; }
    bool IsEnabled() const override { return m_enabled; }
    
    void Update(float deltaTime) override {
        if (!m_enabled || !m_initialized) return;
        OnUpdate(deltaTime);
    }
    
    void Render() override {
        if (!m_enabled || !m_initialized) return;
        OnRender();
    }
    
protected:
    // Override these in derived classes
    virtual bool OnInitialize() = 0;
    virtual void OnShutdown() = 0;
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() = 0;
};

// ==================== Template Implementations ====================

template<typename T>
void IEventBus::Subscribe(const EventHandler<T>& handler, const std::string& subscriberId) {
    auto wrapper = [handler](const IEvent& event) {
        handler(static_cast<const T&>(event));
    };
    SubscribeImpl(typeid(T), wrapper, subscriberId);
}

template<typename T>
void IEventBus::Unsubscribe(const std::string& subscriberId) {
    UnsubscribeImpl(typeid(T), subscriberId);
}

template<typename T>
void IEventBus::Publish(const T& event) {
    PublishImpl(event);
}

template<typename T>
void IDataProvider::SetData(const std::string& key, const T& data) {
    auto sharedData = std::make_shared<T>(data);
    SetDataImpl(key, std::static_pointer_cast<void>(sharedData), typeid(T));
}

template<typename T>
T IDataProvider::GetData(const std::string& key, const T& defaultValue) const {
    auto data = GetDataImpl(key, typeid(T));
    if (data) {
        return *static_cast<T*>(data.get());
    }
    return defaultValue;
}

} // namespace NSys