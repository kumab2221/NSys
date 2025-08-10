#include "test_security_manager_common.h"

namespace NSys {
namespace Testing {

// ==================== UTC-001: SecurityManager初期化テスト ====================

class SecurityManagerBasicTest : public SecurityManagerTestBase {
protected:
    void SetUp() override {
        SecurityManagerTestBase::SetUp();
    }
};

TEST_F(SecurityManagerBasicTest, UTC001_SecurityManagerInitialization) {
    // テストID: UTC-001
    // テスト名: SecurityManager正常初期化
    // テスト目的: SecurityManagerが正常に初期化されることを確認
    
    // 事前条件: SecurityManagerのインスタンスが未作成状態
    ASSERT_FALSE(securityManager->IsSecureModeEnabled());
    ASSERT_EQ(SecurityStatus::Uninitialized, securityManager->GetSecurityStatus());
    
    // テスト手順:
    // 1. SecurityManagerのインスタンスを作成
    // 2. Initialize()メソッドを呼び出し
    SecurityConfiguration defaultConfig = SecurityConfiguration::Default();
    bool initResult = securityManager->Initialize(defaultConfig);
    
    // 3. 戻り値とセキュリティ状態を確認
    // 期待結果:
    // - Initialize()がtrueを返す（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(initResult) << "SecurityManager initialization should succeed";
    
    // - IsSecureModeEnabled()がtrueを返す（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(securityManager->IsSecureModeEnabled()) << "Secure mode should be enabled after initialization";
    
    // - GetSecurityStatus()が初期化済み状態を返す（Red Phase: スタブ実装では Uninitialized を返すので失敗する）
    EXPECT_EQ(SecurityStatus::Active, securityManager->GetSecurityStatus()) << "Security status should be Active after initialization";
    
    // エラーがないことを確認（Red Phase: スタブ実装ではエラーが設定されるので失敗する）
    EXPECT_FALSE(securityManager->HasErrors()) << "No errors should be present after successful initialization";
}

TEST_F(SecurityManagerBasicTest, UTC002_SecurityManagerCustomConfigInitialization) {
    // テストID: UTC-002
    // テスト名: カスタム設定での初期化
    // テスト目的: カスタムセキュリティ設定での初期化が正常動作することを確認
    
    // 事前条件:
    // - 有効なセキュリティ設定ファイル(test_security.ini)が存在
    // - SecurityManagerが未初期化状態
    ASSERT_EQ(SecurityStatus::Uninitialized, securityManager->GetSecurityStatus());
    
    // テスト手順:
    // 1. SecurityConfigurationオブジェクトを作成し、カスタム設定を設定
    SecurityConfiguration customConfig;
    customConfig.enableDLLSignatureVerification = true;
    customConfig.enableFileAccessControl = true;
    customConfig.enableDLLHijackingPrevention = true;
    customConfig.enableSecurityLogging = true;
    customConfig.securityLogPath = "custom_security.log";
    customConfig.maxLogEntries = 5000;
    
    // 2. Initialize(customConfig)を呼び出し
    bool initResult = securityManager->Initialize(customConfig);
    
    // 3. 設定値が正しく反映されているか確認
    // 期待結果:
    // - Initialize()がtrueを返す（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(initResult) << "SecurityManager initialization with custom config should succeed";
    
    // - カスタム設定値が有効になっている（Red Phase: スタブ実装では設定が反映されないので失敗する）
    SecurityConfiguration currentConfig = securityManager->GetCurrentSecurityPolicy();
    EXPECT_EQ("custom_security.log", currentConfig.securityLogPath) << "Custom security log path should be applied";
    EXPECT_EQ(5000, currentConfig.maxLogEntries) << "Custom max log entries should be applied";
}

TEST_F(SecurityManagerBasicTest, UTC003_SecurityManagerDuplicateInitialization) {
    // テストID: UTC-003
    // テスト名: 重複初期化の適切な処理
    // テスト目的: 既に初期化済みのSecurityManagerの再初期化が適切に処理されることを確認
    
    // 事前条件: SecurityManagerが既に初期化済み
    SecurityConfiguration config = SecurityConfiguration::Default();
    securityManager->Initialize(config); // 最初の初期化（失敗するがテストのため実行）
    
    // テスト手順:
    // 1. 初期化済みSecurityManagerに対してInitialize()を再実行
    bool secondInitResult = securityManager->Initialize(config);
    
    // 2. 戻り値と内部状態を確認
    // 期待結果:
    // - 適切なエラーコードまたは警告が返される（Red Phase: スタブ実装では常にfalseなので期待通り）
    EXPECT_FALSE(secondInitResult) << "Duplicate initialization should return false or appropriate warning";
    
    // - システムが不安定にならない
    EXPECT_NO_THROW({
        securityManager->GetSecurityStatus();
        securityManager->IsSecureModeEnabled();
    }) << "System should remain stable after duplicate initialization attempt";
}

TEST_F(SecurityManagerBasicTest, UTC004_SecurityManagerShutdown) {
    // テストID: UTC-004
    // テスト名: 適切なシャットダウン処理
    // テスト目的: SecurityManagerが適切にシャットダウンされることを確認
    
    // 事前条件: SecurityManagerが初期化済み
    SecurityConfiguration config = SecurityConfiguration::Default();
    securityManager->Initialize(config); // 失敗するがテストのため実行
    
    // テスト手順:
    // 1. Shutdown()メソッドを呼び出し
    EXPECT_NO_THROW({
        securityManager->Shutdown();
    }) << "Shutdown should not throw exceptions";
    
    // 2. 内部リソースが解放されているか確認
    // 3. セキュリティ状態を確認
    // 期待結果:
    // - IsSecureModeEnabled()がfalseを返す（Red Phase: スタブ実装では常にfalseなので期待通り）
    EXPECT_FALSE(securityManager->IsSecureModeEnabled()) << "Secure mode should be disabled after shutdown";
    
    // - SecurityStatusがUninitialized状態になる
    EXPECT_EQ(SecurityStatus::Uninitialized, securityManager->GetSecurityStatus()) << "Security status should be Uninitialized after shutdown";
    
    // - メモリリークが発生しない（デストラクタ呼び出しで確認）
    // この部分は実際のメモリ監視ツールで確認するが、テストでは例外が発生しないことを確認
    EXPECT_NO_THROW({
        securityManager.reset();
        securityManager = std::make_unique<SecurityManager>();
    }) << "Memory cleanup should not cause exceptions";
}

// ==================== 基本的なエラーハンドリングテスト ====================

TEST_F(SecurityManagerBasicTest, ErrorHandling_GetLastError) {
    // エラー情報の取得テスト
    
    // 初期状態ではエラーなし
    EXPECT_FALSE(securityManager->HasErrors()) << "Should have no errors initially";
    EXPECT_TRUE(securityManager->GetLastError().empty()) << "Last error should be empty initially";
    
    // 失敗する操作を実行
    bool result = securityManager->Initialize();
    
    // エラー情報が設定されるか確認（Red Phase: スタブ実装でエラーが設定される）
    EXPECT_TRUE(securityManager->HasErrors()) << "Should have errors after failed operation";
    EXPECT_FALSE(securityManager->GetLastError().empty()) << "Last error should not be empty after failed operation";
    EXPECT_THAT(securityManager->GetLastError(), testing::HasSubstr("not implemented")) << "Error message should indicate implementation status";
    
    // エラークリア
    securityManager->ClearErrors();
    EXPECT_FALSE(securityManager->HasErrors()) << "Should have no errors after clearing";
    EXPECT_TRUE(securityManager->GetLastError().empty()) << "Last error should be empty after clearing";
}

TEST_F(SecurityManagerBasicTest, Configuration_ResetToDefaults) {
    // デフォルト設定へのリセットテスト
    
    // テスト手順: ResetToDefaults()を呼び出し
    EXPECT_NO_THROW({
        securityManager->ResetToDefaults();
    }) << "ResetToDefaults should not throw exceptions";
    
    // 設定がデフォルトに戻るか確認
    SecurityConfiguration config = securityManager->GetCurrentSecurityPolicy();
    SecurityConfiguration defaultConfig = SecurityConfiguration::Default();
    
    EXPECT_EQ(defaultConfig.enableDLLSignatureVerification, config.enableDLLSignatureVerification) << "DLL signature verification should be default";
    EXPECT_EQ(defaultConfig.enableFileAccessControl, config.enableFileAccessControl) << "File access control should be default";
    EXPECT_EQ(defaultConfig.enableDLLHijackingPrevention, config.enableDLLHijackingPrevention) << "DLL hijacking prevention should be default";
    
    // エラーがクリアされるか確認
    EXPECT_FALSE(securityManager->HasErrors()) << "Errors should be cleared after reset";
}

} // namespace Testing
} // namespace NSys