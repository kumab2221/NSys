# NSys アーキテクチャ設計

## システム概要

NSysは、プラグインベースのImGuiアプリケーション開発プラットフォームです。モジュラー設計により、DLL形式のプラグインを動的に読み込み、統一されたGUIフレームワークで多様な機能を提供します。

## アーキテクチャパターン

### 採用パターン: プラグインアーキテクチャ + レイヤードアーキテクチャ

- **パターン**: プラグインアーキテクチャをベースとしたモジュラーシステム
- **理由**: 
  - 機能の独立性と拡張性を確保
  - 各プラグインの個別開発・テスト・デプロイが可能
  - サードパーティ開発者による拡張を支援
  - コアシステムとプラグインの分離によるメンテナンス性向上

### セキュリティモデル

- **サンドボックス**: プラグイン間の分離と制御
- **権限管理**: ファイルアクセス、メモリアクセスの制御
- **検証機能**: DLLの整合性チェック

## システム構成

### 全体アーキテクチャ

```
┌─────────────────────────────────────────────────────────────┐
│                    NSys Main Application                    │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   Presentation  │  │      Plugin     │  │   Security   │ │
│  │      Layer      │  │    Management   │  │    Manager   │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   Application   │  │     Plugin      │  │   Resource   │ │
│  │      Layer      │  │       API       │  │   Manager    │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   Service       │  │    ImGui Core   │  │   Platform   │ │
│  │     Layer       │  │                 │  │    Layer     │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                       Plugin DLLs                          │
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   PluginTest    │  │  CSVNodeEditor  │  │   Custom     │ │
│  │      DLL        │  │      DLL        │  │  Plugin DLL  │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## コンポーネント構成

### メインアプリケーション (NSys.exe)

#### Presentation Layer (プレゼンテーション層)
- **ImGuiWindowManager**: ウィンドウ管理とレイアウト制御
- **MenuManager**: メニューバーとコンテキストメニューの管理
- **ThemeManager**: UI テーマとスタイルの管理
- **InputManager**: キーボード・マウス入力の処理

#### Application Layer (アプリケーション層)
- **PluginManager**: プラグインライフサイクル管理
- **EventBus**: プラグイン間通信とイベント配信
- **ConfigurationManager**: アプリケーション設定の管理
- **LocalizationManager**: 多言語対応とローカライゼーション

#### Service Layer (サービス層)
- **FileSystemService**: ファイル操作の抽象化
- **LoggingService**: ログ出力とデバッグ情報管理
- **MemoryManager**: メモリ使用量監視と最適化
- **ErrorHandler**: エラー処理と例外管理

### プラグインシステム

#### Plugin API
- **IPlugin**: プラグインの基底インターフェース
- **IMenuProvider**: メニュー登録機能
- **IWindowProvider**: ウィンドウ作成機能
- **IDataProvider**: データ共有機能

#### Plugin Manager
```cpp
class PluginManager {
private:
    std::vector<std::unique_ptr<IPlugin>> m_plugins;
    std::map<std::string, HMODULE> m_loadedDLLs;
    SecurityManager* m_security;
    
public:
    bool LoadPlugin(const std::string& dllPath);
    bool UnloadPlugin(const std::string& pluginName);
    std::vector<IPlugin*> GetLoadedPlugins();
    bool ValidatePlugin(const std::string& dllPath);
};
```

### 使用ライブラリとフレームワーク

#### GUI Framework
- **ImGui**: Immediate Mode GUI ライブラリ
- **バージョン**: docking branch (最新)
- **レンダリングバックエンド**: DirectX 12
- **プラットフォームバックエンド**: Win32

#### 外部ライブラリ管理
- **Git Submodules**: 外部依存関係の管理
- **自動初期化**: setup.bat/setup.ps1 による環境構築

#### 開発・ビルドツール
- **Visual Studio 2019/2022**: メインIDE
- **MSVC**: C++コンパイラ
- **Windows SDK**: プラットフォーム依存機能

## データベース/ストレージ設計

NSysは軽量なデスクトップアプリケーションとして、従来的なデータベースは使用しません：

### 設定データ
- **Format**: INI ファイル形式
- **Location**: `%APPDATA%/NSys/config.ini`
- **ImGui Settings**: `imgui.ini`（ImGui標準設定ファイル）

### プラグイン設定
- **Per-Plugin Config**: `%APPDATA%/NSys/plugins/{plugin-name}.ini`
- **Plugin Registry**: `%APPDATA%/NSys/plugins.registry`

### ログファイル
- **Application Log**: `%APPDATA%/NSys/logs/nsys_{date}.log`
- **Plugin Log**: `%APPDATA%/NSys/logs/plugins/{plugin-name}_{date}.log`

## セキュリティアーキテクチャ

### DLL セキュリティ
```cpp
class SecurityManager {
public:
    bool ValidateDLLSignature(const std::string& dllPath);
    bool CheckDLLIntegrity(const std::string& dllPath);
    bool VerifyPluginInterface(IPlugin* plugin);
    void SetPluginPermissions(IPlugin* plugin, const PermissionSet& permissions);
private:
    std::map<std::string, PermissionSet> m_pluginPermissions;
    CertificateValidator m_certValidator;
};
```

### アクセス制御
- **ファイルシステム**: プラグインのファイルアクセスを制限
- **メモリ保護**: プラグイン間のメモリ分離
- **API制限**: 危険なシステムAPIへのアクセス制御

## パフォーマンス設計

### メモリ管理
```cpp
class MemoryManager {
private:
    size_t m_totalMemoryLimit;      // 1GB制限
    size_t m_perPluginMemoryLimit;  // プラグインあたり100MB制限
    std::map<IPlugin*, MemoryUsage> m_pluginMemoryUsage;
    
public:
    bool CheckMemoryLimits(IPlugin* plugin, size_t requestedSize);
    void TrackMemoryUsage(IPlugin* plugin, void* ptr, size_t size);
    void ReleasePluginMemory(IPlugin* plugin);
    MemoryStats GetSystemMemoryStats();
};
```

### 描画最適化
- **60FPS維持**: VSync対応とフレームレート制御
- **バッファリング**: ImGui描画コマンドの効率的なバッファリング
- **リソース管理**: テクスチャとフォントの動的読み込み

## 拡張性設計

### プラグイン開発フレームワーク
```cpp
// プラグイン開発者向けベースクラス
class PluginBase : public IPlugin {
protected:
    ImGuiContext* GetImGuiContext();
    IFileSystem* GetFileSystem();
    ILoggingService* GetLogger();
    
public:
    virtual void Initialize() = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;
    virtual const char* GetName() const = 0;
    virtual const char* GetVersion() const = 0;
};
```

### APIバージョニング
```cpp
#define NSYS_PLUGIN_API_VERSION_MAJOR 1
#define NSYS_PLUGIN_API_VERSION_MINOR 0
#define NSYS_PLUGIN_API_VERSION_PATCH 0

struct PluginInfo {
    uint32_t apiVersionMajor;
    uint32_t apiVersionMinor;
    uint32_t apiVersionPatch;
    const char* pluginName;
    const char* pluginVersion;
    const char* description;
    const char* author;
};
```

### プラグイン間通信
```cpp
class EventBus {
public:
    template<typename T>
    void Subscribe(IPlugin* plugin, std::function<void(const T&)> handler);
    
    template<typename T>
    void Publish(const T& event);
    
    void UnsubscribeAll(IPlugin* plugin);
    
private:
    std::map<std::type_index, std::vector<EventHandler>> m_handlers;
};
```

## 国際化・ローカライゼーション

### 多言語サポート
```cpp
class LocalizationManager {
private:
    std::map<std::string, std::map<std::string, std::string>> m_translations;
    std::string m_currentLanguage;
    
public:
    bool LoadLanguageFile(const std::string& language, const std::string& filePath);
    std::string GetText(const std::string& key, const std::string& defaultText = "");
    void SetLanguage(const std::string& language);
    std::vector<std::string> GetAvailableLanguages();
};

// 使用例
#define _(key) LocalizationManager::Instance().GetText(key, key)
ImGui::Text(_("File"));  // "ファイル" または "File"
```

### 文字エンコーディング
- **内部エンコーディング**: UTF-8
- **ファイルI/O**: UTF-8, Shift-JIS 自動判定
- **ImGui対応**: 日本語フォント自動読み込み

## 配置・デプロイ戦略

### ディレクトリ構造
```
NSys/
├── NSys.exe                    # メインアプリケーション
├── config/                     # 設定ファイル
│   ├── default.ini
│   └── plugins.registry
├── plugins/                    # プラグインディレクトリ
│   ├── CSVNodeEditor/
│   │   └── CSVNodeEditor.dll
│   └── PluginTest/
│       └── PluginTest.dll
├── resources/                  # リソースファイル
│   ├── fonts/
│   ├── themes/
│   └── localization/
└── logs/                       # ログファイル
```

### インストール・アップデート
- **Portable**: レジストリを使用しないポータブル設計
- **自動更新**: プラグインの個別更新対応
- **バックアップ**: 設定とプラグインデータの自動バックアップ

## 品質属性

### 可用性 (Availability)
- **目標**: 99.9% (8時間/日使用想定)
- **障害回復**: プラグインクラッシュ時のメインアプリ継続動作
- **ロードバランシング**: N/A (スタンドアロンアプリケーション)

### 性能 (Performance)
- **応答時間**: UI操作 < 100ms
- **スループット**: 60FPS描画維持
- **メモリ使用量**: < 1GB (全体), < 100MB (プラグインあたり)
- **起動時間**: < 5秒 (プラグイン読み込み含む)

### セキュリティ (Security)
- **認証**: N/A (ローカルアプリケーション)
- **認可**: プラグインアクセス制御
- **データ暗号化**: 設定ファイルの暗号化オプション
- **監査**: プラグイン動作ログ

### 保守性 (Maintainability)
- **モジュール化**: プラグインベースアーキテクチャ
- **テスト可能性**: 各プラグインの独立テスト
- **コード品質**: C++17準拠、静的解析ツール使用
- **ドキュメント**: APIドキュメント、アーキテクチャ仕様

## リスク分析と対策

### 技術リスク
1. **ImGuiバージョン互換性**
   - **リスク**: ImGui更新時のAPI変更
   - **対策**: サブモジュール固定、段階的更新

2. **メモリリーク**
   - **リスク**: プラグインによるメモリリーク
   - **対策**: メモリ使用量監視、自動リーク検出

3. **プラグインクラッシュ**
   - **リスク**: 不安定なプラグインによるシステムダウン
   - **対策**: プロセス分離、例外処理の強化