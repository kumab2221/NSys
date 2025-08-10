# TASK-102: セキュリティマネージャー実装 - 要件定義

## タスク概要

NSys プラットフォームにおける包括的セキュリティ管理システムを実装する。DLL署名検証機能、ファイルアクセス権限制御、DLLハイジャック攻撃防止対策を含む高度なセキュリティフレームワークを提供し、TASK-101のプラグインマネージャーと完全に連携してセキュアなプラグイン実行環境を実現する。

## 詳細要件

### 1. SecurityManager クラス実装 (NFR-101, NFR-102, NFR-103)

**目的**: 包括的で多層的なセキュリティ保護を提供する統合セキュリティ管理システム

**機能要件**:
- DLL署名検証システム
- ファイルアクセス権限制御
- DLLハイジャック攻撃防止
- セキュリティポリシー管理
- 侵入検知・ログ記録
- 権限昇格攻撃防止
- セキュリティ監査機能

**クラス仕様**:
```cpp
class SecurityManager {
public:
    // ライフサイクル管理
    bool Initialize(const SecurityConfiguration& config = SecurityConfiguration::Default());
    void Shutdown();
    void Update(float deltaTime);
    
    // DLL署名検証
    bool VerifyDLLSignature(const std::string& dllPath);
    bool VerifyDLLSignature(const std::string& dllPath, const SignaturePolicy& policy);
    bool VerifyCertificateChain(const std::string& dllPath);
    bool IsTrustedPublisher(const std::string& publisherName);
    bool IsRevoked(const std::string& certificateThumbprint);
    
    // ファイルアクセス制御
    bool CheckFileAccess(const std::string& filePath, FileAccessType accessType);
    bool CheckFileAccess(const std::string& filePath, FileAccessType accessType, const std::string& requestorContext);
    bool SetFileAccessPolicy(const std::string& pattern, const FileAccessPolicy& policy);
    bool RemoveFileAccessPolicy(const std::string& pattern);
    std::vector<FileAccessPolicy> GetActiveFileAccessPolicies();
    
    // DLLハイジャック防止
    bool ValidateDLLLoadPath(const std::string& dllPath);
    bool CheckDLLIntegrity(const std::string& dllPath);
    bool IsSecureLoadPath(const std::string& dllPath);
    bool DetectDLLHijackingAttempt(const std::string& dllPath, const std::string& expectedPath);
    
    // セキュリティポリシー管理
    bool LoadSecurityPolicy(const std::string& policyFile);
    bool SaveSecurityPolicy(const std::string& policyFile);
    bool UpdateSecurityPolicy(const SecurityPolicy& policy);
    SecurityPolicy GetCurrentSecurityPolicy();
    bool EnforceSecurityPolicy(const std::string& operation, const OperationContext& context);
    
    // 侵入検知・監視
    bool DetectSuspiciousActivity(const ActivityContext& context);
    bool IsBlacklistedProcess(const std::string& processName);
    bool IsBlacklistedModule(const std::string& modulePath);
    void RegisterSecurityEventHandler(ISecurityEventHandler* handler);
    void UnregisterSecurityEventHandler(ISecurityEventHandler* handler);
    
    // セキュリティ監査
    void LogSecurityEvent(SecurityEventType eventType, const std::string& description, 
                         const std::map<std::string, std::string>& context = {});
    std::vector<SecurityEvent> GetRecentSecurityEvents(int maxCount = 100);
    bool ExportSecurityLog(const std::string& outputPath, const SecurityLogFilter& filter = {});
    SecurityStatistics GetSecurityStatistics();
    
    // 権限管理
    bool CheckPermission(const std::string& operation, const SecurityContext& context);
    bool GrantPermission(const std::string& operation, const SecurityContext& context);
    bool RevokePermission(const std::string& operation, const SecurityContext& context);
    std::vector<Permission> GetGrantedPermissions(const SecurityContext& context);
    
    // セキュリティ状態管理
    SecurityStatus GetSecurityStatus();
    bool IsSecureModeEnabled();
    void SetSecureMode(bool enabled);
    bool HasActiveThreats();
    std::vector<SecurityThreat> GetActiveThreats();
    
    // TASK-101 PluginManager統合
    bool ValidatePlugin(const std::string& pluginPath, const PluginSecurityContext& context);
    bool AuthorizePluginLoad(const PluginInfo& pluginInfo, const SecurityContext& context);
    bool MonitorPluginBehavior(const std::string& pluginName, const PluginBehaviorContext& context);
    void RegisterPluginSecurityHandler(const std::string& pluginName, IPluginSecurityHandler* handler);
    
    // 設定・永続化
    bool LoadConfiguration(const std::string& configFile = "security.ini");
    bool SaveConfiguration(const std::string& configFile = "security.ini");
    void ResetToDefaults();
    
private:
    // 内部セキュリティ処理
    bool ValidateSignatureInternal(const std::string& dllPath, const SignatureValidationContext& context);
    bool CheckFileAccessInternal(const std::string& filePath, FileAccessType accessType, const SecurityContext& context);
    bool DetectDLLHijackingInternal(const std::string& dllPath, const DLLHijackingContext& context);
    
    // ポリシー処理
    bool EvaluateAccessPolicy(const std::string& resourcePath, const AccessPolicyContext& context);
    bool ApplySecurityPolicy(const std::string& operation, const SecurityPolicyContext& context);
    
    // 脅威分析
    ThreatLevel AnalyzeThreatLevel(const SecurityContext& context);
    void UpdateThreatDatabase();
    bool CorrelateSecurityEvents(const std::vector<SecurityEvent>& events);
    
    // ログ・監査
    void WriteSecurityLog(const SecurityEvent& event);
    void RotateSecurityLogs();
    void PurgeOldSecurityLogs();
    
    // スレッド安全性
    mutable std::shared_mutex m_securityMutex;
    mutable std::mutex m_logMutex;
    std::atomic<bool> m_securityEnabled{true};
    std::atomic<bool> m_shutdownRequested{false};
};
```

### 2. DLL署名検証機能 (NFR-101)

**目的**: 信頼されていないプラグインの実行を防止し、改ざんされたDLLの検出

**機能要件**:
- Authenticode署名の検証
- 証明書チェーンの検証
- 証明書失効リスト（CRL）チェック
- 信頼できる発行者の管理
- タイムスタンプ検証
- カスタム署名ポリシーの適用

**実装仕様**:
```cpp
enum class SignatureVerificationResult {
    Valid,
    Invalid,
    NotSigned,
    RevokedCertificate,
    ExpiredCertificate,
    UntrustedPublisher,
    TamperedFile,
    VerificationFailed
};

struct SignatureInfo {
    std::string subjectName;
    std::string issuerName;
    std::string serialNumber;
    std::string thumbprint;
    std::chrono::system_clock::time_point validFrom;
    std::chrono::system_clock::time_point validTo;
    bool hasTimestamp = false;
    std::chrono::system_clock::time_point timestampTime;
    SignatureAlgorithm algorithm = SignatureAlgorithm::SHA256RSA;
};

struct SignaturePolicy {
    bool requireValidSignature = true;
    bool allowSelfSigned = false;
    bool requireTimestamp = true;
    bool checkCertificateRevocation = true;
    bool requireTrustedPublisher = true;
    std::vector<std::string> trustedPublishers;
    std::vector<std::string> blockedThumbprints;
    std::chrono::seconds maxCertificateAge{365 * 24 * 3600}; // 1年
};

class DigitalSignatureVerifier {
private:
    std::vector<std::string> m_trustedPublishers;
    std::vector<std::string> m_revokedCertificates;
    SignaturePolicy m_defaultPolicy;
    std::mutex m_verifierMutex;
    
public:
    SignatureVerificationResult VerifySignature(const std::string& filePath, 
                                               const SignaturePolicy& policy = {}) {
        std::lock_guard<std::mutex> lock(m_verifierMutex);
        
        // WinTrust APIを使用したAuthenticode検証
        WINTRUST_FILE_INFO fileInfo = {};
        WINTRUST_DATA winTrustData = {};
        GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        
        // ファイル情報設定
        std::wstring wideFilePath = ConvertToWideString(filePath);
        fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
        fileInfo.pcwszFilePath = wideFilePath.c_str();
        fileInfo.hFile = nullptr;
        fileInfo.pgKnownSubject = nullptr;
        
        // WinTrustData設定
        winTrustData.cbStruct = sizeof(WINTRUST_DATA);
        winTrustData.pPolicyCallbackData = nullptr;
        winTrustData.pSIPClientData = nullptr;
        winTrustData.dwUIChoice = WTD_UI_NONE;
        winTrustData.fdwRevocationChecks = policy.checkCertificateRevocation ? 
                                          WTD_REVOKE_WHOLECHAIN : WTD_REVOKE_NONE;
        winTrustData.dwUnionChoice = WTD_CHOICE_FILE;
        winTrustData.pFile = &fileInfo;
        winTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
        winTrustData.hWVTStateData = nullptr;
        winTrustData.pwszURLReference = nullptr;
        winTrustData.dwProvFlags = WTD_SAFER_FLAG;
        
        // 署名検証実行
        LONG verifyResult = WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), 
                                          &guidAction, &winTrustData);
        
        // 結果の処理
        SignatureVerificationResult result = ProcessVerificationResult(verifyResult);
        
        // 追加検証（証明書情報取得・検証）
        if (result == SignatureVerificationResult::Valid) {
            SignatureInfo sigInfo;
            if (ExtractSignatureInfo(filePath, sigInfo)) {
                result = ValidateSignatureInfo(sigInfo, policy);
            }
        }
        
        // クリーンアップ
        winTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &guidAction, &winTrustData);
        
        return result;
    }
    
private:
    SignatureVerificationResult ProcessVerificationResult(LONG result) {
        switch (result) {
            case ERROR_SUCCESS:
                return SignatureVerificationResult::Valid;
            case TRUST_E_NOSIGNATURE:
                return SignatureVerificationResult::NotSigned;
            case TRUST_E_EXPLICIT_DISTRUST:
                return SignatureVerificationResult::UntrustedPublisher;
            case TRUST_E_SUBJECT_NOT_TRUSTED:
                return SignatureVerificationResult::UntrustedPublisher;
            case CERT_E_EXPIRED:
                return SignatureVerificationResult::ExpiredCertificate;
            case CERT_E_REVOKED:
                return SignatureVerificationResult::RevokedCertificate;
            case TRUST_E_BAD_DIGEST:
                return SignatureVerificationResult::TamperedFile;
            default:
                return SignatureVerificationResult::VerificationFailed;
        }
    }
    
    bool ExtractSignatureInfo(const std::string& filePath, SignatureInfo& sigInfo) {
        // PKCS#7署名情報の抽出
        HCERTSTORE hStore = nullptr;
        HCRYPTMSG hMsg = nullptr;
        PCCERT_CONTEXT pCertContext = nullptr;
        
        try {
            // ファイルから署名メッセージを取得
            if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                                ConvertToWideString(filePath).c_str(),
                                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                                CERT_QUERY_FORMAT_FLAG_BINARY,
                                0, nullptr, nullptr, nullptr,
                                &hStore, &hMsg, nullptr)) {
                return false;
            }
            
            // 署名者証明書を取得
            DWORD signerIndex = 0;
            DWORD cbData = sizeof(DWORD);
            if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_COUNT_PARAM, 0, &signerIndex, &cbData)) {
                return false;
            }
            
            if (signerIndex == 0) {
                return false;
            }
            
            // 証明書情報を取得
            PCMSG_SIGNER_INFO pSignerInfo = nullptr;
            DWORD cbSignerInfo = 0;
            if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &cbSignerInfo)) {
                return false;
            }
            
            pSignerInfo = reinterpret_cast<PCMSG_SIGNER_INFO>(new BYTE[cbSignerInfo]);
            if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, pSignerInfo, &cbSignerInfo)) {
                delete[] pSignerInfo;
                return false;
            }
            
            // 証明書コンテキストを取得
            CERT_INFO certInfo = {};
            certInfo.Issuer = pSignerInfo->Issuer;
            certInfo.SerialNumber = pSignerInfo->SerialNumber;
            
            pCertContext = CertFindCertificateInStore(hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                    0, CERT_FIND_SUBJECT_CERT, &certInfo, nullptr);
            
            if (pCertContext) {
                // 証明書情報を抽出
                ExtractCertificateDetails(pCertContext, sigInfo);
                
                delete[] pSignerInfo;
                return true;
            }
            
            delete[] pSignerInfo;
            return false;
        }
        catch (...) {
            if (pCertContext) CertFreeCertificateContext(pCertContext);
            if (hStore) CertCloseStore(hStore, 0);
            if (hMsg) CryptMsgClose(hMsg);
            return false;
        }
    }
    
    SignatureVerificationResult ValidateSignatureInfo(const SignatureInfo& sigInfo, 
                                                     const SignaturePolicy& policy) {
        // タイムスタンプ要求チェック
        if (policy.requireTimestamp && !sigInfo.hasTimestamp) {
            return SignatureVerificationResult::Invalid;
        }
        
        // 証明書有効期限チェック
        auto now = std::chrono::system_clock::now();
        if (sigInfo.validTo < now) {
            return SignatureVerificationResult::ExpiredCertificate;
        }
        
        // 証明書古すぎるかチェック
        if (now - sigInfo.validFrom > policy.maxCertificateAge) {
            return SignatureVerificationResult::ExpiredCertificate;
        }
        
        // 信頼できる発行者チェック
        if (policy.requireTrustedPublisher) {
            bool isTrusted = false;
            for (const auto& trustedPublisher : policy.trustedPublishers) {
                if (sigInfo.subjectName.find(trustedPublisher) != std::string::npos ||
                    sigInfo.issuerName.find(trustedPublisher) != std::string::npos) {
                    isTrusted = true;
                    break;
                }
            }
            if (!isTrusted) {
                return SignatureVerificationResult::UntrustedPublisher;
            }
        }
        
        // ブロックされた証明書チェック
        for (const auto& blockedThumbprint : policy.blockedThumbprints) {
            if (sigInfo.thumbprint == blockedThumbprint) {
                return SignatureVerificationResult::RevokedCertificate;
            }
        }
        
        return SignatureVerificationResult::Valid;
    }
};
```

### 3. ファイルアクセス権限制御 (NFR-102)

**目的**: プラグインのファイルアクセスを制限し、システムファイルへの不正アクセスを防止

**機能要件**:
- ファイル・ディレクトリアクセス権限チェック
- パストラバーサル攻撃防止
- サンドボックス環境の提供
- アクセスポリシー管理
- ファイルアクセス監査ログ
- 動的権限変更機能

**実装仕様**:
```cpp
enum class FileAccessType {
    Read,
    Write,
    Execute,
    Delete,
    Create,
    Modify,
    ListDirectory,
    ChangePermissions
};

enum class AccessDecision {
    Allow,
    Deny,
    Audit,
    AuditAndAllow,
    AuditAndDeny
};

struct FileAccessPolicy {
    std::string pathPattern;              // ファイルパスのパターン（ワイルドカード対応）
    std::vector<FileAccessType> allowedAccesses;  // 許可されたアクセスタイプ
    std::vector<FileAccessType> deniedAccesses;   // 拒否されたアクセスタイプ
    std::vector<std::string> trustedProcesses;    // 信頼できるプロセス名
    bool inheritFromParent = true;        // 親ディレクトリからの継承
    bool recursiveApply = true;           // サブディレクトリへの再帰適用
    AccessDecision defaultDecision = AccessDecision::Deny;
    uint32_t priority = 0;               // ポリシー優先度（高い数値が優先）
};

struct AccessAuditLog {
    std::chrono::system_clock::time_point timestamp;
    std::string filePath;
    FileAccessType accessType;
    AccessDecision decision;
    std::string processName;
    std::string processId;
    std::string userName;
    std::string reason;
    std::map<std::string, std::string> additionalContext;
};

class FileAccessController {
private:
    std::vector<FileAccessPolicy> m_accessPolicies;
    std::map<std::string, std::vector<std::string>> m_sandboxPaths; // プロセス別サンドボックス
    std::deque<AccessAuditLog> m_auditLogs;
    mutable std::shared_mutex m_policiesMutex;
    mutable std::mutex m_auditMutex;
    bool m_auditEnabled = true;
    
public:
    bool CheckFileAccess(const std::string& filePath, FileAccessType accessType, 
                        const SecurityContext& context = {}) {
        // パス正規化とセキュリティチェック
        std::string normalizedPath = NormalizePath(filePath);
        if (!ValidatePath(normalizedPath)) {
            LogAccessAttempt(filePath, accessType, AccessDecision::Deny, "Invalid path", context);
            return false;
        }
        
        // パストラバーサル攻撃チェック
        if (DetectPathTraversalAttack(normalizedPath)) {
            LogAccessAttempt(filePath, accessType, AccessDecision::Deny, "Path traversal attack", context);
            return false;
        }
        
        // サンドボックスチェック
        if (!CheckSandboxAccess(normalizedPath, context)) {
            LogAccessAttempt(filePath, accessType, AccessDecision::Deny, "Sandbox violation", context);
            return false;
        }
        
        // ポリシーベースのアクセス制御
        AccessDecision decision = EvaluateAccessPolicies(normalizedPath, accessType, context);
        
        // 監査ログ記録
        if (m_auditEnabled && (decision == AccessDecision::Audit || 
                              decision == AccessDecision::AuditAndAllow ||
                              decision == AccessDecision::AuditAndDeny)) {
            LogAccessAttempt(filePath, accessType, decision, "Policy evaluation", context);
        }
        
        return (decision == AccessDecision::Allow || decision == AccessDecision::AuditAndAllow);
    }
    
    bool SetFileAccessPolicy(const FileAccessPolicy& policy) {
        std::unique_lock<std::shared_mutex> lock(m_policiesMutex);
        
        // 既存ポリシーの検索・更新または新規追加
        auto it = std::find_if(m_accessPolicies.begin(), m_accessPolicies.end(),
                              [&policy](const FileAccessPolicy& existing) {
                                  return existing.pathPattern == policy.pathPattern;
                              });
        
        if (it != m_accessPolicies.end()) {
            *it = policy;
        } else {
            m_accessPolicies.push_back(policy);
        }
        
        // 優先度でソート
        std::sort(m_accessPolicies.begin(), m_accessPolicies.end(),
                 [](const FileAccessPolicy& a, const FileAccessPolicy& b) {
                     return a.priority > b.priority;
                 });
        
        return true;
    }
    
    bool SetupSandbox(const std::string& processName, const std::vector<std::string>& allowedPaths) {
        std::unique_lock<std::shared_mutex> lock(m_policiesMutex);
        
        // 各パスを正規化して保存
        std::vector<std::string> normalizedPaths;
        for (const auto& path : allowedPaths) {
            std::string normalized = NormalizePath(path);
            if (ValidatePath(normalized)) {
                normalizedPaths.push_back(normalized);
            }
        }
        
        m_sandboxPaths[processName] = normalizedPaths;
        return true;
    }
    
private:
    std::string NormalizePath(const std::string& path) {
        // パスの正規化（".", "..", 重複する区切り文字の処理など）
        std::string normalized = path;
        
        // バックスラッシュをスラッシュに統一
        std::replace(normalized.begin(), normalized.end(), '\\', '/');
        
        // 重複するスラッシュを削除
        std::regex duplicateSlash("/+");
        normalized = std::regex_replace(normalized, duplicateSlash, "/");
        
        // 絶対パスに変換
        if (!normalized.empty() && normalized[0] != '/') {
            char currentDir[MAX_PATH];
            if (GetCurrentDirectoryA(sizeof(currentDir), currentDir) > 0) {
                std::string currentDirStr(currentDir);
                std::replace(currentDirStr.begin(), currentDirStr.end(), '\\', '/');
                normalized = currentDirStr + "/" + normalized;
            }
        }
        
        // "." と ".." を解決
        std::vector<std::string> pathComponents;
        std::istringstream pathStream(normalized);
        std::string component;
        
        while (std::getline(pathStream, component, '/')) {
            if (component.empty() || component == ".") {
                continue;
            } else if (component == "..") {
                if (!pathComponents.empty()) {
                    pathComponents.pop_back();
                }
            } else {
                pathComponents.push_back(component);
            }
        }
        
        // パスを再構築
        std::string result;
        for (size_t i = 0; i < pathComponents.size(); ++i) {
            result += "/" + pathComponents[i];
        }
        
        return result.empty() ? "/" : result;
    }
    
    bool ValidatePath(const std::string& path) {
        // 危険な文字やパターンをチェック
        static const std::vector<std::string> dangerousPatterns = {
            "..",              // ディレクトリトラバーサル
            "<",               // リダイレクト攻撃
            ">",               // リダイレクト攻撃
            "|",               // パイプ攻撃
            "&",               // コマンド連結攻撃
            ";",               // コマンド区切り攻撃
            "*",               // ワイルドカード攻撃（制限パターン）
            "?",               // ワイルドカード攻撃（制限パターン）
            "\0"               // ヌル文字攻撃
        };
        
        for (const auto& pattern : dangerousPatterns) {
            if (path.find(pattern) != std::string::npos) {
                return false;
            }
        }
        
        // パスの長さチェック
        if (path.length() > MAX_PATH) {
            return false;
        }
        
        // 制御文字チェック
        for (char c : path) {
            if (c >= 0 && c < 32) {  // ASCII制御文字
                return false;
            }
        }
        
        return true;
    }
    
    bool DetectPathTraversalAttack(const std::string& normalizedPath) {
        // 正規化後でも危険なパターンが残っていないかチェック
        static const std::vector<std::string> traversalPatterns = {
            "/../",
            "\\..\\",
            "%2e%2e%2f",      // URLエンコードされた "../"
            "%2e%2e%5c",      // URLエンコードされた "..\"
            "..%2f",          // 部分URLエンコード
            "..%5c"           // 部分URLエンコード
        };
        
        std::string lowerPath = ToLowerCase(normalizedPath);
        for (const auto& pattern : traversalPatterns) {
            if (lowerPath.find(ToLowerCase(pattern)) != std::string::npos) {
                return true;
            }
        }
        
        return false;
    }
    
    bool CheckSandboxAccess(const std::string& filePath, const SecurityContext& context) {
        if (context.processName.empty()) {
            return true; // プロセス名が不明な場合は通常の制御に任せる
        }
        
        std::shared_lock<std::shared_mutex> lock(m_policiesMutex);
        
        auto it = m_sandboxPaths.find(context.processName);
        if (it == m_sandboxPaths.end()) {
            return true; // サンドボックスが設定されていない場合は許可
        }
        
        // 許可されたパスのいずれかに含まれるかチェック
        for (const auto& allowedPath : it->second) {
            if (filePath.find(allowedPath) == 0) {
                return true;
            }
        }
        
        return false;
    }
    
    AccessDecision EvaluateAccessPolicies(const std::string& filePath, FileAccessType accessType, 
                                         const SecurityContext& context) {
        std::shared_lock<std::shared_mutex> lock(m_policiesMutex);
        
        // 優先度順でポリシーを評価
        for (const auto& policy : m_accessPolicies) {
            if (PathMatchesPattern(filePath, policy.pathPattern)) {
                // プロセス名チェック
                if (!policy.trustedProcesses.empty()) {
                    bool processMatched = false;
                    for (const auto& trustedProcess : policy.trustedProcesses) {
                        if (context.processName == trustedProcess) {
                            processMatched = true;
                            break;
                        }
                    }
                    if (!processMatched) {
                        continue; // このポリシーは適用されない
                    }
                }
                
                // 明示的に拒否されているアクセスタイプかチェック
                if (std::find(policy.deniedAccesses.begin(), policy.deniedAccesses.end(), accessType) 
                    != policy.deniedAccesses.end()) {
                    return AccessDecision::Deny;
                }
                
                // 明示的に許可されているアクセスタイプかチェック
                if (std::find(policy.allowedAccesses.begin(), policy.allowedAccesses.end(), accessType) 
                    != policy.allowedAccesses.end()) {
                    return AccessDecision::Allow;
                }
                
                // デフォルト判定を返す
                return policy.defaultDecision;
            }
        }
        
        // どのポリシーにもマッチしない場合はデフォルト拒否
        return AccessDecision::Deny;
    }
    
    void LogAccessAttempt(const std::string& filePath, FileAccessType accessType, 
                         AccessDecision decision, const std::string& reason,
                         const SecurityContext& context) {
        if (!m_auditEnabled) return;
        
        std::lock_guard<std::mutex> lock(m_auditMutex);
        
        AccessAuditLog log;
        log.timestamp = std::chrono::system_clock::now();
        log.filePath = filePath;
        log.accessType = accessType;
        log.decision = decision;
        log.processName = context.processName;
        log.processId = context.processId;
        log.userName = context.userName;
        log.reason = reason;
        log.additionalContext = context.additionalData;
        
        m_auditLogs.push_back(log);
        
        // ログサイズ制限
        if (m_auditLogs.size() > MAX_AUDIT_LOG_SIZE) {
            m_auditLogs.pop_front();
        }
    }
    
    bool PathMatchesPattern(const std::string& path, const std::string& pattern) {
        // 単純なワイルドカードマッチング実装
        return WildcardMatch(path, pattern);
    }
    
    bool WildcardMatch(const std::string& text, const std::string& pattern) {
        size_t textLen = text.length();
        size_t patternLen = pattern.length();
        
        // DP テーブルを使用したワイルドカードマッチング
        std::vector<std::vector<bool>> dp(textLen + 1, std::vector<bool>(patternLen + 1, false));
        
        dp[0][0] = true;
        
        // "*" のみのパターンを処理
        for (size_t j = 1; j <= patternLen; ++j) {
            if (pattern[j-1] == '*') {
                dp[0][j] = dp[0][j-1];
            }
        }
        
        for (size_t i = 1; i <= textLen; ++i) {
            for (size_t j = 1; j <= patternLen; ++j) {
                if (pattern[j-1] == '*') {
                    dp[i][j] = dp[i][j-1] || dp[i-1][j];
                } else if (pattern[j-1] == '?' || pattern[j-1] == text[i-1]) {
                    dp[i][j] = dp[i-1][j-1];
                }
            }
        }
        
        return dp[textLen][patternLen];
    }
};
```

### 4. DLLハイジャック攻撃防止対策 (NFR-103)

**目的**: DLLサーチパス攻撃とDLL植え込み攻撃を防止し、安全なDLL読み込みを保証

**機能要件**:
- セキュアDLLロードパス検証
- DLL整合性チェック
- プリロードDLL検出
- DLLサーチパス操作防止
- 実行時DLL検証
- 動的DLL注入防止

**実装仕様**:
```cpp
enum class DLLHijackingThreatType {
    SearchPathHijacking,    // DLLサーチパス攻撃
    DLLPlanting,           // DLL植え込み攻撃
    DLLPreloading,         // DLLプリロード攻撃
    DLLInjection,          // DLL注入攻撃
    SideBySideHijacking,   // Side-by-Side攻撃
    ManifestHijacking      // マニフェスト攻撃
};

struct DLLSecurityInfo {
    std::string dllPath;
    std::string expectedPath;
    std::string actualLoadedFrom;
    bool isSecureLocation = false;
    bool hasValidSignature = false;
    bool matchesExpectedHash = false;
    std::string fileHash;
    std::string expectedHash;
    DLLHijackingThreatType detectedThreat = DLLHijackingThreatType::SearchPathHijacking;
};

struct SecureLoadConfiguration {
    bool enforceFullPath = true;           // フルパス指定を強制
    bool restrictSearchPath = true;        // DLLサーチパスを制限
    bool validateDLLIntegrity = true;      // DLL整合性を検証
    bool requireSignedDLLs = true;         // 署名済みDLL必須
    bool detectPreloadedDLLs = true;       // プリロードDLL検出
    std::vector<std::string> trustedPaths; // 信頼できるロードパス
    std::vector<std::string> blockedPaths; // ブロック対象パス
};

class DLLHijackingPrevention {
private:
    SecureLoadConfiguration m_config;
    std::map<std::string, std::string> m_expectedDLLHashes; // DLL名 -> 期待値ハッシュ
    std::set<std::string> m_trustedDirectories;
    std::set<HMODULE> m_securelyLoadedModules;
    mutable std::mutex m_preventionMutex;
    
public:
    bool ValidateDLLLoadPath(const std::string& dllPath) {
        std::lock_guard<std::mutex> lock(m_preventionMutex);
        
        // パス正規化
        std::string normalizedPath = GetFullPath(dllPath);
        if (normalizedPath.empty()) {
            return false;
        }
        
        // フルパス指定チェック
        if (m_config.enforceFullPath && !IsAbsolutePath(normalizedPath)) {
            return false;
        }
        
        // 信頼できるディレクトリからの読み込みかチェック
        if (!IsFromTrustedDirectory(normalizedPath)) {
            return false;
        }
        
        // ブロックされたパスからの読み込みでないかチェック
        if (IsFromBlockedDirectory(normalizedPath)) {
            return false;
        }
        
        return true;
    }
    
    bool CheckDLLIntegrity(const std::string& dllPath) {
        if (!m_config.validateDLLIntegrity) {
            return true;
        }
        
        std::string dllName = GetFileName(dllPath);
        auto expectedHashIt = m_expectedDLLHashes.find(dllName);
        
        if (expectedHashIt == m_expectedDLLHashes.end()) {
            // 期待値ハッシュが登録されていない場合
            if (m_config.requireSignedDLLs) {
                // 署名チェックで代替
                return VerifyDLLSignature(dllPath);
            }
            return true; // ハッシュチェック不要
        }
        
        // ファイルハッシュ計算
        std::string actualHash = CalculateFileHash(dllPath, HashAlgorithm::SHA256);
        return (actualHash == expectedHashIt->second);
    }
    
    bool DetectDLLHijackingAttempt(const std::string& requestedDLL, const std::string& actualLoadedPath) {
        std::lock_guard<std::mutex> lock(m_preventionMutex);
        
        // 期待されるロードパス
        std::vector<std::string> expectedPaths = GetExpectedLoadPaths(requestedDLL);
        
        // 実際のロードパスが期待されるパスの中にあるかチェック
        for (const auto& expectedPath : expectedPaths) {
            if (NormalizePath(actualLoadedPath) == NormalizePath(expectedPath)) {
                return false; // 正当なロード
            }
        }
        
        // システムディレクトリからのロードかチェック
        if (IsSystemDirectory(GetDirectoryPath(actualLoadedPath))) {
            return false; // システムDLLは安全
        }
        
        // アプリケーションディレクトリからのロードかチェック
        if (IsApplicationDirectory(GetDirectoryPath(actualLoadedPath))) {
            return false; // アプリケーションDLLは安全（設定次第）
        }
        
        // 予期しない場所からのロードを検出
        LogSecurityEvent(SecurityEventType::DLLHijackingAttempt, 
                        "Unexpected DLL load location detected",
                        {{"requested", requestedDLL}, {"actual", actualLoadedPath}});
        
        return true; // DLLハイジャック攻撃の可能性
    }
    
    HMODULE SecureLoadLibrary(const std::string& dllPath) {
        std::lock_guard<std::mutex> lock(m_preventionMutex);
        
        // 事前検証
        if (!ValidateDLLLoadPath(dllPath)) {
            SetLastError(ERROR_ACCESS_DENIED);
            return nullptr;
        }
        
        if (!CheckDLLIntegrity(dllPath)) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
            return nullptr;
        }
        
        // セキュアロードフラグでDLLを読み込み
        DWORD loadFlags = 0;
        if (m_config.restrictSearchPath) {
            loadFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32;
            loadFlags |= LOAD_LIBRARY_SEARCH_USER_DIRS;
        }
        
        std::wstring wideDllPath = ConvertToWideString(dllPath);
        HMODULE hModule = LoadLibraryExW(wideDllPath.c_str(), nullptr, loadFlags);
        
        if (hModule != nullptr) {
            // ロード後検証
            std::string actualLoadedPath = GetModuleFileName(hModule);
            if (DetectDLLHijackingAttempt(dllPath, actualLoadedPath)) {
                FreeLibrary(hModule);
                SetLastError(ERROR_ACCESS_DENIED);
                return nullptr;
            }
            
            // 成功時は追跡リストに追加
            m_securelyLoadedModules.insert(hModule);
        }
        
        return hModule;
    }
    
    bool SecureFreeLibrary(HMODULE hModule) {
        std::lock_guard<std::mutex> lock(m_preventionMutex);
        
        // セキュアロードされたモジュールかチェック
        auto it = m_securelyLoadedModules.find(hModule);
        if (it == m_securelyLoadedModules.end()) {
            // 追跡対象外のモジュール
            return FreeLibrary(hModule) != 0;
        }
        
        // アンロード実行
        BOOL result = FreeLibrary(hModule);
        if (result) {
            m_securelyLoadedModules.erase(it);
        }
        
        return result != 0;
    }
    
    bool DetectPreloadedDLLs() {
        if (!m_config.detectPreloadedDLLs) {
            return false;
        }
        
        // プロセスにロードされている全モジュールを列挙
        HANDLE hProcess = GetCurrentProcess();
        HMODULE hModules[1024];
        DWORD cbNeeded;
        
        if (!EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
            return false;
        }
        
        DWORD moduleCount = cbNeeded / sizeof(HMODULE);
        bool suspiciousModulesFound = false;
        
        for (DWORD i = 0; i < moduleCount; ++i) {
            std::string modulePath = GetModuleFileName(hModules[i]);
            std::string moduleName = GetFileName(modulePath);
            
            // 予期しないDLLがプリロードされていないかチェック
            if (IsSuspiciousPreloadedDLL(modulePath, moduleName)) {
                LogSecurityEvent(SecurityEventType::SuspiciousPreloadedDLL,
                               "Suspicious preloaded DLL detected",
                               {{"module", modulePath}});
                suspiciousModulesFound = true;
            }
        }
        
        return suspiciousModulesFound;
    }
    
private:
    std::vector<std::string> GetExpectedLoadPaths(const std::string& dllName) {
        std::vector<std::string> paths;
        
        // システムディレクトリ
        char systemDir[MAX_PATH];
        if (GetSystemDirectoryA(systemDir, sizeof(systemDir)) > 0) {
            paths.push_back(std::string(systemDir) + "\\" + dllName);
        }
        
        // Windows ディレクトリ
        char windowsDir[MAX_PATH];
        if (GetWindowsDirectoryA(windowsDir, sizeof(windowsDir)) > 0) {
            paths.push_back(std::string(windowsDir) + "\\" + dllName);
        }
        
        // アプリケーションディレクトリ
        char appDir[MAX_PATH];
        if (GetModuleFileNameA(nullptr, appDir, sizeof(appDir)) > 0) {
            std::string appPath(appDir);
            size_t lastSlash = appPath.find_last_of("\\");
            if (lastSlash != std::string::npos) {
                paths.push_back(appPath.substr(0, lastSlash + 1) + dllName);
            }
        }
        
        // 信頼できる追加パス
        for (const auto& trustedPath : m_config.trustedPaths) {
            paths.push_back(trustedPath + "\\" + dllName);
        }
        
        return paths;
    }
    
    bool IsFromTrustedDirectory(const std::string& filePath) {
        std::string directory = GetDirectoryPath(filePath);
        
        // システムディレクトリチェック
        if (IsSystemDirectory(directory)) {
            return true;
        }
        
        // 設定された信頼ディレクトリチェック
        for (const auto& trustedDir : m_trustedDirectories) {
            if (directory.find(trustedDir) == 0) {
                return true;
            }
        }
        
        return false;
    }
    
    bool IsFromBlockedDirectory(const std::string& filePath) {
        std::string directory = GetDirectoryPath(filePath);
        
        for (const auto& blockedPath : m_config.blockedPaths) {
            if (directory.find(blockedPath) == 0) {
                return true;
            }
        }
        
        return false;
    }
    
    bool IsSuspiciousPreloadedDLL(const std::string& modulePath, const std::string& moduleName) {
        // 既知の危険なDLLパターンチェック
        static const std::vector<std::string> suspiciousPatterns = {
            "hook",
            "inject",
            "detour",
            "proxy",
            "fake",
            "malware"
        };
        
        std::string lowerModuleName = ToLowerCase(moduleName);
        for (const auto& pattern : suspiciousPatterns) {
            if (lowerModuleName.find(pattern) != std::string::npos) {
                return true;
            }
        }
        
        // 一時ディレクトリからのロード
        std::string lowerPath = ToLowerCase(modulePath);
        if (lowerPath.find("\\temp\\") != std::string::npos ||
            lowerPath.find("\\tmp\\") != std::string::npos) {
            return true;
        }
        
        return false;
    }
    
    std::string CalculateFileHash(const std::string& filePath, HashAlgorithm algorithm) {
        // ファイルハッシュ計算の実装
        HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            return "";
        }
        
        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        DWORD algId = (algorithm == HashAlgorithm::SHA256) ? CALG_SHA_256 : CALG_SHA1;
        
        try {
            if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
                throw std::runtime_error("CryptAcquireContext failed");
            }
            
            if (!CryptCreateHash(hProv, algId, 0, 0, &hHash)) {
                throw std::runtime_error("CryptCreateHash failed");
            }
            
            constexpr DWORD BUFFER_SIZE = 8192;
            BYTE buffer[BUFFER_SIZE];
            DWORD bytesRead;
            
            while (ReadFile(hFile, buffer, BUFFER_SIZE, &bytesRead, nullptr) && bytesRead > 0) {
                if (!CryptHashData(hHash, buffer, bytesRead, 0)) {
                    throw std::runtime_error("CryptHashData failed");
                }
            }
            
            DWORD hashSize = 0;
            DWORD hashSizeLen = sizeof(DWORD);
            if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashSize, &hashSizeLen, 0)) {
                throw std::runtime_error("CryptGetHashParam failed");
            }
            
            std::vector<BYTE> hashBytes(hashSize);
            if (!CryptGetHashParam(hHash, HP_HASHVAL, hashBytes.data(), &hashSize, 0)) {
                throw std::runtime_error("CryptGetHashParam failed");
            }
            
            // バイトを16進文字列に変換
            std::stringstream ss;
            for (DWORD i = 0; i < hashSize; ++i) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hashBytes[i]);
            }
            
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            
            return ss.str();
        }
        catch (...) {
            if (hHash) CryptDestroyHash(hHash);
            if (hProv) CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            return "";
        }
    }
};
```

### 5. TASK-101 プラグインマネージャー統合

**統合要件**:
```cpp
class SecurityIntegratedPluginManager : public PluginManager {
private:
    std::unique_ptr<SecurityManager> m_securityManager;
    
public:
    bool Initialize(const std::string& pluginDirectory = "plugins") override {
        // セキュリティマネージャーの初期化
        m_securityManager = std::make_unique<SecurityManager>();
        if (!m_securityManager->Initialize()) {
            return false;
        }
        
        // セキュリティポリシーの設定
        SetupSecurityPolicies();
        
        return PluginManager::Initialize(pluginDirectory);
    }
    
    bool LoadPlugin(const std::string& pluginPath) override {
        // セキュリティ検証
        if (!m_securityManager->ValidatePlugin(pluginPath, CreateSecurityContext())) {
            LogSecurityEvent(SecurityEventType::PluginLoadDenied, "Plugin failed security validation", 
                           {{"plugin", pluginPath}});
            return false;
        }
        
        // DLL署名検証
        if (!m_securityManager->VerifyDLLSignature(pluginPath)) {
            LogSecurityEvent(SecurityEventType::InvalidSignature, "Plugin signature verification failed",
                           {{"plugin", pluginPath}});
            return false;
        }
        
        return PluginManager::LoadPlugin(pluginPath);
    }
    
private:
    void SetupSecurityPolicies() {
        // プラグイン用ファイルアクセスポリシー
        FileAccessPolicy pluginPolicy;
        pluginPolicy.pathPattern = GetApplicationDirectory() + "/plugins/*";
        pluginPolicy.allowedAccesses = {FileAccessType::Read, FileAccessType::Execute};
        pluginPolicy.defaultDecision = AccessDecision::Allow;
        
        m_securityManager->SetFileAccessPolicy("plugin_access", pluginPolicy);
        
        // データディレクトリアクセス許可
        FileAccessPolicy dataPolicy;
        dataPolicy.pathPattern = GetApplicationDirectory() + "/data/*";
        dataPolicy.allowedAccesses = {FileAccessType::Read, FileAccessType::Write, FileAccessType::Create};
        dataPolicy.defaultDecision = AccessDecision::AuditAndAllow;
        
        m_securityManager->SetFileAccessPolicy("data_access", dataPolicy);
        
        // システムファイルアクセス拒否
        FileAccessPolicy systemPolicy;
        systemPolicy.pathPattern = "C:/Windows/*";
        systemPolicy.deniedAccesses = {FileAccessType::Write, FileAccessType::Delete, FileAccessType::Create, FileAccessType::Modify};
        systemPolicy.defaultDecision = AccessDecision::AuditAndDeny;
        
        m_securityManager->SetFileAccessPolicy("system_protection", systemPolicy);
    }
};
```

## 品質要件

### パフォーマンス要件
- **DLL署名検証時間**: 単一DLLあたり500ms以内
- **ファイルアクセス制御判定**: アクセス判定あたり10ms以内  
- **セキュリティログ書き込み**: ログエントリあたり5ms以内
- **DLLハイジャック検出**: プロセス起動時5秒以内
- **メモリ使用量**: セキュリティマネージャー全体で50MB以内

### 信頼性要件
- **セキュリティポリシー整合性**: ポリシー変更時の無矛盾性保証
- **ログの永続性**: システムクラッシュ時でもセキュリティログ保持
- **フォールセーフ**: セキュリティシステム障害時の安全な動作継続
- **回復力**: 攻撃試行時のシステム安定性維持
- **データ整合性**: セキュリティ関連データの破損検出・修復

### セキュリティ要件
- **多重防御**: 複数の独立したセキュリティ層による保護
- **最小権限原則**: 必要最小限の権限のみ付与
- **監査証跡**: 全セキュリティ関連活動の記録
- **改ざん検出**: セキュリティ設定・ログの改ざん検出
- **暗号化**: 機密性の高いセキュリティデータの暗号化保存

## 受け入れ基準

### セキュリティテスト
- [ ] 署名のないDLLが適切に拒否される
- [ ] 改ざんされたDLLの検出ができる
- [ ] 信頼できない発行者からのDLLが拒否される
- [ ] 期限切れ証明書のDLLが拒否される
- [ ] 失効された証明書のDLLが拒否される

### ファイルアクセス制御テスト
- [ ] システムディレクトリへの書き込みが拒否される
- [ ] パストラバーサル攻撃が防がれる
- [ ] 許可されたディレクトリへのアクセスが正常動作する
- [ ] サンドボックス制限が適切に機能する
- [ ] ファイルアクセスログが正確に記録される

### DLLハイジャック防止テスト  
- [ ] カレントディレクトリからの不正DLL読み込みが防がれる
- [ ] DLLサーチパス攻撃が検出される
- [ ] 予期しない場所からのDLL読み込みが警告される
- [ ] プリロードDLLの検出ができる
- [ ] セキュアロード機能が正常動作する

### 統合テスト
- [ ] プラグインマネージャーとの連携が正常動作する
- [ ] セキュリティポリシーの動的変更ができる
- [ ] セキュリティイベントの通知が正常動作する
- [ ] 複数セキュリティ機能の同時動作が安定している
- [ ] セキュリティログの永続化が正常動作する

### パフォーマンステスト
- [ ] DLL署名検証が500ms以内で完了する
- [ ] ファイルアクセス判定が10ms以内で完了する
- [ ] 32個プラグイン同時検証が10秒以内で完了する
- [ ] セキュリティログ書き込みが5ms以内で完了する
- [ ] メモリ使用量が50MB以下を維持する

### エラーハンドリングテスト
- [ ] 署名検証失敗時に適切なエラーメッセージが表示される
- [ ] 不正アクセス試行が記録される
- [ ] セキュリティシステム障害時の適切なフォールバック
- [ ] 攻撃検出時の適切な対応処理
- [ ] セキュリティログ満杯時の適切な処理

## 実装制約

### 技術制約
- Windows 10/11 Authenticode APIの使用必須
- C++20標準準拠の実装
- Visual Studio 2019/2022でのコンパイル必須  
- TASK-101プラグインマネージャーとの統合必須
- マルチスレッド安全性の確保必須

### 設計制約
- 既存セキュリティ機能との互換性維持
- パフォーマンス影響の最小化
- 設定変更時の動的反映対応
- ログローテーション機能の実装
- セキュリティポリシーの階層化サポート

### セキュリティ制約
- セキュリティ設定の暗号化保存
- セキュリティログの改ざん防止
- 管理者権限での実行時のみ設定変更許可
- セキュリティクリティカルな処理の監査ログ必須
- 攻撃試行時の自動防御機能

## 依存関係

### 内部依存
- TASK-101: プラグインマネージャー（統合連携）
- TASK-002: ServiceLocator（ログサービス連携）
- 既存のプラグインインターフェース（interfaces.h）

### 外部依存
- Windows Crypto API (Authenticode署名検証)
- Windows Security API (ファイルアクセス制御)
- Windows Process API (プロセス・モジュール列挙)
- C++20 Standard Library (暗号化、並行処理)

## 実装順序

1. **SecurityManager基本クラス実装** - コアセキュリティ管理機能
2. **DigitalSignatureVerifier実装** - DLL署名検証システム  
3. **FileAccessController実装** - ファイルアクセス制御システム
4. **DLLHijackingPrevention実装** - DLLハイジャック防止システム
5. **セキュリティポリシー管理機能** - 動的ポリシー制御
6. **セキュリティ監査・ログシステム** - 包括的ログ機能
7. **TASK-101プラグインマネージャー統合** - セキュリティ統合
8. **脅威検出・対応システム** - 高度セキュリティ機能
9. **パフォーマンス最適化** - 速度・メモリ最適化
10. **統合テスト・セキュリティ検証** - 包括的セキュリティテスト

## セキュリティ考慮事項

### 攻撃シナリオと対策

**DLLハイジャック攻撃**:
- 攻撃手法: 悪意のあるDLLを正規DLLより先に読み込ませる
- 対策: セキュアロードパス、DLL整合性検証、プリロード検出

**署名回避攻撃**:
- 攻撃手法: 署名検証を迂回または偽装する  
- 対策: 証明書チェーン検証、CRL確認、タイムスタンプ検証

**権限昇格攻撃**:
- 攻撃手法: ファイルシステム脆弱性を利用した権限昇格
- 対策: 最小権限原則、サンドボックス、アクセス制御

**データ改ざん攻撃**:
- 攻撃手法: セキュリティ設定やログの改ざん
- 対策: 整合性チェック、暗号化保存、改ざん検出

この要件定義に基づき、TDD方式での段階的実装により、包括的で高性能なセキュリティマネージャーシステムを実現します。