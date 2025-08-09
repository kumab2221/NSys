# TASK-002: CoreシステムI/F実装 - 要件定義

## タスク概要

NSys プラットフォームの核となるインターフェースシステムを実装する。プラグインシステムの基盤となる `IPlugin` インターフェース、UI統合のための `IMenuProvider` および `IWindowProvider` インターフェース、さらに全体のサービス管理を行う `ServiceLocator` を含む。

## 詳細要件

### 1. NSys::IPlugin 基底インターフェース (REQ-001)

**目的**: すべてのプラグインが実装すべき基本契約を定義

**機能要件**:
- プラグインライフサイクル管理（初期化、更新、描画、終了）
- プラグイン情報の取得（名前、バージョン、説明）
- 有効/無効状態の管理
- APIバージョン互換性チェック

**インターフェース仕様**:
```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // ライフサイクル
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    
    // プラグイン情報
    virtual PluginInfo GetPluginInfo() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetVersion() const = 0;
    
    // 状態管理
    virtual bool IsInitialized() const = 0;
    virtual void SetEnabled(bool enabled) = 0;
    virtual bool IsEnabled() const = 0;
};
```

### 2. NSys::IMenuProvider インターフェース (PLUGIN-101)

**目的**: プラグインがアプリケーションメニューに項目を追加する機能を提供

**機能要件**:
- 階層構造のメニュー項目定義
- メニューアクション（コールバック）の関連付け
- ショートカットキーの設定
- セパレーター対応
- 動的メニュー項目の更新

**インターフェース仕様**:
```cpp
class IMenuProvider {
public:
    struct MenuItem {
        std::string label;
        std::function<void()> callback;
        std::string shortcut;
        bool separator = false;
        std::vector<MenuItem> children;
    };
    
    virtual std::vector<MenuItem> GetMenuItems() = 0;
    virtual void OnMenuItemSelected(const std::string& itemId) = 0;
};
```

### 3. NSys::IWindowProvider インターフェース (REQ-005)

**目的**: プラグインがImGuiウィンドウを提供する機能

**機能要件**:
- ウィンドウ設定の定義（タイトル、サイズ、属性）
- ウィンドウの描画処理
- ウィンドウ開閉状態の管理
- ドッキング対応
- ウィンドウ最小・最大サイズの設定

**インターフェース仕様**:
```cpp
class IWindowProvider {
public:
    struct WindowInfo {
        std::string title;
        ImVec2 defaultSize = {800, 600};
        ImVec2 minSize = {200, 150};
        bool dockable = true;
        bool closeable = true;
        bool collapsible = true;
        bool resizable = true;
    };
    
    virtual std::vector<WindowInfo> GetWindows() = 0;
    virtual void RenderWindow(const std::string& windowId) = 0;
    virtual bool IsWindowOpen(const std::string& windowId) = 0;
    virtual void SetWindowOpen(const std::string& windowId, bool open) = 0;
};
```

### 4. NSys::ServiceLocator 実装 (REQ-005)

**目的**: システム全体で使用するサービスの一元管理

**機能要件**:
- サービスインスタンスの登録・取得
- サービスライフサイクル管理
- 安全なサービスアクセス（nullチェック）
- シングルトンパターンの実装
- スレッドセーフなアクセス

**クラス仕様**:
```cpp
class ServiceLocator {
public:
    // Service getters
    static IFileSystemService* GetFileSystemService();
    static ILoggingService* GetLoggingService();
    static IMemoryService* GetMemoryService();
    static IConfigurationService* GetConfigurationService();
    static ILocalizationService* GetLocalizationService();
    static IEventBus* GetEventBus();
    static IPluginManager* GetPluginManager();
    
    // Service registration (internal use)
    static void RegisterFileSystemService(std::unique_ptr<IFileSystemService> service);
    static void RegisterLoggingService(std::unique_ptr<ILoggingService> service);
    // ... 他のサービス登録メソッド
    
    static void Shutdown();
};
```

### 5. APIVersion 管理システム

**目的**: プラグインとのAPIバージョン互換性を管理

**機能要件**:
- セマンティックバージョニング（Major.Minor.Patch）
- バージョン互換性チェック
- 現在のAPIバージョン定数定義

**構造体仕様**:
```cpp
struct APIVersion {
    uint32_t major = 1;
    uint32_t minor = 0;
    uint32_t patch = 0;
    
    bool IsCompatible(const APIVersion& other) const;
};

constexpr APIVersion CURRENT_API_VERSION = {1, 0, 0};
```

## 品質要件

### パフォーマンス要件
- サービスアクセス時間: 100ns以内（インライン展開による）
- プラグインライフサイクル処理: 1秒以内
- メニュー項目取得: 10ms以内

### 信頼性要件
- nullptr チェック機能
- 例外安全性（RAII パターン）
- メモリリーク防止

### 保守性要件
- インターフェース安定性（下位互換性）
- 明確な責任分離
- 拡張可能な設計

## 受け入れ基準

### 基本機能
- [ ] `IPlugin` インターフェースが定義され、コンパイルが成功する
- [ ] `IMenuProvider` インターフェースが定義され、メニュー構造を表現できる
- [ ] `IWindowProvider` インターフェースが定義され、ウィンドウ情報を提供できる
- [ ] `ServiceLocator` クラスが実装され、サービス登録・取得ができる
- [ ] `APIVersion` 構造体が実装され、バージョン互換性チェックができる

### 統合テスト
- [ ] サンプルプラグインが `IPlugin` を継承し、正常に動作する
- [ ] プラグインがメニュー項目を提供し、アプリケーションで表示される
- [ ] プラグインがウィンドウを提供し、ImGuiで描画される
- [ ] `ServiceLocator` を介してサービスにアクセスできる

### エラーハンドリング
- [ ] 未登録サービスへのアクセス時に適切なエラー処理される
- [ ] API バージョン不互換時に適切な警告が表示される
- [ ] プラグイン初期化失敗時に適切にハンドリングされる

## 実装制約

### 技術制約
- C++20 標準に準拠
- ImGui ライブラリ使用必須
- Windows プラットフォーム対応
- Visual Studio 2019/2022 でコンパイル可能

### 設計制約
- ヘッダーオンリーの実装（テンプレート部分）
- プラットフォーム依存部分の分離
- 循環依存の回避
- ABI 安定性の確保

## 依存関係

### 内部依存
- ImGui ライブラリ（ImVec2, ImGuiContext）
- C++ 標準ライブラリ（memory, functional, vector, string）

### 外部依存
- Windows SDK（プラットフォーム固有機能用）
- DirectX 12（将来の拡張用）

## 実装順序

1. **基本インターフェース定義** - interfaces.h の更新
2. **APIVersion 構造体実装**
3. **ServiceLocator クラス実装**
4. **基底クラス PluginBase 実装**（実装補助）
5. **統合テスト用サンプルプラグイン作成**

このTDD要件定義に基づいて、次ステップではテストケースを設計します。