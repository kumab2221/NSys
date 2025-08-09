#include "interfaces.h" // <- これが存在しないため失敗

int main() {
    // interfaces.h が存在しないため、コンパイルエラー
    NSys::APIVersion version;
    NSys::ServiceLocator::GetLoggingService();
    return 0;
}