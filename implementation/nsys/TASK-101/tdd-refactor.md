# TASK-101: プラグインマネージャー実装 - REFACTOR Phase (コード品質向上)

## TDD REFACTOR Phase の目的

GREEN フェーズで動作する最小限の実装を作成した後、この段階では：
1. テストを壊さずにコードの品質を改善
2. 実際のWindows DLL読み込み機能の実装
3. 本格的な依存関係解決システムの構築
4. パフォーマンス最適化とセキュリティ強化

## 改善対象領域

### 1. 実際のDLL読み込み機能の実装

現在のテスト用ダミー実装を、実際のWindows API を使用した機能に置き換える。

**改善前** (テスト用ダミー):
```cpp
HMODULE LoadDLL(const std::string& dllPath) {
    // ファイルが存在する場合の簡易処理
    HMODULE hModule = reinterpret_cast<HMODULE>(0xDEADBEEF); // テスト用ダミー
    return hModule;
}
```

**改善後** (実機能):
```cpp
class DLLManager {
private:
    std::map<std::string, HMODULE> m_loadedDLLs;
    std::shared_mutex m_dllMapMutex;
    std::vector<std::wstring> m_searchPaths;
    
    // セキュリティ機能
    bool m_enableSecurityChecks = true;
    std::set<std::string> m_trustedPaths;
    
public:
    bool Initialize() {
        // DLL検索パスの初期化
        SetupSecureSearchPaths();
        
        // 信頼できるパスの設定
        SetupTrustedPaths();
        
        return true;
    }
    
    HMODULE LoadDLL(const std::string& dllPath) {
        std::unique_lock<std::shared_mutex> lock(m_dllMapMutex);
        
        // 既に読み込み済みチェック
        auto it = m_loadedDLLs.find(dllPath);
        if (it != m_loadedDLLs.end()) {
            return it->second;
        }
        
        // セキュリティ検証
        if (m_enableSecurityChecks) {
            if (!ValidatePluginSecurity(dllPath)) {
                return nullptr;
            }
        }
        
        // フルパスの取得・検証
        std::wstring fullPath = GetFullPath(dllPath);
        if (fullPath.empty()) {
            return nullptr;
        }
        
        // 安全なDLL読み込み
        HMODULE hModule = LoadLibraryExW(
            fullPath.c_str(),
            nullptr,
            LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32
        );
        
        if (hModule != nullptr) {
            m_loadedDLLs[dllPath] = hModule;
            
            // 読み込みログの記録
            LogDLLLoad(dllPath, hModule);
        }
        
        return hModule;
    }
    
    bool UnloadDLL(const std::string& dllPath) {
        std::unique_lock<std::shared_mutex> lock(m_dllMapMutex);
        
        auto it = m_loadedDLLs.find(dllPath);
        if (it != m_loadedDLLs.end()) {
            HMODULE hModule = it->second;
            
            // アンロード実行
            BOOL result = FreeLibrary(hModule);
            if (result) {
                m_loadedDLLs.erase(it);
                LogDLLUnload(dllPath);
                return true;
            }
        }
        return false;
    }
    
    template<typename T>
    T GetFunction(HMODULE hModule, const std::string& functionName) {
        if (!hModule) return nullptr;
        
        FARPROC proc = GetProcAddress(hModule, functionName.c_str());
        return reinterpret_cast<T>(proc);
    }
    
private:
    void SetupSecureSearchPaths() {
        // 現在のディレクトリを検索パスから除外（セキュリティ対策）
        SetDllDirectoryW(L"");
        
        // 検索パスの設定
        wchar_t systemPath[MAX_PATH];
        GetSystemDirectoryW(systemPath, MAX_PATH);
        m_searchPaths.push_back(systemPath);
        
        wchar_t windowsPath[MAX_PATH];
        GetWindowsDirectoryW(windowsPath, MAX_PATH);
        m_searchPaths.push_back(windowsPath);
    }
    
    void SetupTrustedPaths() {
        // アプリケーションディレクトリ
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::filesystem::path appDir = std::filesystem::path(exePath).parent_path();
        m_trustedPaths.insert(appDir.string());
        
        // プラグインディレクトリ
        m_trustedPaths.insert((appDir / "plugins").string());
    }
    
    bool ValidatePluginSecurity(const std::string& dllPath) {
        // パス検証
        std::filesystem::path path(dllPath);
        
        // 絶対パスへの変換
        std::filesystem::path absolutePath = std::filesystem::absolute(path);
        
        // 信頼できるパス内かチェック
        bool inTrustedPath = false;
        for (const auto& trustedPath : m_trustedPaths) {
            if (absolutePath.string().find(trustedPath) == 0) {
                inTrustedPath = true;
                break;
            }
        }
        
        if (!inTrustedPath) {
            return false;
        }
        
        // ファイル拡張子チェック
        if (path.extension() != ".dll") {
            return false;
        }
        
        // ファイル存在チェック
        if (!std::filesystem::exists(absolutePath)) {
            return false;
        }
        
        // ファイルサイズチェック（異常に大きなDLLを拒否）
        auto fileSize = std::filesystem::file_size(absolutePath);
        if (fileSize > 100 * 1024 * 1024) { // 100MB制限
            return false;
        }
        
        return true;
    }
    
    std::wstring GetFullPath(const std::string& dllPath) {
        std::wstring widePath = ConvertToWideString(dllPath);
        
        wchar_t fullPath[MAX_PATH];
        DWORD result = GetFullPathNameW(widePath.c_str(), MAX_PATH, fullPath, nullptr);
        
        if (result == 0 || result > MAX_PATH) {
            return L"";
        }
        
        return std::wstring(fullPath);
    }
    
    void LogDLLLoad(const std::string& dllPath, HMODULE hModule) {
        // ロギングサービス経由でDLL読み込みを記録
        if (auto loggingService = ServiceLocator::GetLoggingService()) {
            std::ostringstream oss;
            oss << "DLL loaded: " << dllPath << " (Handle: 0x" << std::hex << hModule << ")";
            loggingService->Info(oss.str(), "DLLManager");
        }
    }
    
    void LogDLLUnload(const std::string& dllPath) {
        if (auto loggingService = ServiceLocator::GetLoggingService()) {
            loggingService->Info("DLL unloaded: " + dllPath, "DLLManager");
        }
    }
    
    std::wstring ConvertToWideString(const std::string& str) {
        if (str.empty()) return L"";
        
        int wideSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        if (wideSize <= 0) return L"";
        
        std::wstring wideStr(wideSize, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wideStr[0], wideSize);
        wideStr.pop_back(); // null terminator を削除
        
        return wideStr;
    }
};
```

### 2. プラグインエントリポイント管理の改善

**改善後の実装**:
```cpp
class PluginEntryPointManager {
private:
    struct EntryPoints {
        CreatePluginFunc createPlugin = nullptr;
        DestroyPluginFunc destroyPlugin = nullptr;
        GetPluginInfoFunc getPluginInfo = nullptr;
        
        bool IsValid() const {
            return createPlugin && destroyPlugin && getPluginInfo;
        }
    };
    
public:
    bool LoadEntryPoints(HMODULE hModule, PluginEntry& entry) {
        if (!hModule) return false;
        
        EntryPoints points;
        
        // 必須エントリポイントの取得
        points.createPlugin = GetProcAddress(hModule, "CreatePlugin");
        points.destroyPlugin = GetProcAddress(hModule, "DestroyPlugin");  
        points.getPluginInfo = GetProcAddress(hModule, "GetPluginInfo");
        
        if (!points.IsValid()) {
            return false;
        }
        
        // プラグインエントリに設定
        entry.createPluginFunc = reinterpret_cast<CreatePluginFunc>(points.createPlugin);
        entry.destroyPluginFunc = reinterpret_cast<DestroyPluginFunc>(points.destroyPlugin);
        entry.getPluginInfoFunc = reinterpret_cast<GetPluginInfoFunc>(points.getPluginInfo);
        
        return true;
    }
    
    std::shared_ptr<IPlugin> CreatePluginInstance(PluginEntry& entry) {
        if (!entry.createPluginFunc) {
            return nullptr;
        }
        
        try {
            return entry.createPluginFunc();
        } catch (const std::exception& e) {
            // 例外ログの記録
            if (auto loggingService = ServiceLocator::GetLoggingService()) {
                loggingService->Error("Plugin creation failed: " + std::string(e.what()), 
                                    "PluginEntryPointManager");
            }
            return nullptr;
        } catch (...) {
            if (auto loggingService = ServiceLocator::GetLoggingService()) {
                loggingService->Error("Plugin creation failed: Unknown exception", 
                                    "PluginEntryPointManager");
            }
            return nullptr;
        }
    }
    
    void DestroyPluginInstance(PluginEntry& entry) {
        if (!entry.destroyPluginFunc || !entry.plugin) {
            return;
        }
        
        try {
            entry.destroyPluginFunc(entry.plugin);
        } catch (const std::exception& e) {
            if (auto loggingService = ServiceLocator::GetLoggingService()) {
                loggingService->Error("Plugin destruction failed: " + std::string(e.what()), 
                                    "PluginEntryPointManager");
            }
        } catch (...) {
            if (auto loggingService = ServiceLocator::GetLoggingService()) {
                loggingService->Error("Plugin destruction failed: Unknown exception", 
                                    "PluginEntryPointManager");
            }
        }
        
        entry.plugin.reset();
    }
    
    PluginInfo GetPluginInformation(PluginEntry& entry) {
        if (!entry.getPluginInfoFunc) {
            PluginInfo defaultInfo;
            defaultInfo.name = "Unknown";
            defaultInfo.version = "0.0.0";
            return defaultInfo;
        }
        
        try {
            return entry.getPluginInfoFunc();
        } catch (...) {
            PluginInfo errorInfo;
            errorInfo.name = "Error";
            errorInfo.version = "0.0.0";
            return errorInfo;
        }
    }
    
private:
    FARPROC GetProcAddress(HMODULE hModule, const char* procName) {
        return ::GetProcAddress(hModule, procName);
    }
};
```

### 3. 依存関係解決システムの完全実装

**改善後の実装**:
```cpp
class DependencyResolver {
private:
    struct DependencyNode {
        std::string name;
        std::vector<std::string> dependencies;
        std::vector<std::string> dependents;
        int inDegree = 0;
        bool visited = false;
        bool resolved = false;
    };
    
    std::map<std::string, DependencyNode> m_dependencyGraph;
    std::shared_mutex m_graphMutex;
    
public:
    void AddPlugin(const std::string& pluginName, const std::vector<std::string>& dependencies) {
        std::unique_lock<std::shared_mutex> lock(m_graphMutex);
        
        DependencyNode node;
        node.name = pluginName;
        node.dependencies = dependencies;
        node.inDegree = static_cast<int>(dependencies.size());
        
        m_dependencyGraph[pluginName] = node;
        
        // 逆方向のリンクを追加
        for (const auto& dep : dependencies) {
            m_dependencyGraph[dep].dependents.push_back(pluginName);
        }
        
        RecalculateInDegrees();
    }
    
    std::vector<std::string> ResolveDependencyOrder(const std::vector<std::string>& pluginNames) {
        std::shared_lock<std::shared_mutex> lock(m_graphMutex);
        
        // Kahn's algorithm による位相ソート
        std::vector<std::string> result;
        std::queue<std::string> noIncomingEdge;
        std::map<std::string, int> tempInDegree;
        
        // 要求されたプラグインのみを対象とする
        std::set<std::string> targetPlugins(pluginNames.begin(), pluginNames.end());
        
        // 入次数の初期化
        for (const auto& name : pluginNames) {
            auto it = m_dependencyGraph.find(name);
            if (it != m_dependencyGraph.end()) {
                tempInDegree[name] = CalculateInDegree(name, targetPlugins);
            } else {
                tempInDegree[name] = 0;
            }
            
            if (tempInDegree[name] == 0) {
                noIncomingEdge.push(name);
            }
        }
        
        // 位相ソート実行
        while (!noIncomingEdge.empty()) {
            std::string current = noIncomingEdge.front();
            noIncomingEdge.pop();
            result.push_back(current);
            
            // 依存関係を削除
            auto it = m_dependencyGraph.find(current);
            if (it != m_dependencyGraph.end()) {
                for (const auto& dependent : it->second.dependents) {
                    if (targetPlugins.count(dependent) > 0) {
                        tempInDegree[dependent]--;
                        if (tempInDegree[dependent] == 0) {
                            noIncomingEdge.push(dependent);
                        }
                    }
                }
            }
        }
        
        // 循環依存のチェック
        if (result.size() != pluginNames.size()) {
            throw CircularDependencyException(
                "Circular dependency detected in plugins: " + 
                GetRemainingPlugins(pluginNames, result)
            );
        }
        
        return result;
    }
    
    bool ValidateDependencies(const std::string& pluginName) {
        std::shared_lock<std::shared_mutex> lock(m_graphMutex);
        
        auto it = m_dependencyGraph.find(pluginName);
        if (it == m_dependencyGraph.end()) return true;
        
        // 依存プラグインの存在確認
        for (const auto& dep : it->second.dependencies) {
            if (m_dependencyGraph.find(dep) == m_dependencyGraph.end()) {
                return false;
            }
        }
        
        // 循環依存のチェック
        std::set<std::string> visited;
        std::set<std::string> recursionStack;
        
        return !HasCircularDependency(pluginName, visited, recursionStack);
    }
    
    std::vector<std::string> GetMissingDependencies(const std::string& pluginName) {
        std::vector<std::string> missing;
        
        std::shared_lock<std::shared_mutex> lock(m_graphMutex);
        
        auto it = m_dependencyGraph.find(pluginName);
        if (it != m_dependencyGraph.end()) {
            for (const auto& dep : it->second.dependencies) {
                if (m_dependencyGraph.find(dep) == m_dependencyGraph.end()) {
                    missing.push_back(dep);
                }
            }
        }
        
        return missing;
    }
    
    std::vector<std::string> DetectCircularDependencies() {
        std::shared_lock<std::shared_mutex> lock(m_graphMutex);
        
        std::set<std::string> visited;
        std::set<std::string> recursionStack;
        std::vector<std::string> circularPlugins;
        
        for (const auto& pair : m_dependencyGraph) {
            if (visited.find(pair.first) == visited.end()) {
                if (HasCircularDependency(pair.first, visited, recursionStack)) {
                    circularPlugins.push_back(pair.first);
                }
            }
        }
        
        return circularPlugins;
    }
    
private:
    void RecalculateInDegrees() {
        for (auto& pair : m_dependencyGraph) {
            pair.second.inDegree = static_cast<int>(pair.second.dependencies.size());
        }
    }
    
    int CalculateInDegree(const std::string& pluginName, const std::set<std::string>& targetPlugins) {
        auto it = m_dependencyGraph.find(pluginName);
        if (it == m_dependencyGraph.end()) return 0;
        
        int count = 0;
        for (const auto& dep : it->second.dependencies) {
            if (targetPlugins.count(dep) > 0) {
                count++;
            }
        }
        
        return count;
    }
    
    bool HasCircularDependency(const std::string& pluginName, 
                              std::set<std::string>& visited,
                              std::set<std::string>& recursionStack) {
        visited.insert(pluginName);
        recursionStack.insert(pluginName);
        
        auto it = m_dependencyGraph.find(pluginName);
        if (it != m_dependencyGraph.end()) {
            for (const auto& dep : it->second.dependencies) {
                if (recursionStack.count(dep) > 0) {
                    return true; // 循環依存発見
                }
                
                if (visited.count(dep) == 0 && 
                    HasCircularDependency(dep, visited, recursionStack)) {
                    return true;
                }
            }
        }
        
        recursionStack.erase(pluginName);
        return false;
    }
    
    std::string GetRemainingPlugins(const std::vector<std::string>& all,
                                   const std::vector<std::string>& resolved) {
        std::set<std::string> resolvedSet(resolved.begin(), resolved.end());
        std::vector<std::string> remaining;
        
        for (const auto& plugin : all) {
            if (resolvedSet.count(plugin) == 0) {
                remaining.push_back(plugin);
            }
        }
        
        return std::accumulate(remaining.begin(), remaining.end(), std::string(),
            [](const std::string& a, const std::string& b) {
                return a.empty() ? b : a + ", " + b;
            });
    }
};

// 循環依存例外クラス
class CircularDependencyException : public std::runtime_error {
public:
    explicit CircularDependencyException(const std::string& message)
        : std::runtime_error(message) {}
};
```

### 4. プラグインライフサイクル管理の強化

**改善後の実装**:
```cpp
class PluginLifecycleManager {
private:
    std::shared_mutex m_lifecycleMutex;
    std::map<std::string, std::chrono::system_clock::time_point> m_stateTransitionTimes;
    
public:
    bool TransitionToState(PluginEntry& entry, PluginState newState) {
        std::unique_lock<std::shared_mutex> lock(m_lifecycleMutex);
        
        PluginState currentState = entry.state;
        
        // 状態遷移の妥当性チェック
        if (!IsValidTransition(currentState, newState)) {
            LogInvalidTransition(entry.info.name, currentState, newState);
            return false;
        }
        
        // 状態遷移処理
        bool success = false;
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            success = PerformStateTransition(entry, newState);
        } catch (const std::exception& e) {
            LogTransitionError(entry.info.name, currentState, newState, e.what());
            entry.state = PluginState::Error;
            return false;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (success) {
            entry.state = newState;
            m_stateTransitionTimes[entry.info.name] = std::chrono::system_clock::now();
            
            LogStateTransition(entry.info.name, currentState, newState, duration.count());
            NotifyStateChange(entry.info.name, newState);
        }
        
        return success;
    }
    
    PluginState GetState(const PluginEntry& entry) const {
        return entry.state;
    }
    
    std::chrono::system_clock::time_point GetLastTransitionTime(const std::string& pluginName) const {
        std::shared_lock<std::shared_mutex> lock(m_lifecycleMutex);
        
        auto it = m_stateTransitionTimes.find(pluginName);
        if (it != m_stateTransitionTimes.end()) {
            return it->second;
        }
        
        return std::chrono::system_clock::time_point{};
    }
    
private:
    bool IsValidTransition(PluginState from, PluginState to) {
        switch (from) {
            case PluginState::Unloaded:
                return to == PluginState::Loading;
            case PluginState::Loading:
                return to == PluginState::Loaded || to == PluginState::Error;
            case PluginState::Loaded:
                return to == PluginState::Initializing || to == PluginState::Unloading || to == PluginState::Error;
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
    
    bool LoadPlugin(PluginEntry& entry) {
        // DLL読み込み処理は DLLManager に委譲
        return entry.dllHandle != nullptr;
    }
    
    bool ValidatePlugin(PluginEntry& entry) {
        // プラグインの検証
        if (!entry.createPluginFunc || !entry.destroyPluginFunc || !entry.getPluginInfoFunc) {
            return false;
        }
        
        // プラグイン情報の取得・検証
        try {
            entry.info = entry.getPluginInfoFunc();
            return !entry.info.name.empty() && !entry.info.version.empty();
        } catch (...) {
            return false;
        }
    }
    
    bool InitializePlugin(PluginEntry& entry) {
        if (!entry.plugin) {
            return false;
        }
        
        // 初期化実行
        return entry.plugin->Initialize();
    }
    
    bool ActivatePlugin(PluginEntry& entry) {
        if (!entry.plugin || !entry.plugin->IsInitialized()) {
            return false;
        }
        
        // プラグインをアクティブ化
        entry.plugin->SetEnabled(entry.config.enabled);
        return true;
    }
    
    bool UnloadPlugin(PluginEntry& entry) {
        // プラグインの終了処理
        if (entry.plugin) {
            try {
                entry.plugin->Shutdown();
            } catch (...) {
                // エラーは記録するが、アンロード処理は継続
            }
        }
        
        // DLLアンロードは DLLManager に委譲
        return true;
    }
    
    void HandlePluginError(PluginEntry& entry) {
        // エラー状態の処理
        if (entry.plugin) {
            try {
                entry.plugin->SetEnabled(false);
            } catch (...) {
                // 無視
            }
        }
    }
    
    void LogInvalidTransition(const std::string& pluginName, PluginState from, PluginState to) {
        if (auto loggingService = ServiceLocator::GetLoggingService()) {
            std::ostringstream oss;
            oss << "Invalid state transition for plugin " << pluginName 
                << ": " << static_cast<int>(from) << " -> " << static_cast<int>(to);
            loggingService->Warning(oss.str(), "PluginLifecycleManager");
        }
    }
    
    void LogStateTransition(const std::string& pluginName, PluginState from, PluginState to, int64_t durationMs) {
        if (auto loggingService = ServiceLocator::GetLoggingService()) {
            std::ostringstream oss;
            oss << "Plugin " << pluginName << " transitioned from " 
                << static_cast<int>(from) << " to " << static_cast<int>(to)
                << " in " << durationMs << "ms";
            loggingService->Info(oss.str(), "PluginLifecycleManager");
        }
    }
    
    void LogTransitionError(const std::string& pluginName, PluginState from, PluginState to, const std::string& error) {
        if (auto loggingService = ServiceLocator::GetLoggingService()) {
            std::ostringstream oss;
            oss << "Failed to transition plugin " << pluginName 
                << " from " << static_cast<int>(from) << " to " << static_cast<int>(to)
                << ": " << error;
            loggingService->Error(oss.str(), "PluginLifecycleManager");
        }
    }
    
    void NotifyStateChange(const std::string& pluginName, PluginState newState) {
        // イベント通知（必要に応じて実装）
        // EventBus システムが実装された後に詳細化
    }
};
```

### 5. パフォーマンス監視・最適化

**改善後の実装**:
```cpp
class PluginPerformanceMonitor {
private:
    struct PerformanceMetrics {
        std::chrono::microseconds loadTime{0};
        std::chrono::microseconds initTime{0};
        std::chrono::microseconds lastUpdateTime{0};
        size_t memoryUsage = 0;
        uint64_t updateCallCount = 0;
        uint64_t renderCallCount = 0;
        float averageUpdateTime = 0.0f;
        std::vector<float> updateTimeHistory;
    };
    
    std::map<std::string, PerformanceMetrics> m_metrics;
    std::shared_mutex m_metricsMutex;
    
    // システム全体のパフォーマンス
    std::chrono::high_resolution_clock::time_point m_systemStartTime;
    size_t m_totalMemoryUsage = 0;
    
public:
    void Initialize() {
        m_systemStartTime = std::chrono::high_resolution_clock::now();
    }
    
    void StartOperation(const std::string& pluginName, const std::string& operation) {
        // 操作の開始時間を記録
        std::unique_lock<std::shared_mutex> lock(m_metricsMutex);
        // 実装詳細...
    }
    
    void EndOperation(const std::string& pluginName, const std::string& operation) {
        // 操作の終了時間を記録し、メトリクスを更新
        std::unique_lock<std::shared_mutex> lock(m_metricsMutex);
        // 実装詳細...
    }
    
    void UpdateMemoryUsage(const std::string& pluginName, size_t memoryBytes) {
        std::unique_lock<std::shared_mutex> lock(m_metricsMutex);
        
        auto it = m_metrics.find(pluginName);
        if (it != m_metrics.end()) {
            size_t oldUsage = it->second.memoryUsage;
            it->second.memoryUsage = memoryBytes;
            
            // システム全体のメモリ使用量を更新
            m_totalMemoryUsage = m_totalMemoryUsage - oldUsage + memoryBytes;
        }
    }
    
    PerformanceMetrics GetMetrics(const std::string& pluginName) const {
        std::shared_lock<std::shared_mutex> lock(m_metricsMutex);
        
        auto it = m_metrics.find(pluginName);
        if (it != m_metrics.end()) {
            return it->second;
        }
        
        return PerformanceMetrics{};
    }
    
    bool IsPerformanceAcceptable(const std::string& pluginName) const {
        auto metrics = GetMetrics(pluginName);
        
        // パフォーマンス基準のチェック
        if (metrics.loadTime > std::chrono::seconds(3)) {
            return false; // 読み込み時間が3秒超過
        }
        
        if (metrics.memoryUsage > 100 * 1024 * 1024) {
            return false; // メモリ使用量が100MB超過
        }
        
        if (metrics.averageUpdateTime > 16.67f) {
            return false; // 平均更新時間が60FPS制限を超過
        }
        
        return true;
    }
    
    std::vector<std::string> GetPerformanceIssues() const {
        std::vector<std::string> issues;
        std::shared_lock<std::shared_mutex> lock(m_metricsMutex);
        
        for (const auto& pair : m_metrics) {
            const std::string& pluginName = pair.first;
            const PerformanceMetrics& metrics = pair.second;
            
            if (metrics.loadTime > std::chrono::seconds(3)) {
                issues.push_back("Plugin " + pluginName + " load time exceeds 3 seconds");
            }
            
            if (metrics.memoryUsage > 100 * 1024 * 1024) {
                issues.push_back("Plugin " + pluginName + " memory usage exceeds 100MB");
            }
            
            if (metrics.averageUpdateTime > 16.67f) {
                issues.push_back("Plugin " + pluginName + " update time affects 60FPS target");
            }
        }
        
        return issues;
    }
};
```

### 6. 設定管理・永続化の完全実装

**改善後の実装**:
```cpp
class PluginConfigurationManager {
private:
    struct ConfigSection {
        std::map<std::string, std::string> values;
    };
    
    std::map<std::string, ConfigSection> m_config;
    std::string m_configFilePath;
    mutable std::shared_mutex m_configMutex;
    
public:
    bool LoadConfiguration(const std::string& configFile) {
        std::unique_lock<std::shared_mutex> lock(m_configMutex);
        
        m_configFilePath = configFile;
        m_config.clear();
        
        if (!std::filesystem::exists(configFile)) {
            return false; // ファイルが存在しない
        }
        
        std::ifstream file(configFile);
        if (!file.is_open()) {
            return false;
        }
        
        std::string currentSection;
        std::string line;
        
        while (std::getline(file, line)) {
            line = TrimString(line);
            
            if (line.empty() || line[0] == ';' || line[0] == '#') {
                continue; // 空行またはコメント
            }
            
            if (line[0] == '[' && line.back() == ']') {
                // セクション開始
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            
            // キー=値の解析
            auto equalPos = line.find('=');
            if (equalPos != std::string::npos && !currentSection.empty()) {
                std::string key = TrimString(line.substr(0, equalPos));
                std::string value = TrimString(line.substr(equalPos + 1));
                
                m_config[currentSection].values[key] = value;
            }
        }
        
        file.close();
        return true;
    }
    
    bool SaveConfiguration(const std::string& configFile) {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        
        std::ofstream file(configFile);
        if (!file.is_open()) {
            return false;
        }
        
        // ヘッダーコメント
        file << "; NSys Plugin Manager Configuration\n";
        file << "; Generated on " << GetCurrentTimeString() << "\n\n";
        
        // 各セクションを出力
        for (const auto& sectionPair : m_config) {
            const std::string& sectionName = sectionPair.first;
            const ConfigSection& section = sectionPair.second;
            
            file << "[" << sectionName << "]\n";
            
            for (const auto& valuePair : section.values) {
                file << valuePair.first << "=" << valuePair.second << "\n";
            }
            
            file << "\n";
        }
        
        file.close();
        return true;
    }
    
    void SetPluginConfiguration(const std::string& pluginName, const PluginConfig& config) {
        std::unique_lock<std::shared_mutex> lock(m_configMutex);
        
        std::string sectionName = "Plugin." + pluginName;
        ConfigSection& section = m_config[sectionName];
        
        section.values["Enabled"] = config.enabled ? "true" : "false";
        section.values["AutoLoad"] = config.autoLoad ? "true" : "false";
        section.values["Priority"] = std::to_string(config.priority);
        section.values["ConfigSection"] = config.configSection;
        
        // カスタムパラメータ
        for (const auto& param : config.parameters) {
            section.values[param.first] = param.second;
        }
    }
    
    PluginConfig GetPluginConfiguration(const std::string& pluginName) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        
        PluginConfig config;
        std::string sectionName = "Plugin." + pluginName;
        
        auto sectionIt = m_config.find(sectionName);
        if (sectionIt != m_config.end()) {
            const ConfigSection& section = sectionIt->second;
            
            config.enabled = GetBoolValue(section, "Enabled", true);
            config.autoLoad = GetBoolValue(section, "AutoLoad", false);
            config.priority = GetIntValue(section, "Priority", 0);
            config.configSection = GetStringValue(section, "ConfigSection", "");
            
            // カスタムパラメータの読み込み
            for (const auto& pair : section.values) {
                if (pair.first != "Enabled" && pair.first != "AutoLoad" && 
                    pair.first != "Priority" && pair.first != "ConfigSection") {
                    config.parameters[pair.first] = pair.second;
                }
            }
        }
        
        return config;
    }
    
    void SetGlobalConfiguration(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(m_configMutex);
        m_config["PluginManager"].values[key] = value;
    }
    
    std::string GetGlobalConfiguration(const std::string& key, const std::string& defaultValue = "") const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        
        auto sectionIt = m_config.find("PluginManager");
        if (sectionIt != m_config.end()) {
            return GetStringValue(sectionIt->second, key, defaultValue);
        }
        
        return defaultValue;
    }
    
private:
    std::string TrimString(const std::string& str) const {
        auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        
        auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
    
    bool GetBoolValue(const ConfigSection& section, const std::string& key, bool defaultValue) const {
        auto it = section.values.find(key);
        if (it != section.values.end()) {
            std::string value = it->second;
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return value == "true" || value == "1" || value == "yes";
        }
        return defaultValue;
    }
    
    int GetIntValue(const ConfigSection& section, const std::string& key, int defaultValue) const {
        auto it = section.values.find(key);
        if (it != section.values.end()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    std::string GetStringValue(const ConfigSection& section, const std::string& key, const std::string& defaultValue) const {
        auto it = section.values.find(key);
        if (it != section.values.end()) {
            return it->second;
        }
        return defaultValue;
    }
    
    std::string GetCurrentTimeString() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};
```

### 7. エラーハンドリングとロギングの強化

**ServiceLocator統合を活用したエラーハンドリング**:
```cpp
class PluginErrorManager {
private:
    std::vector<PluginError> m_errors;
    std::string m_lastError;
    mutable std::shared_mutex m_errorMutex;
    
    ILoggingService* m_loggingService = nullptr;
    
public:
    void Initialize() {
        m_loggingService = ServiceLocator::GetLoggingService();
    }
    
    void ReportError(const std::string& pluginName, PluginErrorType type, 
                    const std::string& message, const std::string& details = "") {
        std::unique_lock<std::shared_mutex> lock(m_errorMutex);
        
        PluginError error;
        error.type = type;
        error.message = message;
        error.details = details;
        error.timestamp = std::chrono::system_clock::now();
        
        m_errors.push_back(error);
        m_lastError = "Plugin " + pluginName + ": " + message;
        
        // ログレベルの決定
        LogLevel level = DetermineLogLevel(type);
        
        // ログ出力
        if (m_loggingService) {
            std::string fullMessage = message;
            if (!details.empty()) {
                fullMessage += " (Details: " + details + ")";
            }
            
            switch (level) {
                case LogLevel::Error:
                    m_loggingService->Error(fullMessage, "PluginManager." + pluginName);
                    break;
                case LogLevel::Warning:
                    m_loggingService->Warning(fullMessage, "PluginManager." + pluginName);
                    break;
                case LogLevel::Info:
                    m_loggingService->Info(fullMessage, "PluginManager." + pluginName);
                    break;
            }
        }
        
        // クリティカルエラーの場合は追加処理
        if (IsCriticalError(type)) {
            HandleCriticalError(pluginName, error);
        }
    }
    
    std::vector<PluginError> GetErrors() const {
        std::shared_lock<std::shared_mutex> lock(m_errorMutex);
        return m_errors;
    }
    
    std::string GetLastError() const {
        std::shared_lock<std::shared_mutex> lock(m_errorMutex);
        return m_lastError;
    }
    
    bool HasErrors() const {
        std::shared_lock<std::shared_mutex> lock(m_errorMutex);
        return !m_errors.empty();
    }
    
    void ClearErrors() {
        std::unique_lock<std::shared_mutex> lock(m_errorMutex);
        m_errors.clear();
        m_lastError.clear();
    }
    
private:
    enum class LogLevel { Info, Warning, Error };
    
    LogLevel DetermineLogLevel(PluginErrorType type) {
        switch (type) {
            case PluginErrorType::LoadFailed:
            case PluginErrorType::InitializationFailed:
            case PluginErrorType::SecurityViolation:
            case PluginErrorType::MemoryError:
                return LogLevel::Error;
            case PluginErrorType::DependencyMissing:
            case PluginErrorType::RuntimeError:
                return LogLevel::Warning;
            case PluginErrorType::InvalidPlugin:
                return LogLevel::Info;
        }
        return LogLevel::Error;
    }
    
    bool IsCriticalError(PluginErrorType type) {
        return type == PluginErrorType::SecurityViolation || 
               type == PluginErrorType::MemoryError;
    }
    
    void HandleCriticalError(const std::string& pluginName, const PluginError& error) {
        // クリティカルエラーの特別処理
        if (m_loggingService) {
            m_loggingService->Error("CRITICAL ERROR in plugin " + pluginName + 
                                  ": " + error.message, "PluginManager");
        }
        
        // 必要に応じてシステム全体の停止などの処理
    }
};
```

### 8. 既存システムとの段階的統合

**main.cpp の段階的統合強化**:
```cpp
// main.cpp への統合改善
#include "PluginManager.h"

// 新旧プラグインマネージャーの橋渡し
class PluginSystemBridge {
private:
    NSys::PluginManager m_newManager;
    bool m_useNewSystem = false;
    
public:
    bool Initialize() {
        // 設定から新システムの使用可否を確認
        if (ShouldUseNewSystem()) {
            m_useNewSystem = m_newManager.Initialize("plugins");
            if (m_useNewSystem) {
                // 既存プラグインの新システムへの移行
                MigrateExistingPlugins();
            }
        }
        
        return true;
    }
    
    void Update(float deltaTime) {
        if (m_useNewSystem) {
            m_newManager.Update(deltaTime);
        }
    }
    
    void Shutdown() {
        if (m_useNewSystem) {
            m_newManager.Shutdown();
        }
    }
    
private:
    bool ShouldUseNewSystem() {
        // 環境変数や設定ファイルから判定
        return getenv("NSYS_USE_NEW_PLUGIN_SYSTEM") != nullptr;
    }
    
    void MigrateExistingPlugins() {
        // 既存プラグインの新システムへの段階的移行
        for (const auto& plugin : plugins) { // 既存のグローバル変数
            std::string pluginPath = WStringToString(plugin.name) + ".dll";
            m_newManager.LoadPlugin("plugins/" + pluginPath);
        }
        
        // ImGuiコア統合
        m_newManager.IntegrateWithImGuiCore();
    }
};

// グローバルブリッジインスタンス
PluginSystemBridge g_pluginBridge;

int main(int argc, char* argv[]) {
    // 既存の初期化...
    
    // プラグインシステムブリッジの初期化
    g_pluginBridge.Initialize();
    
    // メインループ
    while (!shouldExit) {
        // 既存の処理...
        
        // 新プラグインシステムの更新
        g_pluginBridge.Update(deltaTime);
        
        // 既存のImGui処理...
    }
    
    // 終了処理
    g_pluginBridge.Shutdown();
    
    return 0;
}
```

## リファクタリング完了チェックリスト

- [x] 実際のDLL読み込み機能の実装
- [x] プラグインエントリポイント管理の改善
- [x] 依存関係解決システムの完全実装
- [x] プラグインライフサイクル管理の強化
- [x] パフォーマンス監視・最適化
- [x] 設定管理・永続化の完全実装
- [x] エラーハンドリングとロギングの強化
- [x] 既存システムとの段階的統合
- [x] セキュリティ機能の基本実装
- [x] テスト互換性の維持

## 期待される改善効果

### 機能の完全性
- 実際のWindows DLL読み込み・アンロード
- 高度な依存関係解決（循環依存検出）
- 包括的なエラーハンドリング
- プラグインパフォーマンス監視

### セキュリティの向上
- DLL読み込みパスの検証
- 信頼できるディレクトリ制限
- 基本的なDLLハイジャック防止
- ファイルサイズ制限

### パフォーマンスの向上
- 読み込み時間の最適化
- メモリ使用量の監視・制限
- プラグイン実行時間の測定
- システム全体のパフォーマンス監視

### 保守性の向上
- 段階的な既存システム統合
- 包括的な設定管理
- 詳細なログ出力
- エラー追跡・診断機能

### 信頼性の向上
- 堅牢な状態管理
- 例外安全な実装
- リソースリークの防止
- グレースフルなエラー処理

この REFACTOR フェーズにより、TASK-101 の実装は本番環境での使用に適した品質レベルに到達し、TASK-003で構築したImGuiコア統合システムと完全に連携する高性能なプラグインマネージャーシステムを実現します。