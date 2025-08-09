# NSys データフロー図

## システムブートストラップフロー

```mermaid
flowchart TD
    A[アプリケーション起動] --> B[設定ファイル読み込み]
    B --> C[ImGuiコンテキスト初期化]
    C --> D[DirectX12初期化]
    D --> E[ウィンドウマネージャー初期化]
    E --> F[プラグインディレクトリスキャン]
    F --> G[プラグインDLL検証]
    G --> H{DLL有効?}
    H -->|Yes| I[プラグイン読み込み]
    H -->|No| J[エラーログ出力]
    I --> K[プラグイン初期化]
    J --> K
    K --> L[メニューバー構築]
    L --> M[メインループ開始]
```

## プラグイン読み込みフロー

```mermaid
sequenceDiagram
    participant PM as PluginManager
    participant SM as SecurityManager
    participant DLL as Plugin DLL
    participant IM as ImGuiWindowManager
    participant MM as MenuManager

    PM->>SM: ValidateDLL(dllPath)
    SM->>SM: 署名検証
    SM->>SM: 整合性チェック
    SM-->>PM: 検証結果
    
    alt DLL有効
        PM->>DLL: LoadLibrary()
        PM->>DLL: GetProcAddress("CreatePlugin")
        DLL-->>PM: CreatePlugin関数ポインタ
        PM->>DLL: CreatePlugin()
        DLL-->>PM: IPlugin*インスタンス
        
        PM->>DLL: Initialize()
        DLL->>IM: ウィンドウ登録要求
        DLL->>MM: メニュー項目登録
        
        PM->>PM: プラグインリストに追加
    else DLL無効
        PM->>PM: エラーログ出力
    end
```

## メインループ データフロー

```mermaid
flowchart LR
    subgraph "メインループ (60FPS)"
        A[入力イベント処理] --> B[ImGui新フレーム開始]
        B --> C[メニューバー描画]
        C --> D[プラグインRender呼び出し]
        D --> E[ImGuiフレーム終了]
        E --> F[DirectX描画]
        F --> G[VSync待機]
        G --> A
    end
    
    subgraph "プラグイン処理"
        D --> H[Plugin1::Render]
        D --> I[Plugin2::Render]
        D --> J[Plugin3::Render]
        H --> K[ImGuiウィンドウ描画]
        I --> L[ImGuiウィンドウ描画]
        J --> M[ImGuiウィンドウ描画]
    end
```

## プラグイン間通信フロー

```mermaid
sequenceDiagram
    participant P1 as Plugin A
    participant EB as EventBus
    participant P2 as Plugin B
    participant P3 as Plugin C

    Note over P1,P3: 初期化時の購読登録
    P2->>EB: Subscribe<DataChangedEvent>()
    P3->>EB: Subscribe<DataChangedEvent>()
    
    Note over P1,P3: イベント送信
    P1->>P1: データ変更処理
    P1->>EB: Publish(DataChangedEvent)
    
    Note over P1,P3: イベント配信
    EB->>P2: OnDataChanged(event)
    EB->>P3: OnDataChanged(event)
    
    P2->>P2: データ更新処理
    P3->>P3: データ更新処理
```

## ファイルシステムアクセスフロー

```mermaid
flowchart TD
    A[プラグインファイル操作要求] --> B[FileSystemService]
    B --> C{アクセス権限チェック}
    C -->|許可| D[ファイル操作実行]
    C -->|拒否| E[アクセス拒否エラー]
    
    D --> F{操作種類}
    F -->|読み込み| G[ファイル読み込み]
    F -->|書き込み| H[ファイル書き込み]
    F -->|削除| I[ファイル削除]
    
    G --> J[結果返却]
    H --> K{書き込み権限}
    I --> L{削除権限}
    
    K -->|許可| M[書き込み実行]
    K -->|拒否| N[権限エラー]
    L -->|許可| O[削除実行]
    L -->|拒否| P[権限エラー]
    
    M --> J
    O --> J
    N --> J
    P --> J
    E --> J
```

## メモリ管理フロー

```mermaid
sequenceDiagram
    participant P as Plugin
    participant MM as MemoryManager
    participant OS as Operating System

    Note over P,OS: メモリ確保要求
    P->>MM: RequestMemory(size)
    MM->>MM: CheckLimits(plugin, size)
    
    alt メモリ制限内
        MM->>OS: malloc(size)
        OS-->>MM: pointer
        MM->>MM: TrackAllocation(plugin, ptr, size)
        MM-->>P: pointer
    else メモリ制限超過
        MM-->>P: OutOfMemoryError
    end
    
    Note over P,OS: メモリ解放
    P->>MM: FreeMemory(ptr)
    MM->>MM: UntrackAllocation(plugin, ptr)
    MM->>OS: free(ptr)
```

## エラーハンドリングフロー

```mermaid
flowchart TD
    A[プラグインでエラー発生] --> B{エラー種類}
    B -->|Critical| C[システム停止処理]
    B -->|Plugin| D[プラグイン無効化]
    B -->|Warning| E[警告ログ出力]
    
    C --> F[全プラグイン終了処理]
    F --> G[設定保存]
    G --> H[アプリケーション終了]
    
    D --> I[プラグインアンロード]
    I --> J[エラー通知UI表示]
    J --> K[システム継続動作]
    
    E --> L[ログファイル記録]
    L --> K
```

## 設定管理フロー

```mermaid
flowchart LR
    subgraph "起動時"
        A[アプリケーション設定読み込み] --> B[デフォルト値適用]
        B --> C[プラグイン設定読み込み]
        C --> D[設定値配信]
    end
    
    subgraph "実行時"
        E[設定変更要求] --> F[設定値検証]
        F --> G{有効な値?}
        G -->|Yes| H[設定値更新]
        G -->|No| I[エラー通知]
        H --> J[関連プラグインに通知]
        I --> K[現在値維持]
    end
    
    subgraph "終了時"
        L[アプリケーション終了] --> M[現在設定値取得]
        M --> N[設定ファイル書き込み]
        N --> O[プラグイン設定保存]
    end
```

## ローカライゼーションフロー

```mermaid
sequenceDiagram
    participant UI as UI Component
    participant LM as LocalizationManager
    participant File as Language File

    Note over UI,File: 初期化時
    LM->>File: LoadLanguageFile("ja")
    File-->>LM: 翻訳データ
    
    Note over UI,File: 描画時
    UI->>LM: GetText("menu.file")
    LM->>LM: Lookup translation
    alt 翻訳存在
        LM-->>UI: "ファイル"
    else 翻訳なし
        LM-->>UI: "menu.file" (デフォルト)
    end
    
    UI->>UI: ImGui::Text(translatedText)
```

## ログ出力フロー

```mermaid
flowchart TD
    A[ログ出力要求] --> B{ログレベル}
    B -->|Debug| C{デバッグモード?}
    B -->|Info| D[コンソール出力]
    B -->|Warning| E[ファイル出力]
    B -->|Error| F[ファイル出力 + UI通知]
    B -->|Critical| G[即座にファイル出力]
    
    C -->|Yes| D
    C -->|No| H[破棄]
    
    D --> I[ログファイル書き込み]
    E --> I
    F --> I
    G --> I
    
    I --> J[ログローテーション実行]
```

## プラグイン アンロードフロー

```mermaid
sequenceDiagram
    participant PM as PluginManager
    participant P as Plugin
    participant MM as MenuManager
    participant IM as ImGuiWindowManager
    participant EB as EventBus

    PM->>P: Shutdown()
    P->>P: リソース解放
    P->>MM: メニュー項目削除要求
    P->>IM: ウィンドウ削除要求
    P-->>PM: 終了完了
    
    PM->>EB: UnsubscribeAll(plugin)
    PM->>MM: RemovePluginMenus(plugin)
    PM->>IM: ClosePluginWindows(plugin)
    
    PM->>PM: FreeLibrary(dllHandle)
    PM->>PM: プラグインリストから削除
    
    Note over PM: プラグインアンロード完了
```

## データ永続化フロー

```mermaid
flowchart TD
    subgraph "保存処理"
        A[データ変更通知] --> B[保存タイマー開始]
        B --> C{タイマー期限?}
        C -->|Yes| D[設定データ収集]
        C -->|No| E[待機]
        E --> C
        
        D --> F[INIファイル生成]
        F --> G[一時ファイル作成]
        G --> H[原子的ファイル置換]
        H --> I[バックアップ作成]
    end
    
    subgraph "読み込み処理"
        J[アプリケーション起動] --> K[設定ファイル存在確認]
        K --> L{ファイル有効?}
        L -->|Yes| M[設定ファイル読み込み]
        L -->|No| N[デフォルト設定使用]
        
        M --> O[設定値検証]
        O --> P{設定値有効?}
        P -->|Yes| Q[設定値適用]
        P -->|No| R[デフォルト値で補完]
        N --> Q
        R --> Q
    end
```