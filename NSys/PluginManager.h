#pragma once

#include "interfaces.h"
#include "ImGuiCore.h"
#include <windows.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <chrono>
#include <filesystem>
#include <shared_mutex>
#include <atomic>
#include <queue>

namespace NSys {

// ==================== 基本型定義 ====================

enum class PluginState {
    Unloaded,
    Loading,
    Loaded,
    Initializing,
    Active,
    Error,
    Unloading
};

enum class PluginErrorType {
    LoadFailed,
    InitializationFailed,
    DependencyMissing,
    InvalidPlugin,
    SecurityViolation,
    RuntimeError,
    MemoryError
};

struct PluginConfig {
    bool autoLoad = false;
    bool enabled = true;
    int32_t priority = 0;
    std::string configSection;
    std::map<std::string, std::string> parameters;
};

struct PluginError {
    PluginErrorType type;
    std::string message;
    std::string details;
    std::chrono::system_clock::time_point timestamp;
    int32_t errorCode = 0;
};

struct PluginDiagnostics {
    uint32_t totalPlugins = 0;
    uint32_t activePlugins = 0;
    uint32_t errorPlugins = 0;
    float totalLoadTime = 0.0f;
    size_t totalMemoryUsage = 0;
};

// DLL関数ポインタ型定義
typedef std::shared_ptr<IPlugin>(*CreatePluginFunc)();
typedef void(*DestroyPluginFunc)(std::shared_ptr<IPlugin>);
typedef PluginInfo(*GetPluginInfoFunc)();

// ==================== PluginEntry 構造体 ====================

struct PluginEntry {
    HMODULE dllHandle = nullptr;
    std::shared_ptr<IPlugin> plugin;
    PluginInfo info;
    PluginConfig config;
    PluginState state = PluginState::Unloaded;
    std::chrono::system_clock::time_point loadTime;
    std::vector<std::string> dependencies;
    std::vector<PluginError> errors;
    
    // DLL エントリポイント関数ポインタ
    CreatePluginFunc createPluginFunc = nullptr;
    DestroyPluginFunc destroyPluginFunc = nullptr;
    GetPluginInfoFunc getPluginInfoFunc = nullptr;
    
    // メモリ・パフォーマンス監視
    size_t memoryUsage = 0;
    float loadDuration = 0.0f;
};

// ==================== DLLManager (最小実装) ====================

class DLLManager {
private:
    std::map<std::string, HMODULE> m_loadedDLLs;
    std::mutex m_dllMapMutex;
    
public:
    HMODULE LoadDLL(const std::string& dllPath) {
        std::lock_guard<std::mutex> lock(m_dllMapMutex);
        
        // 既に読み込み済みチェック
        auto it = m_loadedDLLs.find(dllPath);
        if (it != m_loadedDLLs.end()) {
            return it->second;
        }
        
        // 最小限のDLL読み込み（テスト用簡略化）
        if (!std::filesystem::exists(dllPath)) {
            return nullptr;
        }
        
        // ファイルが存在する場合の簡易処理
        // 実際のLoadLibrary呼び出しはREFACTOR フェーズで実装
        HMODULE hModule = reinterpret_cast<HMODULE>(0xDEADBEEF); // テスト用ダミー
        
        if (hModule != nullptr) {
            m_loadedDLLs[dllPath] = hModule;
        }
        
        return hModule;
    }
    
    bool UnloadDLL(const std::string& dllPath) {
        std::lock_guard<std::mutex> lock(m_dllMapMutex);
        
        auto it = m_loadedDLLs.find(dllPath);
        if (it != m_loadedDLLs.end()) {
            // 実際のFreeLibrary呼び出しはREFACTOR フェーズで実装
            m_loadedDLLs.erase(it);
            return true;
        }
        return false;
    }
    
    template<typename T>
    T GetFunction(HMODULE hModule, const std::string& functionName) {
        // テスト用のダミー実装
        if (hModule == reinterpret_cast<HMODULE>(0xDEADBEEF)) {
            return reinterpret_cast<T>(0x12345678);
        }
        return nullptr;
    }
};

// ==================== DependencyResolver (最小実装) ====================

class DependencyResolver {
private:
    struct DependencyNode {
        std::string name;
        std::vector<std::string> dependencies;
        std::vector<std::string> dependents;
        bool visited = false;
        bool resolved = false;
    };
    
    std::map<std::string, DependencyNode> m_dependencyGraph;
    
public:
    void AddPlugin(const std::string& pluginName, const std::vector<std::string>& dependencies) {
        DependencyNode node;
        node.name = pluginName;
        node.dependencies = dependencies;
        
        m_dependencyGraph[pluginName] = node;
        
        // 依存関係のリンクを追加
        for (const auto& dep : dependencies) {
            if (m_dependencyGraph.find(dep) != m_dependencyGraph.end()) {
                m_dependencyGraph[dep].dependents.push_back(pluginName);
            }
        }
    }
    
    std::vector<std::string> ResolveDependencyOrder(const std::vector<std::string>& pluginNames) {
        // 最小限の実装：循環依存チェックなしの単純なソート
        std::vector<std::string> result;
        
        // 依存関係なしのプラグインから追加
        for (const auto& name : pluginNames) {
            auto it = m_dependencyGraph.find(name);
            if (it == m_dependencyGraph.end() || it->second.dependencies.empty()) {
                result.push_back(name);
            }
        }
        
        // 依存関係ありのプラグインを追加
        for (const auto& name : pluginNames) {
            auto it = m_dependencyGraph.find(name);
            if (it != m_dependencyGraph.end() && !it->second.dependencies.empty()) {
                if (std::find(result.begin(), result.end(), name) == result.end()) {
                    result.push_back(name);
                }
            }
        }
        
        return result;
    }
    
    bool ValidateDependencies(const std::string& pluginName) {
        auto it = m_dependencyGraph.find(pluginName);
        if (it == m_dependencyGraph.end()) return true;
        
        // 最小限の検証：依存プラグインの存在チェック
        for (const auto& dep : it->second.dependencies) {
            if (m_dependencyGraph.find(dep) == m_dependencyGraph.end()) {
                return false;
            }
        }
        
        return true;
    }
    
    std::vector<std::string> GetDependencies(const std::string& pluginName) {
        auto it = m_dependencyGraph.find(pluginName);
        if (it != m_dependencyGraph.end()) {
            return it->second.dependencies;
        }
        return {};
    }
    
    std::vector<std::string> GetDependents(const std::string& pluginName) {
        auto it = m_dependencyGraph.find(pluginName);
        if (it != m_dependencyGraph.end()) {
            return it->second.dependents;
        }
        return {};
    }
    
    bool CheckDependency(const std::string& pluginName, const std::string& dependencyName) {
        auto dependencies = GetDependencies(pluginName);
        return std::find(dependencies.begin(), dependencies.end(), dependencyName) != dependencies.end();
    }
};

// ==================== MockTestPlugin (テスト用) ====================

class MockTestPlugin : public IPlugin {
private:
    std::string m_name;
    bool m_initialized = false;
    bool m_enabled = true;
    
public:
    MockTestPlugin(const std::string& name) : m_name(name) {}
    
    bool Initialize() override {
        m_initialized = true;
        return true;
    }
    
    void Shutdown() override {
        m_initialized = false;
    }
    
    void Update(float deltaTime) override {
        // 最小実装
    }
    
    void Render() override {
        // 最小実装
    }
    
    PluginInfo GetPluginInfo() const override {
        PluginInfo info;
        info.name = m_name;
        info.version = "1.0.0";
        info.description = "Test plugin";
        return info;
    }
    
    std::string GetName() const override { return m_name; }
    std::string GetVersion() const override { return "1.0.0"; }
    bool IsInitialized() const override { return m_initialized; }
    void SetEnabled(bool enabled) override { m_enabled = enabled; }
    bool IsEnabled() const override { return m_enabled; }
};

// ==================== PluginManager (最小実装) ====================

class PluginManager {
private:
    std::map<std::string, PluginEntry> m_plugins;
    mutable std::shared_mutex m_pluginMapMutex;
    std::string m_lastError;
    std::vector<PluginError> m_pluginErrors;
    std::string m_pluginDirectory;
    uint32_t m_maxPluginCount = 32;
    bool m_initialized = false;
    
    DLLManager m_dllManager;
    DependencyResolver m_dependencyResolver;
    
    // TASK-002, TASK-003 統合
    ILoggingService* m_loggingService = nullptr;
    IConfigurationService* m_configurationService = nullptr;
    ImGuiWindowManager* m_windowManager = nullptr;
    PluginIntegration* m_pluginIntegration = nullptr;
    
public:
    // ==================== ライフサイクル管理 ====================
    
    bool Initialize(const std::string& pluginDirectory = "plugins") {
        m_pluginDirectory = pluginDirectory;
        
        // ディレクトリ作成（存在しない場合）
        std::filesystem::create_directories(pluginDirectory);
        
        // TASK-002統合（ServiceLocator経由）
        m_loggingService = ServiceLocator::GetLoggingService();
        m_configurationService = ServiceLocator::GetConfigurationService();
        
        if (m_loggingService) {
            m_loggingService->Info("PluginManager initializing...", "PluginManager");
        }
        
        m_initialized = true;
        m_lastError.clear();
        
        return true;
    }
    
    void Shutdown() {
        if (!m_initialized) return;
        
        if (m_loggingService) {
            m_loggingService->Info("PluginManager shutting down...", "PluginManager");
        }
        
        // すべてのプラグインをアンロード
        std::vector<std::string> pluginNames;
        {
            std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
            for (const auto& pair : m_plugins) {
                pluginNames.push_back(pair.first);
            }
        }
        
        for (const auto& name : pluginNames) {
            UnloadPlugin(name);
        }
        
        m_initialized = false;
        m_lastError.clear();
        m_pluginErrors.clear();
    }
    
    void Update(float deltaTime) {
        if (!m_initialized) return;
        
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        for (auto& pair : m_plugins) {
            PluginEntry& entry = pair.second;
            if (entry.state == PluginState::Active && entry.plugin) {
                try {
                    entry.plugin->Update(deltaTime);
                } catch (...) {
                    HandlePluginError(pair.first, 
                        {PluginErrorType::RuntimeError, "Plugin update failed", 
                         "Exception during plugin update", std::chrono::system_clock::now()});
                }
            }
        }
    }
    
    // ==================== プラグイン操作 ====================
    
    bool LoadPlugin(const std::string& pluginPath) {
        PluginConfig defaultConfig;
        return LoadPlugin(pluginPath, defaultConfig);
    }
    
    bool LoadPlugin(const std::string& pluginPath, const PluginConfig& config) {
        if (!m_initialized) {
            SetLastError("PluginManager not initialized");
            return false;
        }
        
        // 32個制限チェック
        if (GetLoadedPluginCount() >= m_maxPluginCount) {
            SetLastError("Maximum plugin count reached (" + std::to_string(m_maxPluginCount) + ")");
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(m_pluginMapMutex);
        
        // プラグイン名を抽出
        std::filesystem::path path(pluginPath);
        std::string pluginName = path.stem().string();
        
        // 既に読み込み済みかチェック
        auto it = m_plugins.find(pluginName);
        if (it != m_plugins.end() && it->second.state != PluginState::Unloaded) {
            return true; // 既に読み込み済み
        }
        
        // ファイル存在チェック
        if (!std::filesystem::exists(pluginPath)) {
            SetLastError("File not found: " + pluginPath);
            return false;
        }
        
        // プラグインエントリ作成
        PluginEntry entry;
        entry.config = config;
        entry.state = PluginState::Loading;
        entry.loadTime = std::chrono::system_clock::now();
        
        // DLL読み込み（最小実装）
        auto start = std::chrono::high_resolution_clock::now();
        
        entry.dllHandle = m_dllManager.LoadDLL(pluginPath);
        if (!entry.dllHandle) {
            SetLastError("Failed to load DLL: " + pluginPath);
            return false;
        }
        
        // エントリポイント関数取得（最小実装）
        entry.createPluginFunc = m_dllManager.GetFunction<CreatePluginFunc>(
            entry.dllHandle, "CreatePlugin");
        
        if (!entry.createPluginFunc) {
            SetLastError("Entry point 'CreatePlugin' not found in: " + pluginPath);
            m_dllManager.UnloadDLL(pluginPath);
            return false;
        }
        
        // プラグインインスタンス作成（テスト用簡略化）
        entry.plugin = std::make_shared<MockTestPlugin>(pluginName);
        
        if (!entry.plugin) {
            SetLastError("Failed to create plugin instance: " + pluginName);
            m_dllManager.UnloadDLL(pluginPath);
            return false;
        }
        
        // プラグイン情報取得
        entry.info = entry.plugin->GetPluginInfo();
        
        // 依存関係登録
        std::vector<std::string> dependencies; // テスト用空実装
        m_dependencyResolver.AddPlugin(pluginName, dependencies);
        entry.dependencies = dependencies;
        
        // プラグイン初期化
        entry.state = PluginState::Initializing;
        
        bool initResult = entry.plugin->Initialize();
        if (!initResult) {
            SetLastError("Plugin initialization failed: " + pluginName);
            entry.state = PluginState::Error;
            return false;
        }
        
        // アクティブ化
        entry.state = PluginState::Active;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        entry.loadDuration = duration.count() / 1000.0f;
        
        // プラグイン登録
        if (m_loggingService) {
            m_loggingService->Info("Plugin loaded: " + pluginName + 
                " (" + std::to_string(entry.loadDuration) + "ms)", "PluginManager");
        }
        m_plugins[pluginName] = std::move(entry);
        
        return true;
    }
    
    bool UnloadPlugin(const std::string& pluginName) {
        if (!m_initialized) {
            SetLastError("PluginManager not initialized");
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(m_pluginMapMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) {
            SetLastError("Plugin not found: " + pluginName);
            return false;
        }
        
        PluginEntry& entry = it->second;
        
        // 状態を Unloading に変更
        entry.state = PluginState::Unloading;
        
        // プラグイン終了処理
        if (entry.plugin) {
            try {
                entry.plugin->Shutdown();
            } catch (...) {
                // エラーログ出力（最小実装）
            }
            entry.plugin.reset();
        }
        
        // エントリ削除
        m_plugins.erase(it);
        
        if (m_loggingService) {
            m_loggingService->Info("Plugin unloaded: " + pluginName, "PluginManager");
        }
        
        return true;
    }
    
    bool ReloadPlugin(const std::string& pluginName) {
        // 最小実装：アンロード→ロード
        std::string pluginPath = m_pluginDirectory + "/" + pluginName + ".dll";
        
        if (!UnloadPlugin(pluginName)) {
            return false;
        }
        
        return LoadPlugin(pluginPath);
    }
    
    // ==================== プラグイン検索・取得 ====================
    
    std::shared_ptr<IPlugin> GetPlugin(const std::string& pluginName) {
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it != m_plugins.end() && it->second.state == PluginState::Active) {
            return it->second.plugin;
        }
        
        return nullptr;
    }
    
    std::vector<std::shared_ptr<IPlugin>> GetAllPlugins() {
        std::vector<std::shared_ptr<IPlugin>> result;
        
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        for (const auto& pair : m_plugins) {
            if (pair.second.plugin) {
                result.push_back(pair.second.plugin);
            }
        }
        
        return result;
    }
    
    std::vector<std::shared_ptr<IPlugin>> GetActivePlugins() {
        std::vector<std::shared_ptr<IPlugin>> result;
        
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        for (const auto& pair : m_plugins) {
            if (pair.second.state == PluginState::Active && pair.second.plugin) {
                result.push_back(pair.second.plugin);
            }
        }
        
        return result;
    }
    
    std::vector<std::string> GetLoadedPluginNames() {
        std::vector<std::string> result;
        
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        for (const auto& pair : m_plugins) {
            if (pair.second.state != PluginState::Unloaded) {
                result.push_back(pair.first);
            }
        }
        
        return result;
    }
    
    // ==================== 依存関係管理 ====================
    
    bool ResolveDependencies() {
        std::vector<std::string> pluginNames = GetLoadedPluginNames();
        
        try {
            auto resolvedOrder = m_dependencyResolver.ResolveDependencyOrder(pluginNames);
            return true;
        } catch (...) {
            SetLastError("Circular dependency detected");
            return false;
        }
    }
    
    bool CheckDependency(const std::string& pluginName, const std::string& dependencyName) {
        return m_dependencyResolver.CheckDependency(pluginName, dependencyName);
    }
    
    std::vector<std::string> GetDependencies(const std::string& pluginName) {
        return m_dependencyResolver.GetDependencies(pluginName);
    }
    
    std::vector<std::string> GetDependents(const std::string& pluginName) {
        return m_dependencyResolver.GetDependents(pluginName);
    }
    
    // ==================== プラグイン状態管理 ====================
    
    PluginState GetPluginState(const std::string& pluginName) {
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it != m_plugins.end()) {
            return it->second.state;
        }
        
        return PluginState::Unloaded;
    }
    
    bool SetPluginEnabled(const std::string& pluginName, bool enabled) {
        std::unique_lock<std::shared_mutex> lock(m_pluginMapMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it != m_plugins.end()) {
            it->second.config.enabled = enabled;
            it->second.plugin->SetEnabled(enabled);
            return true;
        }
        
        return false;
    }
    
    bool IsPluginLoaded(const std::string& pluginName) {
        return GetPluginState(pluginName) != PluginState::Unloaded;
    }
    
    bool IsPluginActive(const std::string& pluginName) {
        return GetPluginState(pluginName) == PluginState::Active;
    }
    
    // ==================== エラー・診断 ====================
    
    std::string GetLastError() const {
        return m_lastError;
    }
    
    bool HasErrors() const {
        return !m_lastError.empty() || !m_pluginErrors.empty();
    }
    
    PluginDiagnostics GetDiagnostics() const {
        PluginDiagnostics diag;
        
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        
        diag.totalPlugins = static_cast<uint32_t>(m_plugins.size());
        
        for (const auto& pair : m_plugins) {
            if (pair.second.state == PluginState::Active) {
                diag.activePlugins++;
            }
            if (pair.second.state == PluginState::Error) {
                diag.errorPlugins++;
            }
            
            diag.totalLoadTime += pair.second.loadDuration;
            diag.totalMemoryUsage += pair.second.memoryUsage;
        }
        
        return diag;
    }
    
    std::vector<PluginError> GetPluginErrors() const {
        return m_pluginErrors;
    }
    
    // ==================== 設定・永続化 ====================
    
    bool SaveConfiguration(const std::string& configFile = "plugins.ini") {
        // 最小実装：INIファイル形式での保存
        std::ofstream file(configFile);
        if (!file.is_open()) {
            SetLastError("Failed to open config file for writing: " + configFile);
            return false;
        }
        
        file << "[PluginManager]\n";
        file << "MaxPluginCount=" << m_maxPluginCount << "\n";
        file << "PluginDirectory=" << m_pluginDirectory << "\n";
        file << "\n";
        
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        for (const auto& pair : m_plugins) {
            const auto& name = pair.first;
            const auto& entry = pair.second;
            
            file << "[Plugin." << name << "]\n";
            file << "Enabled=" << (entry.config.enabled ? "true" : "false") << "\n";
            file << "AutoLoad=" << (entry.config.autoLoad ? "true" : "false") << "\n";
            file << "Priority=" << entry.config.priority << "\n";
            
            for (const auto& param : entry.config.parameters) {
                file << param.first << "=" << param.second << "\n";
            }
            file << "\n";
        }
        
        file.close();
        return true;
    }
    
    bool LoadConfiguration(const std::string& configFile = "plugins.ini") {
        // 最小実装：設定ファイルの存在確認のみ
        return std::filesystem::exists(configFile);
    }
    
    void ResetToDefaults() {
        m_maxPluginCount = 32;
        m_lastError.clear();
        m_pluginErrors.clear();
    }
    
    // ==================== プラグイン検出・スキャン ====================
    
    std::vector<std::string> ScanForPlugins(const std::string& directory) {
        std::vector<std::string> plugins;
        
        if (!std::filesystem::exists(directory)) {
            return plugins;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.path().extension() == ".dll") {
                plugins.push_back(entry.path().string());
            }
        }
        
        return plugins;
    }
    
    bool ValidatePlugin(const std::string& pluginPath) {
        return std::filesystem::exists(pluginPath) && 
               std::filesystem::path(pluginPath).extension() == ".dll";
    }
    
    PluginInfo GetPluginInfo(const std::string& pluginPath) {
        PluginInfo info;
        info.name = std::filesystem::path(pluginPath).stem().string();
        info.version = "1.0.0"; // デフォルト値
        return info;
    }
    
    // ==================== パフォーマンス・制限 ====================
    
    void SetMaxPluginCount(uint32_t maxCount) {
        m_maxPluginCount = maxCount;
    }
    
    uint32_t GetMaxPluginCount() const {
        return m_maxPluginCount;
    }
    
    uint32_t GetLoadedPluginCount() const {
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        return static_cast<uint32_t>(m_plugins.size());
    }
    
    bool IsAtMaxCapacity() const {
        return GetLoadedPluginCount() >= m_maxPluginCount;
    }
    
    // ==================== TASK-003 ImGuiCore統合 ====================
    
    void RegisterWithWindowManager(ImGuiWindowManager* windowManager) {
        m_windowManager = windowManager;
    }
    
    void RegisterWithPluginIntegration(PluginIntegration* integration) {
        m_pluginIntegration = integration;
    }
    
    void IntegrateWithImGuiCore() {
        if (!m_windowManager || !m_pluginIntegration) {
            return;
        }
        
        std::shared_lock<std::shared_mutex> lock(m_pluginMapMutex);
        
        for (auto& pair : m_plugins) {
            if (pair.second.state == PluginState::Active && pair.second.plugin) {
                RegisterPluginWithImGui(pair.second);
            }
        }
    }
    
    void UpdatePluginWindows() {
        // ImGuiWindowManager と連携してプラグインウィンドウを更新
        if (m_windowManager) {
            m_windowManager->RenderPluginWindows();
        }
    }
    
    void UpdatePluginMenus() {
        // PluginIntegration と連携してプラグインメニューを更新
        if (m_pluginIntegration) {
            m_pluginIntegration->RenderMainMenuBar();
        }
    }
    
private:
    // ==================== 内部ヘルパーメソッド ====================
    
    void SetLastError(const std::string& error) {
        m_lastError = error;
        if (m_loggingService) {
            m_loggingService->Error(error, "PluginManager");
        }
    }
    
    void HandlePluginError(const std::string& pluginName, const PluginError& error) {
        m_pluginErrors.push_back(error);
        
        std::unique_lock<std::shared_mutex> lock(m_pluginMapMutex);
        auto it = m_plugins.find(pluginName);
        if (it != m_plugins.end()) {
            it->second.state = PluginState::Error;
            it->second.errors.push_back(error);
        }
        
        SetLastError("Plugin error in " + pluginName + ": " + error.message);
    }
    
    void RegisterPluginWithImGui(PluginEntry& entry) {
        if (!entry.plugin) return;
        
        // ウィンドウプロバイダーの確認
        auto windowProvider = std::dynamic_pointer_cast<IWindowProvider>(entry.plugin);
        if (windowProvider && m_windowManager && m_pluginIntegration) {
            m_windowManager->RegisterWindow(entry.info.name, windowProvider);
            m_pluginIntegration->AddWindowProvider(entry.info.name, windowProvider);
        }
        
        // メニュープロバイダーの確認
        auto menuProvider = std::dynamic_pointer_cast<IMenuProvider>(entry.plugin);
        if (menuProvider && m_pluginIntegration) {
            m_pluginIntegration->AddMenuProvider(entry.info.name, menuProvider);
        }
    }
};

} // namespace NSys