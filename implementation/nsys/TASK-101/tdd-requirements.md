# TASK-101: プラグインマネージャー実装 - 要件定義

## タスク概要

NSys プラットフォームにおける高性能・高信頼性のプラグインマネージャーシステムを実装する。DLL動的読み込み・アンロード機能、プラグインライフサイクル管理、依存関係チェック機能を含む包括的なプラグイン管理システムを提供し、TASK-003のImGuiコア統合システムと完全に連携する。

## 詳細要件

### 1. PluginManager クラス実装 (REQ-002, PLUGIN-001)

**目的**: 拡張性とセキュリティを両立した高性能プラグイン管理システムを提供

**機能要件**:
- プラグインの動的読み込み・アンロード
- プラグインライフサイクルの完全管理
- プラグイン間依存関係の解決
- マルチスレッド対応の安全な管理
- プラグイン状態の監視・診断
- 設定ベースのプラグイン制御

**クラス仕様**:
```cpp
class PluginManager {
public:
    // ライフサイクル管理
    bool Initialize(const std::string& pluginDirectory = "plugins");
    void Shutdown();
    void Update(float deltaTime);
    
    // プラグイン操作
    bool LoadPlugin(const std::string& pluginPath);
    bool LoadPlugin(const std::string& pluginPath, const PluginConfig& config);
    bool UnloadPlugin(const std::string& pluginName);
    bool ReloadPlugin(const std::string& pluginName);
    
    // プラグイン検索・取得
    std::shared_ptr<IPlugin> GetPlugin(const std::string& pluginName);
    std::vector<std::shared_ptr<IPlugin>> GetAllPlugins();
    std::vector<std::shared_ptr<IPlugin>> GetActivePlugins();
    std::vector<std::string> GetLoadedPluginNames();
    
    // 依存関係管理
    bool ResolveDependencies();
    bool CheckDependency(const std::string& pluginName, const std::string& dependencyName);
    std::vector<std::string> GetDependencies(const std::string& pluginName);
    std::vector<std::string> GetDependents(const std::string& pluginName);
    
    // プラグイン状態管理
    PluginState GetPluginState(const std::string& pluginName);
    bool SetPluginEnabled(const std::string& pluginName, bool enabled);
    bool IsPluginLoaded(const std::string& pluginName);
    bool IsPluginActive(const std::string& pluginName);
    
    // エラー・診断
    std::string GetLastError() const;
    bool HasErrors() const;
    PluginDiagnostics GetDiagnostics() const;
    std::vector<PluginError> GetPluginErrors() const;
    
    // 設定・永続化
    bool SaveConfiguration(const std::string& configFile = "plugins.ini");
    bool LoadConfiguration(const std::string& configFile = "plugins.ini");
    void ResetToDefaults();
    
    // プラグイン検出・スキャン
    std::vector<std::string> ScanForPlugins(const std::string& directory);
    bool ValidatePlugin(const std::string& pluginPath);
    PluginInfo GetPluginInfo(const std::string& pluginPath);
    
    // イベント・通知
    void RegisterPluginEventHandler(IPluginEventHandler* handler);
    void UnregisterPluginEventHandler(IPluginEventHandler* handler);
    
    // パフォーマンス・制限
    void SetMaxPluginCount(uint32_t maxCount);
    uint32_t GetMaxPluginCount() const;
    uint32_t GetLoadedPluginCount() const;
    bool IsAtMaxCapacity() const;
    
    // TASK-003 ImGuiCore統合
    void RegisterWithWindowManager(ImGuiWindowManager* windowManager);
    void RegisterWithPluginIntegration(PluginIntegration* integration);
    void UpdatePluginWindows();
    void UpdatePluginMenus();
    
private:
    // 内部管理
    bool LoadPluginDLL(const std::string& pluginPath, PluginEntry& entry);
    bool UnloadPluginDLL(PluginEntry& entry);
    bool InitializePlugin(PluginEntry& entry);
    bool ShutdownPlugin(PluginEntry& entry);
    
    // 依存関係解決
    std::vector<std::string> ResolveDependencyOrder(const std::vector<std::string>& pluginNames);
    bool ValidateDependencyGraph();
    bool DetectCircularDependencies();
    
    // エラーハンドリング
    void HandlePluginError(const std::string& pluginName, const PluginError& error);
    void LogPluginEvent(const std::string& pluginName, PluginEventType eventType);
    
    // スレッド安全性
    mutable std::shared_mutex m_pluginMapMutex;
    mutable std::mutex m_errorMutex;
    std::atomic<bool> m_shutdownRequested{false};
};
```

### 2. DLL動的読み込み・アンロード機能 (PLUGIN-002)

**目的**: 安全で効率的なプラグインDLLの動的管理

**機能要件**:
- Windows DLLの安全な読み込み・アンロード
- プラグインエントリポイントの検証
- メモリリーク防止機能
- DLLハイジャック攻撃防止（基本レベル）
- 読み込み時間の最適化

**実装仕様**:
```cpp
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
};

enum class PluginState {
    Unloaded,
    Loading,
    Loaded,
    Initializing,
    Active,
    Error,
    Unloading
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

enum class PluginErrorType {
    LoadFailed,
    InitializationFailed,
    DependencyMissing,
    InvalidPlugin,
    SecurityViolation,
    RuntimeError,
    MemoryError
};
```

**DLL管理の実装**:
```cpp
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
        
        // 安全な読み込み（フルパス指定）
        std::wstring widePath = ConvertToWideString(dllPath);
        HMODULE hModule = LoadLibraryExW(widePath.c_str(), nullptr, 
                                        LOAD_LIBRARY_SEARCH_SYSTEM32 | 
                                        LOAD_LIBRARY_SEARCH_USER_DIRS);
        
        if (hModule != nullptr) {
            m_loadedDLLs[dllPath] = hModule;
        }
        
        return hModule;
    }
    
    bool UnloadDLL(const std::string& dllPath) {
        std::lock_guard<std::mutex> lock(m_dllMapMutex);
        
        auto it = m_loadedDLLs.find(dllPath);
        if (it != m_loadedDLLs.end()) {
            BOOL result = FreeLibrary(it->second);
            if (result) {
                m_loadedDLLs.erase(it);
                return true;
            }
        }
        return false;
    }
    
    template<typename T>
    T GetFunction(HMODULE hModule, const std::string& functionName) {
        return reinterpret_cast<T>(GetProcAddress(hModule, functionName.c_str()));
    }
};
```

### 3. プラグインライフサイクル管理 (PLUGIN-002)

**目的**: プラグインの生存期間全体を通じた状態管理

**ライフサイクル状態遷移**:
```
Unloaded → Loading → Loaded → Initializing → Active
                     ↓         ↓             ↓
                   Error ←——————————————————————
                     ↓
                 Unloading → Unloaded
```

**状態管理実装**:
```cpp
class PluginLifecycleManager {
private:
    std::map<std::string, PluginEntry> m_plugins;
    std::shared_mutex m_pluginsMutex;
    
public:
    bool TransitionToState(const std::string& pluginName, PluginState newState) {
        std::unique_lock<std::shared_mutex> lock(m_pluginsMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) return false;
        
        PluginEntry& entry = it->second;
        
        // 状態遷移の妥当性チェック
        if (!IsValidTransition(entry.state, newState)) {
            return false;
        }
        
        // 状態遷移処理
        bool success = PerformStateTransition(entry, newState);
        if (success) {
            entry.state = newState;
            NotifyStateChange(pluginName, newState);
        }
        
        return success;
    }
    
private:
    bool IsValidTransition(PluginState from, PluginState to) {
        switch (from) {
            case PluginState::Unloaded:
                return to == PluginState::Loading;
            case PluginState::Loading:
                return to == PluginState::Loaded || to == PluginState::Error;
            case PluginState::Loaded:
                return to == PluginState::Initializing || to == PluginState::Unloading;
            case PluginState::Initializing:
                return to == PluginState::Active || to == PluginState::Error;
            case PluginState::Active:
                return to == PluginState::Unloading || to == PluginState::Error;
            case PluginState::Error:
                return to == PluginState::Unloading;
            case PluginState::Unloading:
                return to == PluginState::Unloaded;
        }
        return false;
    }
    
    bool PerformStateTransition(PluginEntry& entry, PluginState newState) {
        switch (newState) {
            case PluginState::Loading:
                return LoadPlugin(entry);
            case PluginState::Loaded:
                return ValidatePlugin(entry);
            case PluginState::Initializing:
                return InitializePlugin(entry);
            case PluginState::Active:
                return ActivatePlugin(entry);
            case PluginState::Unloading:
                return UnloadPlugin(entry);
            case PluginState::Error:
                HandlePluginError(entry);
                return true;
        }
        return false;
    }
};
```

### 4. 依存関係チェック機能 (PLUGIN-003)

**目的**: プラグイン間の依存関係を適切に解決し、循環依存を防止

**依存関係解決アルゴリズム**:
```cpp
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
    std::vector<std::string> ResolveDependencyOrder(const std::vector<std::string>& pluginNames) {
        // Kahn's algorithm による位相ソート
        std::vector<std::string> result;
        std::queue<std::string> noIncomingEdge;
        std::map<std::string, int> inDegree;
        
        // グラフの初期化
        for (const auto& name : pluginNames) {
            inDegree[name] = 0;
        }
        
        // 入次数の計算
        for (const auto& name : pluginNames) {
            auto it = m_dependencyGraph.find(name);
            if (it != m_dependencyGraph.end()) {
                for (const auto& dep : it->second.dependencies) {
                    inDegree[dep]++;
                }
            }
        }
        
        // 入次数0のノードを初期キューに追加
        for (const auto& pair : inDegree) {
            if (pair.second == 0) {
                noIncomingEdge.push(pair.first);
            }
        }
        
        // 位相ソート実行
        while (!noIncomingEdge.empty()) {
            std::string current = noIncomingEdge.front();
            noIncomingEdge.pop();
            result.push_back(current);
            
            auto it = m_dependencyGraph.find(current);
            if (it != m_dependencyGraph.end()) {
                for (const auto& dependent : it->second.dependents) {
                    inDegree[dependent]--;
                    if (inDegree[dependent] == 0) {
                        noIncomingEdge.push(dependent);
                    }
                }
            }
        }
        
        // 循環依存チェック
        if (result.size() != pluginNames.size()) {
            throw CircularDependencyException("Circular dependency detected in plugin graph");
        }
        
        return result;
    }
    
    bool ValidateDependencies(const std::string& pluginName) {
        auto it = m_dependencyGraph.find(pluginName);
        if (it == m_dependencyGraph.end()) return true;
        
        for (const auto& dep : it->second.dependencies) {
            if (m_dependencyGraph.find(dep) == m_dependencyGraph.end()) {
                return false; // 依存プラグインが存在しない
            }
        }
        
        return true;
    }
};
```

### 5. パフォーマンス要件 (NFR-002)

**読み込み時間制限**:
- **要件**: プラグインの読み込み時間が3秒以内
- **測定方法**: LoadPlugin() 呼び出しから Active 状態まで
- **最適化手法**: 
  - 並列初期化（依存関係を考慮）
  - 遅延初期化オプション
  - プリロード機能

**メモリ効率**:
- **要件**: プラグインあたり100MB制限
- **実装**: メモリ使用量監視とアラート
- **統合**: TASK-203のメモリマネージャーとの連携

**プラグイン数制限**:
- **要件**: EDGE-101による32個制限
- **実装**: 硬制限とソフト制限の両対応
- **エラー処理**: 制限超過時の適切な警告

### 6. ServiceLocator統合 (TASK-002連携)

**統合要件**:
```cpp
class PluginManager {
private:
    // TASK-002のサービスとの統合
    ILoggingService* m_loggingService = nullptr;
    IConfigurationService* m_configurationService = nullptr;
    
    // TASK-003のImGuiコアとの統合
    ImGuiWindowManager* m_windowManager = nullptr;
    PluginIntegration* m_pluginIntegration = nullptr;
    
public:
    bool Initialize(const std::string& pluginDirectory = "plugins") {
        // ServiceLocatorからサービスを取得
        m_loggingService = ServiceLocator::GetLoggingService();
        m_configurationService = ServiceLocator::GetConfigurationService();
        
        // TASK-003統合
        m_windowManager = ServiceLocator::GetWindowManager();
        m_pluginIntegration = ServiceLocator::GetPluginIntegration();
        
        if (m_loggingService) {
            m_loggingService->Info("PluginManager initializing...", "PluginManager");
        }
        
        return InitializeInternal(pluginDirectory);
    }
    
    void IntegrateWithImGuiCore() {
        if (m_windowManager && m_pluginIntegration) {
            // プラグインウィンドウの統合
            for (auto& pair : m_plugins) {
                RegisterPluginWithImGui(pair.second);
            }
        }
    }
    
private:
    void RegisterPluginWithImGui(PluginEntry& entry) {
        if (entry.plugin && entry.state == PluginState::Active) {
            // ウィンドウプロバイダーの確認
            auto windowProvider = std::dynamic_pointer_cast<IWindowProvider>(entry.plugin);
            if (windowProvider) {
                m_windowManager->RegisterWindow(entry.info.name, windowProvider);
                m_pluginIntegration->AddWindowProvider(entry.info.name, windowProvider);
            }
            
            // メニュープロバイダーの確認
            auto menuProvider = std::dynamic_pointer_cast<IMenuProvider>(entry.plugin);
            if (menuProvider) {
                m_pluginIntegration->AddMenuProvider(entry.info.name, menuProvider);
            }
        }
    }
};
```

## 品質要件

### パフォーマンス要件
- **プラグイン読み込み時間**: 3秒以内（単一プラグイン）
- **依存関係解決時間**: 1秒以内（32個プラグイン）
- **メモリ使用量**: プラグインあたり100MB制限
- **CPU使用率**: プラグイン管理処理で5%以下
- **応答性**: Update()呼び出しで16ms以下

### 信頼性要件
- **プラグインクラッシュ耐性**: 単一プラグインの障害が全体に影響しない
- **メモリリーク防止**: プラグインアンロード時の完全なメモリ解放
- **リソース管理**: ファイルハンドル、スレッドなどの適切な管理
- **エラー回復**: 失敗したプラグインの自動無効化
- **状態整合性**: マルチスレッド環境でのデータ競合防止

### セキュリティ要件 (基本レベル)
- **DLLパス検証**: 相対パス攻撃の防止
- **エントリポイント検証**: 必須関数の存在確認
- **シンボル検証**: プラグインインターフェースの整合性確認
- **権限チェック**: プラグインディレクトリアクセス権限の確認

## 受け入れ基準

### 基本機能
- [ ] PluginManagerが正常に初期化される
- [ ] プラグインDLLの動的読み込みができる
- [ ] プラグインの初期化・アクティブ化ができる
- [ ] プラグインのアンロードが正常に動作する
- [ ] 複数プラグインの同時管理ができる

### 依存関係管理
- [ ] プラグイン間依存関係が正しく解決される
- [ ] 循環依存の検出・防止ができる
- [ ] 依存関係に基づく正しい読み込み順序が実現される
- [ ] 依存プラグインの欠如時に適切にエラーハンドリングされる

### パフォーマンステスト
- [ ] 単一プラグインの読み込みが3秒以内である
- [ ] 32個プラグイン同時読み込みが10秒以内である
- [ ] 依存関係解決処理が1秒以内である
- [ ] メモリ使用量がプラグインあたり100MB以下である
- [ ] プラグイン管理処理のCPU使用率が5%以下である

### エラーハンドリング
- [ ] 不正なDLLファイルが適切に拒否される
- [ ] プラグイン初期化失敗時に適切に処理される
- [ ] メモリ不足時にグレースフルに対応される
- [ ] プラグインクラッシュ時にアプリケーションが継続する
- [ ] 依存関係エラー時に詳細な情報が提供される

### 統合テスト
- [ ] TASK-003のImGuiWindowManagerとの統合が正常動作する
- [ ] ServiceLocatorとの統合が正常動作する
- [ ] 既存のプラグインシステムとの互換性が保たれる
- [ ] 設定ファイルの読み込み・保存が正常動作する
- [ ] プラグイン状態の永続化が正常動作する

## 実装制約

### 技術制約
- C++20 標準に準拠
- Windows 10/11 対応必須
- Visual Studio 2019/2022でコンパイル可能
- TASK-002のServiceLocatorとの統合必須
- TASK-003のImGuiコアとの統合必須

### 設計制約
- 既存のプラグインインターフェース (IPlugin, IMenuProvider, IWindowProvider) の完全互換性
- マルチスレッド安全性の確保
- RAII原則に従ったリソース管理
- 例外安全なコード実装
- 段階的な既存コード統合

### セキュリティ制約
- DLLハイジャック攻撃の基本的防止
- プラグインディレクトリアクセスの制限
- メモリ破損攻撃の基本的防止
- 権限昇格攻撃の防止

## 依存関係

### 内部依存
- TASK-002: ServiceLocator (ILoggingService, IConfigurationService)
- TASK-003: ImGuiコア統合 (ImGuiWindowManager, PluginIntegration)
- 既存のプラグインインターフェース (interfaces.h)

### 外部依存
- Windows SDK (DLL管理API)
- C++20 Standard Library (スレッド、コンテナ)
- 既存のプラグインDLLファイル群

## 実装順序

1. **PluginManager 基本クラス実装** - コアクラスの骨格
2. **DLLManager実装** - 安全なDLL読み込み・アンロード
3. **PluginLifecycleManager実装** - 状態管理システム
4. **DependencyResolver実装** - 依存関係解決システム
5. **ServiceLocator統合** - TASK-002との連携
6. **ImGuiコア統合** - TASK-003との連携
7. **設定・永続化機能** - 設定ファイル管理
8. **エラーハンドリング強化** - 包括的エラー処理
9. **パフォーマンス最適化** - 速度・メモリ最適化
10. **統合テスト・検証** - 全体動作確認

## 移行戦略

### 既存システムからの段階的移行
1. **Phase 1**: 新PluginManagerの並行導入（既存システム維持）
2. **Phase 2**: 新システムでのプラグイン管理開始（読み込みのみ）
3. **Phase 3**: ライフサイクル管理の移行
4. **Phase 4**: 依存関係管理の移行
5. **Phase 5**: 既存システムの段階的削除
6. **Phase 6**: 完全統合とクリーンアップ

この段階的アプローチにより、既存機能を破損させることなく、高品質な新プラグインマネージャーを安全に導入できます。