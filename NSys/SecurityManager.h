#pragma once

#include "interfaces.h"
#include <windows.h>
#include <wintrust.h>
#include <softpub.h>
#include <wincrypt.h>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <functional>
#include <chrono>
#include <filesystem>
#include <shared_mutex>
#include <atomic>
#include <queue>
#include <deque>

namespace NSys {

// ==================== セキュリティ関連型定義 ====================

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

enum class SecurityEventType {
    DLLSignatureVerificationFailed,
    FileAccessDenied,
    DLLHijackingAttempt,
    SuspiciousPreloadedDLL,
    PluginLoadDenied,
    InvalidSignature,
    PathTraversalAttempt,
    SandboxViolation,
    SecurityPolicyViolation,
    SystemIntegrityViolation
};

enum class SecurityStatus {
    Uninitialized,
    Initializing,
    Active,
    Error,
    Disabled
};

enum class ThreatLevel {
    None,
    Low,
    Medium,
    High,
    Critical
};

enum class DLLHijackingThreatType {
    SearchPathHijacking,
    DLLPlanting,
    DLLPreloading,
    DLLInjection,
    SideBySideHijacking,
    ManifestHijacking
};

enum class SignatureAlgorithm {
    SHA1RSA,
    SHA256RSA,
    SHA384RSA,
    SHA512RSA
};

enum class HashAlgorithm {
    SHA1,
    SHA256,
    SHA384,
    SHA512
};

// ==================== セキュリティ構造体定義 ====================

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

struct FileAccessPolicy {
    std::string pathPattern;
    std::vector<FileAccessType> allowedAccesses;
    std::vector<FileAccessType> deniedAccesses;
    std::vector<std::string> trustedProcesses;
    bool inheritFromParent = true;
    bool recursiveApply = true;
    AccessDecision defaultDecision = AccessDecision::Deny;
    uint32_t priority = 0;
};

struct SecurityConfiguration {
    bool enableDLLSignatureVerification = true;
    bool enableFileAccessControl = true;
    bool enableDLLHijackingPrevention = true;
    bool enableSecurityLogging = true;
    bool enableRealTimeMonitoring = true;
    SignaturePolicy defaultSignaturePolicy;
    std::string securityLogPath = "security.log";
    uint32_t maxLogEntries = 10000;
    uint32_t logRotationSizeMB = 100;
    
    static SecurityConfiguration Default() {
        return SecurityConfiguration{};
    }
};

struct SecurityContext {
    std::string processName;
    std::string processId;
    std::string userName;
    std::string requestSource;
    std::map<std::string, std::string> additionalData;
};

struct PluginSecurityContext {
    std::string pluginPath;
    std::string pluginName;
    PluginInfo pluginInfo;
    SecurityContext securityContext;
};

struct PluginBehaviorContext {
    std::string pluginName;
    std::string operation;
    std::map<std::string, std::string> operationParams;
    SecurityContext securityContext;
};

struct OperationContext {
    std::string operation;
    std::string requestedBy;
    std::map<std::string, std::string> parameters;
    SecurityContext securityContext;
};

struct ActivityContext {
    std::string activityType;
    std::string sourceProcess;
    std::string targetResource;
    std::map<std::string, std::string> activityData;
    std::chrono::system_clock::time_point timestamp;
};

struct SecurityEvent {
    SecurityEventType eventType;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    ThreatLevel threatLevel;
    std::map<std::string, std::string> context;
    std::string sourceComponent;
};

struct SecurityThreat {
    std::string threatId;
    ThreatLevel level;
    std::string description;
    std::chrono::system_clock::time_point firstDetected;
    std::chrono::system_clock::time_point lastActivity;
    bool isActive;
    std::vector<SecurityEvent> relatedEvents;
};

struct SecurityStatistics {
    uint64_t totalSecurityEvents = 0;
    uint64_t blockedOperations = 0;
    uint64_t signatureVerifications = 0;
    uint64_t fileAccessChecks = 0;
    uint64_t dllHijackingAttempts = 0;
    uint64_t activeThreats = 0;
    std::chrono::system_clock::time_point lastUpdate;
};

struct Permission {
    std::string operation;
    bool granted;
    std::chrono::system_clock::time_point grantedAt;
    std::chrono::system_clock::time_point expiresAt;
    std::string grantedBy;
};

struct SecurityLogFilter {
    std::vector<SecurityEventType> eventTypes;
    ThreatLevel minThreatLevel = ThreatLevel::None;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    std::string sourceComponent;
    std::string searchText;
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

// ==================== インターフェース定義 ====================

class ISecurityEventHandler {
public:
    virtual ~ISecurityEventHandler() = default;
    virtual void OnSecurityEvent(const SecurityEvent& event) = 0;
    virtual void OnThreatDetected(const SecurityThreat& threat) = 0;
    virtual void OnSecurityStatusChanged(SecurityStatus oldStatus, SecurityStatus newStatus) = 0;
};

class IPluginSecurityHandler {
public:
    virtual ~IPluginSecurityHandler() = default;
    virtual bool OnPluginLoadAttempt(const PluginSecurityContext& context) = 0;
    virtual void OnPluginBehaviorAlert(const PluginBehaviorContext& context) = 0;
    virtual void OnPluginSecurityViolation(const std::string& pluginName, const SecurityEvent& event) = 0;
};

// ==================== SecurityManager クラス ====================

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
    SignatureInfo GetSignatureInfo(const std::string& dllPath);
    SignatureVerificationResult GetLastVerificationResult() const;
    
    // ファイルアクセス制御
    bool CheckFileAccess(const std::string& filePath, FileAccessType accessType);
    bool CheckFileAccess(const std::string& filePath, FileAccessType accessType, const std::string& requestorContext);
    bool SetFileAccessPolicy(const std::string& pattern, const FileAccessPolicy& policy);
    bool RemoveFileAccessPolicy(const std::string& pattern);
    std::vector<FileAccessPolicy> GetActiveFileAccessPolicies();
    bool ValidateFilePath(const std::string& filePath);
    bool IsPathTraversal(const std::string& filePath);
    
    // DLLハイジャック防止
    bool ValidateDLLLoadPath(const std::string& dllPath);
    bool CheckDLLIntegrity(const std::string& dllPath);
    bool IsSecureLoadPath(const std::string& dllPath);
    bool DetectDLLHijackingAttempt(const std::string& dllPath, const std::string& expectedPath);
    bool DetectPreloadedDLLs();
    HMODULE SecureLoadLibrary(const std::string& dllPath);
    bool SecureFreeLibrary(HMODULE hModule);
    
    // セキュリティポリシー管理
    bool LoadSecurityPolicy(const std::string& policyFile);
    bool SaveSecurityPolicy(const std::string& policyFile);
    bool UpdateSecurityPolicy(const SecurityConfiguration& policy);
    SecurityConfiguration GetCurrentSecurityPolicy();
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
    void UnregisterPluginSecurityHandler(const std::string& pluginName);
    
    // 設定・永続化
    bool LoadConfiguration(const std::string& configFile = "security.ini");
    bool SaveConfiguration(const std::string& configFile = "security.ini");
    void ResetToDefaults();
    
    // エラー・診断
    std::string GetLastError() const;
    bool HasErrors() const;
    void ClearErrors();
    
    // デストラクタ
    ~SecurityManager();

private:
    // 内部状態
    SecurityStatus m_securityStatus = SecurityStatus::Uninitialized;
    SecurityConfiguration m_config;
    std::atomic<bool> m_secureMode{true};
    std::atomic<bool> m_shutdownRequested{false};
    std::string m_lastError;
    
    // 同期プリミティブ
    mutable std::shared_mutex m_securityMutex;
    mutable std::mutex m_logMutex;
    mutable std::mutex m_errorMutex;
    
    // イベントハンドラー
    std::vector<ISecurityEventHandler*> m_eventHandlers;
    std::map<std::string, IPluginSecurityHandler*> m_pluginHandlers;
    
    // 内部データ
    std::vector<FileAccessPolicy> m_accessPolicies;
    std::deque<SecurityEvent> m_securityEvents;
    std::vector<SecurityThreat> m_activeThreats;
    std::map<std::string, Permission> m_permissions;
    SecurityStatistics m_statistics;
    SignatureVerificationResult m_lastVerificationResult = SignatureVerificationResult::VerificationFailed;
    
    // 内部メソッド（実装フェーズで詳細化）
    bool ValidateSignatureInternal(const std::string& dllPath, const SignaturePolicy& policy);
    bool CheckFileAccessInternal(const std::string& filePath, FileAccessType accessType, const SecurityContext& context);
    bool DetectDLLHijackingInternal(const std::string& dllPath);
    void WriteSecurityLog(const SecurityEvent& event);
    void NotifyEventHandlers(const SecurityEvent& event);
    void UpdateStatistics(const SecurityEvent& event);
    std::string NormalizePath(const std::string& path);
    bool IsSystemDirectory(const std::string& path);
    bool IsApplicationDirectory(const std::string& path);
};

} // namespace NSys