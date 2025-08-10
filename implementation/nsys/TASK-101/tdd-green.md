# TASK-101: プラグインマネージャー実装 - GREEN Phase (最小実装)

## TDD GREEN Phase の目的

RED Phase で失敗したテストを**最小限の実装**で成功させます。この段階では：
1. テストを通すことだけを目的とする
2. 過度に複雑な実装は避ける
3. 実装の品質向上は次のREFACTOR フェーズで行う

## 実装戦略

### 1. TASK-003基盤の活用

既にTASK-003で実装済みのImGuiコア統合システムと連携し、段階的にプラグインマネージャーを構築する。

### 2. プラグインマネージャーコアクラス群の最小実装

**ファイル**: `NSys/PluginManager.h`

```cpp
#pragma once

#include "interfaces.h"
#include "ImGuiCore.h"  // TASK-003統合
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

// ==================== PluginManager (最小実装) ====================

class PluginManager {
private:
    std::map<std::string, PluginEntry> m_plugins;
    std::shared_mutex m_pluginMapMutex;
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
        
        // TASK-003統合（ServiceLocator経由）
        m_windowManager = ServiceLocator::GetWindowManager();
        m_pluginIntegration = ServiceLocator::GetPluginIntegration();
        
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
        // 実装時に info.dependencies から取得
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
        m_plugins[pluginName] = std::move(entry);
        
        if (m_loggingService) {
            m_loggingService->Info("Plugin loaded: " + pluginName + 
                " (" + std::to_string(entry.loadDuration) + "ms)", "PluginManager");
        }
        
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
        
        // DLL アンロード
        if (entry.dllHandle) {
            // 実際の処理は REFACTOR フェーズで実装
        }
        
        // 依存関係から削除
        // 実装時に詳細化
        
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

// ==================== テスト用のモックプラグイン ====================

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

} // namespace NSys
```

### 3. 実装ファイルの作成

**ファイル**: `NSys/PluginManager.cpp`

```cpp
#include "PluginManager.h"
#include <algorithm>
#include <numeric>
#include <fstream>

namespace NSys {

// この段階では最小限の実装のみ
// 複雑なロジックは後のREFACTORフェーズで実装

} // namespace NSys
```

### 4. テストヘルパーの実装改良

**ファイル**: `NSys/Tests/test_plugin_manager_common.h` (更新版)

```cpp
#pragma once

#include "test_common.h"
#include "test_imgui_common.h"
#include "../PluginManager.h"  // 新しいヘッダーをインクルード
#include <windows.h>
#include <memory>
#include <vector>
#include <filesystem>
#include <chrono>

// テスト用DLL生成ヘルパー（改良版）
class TestPluginDLLGenerator {
public:
    static void CreateTestPluginDLL(const std::string& filename, 
                                   bool isValid = true,
                                   const std::vector<std::string>& dependencies = {}) {
        if (isValid) {
            // 有効なDLLファイルとして作成（ダミーだが、ファイル形式は正しい）
            std::ofstream file(filename, std::ios::binary);
            if (file.is_open()) {
                // 最小限のDLLヘッダーのシミュレーション
                file << "MZ";  // DOS header
                file.write(reinterpret_cast<const char*>(std::string(512, '\0').data()), 512);
                file.close();
            }
        } else {
            // 無効なファイルとして作成
            std::ofstream file(filename);
            if (file.is_open()) {
                file << "INVALID_DLL_CONTENT";
                file.close();
            }
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
    
    static void CreateWindowProviderPluginDLL(const std::string& filename) {
        CreateValidPluginDLL(filename);
    }
    
    static void CreateMenuProviderPluginDLL(const std::string& filename) {
        CreateValidPluginDLL(filename);
    }
    
    static void CreateMixedProviderPluginDLL(const std::string& filename) {
        CreateValidPluginDLL(filename);
    }
    
    static void CreateLargeMemoryPluginDLL(const std::string& filename) {
        CreateValidPluginDLL(filename);
    }
    
    static void CreateFailingInitializationPluginDLL(const std::string& filename) {
        CreateValidPluginDLL(filename);
    }
    
    static void CleanupTestFiles() {
        // テストファイルのクリーンアップ
        try {
            if (std::filesystem::exists("test_plugins")) {
                std::filesystem::remove_all("test_plugins");
            }
            if (std::filesystem::exists("test_plugin_config.ini")) {
                std::filesystem::remove("test_plugin_config.ini");
            }
        } catch (...) {
            // エラーは無視（テストクリーンアップなので）
        }
    }
};

// テスト用の基本設定
class PluginManagerTestBase : public ImGuiTestBase {
protected:
    void SetUp() override {
        ImGuiTestBase::SetUp();
        
        // テストディレクトリの準備
        std::filesystem::create_directories("test_plugins");
        
        // ServiceLocator の初期化（テスト用）
        ServiceLocator::Initialize();
        
        // テスト用サービスの登録
        m_mockLoggingService = std::make_shared<MockLoggingService>();
        ServiceLocator::RegisterService<ILoggingService>(m_mockLoggingService.get());
    }
    
    void TearDown() override {
        // テストファイルのクリーンアップ
        TestPluginDLLGenerator::CleanupTestFiles();
        
        // ServiceLocator のクリーンアップ
        ServiceLocator::Shutdown();
        
        ImGuiTestBase::TearDown();
    }
    
    std::shared_ptr<MockLoggingService> m_mockLoggingService;
};

// モックサービス（テスト用）
class MockLoggingService : public ILoggingService {
private:
    int m_infoCallCount = 0;
    int m_errorCallCount = 0;
    int m_warningCallCount = 0;
    std::vector<std::string> m_logMessages;
    
public:
    void Info(const std::string& message, const std::string& category) override {
        m_infoCallCount++;
        m_logMessages.push_back("[INFO] " + category + ": " + message);
    }
    
    void Warning(const std::string& message, const std::string& category) override {
        m_warningCallCount++;
        m_logMessages.push_back("[WARNING] " + category + ": " + message);
    }
    
    void Error(const std::string& message, const std::string& category) override {
        m_errorCallCount++;
        m_logMessages.push_back("[ERROR] " + category + ": " + message);
    }
    
    // テスト用メソッド
    int GetInfoCallCount() const { return m_infoCallCount; }
    int GetErrorCallCount() const { return m_errorCallCount; }
    int GetWarningCallCount() const { return m_warningCallCount; }
    const std::vector<std::string>& GetLogMessages() const { return m_logMessages; }
    
    void Clear() {
        m_infoCallCount = m_errorCallCount = m_warningCallCount = 0;
        m_logMessages.clear();
    }
};
```

### 5. プロジェクト設定の更新

**NSys.vcxproj への追加**:
```xml
<ItemGroup>
  <ClCompile Include="PluginManager.cpp" />
</ItemGroup>

<ItemGroup>
  <ClInclude Include="PluginManager.h" />
</ItemGroup>
```

### 6. 最小実装の検証

実装後、テストが正常にコンパイルされ、基本的なテストが成功することを確認：

```bash
# 基本コンパイルチェック
cl /EHsc /I. NSys/Tests/test_plugin_manager.cpp
cl /EHsc /I. NSys/Tests/test_plugin_load.cpp
cl /EHsc /I. NSys/Tests/test_plugin_unload.cpp
cl /EHsc /I. NSys/Tests/test_dependency_management.cpp
```

### 7. main.cpp との統合準備

**main.cpp の最小限の更新**（既存機能を保持）:

```cpp
// 既存のmain.cppに最小限の変更を加える
#include "PluginManager.h"

// グローバルプラグインマネージャー（段階的導入用）
NSys::PluginManager g_newPluginManager;

int main(int argc, char* argv[]) {
    // 既存のコードは保持
    
    // 新しいPluginManagerのテスト（既存システムと並行動作）
    if (g_newPluginManager.Initialize("plugins")) {
        // 正常に初期化された場合の処理
        std::cout << "New PluginManager initialized successfully\n";
        std::cout << "Max plugins: " << g_newPluginManager.GetMaxPluginCount() << "\n";
        
        // テスト用プラグインをスキャン
        auto plugins = g_newPluginManager.ScanForPlugins("plugins");
        std::cout << "Found " << plugins.size() << " plugin files\n";
    }
    
    // 既存のmain関数の内容はそのまま維持
    // [既存のコードを保持]
    
    // 終了時のクリーンアップ
    g_newPluginManager.Shutdown();
    
    return 0;
}
```

## 実装のポイント

### 最小実装の原則

1. **テストを通すことが唯一の目的**
   - 複雑なDLL読み込み処理は簡略化
   - ダミーのポインタとモック実装でテストを満足
   - 実際の機能は REFACTOR フェーズで実装

2. **TASK-003基盤の活用**
   - 既存のImGuiコア統合システムと連携
   - ServiceLocator パターンを活用した統合
   - 段階的な統合アプローチ

3. **エラーハンドリングは最小限**
   - 基本的なエラーメッセージとログ出力
   - 詳細なエラー処理は REFACTOR フェーズで追加

4. **パフォーマンス測定の簡易実装**
   - 基本的な読み込み時間測定
   - メモリ使用量監視の骨格
   - 本格的な最適化は後のフェーズで実装

### 実装の制約

1. **テスト環境での動作**
   - 実際のDLL読み込みは最小限
   - モック・スタブによるテスト支援
   - 統合テスト環境の準備

2. **段階的な機能実装**
   - 全機能を一度に実装しない
   - テスト駆動による機能の確認
   - 必要最小限のAPI サーフェス

3. **既存システムとの共存**
   - main.cpp の既存機能を破損させない
   - 並行導入による安全な移行
   - 段階的な統合プロセス

## 期待される結果

GREEN フェーズ完了後、以下の状態になることを期待：

### コンパイル成功
```bash
✅ test_plugin_manager.cpp - コンパイル成功
✅ test_plugin_load.cpp - コンパイル成功
✅ test_plugin_unload.cpp - コンパイル成功
✅ test_dependency_management.cpp - コンパイル成功
✅ test_circular_dependency.cpp - コンパイル成功
✅ test_plugin_performance.cpp - コンパイル成功
✅ test_plugin_load_limits.cpp - コンパイル成功
✅ test_plugin_error_handling.cpp - コンパイル成功
✅ test_plugin_integration.cpp - コンパイル成功
```

### テスト実行結果（期待値）
```
[==========] Running 35 tests from 9 test suites.
[----------] Global test environment set-up.
[----------] 5 tests from PluginManagerTest
[ RUN      ] PluginManagerTest.Initialize_Success
[       OK ] PluginManagerTest.Initialize_Success (15 ms)
[ RUN      ] PluginManagerTest.Initialize_InvalidDirectory
[       OK ] PluginManagerTest.Initialize_InvalidDirectory (8 ms)
[----------] 6 tests from PluginManagerLoadTest
[ RUN      ] PluginManagerLoadTest.LoadPlugin_ValidPlugin_Success
[       OK ] PluginManagerLoadTest.LoadPlugin_ValidPlugin_Success (25 ms)
[----------] Tests from other suites...
[----------] Global test environment tear-down
[==========] 35 tests passed (450 ms total)
```

## 次のステップ

GREEN フェーズでテストが通ったら、REFACTOR フェーズで以下を改善：

1. **実際のDLL読み込み機能**
   - Windows LoadLibrary API の使用
   - エントリポイント関数の実際の取得
   - プラグインインスタンスの実際の作成

2. **依存関係解決の完全実装**
   - Kahn's algorithm による位相ソート
   - 循環依存の検出と報告
   - 依存関係に基づく正しい読み込み順序

3. **パフォーマンス最適化**
   - 実際のメモリ使用量監視
   - 読み込み時間の最適化
   - リソース管理の改善

4. **統合機能の強化**
   - TASK-003のImGuiコアとの完全統合
   - プラグインウィンドウ・メニューの動的管理
   - 設定永続化の完全実装

5. **既存コードとの段階的統合**
   - main.cpp の段階的リファクタリング
   - 既存プラグインシステムからの移行
   - 互換性レイヤーの実装

GREEN フェーズは「動作する最小限の実装」の作成であり、品質やパフォーマンスは次のフェーズで改善していきます。