#pragma once

#include "test_common.h"
#include "../PluginManager.h"
#include <windows.h>
#include <memory>
#include <vector>
#include <filesystem>
#include <chrono>
#include <fstream>

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

// モックサービス（テスト用）
class MockLoggingService : public NSys::ILoggingService {
private:
    int m_infoCallCount = 0;
    int m_errorCallCount = 0;
    int m_warningCallCount = 0;
    std::vector<std::string> m_logMessages;
    
public:
    void Log(NSys::LogLevel level, const std::string& message, const std::string& category) override {
        switch (level) {
            case NSys::LogLevel::Info:
                Info(message, category);
                break;
            case NSys::LogLevel::Warning:
                Warning(message, category);
                break;
            case NSys::LogLevel::Error:
                Error(message, category);
                break;
            default:
                break;
        }
    }
    
    void Debug(const std::string& message, const std::string& category) override {
        m_logMessages.push_back("[DEBUG] " + category + ": " + message);
    }
    
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
    
    void Critical(const std::string& message, const std::string& category) override {
        m_errorCallCount++;
        m_logMessages.push_back("[CRITICAL] " + category + ": " + message);
    }
    
    void SetLogLevel(NSys::LogLevel level) override {}
    NSys::LogLevel GetLogLevel() const override { return NSys::LogLevel::Info; }
    void EnableFileLogging(const std::string& filePath) override {}
    void DisableFileLogging() override {}
    
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

// テスト用の基本設定
class PluginManagerTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        // テストディレクトリの準備
        std::filesystem::create_directories("test_plugins");
        
        // ServiceLocator の初期化（テスト用）
        NSys::ServiceLocator::Shutdown(); // 既存のサービスをクリア
        
        // テスト用サービスの登録
        m_mockLoggingService = std::make_unique<MockLoggingService>();
        NSys::ServiceLocator::RegisterService<NSys::ILoggingService>(m_mockLoggingService.get());
    }
    
    void TearDown() override {
        // テストファイルのクリーンアップ
        TestPluginDLLGenerator::CleanupTestFiles();
        
        // ServiceLocator のクリーンアップ
        NSys::ServiceLocator::Shutdown();
        
        // メモリ管理：raw pointerを使っているため、手動でクリア
        m_mockLoggingService.release(); // unique_ptrから解放（ServiceLocatorが削除を担当）
    }
    
    std::unique_ptr<MockLoggingService> m_mockLoggingService;
};