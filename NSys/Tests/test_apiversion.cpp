#include "test_common.h"

using namespace NSys;

class APIVersionTest : public NSysCoreTestBase {};

TEST_F(APIVersionTest, DefaultConstruction) {
    APIVersion version;
    
    // この時点ではAPIVersionは未実装のため、コンパイルエラーが発生する
    EXPECT_EQ(version.major, 1);
    EXPECT_EQ(version.minor, 0);
    EXPECT_EQ(version.patch, 0);
}

TEST_F(APIVersionTest, CustomConstruction) {
    APIVersion version{2, 1, 3};
    
    // 初期化リスト対応も未実装のため失敗
    EXPECT_EQ(version.major, 2);
    EXPECT_EQ(version.minor, 1);
    EXPECT_EQ(version.patch, 3);
}

TEST_F(APIVersionTest, CompatibilityCheck_SameMajorHigherMinor) {
    APIVersion base{1, 2, 0};
    APIVersion compatible{1, 3, 0};
    
    // IsCompatible メソッドが未実装のため失敗
    EXPECT_TRUE(base.IsCompatible(compatible));
}

TEST_F(APIVersionTest, CurrentAPIVersionDefined) {
    // CURRENT_API_VERSION 定数が未定義のため失敗
    EXPECT_EQ(CURRENT_API_VERSION.major, 1);
    EXPECT_EQ(CURRENT_API_VERSION.minor, 0);
    EXPECT_EQ(CURRENT_API_VERSION.patch, 0);
}