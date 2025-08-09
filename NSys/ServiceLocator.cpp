#include "interfaces.h"

namespace NSys {

// ServiceLocator の静的メンバ変数定義（最小実装）
std::unique_ptr<IFileSystemService> ServiceLocator::s_fileSystemService;
std::unique_ptr<ILoggingService> ServiceLocator::s_loggingService;
std::unique_ptr<IMemoryService> ServiceLocator::s_memoryService;
std::unique_ptr<IConfigurationService> ServiceLocator::s_configurationService;
std::unique_ptr<ILocalizationService> ServiceLocator::s_localizationService;
std::unique_ptr<IEventBus> ServiceLocator::s_eventBus;
std::unique_ptr<IPluginManager> ServiceLocator::s_pluginManager;

} // namespace NSys