#include "SecurityManager.h"
#include <stdexcept>
#include <algorithm>

namespace NSys {

// ==================== ライフサイクル管理 ====================

bool SecurityManager::Initialize(const SecurityConfiguration& config) {
    // Green Phase: 最小限の実装で初期化成功
    m_config = config;
    m_securityStatus = SecurityStatus::Active;
    m_lastError.clear();
    return true;
}

void SecurityManager::Shutdown() {
    // Green Phase: 正常なシャットダウン実装
    m_securityStatus = SecurityStatus::Uninitialized;
    m_lastError.clear();
}

void SecurityManager::Update(float deltaTime) {
    // Green Phase: 基本的な更新処理
    if (m_securityStatus == SecurityStatus::Active) {
        // 最小限の更新処理
    }
}

// ==================== DLL署名検証 ====================

bool SecurityManager::VerifyDLLSignature(const std::string& dllPath) {
    // Green Phase: 最小限の実装（実際の検証はまだ行わない）
    if (dllPath.empty()) {
        m_lastError = "Invalid DLL path";
        m_lastVerificationResult = SignatureVerificationResult::VerificationFailed;
        return false;
    }
    
    // 暫定的に署名有効として扱う
    m_lastVerificationResult = SignatureVerificationResult::Valid;
    return true;
}

bool SecurityManager::VerifyDLLSignature(const std::string& dllPath, const SignaturePolicy& policy) {
    // Green Phase: ポリシー考慮版の最小限実装
    return VerifyDLLSignature(dllPath);
}

bool SecurityManager::VerifyCertificateChain(const std::string& dllPath) {
    // Green Phase: 証明書チェーンの最小限実装
    return !dllPath.empty();
}

bool SecurityManager::IsTrustedPublisher(const std::string& publisherName) {
    // Green Phase: 信頼できる発行者チェックの最小限実装
    return !publisherName.empty();
}

bool SecurityManager::IsRevoked(const std::string& certificateThumbprint) {
    // Green Phase: 証明書失効チェックの最小限実装（常にfalse）
    return false;
}

SignatureInfo SecurityManager::GetSignatureInfo(const std::string& dllPath) {
    // Green Phase: 署名情報取得の最小限実装
    SignatureInfo info{};
    if (!dllPath.empty()) {
        info.subjectName = "Test Subject";
        info.issuerName = "Test Issuer";
    }
    return info;
}

SignatureVerificationResult SecurityManager::GetLastVerificationResult() const {
    return m_lastVerificationResult;
}

// ==================== ファイルアクセス制御 ====================

bool SecurityManager::CheckFileAccess(const std::string& filePath, FileAccessType accessType) {
    // Green Phase: ファイルアクセスチェックの最小限実装
    if (filePath.empty()) {
        m_lastError = "Invalid file path";
        return false;
    }
    return true;
}

bool SecurityManager::CheckFileAccess(const std::string& filePath, FileAccessType accessType, 
                                      const std::string& requestorContext) {
    // Green Phase: コンテキスト付きアクセスチェック
    return CheckFileAccess(filePath, accessType);
}

bool SecurityManager::SetFileAccessPolicy(const std::string& pattern, const FileAccessPolicy& policy) {
    // Green Phase: アクセスポリシー設定の最小限実装
    if (pattern.empty()) {
        m_lastError = "Invalid pattern";
        return false;
    }
    
    // ポリシーを保存
    auto it = std::find_if(m_accessPolicies.begin(), m_accessPolicies.end(),
        [&pattern](const FileAccessPolicy& p) { return p.pathPattern == pattern; });
    
    if (it != m_accessPolicies.end()) {
        *it = policy;
    } else {
        m_accessPolicies.push_back(policy);
    }
    return true;
}

bool SecurityManager::RemoveFileAccessPolicy(const std::string& pattern) {
    // Green Phase: アクセスポリシー削除の最小限実装
    auto it = std::find_if(m_accessPolicies.begin(), m_accessPolicies.end(),
        [&pattern](const FileAccessPolicy& p) { return p.pathPattern == pattern; });
    
    if (it != m_accessPolicies.end()) {
        m_accessPolicies.erase(it);
        return true;
    }
    return false;
}

std::vector<FileAccessPolicy> SecurityManager::GetActiveFileAccessPolicies() {
    // Green Phase: アクティブポリシー取得の実装
    return m_accessPolicies;
}

bool SecurityManager::ValidateFilePath(const std::string& filePath) {
    // Green Phase: ファイルパス検証の最小限実装
    return !filePath.empty() && filePath.find("..") == std::string::npos;
}

bool SecurityManager::IsPathTraversal(const std::string& filePath) {
    // Green Phase: パストラバーサル検出の最小限実装
    return filePath.find("..") != std::string::npos;
}

// ==================== DLLハイジャック防止 ====================

bool SecurityManager::ValidateDLLLoadPath(const std::string& dllPath) {
    // Green Phase: DLL読み込みパス検証の最小限実装
    return !dllPath.empty() && dllPath.find("..") == std::string::npos;
}

bool SecurityManager::CheckDLLIntegrity(const std::string& dllPath) {
    // Green Phase: DLL整合性チェックの最小限実装
    return !dllPath.empty();
}

bool SecurityManager::IsSecureLoadPath(const std::string& dllPath) {
    // Green Phase: セキュアロードパスチェックの最小限実装
    return !dllPath.empty();
}

bool SecurityManager::DetectDLLHijackingAttempt(const std::string& dllPath, const std::string& expectedPath) {
    // Green Phase: DLLハイジャック検出の最小限実装
    return dllPath != expectedPath;
}

bool SecurityManager::DetectPreloadedDLLs() {
    // Green Phase: プリロードDLL検出の最小限実装
    return false;
}

HMODULE SecurityManager::SecureLoadLibrary(const std::string& dllPath) {
    // Green Phase: セキュアライブラリ読み込みの最小限実装
    if (dllPath.empty()) {
        m_lastError = "Invalid DLL path";
        return nullptr;
    }
    return LoadLibraryA(dllPath.c_str());
}

bool SecurityManager::SecureFreeLibrary(HMODULE hModule) {
    // Green Phase: セキュアライブラリ解放の最小限実装
    if (hModule == nullptr) {
        m_lastError = "Invalid module handle";
        return false;
    }
    return FreeLibrary(hModule) != 0;
}

// ==================== セキュリティポリシー管理 ====================

bool SecurityManager::LoadSecurityPolicy(const std::string& policyFile) {
    // Green Phase: セキュリティポリシー読み込みの最小限実装
    if (policyFile.empty()) {
        m_lastError = "Invalid policy file";
        return false;
    }
    return true;
}

bool SecurityManager::SaveSecurityPolicy(const std::string& policyFile) {
    // Green Phase: セキュリティポリシー保存の最小限実装
    if (policyFile.empty()) {
        m_lastError = "Invalid policy file";
        return false;
    }
    return true;
}

bool SecurityManager::UpdateSecurityPolicy(const SecurityConfiguration& policy) {
    // Green Phase: セキュリティポリシー更新の最小限実装
    m_config = policy;
    return true;
}

SecurityConfiguration SecurityManager::GetCurrentSecurityPolicy() {
    // Green Phase: 現在のセキュリティポリシー取得
    return m_config;
}

bool SecurityManager::EnforceSecurityPolicy(const std::string& operation, const OperationContext& context) {
    // Green Phase: セキュリティポリシー施行の最小限実装
    return !operation.empty();
}

// ==================== 侵入検知・監視 ====================

bool SecurityManager::DetectSuspiciousActivity(const ActivityContext& context) {
    // Green Phase: 疑わしい活動検出の最小限実装
    return false;
}

bool SecurityManager::IsBlacklistedProcess(const std::string& processName) {
    // Green Phase: ブラックリストプロセスチェック
    return false;
}

bool SecurityManager::IsBlacklistedModule(const std::string& modulePath) {
    // Green Phase: ブラックリストモジュールチェック
    return false;
}

void SecurityManager::RegisterSecurityEventHandler(ISecurityEventHandler* handler) {
    // Green Phase: セキュリティイベントハンドラー登録
    if (handler) {
        m_eventHandlers.push_back(handler);
    }
}

void SecurityManager::UnregisterSecurityEventHandler(ISecurityEventHandler* handler) {
    // Green Phase: セキュリティイベントハンドラー登録解除
    auto it = std::find(m_eventHandlers.begin(), m_eventHandlers.end(), handler);
    if (it != m_eventHandlers.end()) {
        m_eventHandlers.erase(it);
    }
}

// ==================== セキュリティ監査 ====================

void SecurityManager::LogSecurityEvent(SecurityEventType eventType, const std::string& description, 
                                       const std::map<std::string, std::string>& context) {
    // Green Phase: セキュリティイベントログ記録の最小限実装
    SecurityEvent event{};
    event.eventType = eventType;
    event.description = description;
    event.timestamp = std::chrono::system_clock::now();
    event.context = context;
    
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_securityEvents.push_back(event);
        
        // 最大エントリ数を超えた場合は古いエントリを削除
        if (m_securityEvents.size() > m_config.maxLogEntries) {
            m_securityEvents.pop_front();
        }
    }
}

std::vector<SecurityEvent> SecurityManager::GetRecentSecurityEvents(int maxCount) {
    // Green Phase: 最近のセキュリティイベント取得
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    std::vector<SecurityEvent> events;
    int count = std::min(maxCount, static_cast<int>(m_securityEvents.size()));
    
    auto it = m_securityEvents.rbegin();
    for (int i = 0; i < count && it != m_securityEvents.rend(); ++i, ++it) {
        events.push_back(*it);
    }
    
    return events;
}

bool SecurityManager::ExportSecurityLog(const std::string& outputPath, const SecurityLogFilter& filter) {
    // Green Phase: セキュリティログエクスポートの最小限実装
    if (outputPath.empty()) {
        m_lastError = "Invalid output path";
        return false;
    }
    return true;
}

SecurityStatistics SecurityManager::GetSecurityStatistics() {
    // Green Phase: セキュリティ統計取得の実装
    return m_statistics;
}

// ==================== 権限管理 ====================

bool SecurityManager::CheckPermission(const std::string& operation, const SecurityContext& context) {
    // Green Phase: 権限チェックの最小限実装
    return !operation.empty();
}

bool SecurityManager::GrantPermission(const std::string& operation, const SecurityContext& context) {
    // Green Phase: 権限付与の最小限実装
    if (operation.empty()) {
        m_lastError = "Invalid operation";
        return false;
    }
    
    Permission permission;
    permission.operation = operation;
    permission.granted = true;
    permission.grantedAt = std::chrono::system_clock::now();
    permission.grantedBy = context.userName;
    
    m_permissions[operation] = permission;
    return true;
}

bool SecurityManager::RevokePermission(const std::string& operation, const SecurityContext& context) {
    // Green Phase: 権限取り消しの最小限実装
    auto it = m_permissions.find(operation);
    if (it != m_permissions.end()) {
        m_permissions.erase(it);
        return true;
    }
    return false;
}

std::vector<Permission> SecurityManager::GetGrantedPermissions(const SecurityContext& context) {
    // Green Phase: 付与された権限取得の実装
    std::vector<Permission> permissions;
    for (const auto& pair : m_permissions) {
        permissions.push_back(pair.second);
    }
    return permissions;
}

// ==================== セキュリティ状態管理 ====================

SecurityStatus SecurityManager::GetSecurityStatus() {
    return m_securityStatus;
}

bool SecurityManager::IsSecureModeEnabled() {
    // Green Phase: セキュアモード確認の実装
    return m_secureMode.load();
}

void SecurityManager::SetSecureMode(bool enabled) {
    // Green Phase: セキュアモード設定の実装
    m_secureMode.store(enabled);
}

bool SecurityManager::HasActiveThreats() {
    // Green Phase: アクティブ脅威確認の実装
    return !m_activeThreats.empty();
}

std::vector<SecurityThreat> SecurityManager::GetActiveThreats() {
    // Green Phase: アクティブ脅威取得の実装
    return m_activeThreats;
}

// ==================== TASK-101 PluginManager統合 ====================

bool SecurityManager::ValidatePlugin(const std::string& pluginPath, const PluginSecurityContext& context) {
    // Green Phase: プラグイン検証の最小限実装
    if (pluginPath.empty()) {
        m_lastError = "Invalid plugin path";
        return false;
    }
    
    // DLL署名検証
    if (!VerifyDLLSignature(pluginPath)) {
        return false;
    }
    
    return true;
}

bool SecurityManager::AuthorizePluginLoad(const PluginInfo& pluginInfo, const SecurityContext& context) {
    // Green Phase: プラグインロード認可の最小限実装
    return true;
}

bool SecurityManager::MonitorPluginBehavior(const std::string& pluginName, const PluginBehaviorContext& context) {
    // Green Phase: プラグイン行動監視の最小限実装
    return !pluginName.empty();
}

void SecurityManager::RegisterPluginSecurityHandler(const std::string& pluginName, IPluginSecurityHandler* handler) {
    // Green Phase: プラグインセキュリティハンドラー登録
    if (!pluginName.empty() && handler) {
        m_pluginHandlers[pluginName] = handler;
    }
}

void SecurityManager::UnregisterPluginSecurityHandler(const std::string& pluginName) {
    // Green Phase: プラグインセキュリティハンドラー登録解除
    auto it = m_pluginHandlers.find(pluginName);
    if (it != m_pluginHandlers.end()) {
        m_pluginHandlers.erase(it);
    }
}

// ==================== 設定・永続化 ====================

bool SecurityManager::LoadConfiguration(const std::string& configFile) {
    // Green Phase: 設定読み込みの最小限実装
    if (configFile.empty()) {
        m_lastError = "Invalid config file";
        return false;
    }
    return true;
}

bool SecurityManager::SaveConfiguration(const std::string& configFile) {
    // Green Phase: 設定保存の最小限実装
    if (configFile.empty()) {
        m_lastError = "Invalid config file";
        return false;
    }
    return true;
}

void SecurityManager::ResetToDefaults() {
    // Green Phase: デフォルト設定リセットの実装
    m_config = SecurityConfiguration::Default();
    m_lastError.clear();
    m_accessPolicies.clear();
    m_securityEvents.clear();
    m_activeThreats.clear();
    m_permissions.clear();
    m_statistics = SecurityStatistics{};
}

// ==================== エラー・診断 ====================

std::string SecurityManager::GetLastError() const {
    std::lock_guard<std::mutex> lock(m_errorMutex);
    return m_lastError;
}

bool SecurityManager::HasErrors() const {
    std::lock_guard<std::mutex> lock(m_errorMutex);
    return !m_lastError.empty();
}

void SecurityManager::ClearErrors() {
    std::lock_guard<std::mutex> lock(m_errorMutex);
    m_lastError.clear();
}

// ==================== デストラクタ ====================

SecurityManager::~SecurityManager() {
    if (m_securityStatus != SecurityStatus::Uninitialized) {
        Shutdown();
    }
}

// ==================== 内部メソッド ====================

bool SecurityManager::ValidateSignatureInternal(const std::string& dllPath, const SignaturePolicy& policy) {
    // Green Phase: 内部署名検証の最小限実装
    return !dllPath.empty();
}

bool SecurityManager::CheckFileAccessInternal(const std::string& filePath, FileAccessType accessType, 
                                              const SecurityContext& context) {
    // Green Phase: 内部ファイルアクセスチェックの最小限実装
    return !filePath.empty();
}

bool SecurityManager::DetectDLLHijackingInternal(const std::string& dllPath) {
    // Green Phase: 内部DLLハイジャック検出の最小限実装
    return false;
}

void SecurityManager::WriteSecurityLog(const SecurityEvent& event) {
    // Green Phase: セキュリティログ書き込みの実装
    // LogSecurityEventで実装済み
}

void SecurityManager::NotifyEventHandlers(const SecurityEvent& event) {
    // Green Phase: イベントハンドラー通知の実装
    for (auto* handler : m_eventHandlers) {
        if (handler) {
            handler->OnSecurityEvent(event);
        }
    }
}

void SecurityManager::UpdateStatistics(const SecurityEvent& event) {
    // Green Phase: 統計更新の実装
    m_statistics.totalSecurityEvents++;
    m_statistics.lastUpdate = std::chrono::system_clock::now();
    
    switch (event.eventType) {
        case SecurityEventType::FileAccessDenied:
            m_statistics.blockedOperations++;
            break;
        case SecurityEventType::DLLHijackingAttempt:
            m_statistics.dllHijackingAttempts++;
            break;
        default:
            break;
    }
}

std::string SecurityManager::NormalizePath(const std::string& path) {
    // Green Phase: パス正規化の最小限実装
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return normalized;
}

bool SecurityManager::IsSystemDirectory(const std::string& path) {
    // Green Phase: システムディレクトリチェックの最小限実装
    std::string normalized = NormalizePath(path);
    return normalized.find("System32") != std::string::npos ||
           normalized.find("SysWOW64") != std::string::npos;
}

bool SecurityManager::IsApplicationDirectory(const std::string& path) {
    // Green Phase: アプリケーションディレクトリチェックの最小限実装
    return !path.empty() && path.find("Program Files") != std::string::npos;
}

} // namespace NSys