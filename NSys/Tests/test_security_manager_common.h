#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test_common.h"
#include "SecurityManager.h"
#include "PluginManager.h"
#include <filesystem>
#include <fstream>
#include <chrono>

namespace NSys {
namespace Testing {

// ==================== Mock Classes ====================

class MockSecurityEventHandler : public ISecurityEventHandler {
public:
    MOCK_METHOD(void, OnSecurityEvent, (const SecurityEvent& event), (override));
    MOCK_METHOD(void, OnThreatDetected, (const SecurityThreat& threat), (override));
    MOCK_METHOD(void, OnSecurityStatusChanged, (SecurityStatus oldStatus, SecurityStatus newStatus), (override));
};

class MockPluginSecurityHandler : public IPluginSecurityHandler {
public:
    MOCK_METHOD(bool, OnPluginLoadAttempt, (const PluginSecurityContext& context), (override));
    MOCK_METHOD(void, OnPluginBehaviorAlert, (const PluginBehaviorContext& context), (override));
    MOCK_METHOD(void, OnPluginSecurityViolation, (const std::string& pluginName, const SecurityEvent& event), (override));
};

class MockLoggingService : public ILoggingService {
public:
    MOCK_METHOD(void, Log, (LogLevel level, const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Debug, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Info, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Warning, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Error, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, Critical, (const std::string& message, const std::string& category), (override));
    MOCK_METHOD(void, SetLogLevel, (LogLevel level), (override));
    MOCK_METHOD(LogLevel, GetLogLevel, (), (const, override));
    MOCK_METHOD(void, EnableFileLogging, (const std::string& filePath), (override));
    MOCK_METHOD(void, DisableFileLogging, (), (override));
};

// ==================== Test Fixtures ====================

class SecurityManagerTestBase : public NSysCoreTestBase {
protected:
    void SetUp() override {
        NSysCoreTestBase::SetUp();
        
        // テスト用ディレクトリの作成
        CreateTestDirectories();
        
        // テスト用ファイルの作成
        CreateTestFiles();
        
        // Mock services setup
        mockLoggingService = std::make_unique<MockLoggingService>();
        ServiceLocator::RegisterLoggingService(std::unique_ptr<ILoggingService>(mockLoggingService.get()));
        
        // SecurityManagerの初期化
        securityManager = std::make_unique<SecurityManager>();
    }
    
    void TearDown() override {
        // リソースのクリーンアップ
        securityManager.reset();
        CleanupTestFiles();
        CleanupTestDirectories();
        
        NSysCoreTestBase::TearDown();
    }
    
    void CreateTestDirectories() {
        testDataDir = std::filesystem::current_path() / "test_data";
        testPluginsDir = testDataDir / "plugins";
        testSecureDir = testDataDir / "secure";
        testTempDir = testDataDir / "temp";
        
        std::filesystem::create_directories(testDataDir);
        std::filesystem::create_directories(testPluginsDir);
        std::filesystem::create_directories(testSecureDir);
        std::filesystem::create_directories(testTempDir);
    }
    
    void CleanupTestDirectories() {
        if (std::filesystem::exists(testDataDir)) {
            std::filesystem::remove_all(testDataDir);
        }
    }
    
    void CreateTestFiles() {
        // 有効署名DLLファイル（モック）
        validSignedDLL = testPluginsDir / "signed_valid.dll";
        CreateMockDLLFile(validSignedDLL, true, false);
        
        // 無効署名DLLファイル（モック）
        invalidSignedDLL = testPluginsDir / "signed_invalid.dll";
        CreateMockDLLFile(invalidSignedDLL, false, false);
        
        // 署名なしDLLファイル
        unsignedDLL = testPluginsDir / "unsigned.dll";
        CreateMockDLLFile(unsignedDLL, false, true);
        
        // 期限切れ証明書DLLファイル
        expiredCertDLL = testPluginsDir / "expired_cert.dll";
        CreateMockDLLFile(expiredCertDLL, false, false);
        
        // 信頼されない発行者DLLファイル
        untrustedPublisherDLL = testPluginsDir / "untrusted_publisher.dll";
        CreateMockDLLFile(untrustedPublisherDLL, false, false);
        
        // テスト用テキストファイル
        testReadFile = testDataDir / "allowed_read.txt";
        testWriteFile = testDataDir / "protected_write.txt";
        testExecuteFile = testDataDir / "executable_test.exe";
        
        CreateTestTextFile(testReadFile, "Test content for reading");
        CreateTestTextFile(testWriteFile, "Test content that should be protected");
        CreateTestTextFile(testExecuteFile, "Mock executable content");
    }
    
    void CleanupTestFiles() {
        // ファイルクリーンアップは CreateTestDirectories で削除される
    }
    
    void CreateMockDLLFile(const std::filesystem::path& path, bool validSignature, bool noSignature) {
        std::ofstream file(path, std::ios::binary);
        
        // DLLヘッダーのモック（MZヘッダー）
        file.write("MZ", 2);
        
        // ダミーデータ
        std::vector<char> dummyData(1024, 0);
        file.write(dummyData.data(), dummyData.size());
        
        // 署名情報のモック（実際のAuthenticodeではない）
        if (!noSignature) {
            std::string mockSignature = validSignature ? "VALID_SIG" : "INVALID_SIG";
            file.write(mockSignature.c_str(), mockSignature.length());
        }
        
        file.close();
    }
    
    void CreateTestTextFile(const std::filesystem::path& path, const std::string& content) {
        std::ofstream file(path);
        file << content;
        file.close();
    }
    
    // セキュリティコンテキストのヘルパー作成
    SecurityContext CreateTestSecurityContext(const std::string& processName = "test_process") {
        SecurityContext context;
        context.processName = processName;
        context.processId = "1234";
        context.userName = "test_user";
        context.requestSource = "unit_test";
        context.additionalData["test_mode"] = "true";
        return context;
    }
    
    PluginSecurityContext CreateTestPluginSecurityContext(const std::string& pluginPath) {
        PluginSecurityContext context;
        context.pluginPath = pluginPath;
        context.pluginName = std::filesystem::path(pluginPath).stem().string();
        context.pluginInfo.name = context.pluginName;
        context.pluginInfo.version = "1.0.0";
        context.securityContext = CreateTestSecurityContext();
        return context;
    }
    
    // セキュリティポリシーのヘルパー作成
    SignaturePolicy CreateStrictSignaturePolicy() {
        SignaturePolicy policy;
        policy.requireValidSignature = true;
        policy.allowSelfSigned = false;
        policy.requireTimestamp = true;
        policy.checkCertificateRevocation = true;
        policy.requireTrustedPublisher = true;
        policy.trustedPublishers = {"Microsoft Corporation", "Test Trusted Publisher"};
        policy.maxCertificateAge = std::chrono::seconds(365 * 24 * 3600); // 1年
        return policy;
    }
    
    FileAccessPolicy CreateRestrictiveFileAccessPolicy() {
        FileAccessPolicy policy;
        policy.pathPattern = "*";
        policy.allowedAccesses = {FileAccessType::Read};
        policy.deniedAccesses = {FileAccessType::Write, FileAccessType::Delete, FileAccessType::Execute};
        policy.defaultDecision = AccessDecision::Deny;
        policy.priority = 100;
        return policy;
    }
    
protected:
    std::unique_ptr<SecurityManager> securityManager;
    std::unique_ptr<MockLoggingService> mockLoggingService;
    
    // テスト用ディレクトリ
    std::filesystem::path testDataDir;
    std::filesystem::path testPluginsDir;
    std::filesystem::path testSecureDir;
    std::filesystem::path testTempDir;
    
    // テスト用DLLファイル
    std::filesystem::path validSignedDLL;
    std::filesystem::path invalidSignedDLL;
    std::filesystem::path unsignedDLL;
    std::filesystem::path expiredCertDLL;
    std::filesystem::path untrustedPublisherDLL;
    
    // テスト用データファイル
    std::filesystem::path testReadFile;
    std::filesystem::path testWriteFile;
    std::filesystem::path testExecuteFile;
};

// ==================== Test Helpers ====================

class SecurityTestHelpers {
public:
    static void ExpectSecurityEventLogged(MockLoggingService& mockLogger, const std::string& expectedMessage) {
        EXPECT_CALL(mockLogger, Error(testing::HasSubstr(expectedMessage), testing::_))
            .Times(testing::AtLeast(1));
    }
    
    static void ExpectNoSecurityEventLogged(MockLoggingService& mockLogger) {
        EXPECT_CALL(mockLogger, Error(testing::_, testing::_))
            .Times(0);
    }
    
    static std::string GenerateTestCertificateThumbprint() {
        return "1234567890ABCDEF1234567890ABCDEF12345678";
    }
    
    static std::chrono::system_clock::time_point GetFutureTime(int daysFromNow = 365) {
        auto now = std::chrono::system_clock::now();
        return now + std::chrono::hours(24 * daysFromNow);
    }
    
    static std::chrono::system_clock::time_point GetPastTime(int daysAgo = 30) {
        auto now = std::chrono::system_clock::now();
        return now - std::chrono::hours(24 * daysAgo);
    }
    
    static bool IsPathTraversalAttack(const std::string& path) {
        return path.find("..") != std::string::npos || 
               path.find("%2e%2e") != std::string::npos ||
               path.find("\\..\\") != std::string::npos;
    }
    
    static bool IsValidDLLPath(const std::string& path) {
        return std::filesystem::path(path).extension() == ".dll" &&
               std::filesystem::exists(path);
    }
};

// ==================== Test Data Constants ====================

constexpr int TEST_TIMEOUT_MS = 5000;
constexpr int MAX_PLUGINS_FOR_TEST = 5;
constexpr size_t MAX_LOG_ENTRIES_FOR_TEST = 100;

// テスト用の危険なパス
const std::vector<std::string> DANGEROUS_PATHS = {
    "../../../etc/passwd",
    "..\\..\\..\\Windows\\System32\\config\\SAM",
    "%2e%2e%2f%2e%2e%2f%2e%2e%2fsensitive",
    "C:\\Windows\\System32\\kernel32.dll",
    "\\\\?\\C:\\System\\protected_file.txt"
};

// テスト用の安全なパス
const std::vector<std::string> SAFE_PATHS = {
    "test_file.txt",
    "./local_file.dat",
    "data/user_config.ini",
    "plugins/my_plugin.dll"
};

} // namespace Testing
} // namespace NSys