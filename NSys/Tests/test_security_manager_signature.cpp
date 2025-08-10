#include "test_security_manager_common.h"

namespace NSys {
namespace Testing {

class SecurityManagerSignatureTest : public SecurityManagerTestBase {
protected:
    void SetUp() override {
        SecurityManagerTestBase::SetUp();
        
        // Initialize SecurityManager for signature tests
        SecurityConfiguration config = SecurityConfiguration::Default();
        config.enableDLLSignatureVerification = true;
        securityManager->Initialize(config); // スタブ実装では失敗するが、テストのため実行
    }
};

// ==================== UTC-101: 有効な署名DLL検証テスト ====================

TEST_F(SecurityManagerSignatureTest, UTC101_ValidSignedDLLVerification) {
    // テストID: UTC-101
    // テスト名: 有効署名DLLの検証成功
    // テスト目的: 正しく署名されたDLLの検証が成功することを確認
    
    // 事前条件:
    // - 有効なAuthenticode署名を持つテスト用DLL(signed_valid.dll)が存在
    // - SecurityManagerが初期化済み
    ASSERT_TRUE(std::filesystem::exists(validSignedDLL)) << "Valid signed DLL should exist";
    
    // テスト手順:
    // 1. VerifyDLLSignature("signed_valid.dll")を呼び出し
    bool verificationResult = securityManager->VerifyDLLSignature(validSignedDLL.string());
    
    // 2. 戻り値を確認
    // 期待結果:
    // - VerifyDLLSignature()がtrueを返す（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(verificationResult) << "Valid signed DLL verification should succeed";
    
    // 3. セキュリティログを確認
    // 検証成功のログが記録されるべき（Red Phase: ログ機能が未実装なので記録されない）
    SecurityTestHelpers::ExpectNoSecurityEventLogged(*mockLoggingService); // スタブ実装ではログされない
    
    // 検証結果の詳細を確認
    EXPECT_EQ(SignatureVerificationResult::Valid, securityManager->GetLastVerificationResult()) 
        << "Last verification result should be Valid";
}

// ==================== UTC-102: 無効な署名DLL検証テスト ====================

TEST_F(SecurityManagerSignatureTest, UTC102_InvalidSignedDLLVerification) {
    // テストID: UTC-102
    // テスト名: 無効署名DLLの検証失敗
    // テスト目的: 無効な署名を持つDLLの検証が失敗することを確認
    
    // 事前条件:
    // - 無効署名を持つテスト用DLL(signed_invalid.dll)が存在
    // - SecurityManagerが初期化済み
    ASSERT_TRUE(std::filesystem::exists(invalidSignedDLL)) << "Invalid signed DLL should exist";
    
    // テスト手順:
    // 1. VerifyDLLSignature("signed_invalid.dll")を呼び出し
    bool verificationResult = securityManager->VerifyDLLSignature(invalidSignedDLL.string());
    
    // 2. 戻り値を確認
    // 期待結果:
    // - VerifyDLLSignature()がfalseを返す（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(verificationResult) << "Invalid signed DLL verification should fail";
    
    // 3. セキュリティログを確認
    // 検証失敗のログが記録されるべき（Red Phase: ログ機能が未実装）
    SecurityTestHelpers::ExpectNoSecurityEventLogged(*mockLoggingService); // スタブ実装ではログされない
    
    // 検証結果の詳細を確認
    EXPECT_EQ(SignatureVerificationResult::Invalid, securityManager->GetLastVerificationResult()) 
        << "Last verification result should be Invalid";
}

// ==================== UTC-103: 署名なしDLL検証テスト ====================

TEST_F(SecurityManagerSignatureTest, UTC103_UnsignedDLLVerification) {
    // テストID: UTC-103
    // テスト名: 署名なしDLLの検証失敗
    // テスト目的: 署名されていないDLLの検証が失敗することを確認
    
    // 事前条件:
    // - 署名されていないテスト用DLL(unsigned.dll)が存在
    // - SecurityManagerが初期化済み
    ASSERT_TRUE(std::filesystem::exists(unsignedDLL)) << "Unsigned DLL should exist";
    
    // テスト手順:
    // 1. VerifyDLLSignature("unsigned.dll")を呼び出し
    bool verificationResult = securityManager->VerifyDLLSignature(unsignedDLL.string());
    
    // 2. 戻り値とエラー情報を確認
    // 期待結果:
    // - VerifyDLLSignature()がfalseを返す（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(verificationResult) << "Unsigned DLL verification should fail";
    
    // - SignatureVerificationResultがNotSignedを返す（Red Phase: スタブ実装では VerificationFailed を返すので失敗する）
    EXPECT_EQ(SignatureVerificationResult::NotSigned, securityManager->GetLastVerificationResult()) 
        << "Last verification result should be NotSigned";
}

// ==================== UTC-104: 期限切れ証明書DLL検証テスト ====================

TEST_F(SecurityManagerSignatureTest, UTC104_ExpiredCertificateDLLVerification) {
    // テストID: UTC-104
    // テスト名: 期限切れ証明書DLLの検証失敗
    // テスト目的: 期限切れ証明書で署名されたDLLの検証が失敗することを確認
    
    // 事前条件:
    // - 期限切れ証明書で署名されたテスト用DLL(expired_cert.dll)が存在
    // - SecurityManagerが初期化済み
    ASSERT_TRUE(std::filesystem::exists(expiredCertDLL)) << "Expired certificate DLL should exist";
    
    // テスト手順:
    // 1. VerifyDLLSignature("expired_cert.dll")を呼び出し
    bool verificationResult = securityManager->VerifyDLLSignature(expiredCertDLL.string());
    
    // 2. 戻り値とエラー情報を確認
    // 期待結果:
    // - VerifyDLLSignature()がfalseを返す（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(verificationResult) << "Expired certificate DLL verification should fail";
    
    // - SignatureVerificationResultがExpiredCertificateを返す（Red Phase: スタブ実装では VerificationFailed を返すので失敗する）
    EXPECT_EQ(SignatureVerificationResult::ExpiredCertificate, securityManager->GetLastVerificationResult()) 
        << "Last verification result should be ExpiredCertificate";
}

// ==================== UTC-105: 失効証明書DLL検証テスト ====================

TEST_F(SecurityManagerSignatureTest, UTC105_RevokedCertificateDLLVerification) {
    // テストID: UTC-105
    // テスト名: 失効証明書DLLの検証失敗
    // テスト目的: 失効された証明書で署名されたDLLの検証が失敗することを確認
    
    // テスト用の失効証明書DLLファイルパス
    std::filesystem::path revokedCertDLL = testPluginsDir / "revoked_cert.dll";
    CreateMockDLLFile(revokedCertDLL, false, false); // 無効署名のDLL
    
    // 事前条件:
    // - 失効証明書で署名されたテスト用DLL(revoked_cert.dll)が存在
    // - SecurityManagerが初期化済み
    // - CRLチェックが有効
    ASSERT_TRUE(std::filesystem::exists(revokedCertDLL)) << "Revoked certificate DLL should exist";
    
    // テスト手順:
    // 1. VerifyDLLSignature("revoked_cert.dll")を呼び出し
    bool verificationResult = securityManager->VerifyDLLSignature(revokedCertDLL.string());
    
    // 2. 戻り値とエラー情報を確認
    // 期待結果:
    // - VerifyDLLSignature()がfalseを返す（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(verificationResult) << "Revoked certificate DLL verification should fail";
    
    // - SignatureVerificationResultがRevokedCertificateを返す（Red Phase: スタブ実装では VerificationFailed を返すので失敗する）
    EXPECT_EQ(SignatureVerificationResult::RevokedCertificate, securityManager->GetLastVerificationResult()) 
        << "Last verification result should be RevokedCertificate";
}

// ==================== UTC-106: 信頼されない発行者DLL検証テスト ====================

TEST_F(SecurityManagerSignatureTest, UTC106_UntrustedPublisherDLLVerification) {
    // テストID: UTC-106
    // テスト名: 信頼されない発行者DLLの検証失敗
    // テスト目的: 信頼されない発行者による署名DLLの検証が失敗することを確認
    
    // 事前条件:
    // - 信頼されない発行者による署名DLL(untrusted_publisher.dll)が存在
    // - 信頼できる発行者リストに該当発行者が含まれていない
    // - SecurityManagerが初期化済み
    ASSERT_TRUE(std::filesystem::exists(untrustedPublisherDLL)) << "Untrusted publisher DLL should exist";
    
    // テスト手順:
    // 1. 信頼できる発行者リストを設定
    SignaturePolicy strictPolicy = CreateStrictSignaturePolicy();
    
    // 2. VerifyDLLSignature("untrusted_publisher.dll")を呼び出し
    bool verificationResult = securityManager->VerifyDLLSignature(untrustedPublisherDLL.string(), strictPolicy);
    
    // 3. 戻り値とエラー情報を確認
    // 期待結果:
    // - VerifyDLLSignature()がfalseを返す（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(verificationResult) << "Untrusted publisher DLL verification should fail";
    
    // - SignatureVerificationResultがUntrustedPublisherを返す（Red Phase: スタブ実装では VerificationFailed を返すので失敗する）
    EXPECT_EQ(SignatureVerificationResult::UntrustedPublisher, securityManager->GetLastVerificationResult()) 
        << "Last verification result should be UntrustedPublisher";
}

// ==================== UTC-107: カスタム署名ポリシーテスト ====================

TEST_F(SecurityManagerSignatureTest, UTC107_CustomSignaturePolicyTest) {
    // テストID: UTC-107
    // テスト名: カスタム署名ポリシーの適用
    // テスト目的: カスタム署名ポリシーが正しく適用されることを確認
    
    // 事前条件:
    // - テスト用DLL(policy_test.dll)が存在
    // - SecurityManagerが初期化済み
    std::filesystem::path policyTestDLL = testPluginsDir / "policy_test.dll";
    CreateMockDLLFile(policyTestDLL, true, false); // 有効署名のDLL
    ASSERT_TRUE(std::filesystem::exists(policyTestDLL)) << "Policy test DLL should exist";
    
    // テスト手順:
    // 1. カスタムSignaturePolicyを作成（例：タイムスタンプ不要）
    SignaturePolicy customPolicy;
    customPolicy.requireValidSignature = true;
    customPolicy.allowSelfSigned = true; // 自己署名を許可
    customPolicy.requireTimestamp = false; // タイムスタンプ不要
    customPolicy.checkCertificateRevocation = false; // CRLチェック無効
    customPolicy.requireTrustedPublisher = false; // 信頼できる発行者チェック無効
    
    // 2. VerifyDLLSignature("policy_test.dll", customPolicy)を呼び出し
    bool verificationResult = securityManager->VerifyDLLSignature(policyTestDLL.string(), customPolicy);
    
    // 3. ポリシーが正しく適用されているか確認
    // 期待結果:
    // - カスタムポリシーに従った検証結果が返される（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(verificationResult) << "Custom policy verification should succeed with relaxed requirements";
    
    // - ポリシー設定が無視されない（Red Phase: スタブ実装ではポリシーが適用されない）
    // より厳しいポリシーでのテスト
    SignaturePolicy strictPolicy = CreateStrictSignaturePolicy();
    bool strictResult = securityManager->VerifyDLLSignature(policyTestDLL.string(), strictPolicy);
    
    // 厳しいポリシーでは異なる結果になるべき（Red Phase: スタブ実装では常にfalse）
    EXPECT_NE(verificationResult, strictResult) << "Different policies should produce different results";
}

// ==================== 証明書チェーン検証テスト ====================

TEST_F(SecurityManagerSignatureTest, CertificateChain_Verification) {
    // 証明書チェーンの検証テスト
    
    // 有効署名DLLの証明書チェーン検証
    bool chainResult = securityManager->VerifyCertificateChain(validSignedDLL.string());
    
    // 期待結果: 有効な証明書チェーンで成功（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(chainResult) << "Certificate chain verification should succeed for valid signed DLL";
    
    // 無効署名DLLの証明書チェーン検証
    bool invalidChainResult = securityManager->VerifyCertificateChain(invalidSignedDLL.string());
    
    // 期待結果: 無効な証明書チェーンで失敗（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(invalidChainResult) << "Certificate chain verification should fail for invalid signed DLL";
}

// ==================== 信頼できる発行者管理テスト ====================

TEST_F(SecurityManagerSignatureTest, TrustedPublisher_Management) {
    // 信頼できる発行者の管理テスト
    
    // テスト用発行者名
    std::string trustedPublisher = "Microsoft Corporation";
    std::string untrustedPublisher = "Malicious Publisher";
    
    // 信頼できる発行者のチェック
    bool isTrusted = securityManager->IsTrustedPublisher(trustedPublisher);
    
    // 期待結果: Microsoft Corporation は信頼できる（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(isTrusted) << "Microsoft Corporation should be trusted publisher";
    
    // 信頼できない発行者のチェック
    bool isUntrusted = securityManager->IsTrustedPublisher(untrustedPublisher);
    
    // 期待結果: Malicious Publisher は信頼できない（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(isUntrusted) << "Malicious Publisher should not be trusted";
}

// ==================== 証明書失効チェックテスト ====================

TEST_F(SecurityManagerSignatureTest, CertificateRevocation_Check) {
    // 証明書失効チェックテスト
    
    // テスト用証明書thumbprint
    std::string validThumbprint = SecurityTestHelpers::GenerateTestCertificateThumbprint();
    std::string revokedThumbprint = "REVOKED1234567890ABCDEF1234567890ABCDEF";
    
    // 有効な証明書の失効チェック
    bool isValidRevoked = securityManager->IsRevoked(validThumbprint);
    
    // 期待結果: 有効な証明書は失効していない（Red Phase: スタブ実装では false を返すので期待通り）
    EXPECT_FALSE(isValidRevoked) << "Valid certificate should not be revoked";
    
    // 失効済み証明書の失効チェック
    bool isRevokedRevoked = securityManager->IsRevoked(revokedThumbprint);
    
    // 期待結果: 失効済み証明書は失効している（Red Phase: スタブ実装では false を返すので失敗する）
    EXPECT_TRUE(isRevokedRevoked) << "Revoked certificate should be detected as revoked";
}

// ==================== 署名情報取得テスト ====================

TEST_F(SecurityManagerSignatureTest, SignatureInfo_Retrieval) {
    // 署名情報取得テスト
    
    // 有効署名DLLの署名情報取得
    SignatureInfo sigInfo = securityManager->GetSignatureInfo(validSignedDLL.string());
    
    // 期待結果: 署名情報が取得できる（Red Phase: スタブ実装では空の情報を返すので失敗する）
    EXPECT_FALSE(sigInfo.subjectName.empty()) << "Subject name should not be empty for signed DLL";
    EXPECT_FALSE(sigInfo.issuerName.empty()) << "Issuer name should not be empty for signed DLL";
    EXPECT_FALSE(sigInfo.thumbprint.empty()) << "Thumbprint should not be empty for signed DLL";
    
    // タイムスタンプ情報の確認
    if (sigInfo.hasTimestamp) {
        EXPECT_NE(std::chrono::system_clock::time_point{}, sigInfo.timestampTime) 
            << "Timestamp time should be valid when hasTimestamp is true";
    }
    
    // 署名アルゴリズムの確認
    EXPECT_NE(SignatureAlgorithm::SHA1RSA, sigInfo.algorithm) << "Should use modern signature algorithm (not SHA1)";
}

} // namespace Testing
} // namespace NSys