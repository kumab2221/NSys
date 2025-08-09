# CSVNodeEditor データフロー図

## プラグイン初期化フロー

```mermaid
sequenceDiagram
    participant NSys as NSys Main
    participant Plugin as CSVNodeEditor Plugin
    participant UI as UI Components
    participant NodeFac as Node Factory
    participant Config as Configuration

    NSys->>Plugin: LoadPlugin()
    Plugin->>Plugin: RegisterServices()
    Plugin->>NodeFac: RegisterBuiltinNodes()
    Plugin->>Config: LoadConfiguration()
    Plugin->>UI: InitializeUI()
    
    Plugin->>NSys: RegisterMenuItems()
    Plugin->>NSys: RegisterWindows()
    
    Plugin-->>NSys: Initialization Complete
```

## ワークフロー作成フロー

```mermaid
flowchart TD
    A[ユーザーが新規タブ作成] --> B[EditorTab作成]
    B --> C[NodeGraph初期化]
    C --> D[WorkflowState初期化]
    D --> E[タブをTabManagerに追加]
    E --> F[UIレイアウト更新]
    F --> G[ノードパレット表示]
    G --> H[空のキャンバス表示]
    
    H --> I[ユーザーがノード追加]
    I --> J[NodeFactory.CreateNode]
    J --> K[ノードをNodeGraphに追加]
    K --> L[ノードのUI描画]
    L --> M[ノード間接続待機]
```

## CSVファイル読み込みフロー

```mermaid
sequenceDiagram
    participant User as ユーザー
    participant Reader as CSVReaderNode
    participant Parser as CSVParser
    participant Data as CSVData
    participant Cache as DataCache

    User->>Reader: ファイルパス設定
    User->>Reader: 実行ボタンクリック
    
    Reader->>Reader: ValidateInputs()
    Reader->>Parser: DetectEncoding(filePath)
    Parser-->>Reader: エンコーディング
    Reader->>Parser: DetectDelimiter(sample)
    Parser-->>Reader: 区切り文字
    
    Reader->>Parser: ParseFile(filePath, options)
    Parser->>Parser: ファイル読み込み
    Parser->>Parser: 行ごとに解析
    Parser->>Data: CSVData作成
    Parser-->>Reader: CSVDataインスタンス
    
    Reader->>Cache: SetData(nodeId, csvData)
    Reader->>Reader: setState(Completed)
    
    Note over Reader: 下流ノードに通知
    Reader->>Reader: NotifyOutputChanged()
```

## データ処理パイプライン実行フロー

```mermaid
flowchart LR
    subgraph "実行エンジン"
        A[ExecuteGraph開始] --> B[実行順序計算]
        B --> C[循環参照チェック]
        C --> D{循環あり?}
        D -->|Yes| E[エラー報告]
        D -->|No| F[ノード順次実行]
        F --> G[実行完了]
    end
    
    subgraph "ノード実行"
        H[ノード実行開始] --> I[入力検証]
        I --> J{入力有効?}
        J -->|No| K[エラー状態設定]
        J -->|Yes| L[データ処理実行]
        L --> M[出力データ生成]
        M --> N[キャッシュ保存]
        N --> O[下流ノード通知]
    end
    
    F --> H
    O --> P[次のノード実行]
    P --> F
```

## ノード間データ伝播フロー

```mermaid
sequenceDiagram
    participant N1 as FilterNode
    participant N2 as SortNode
    participant N3 as CSVWriterNode
    participant Cache as DataCacheManager
    participant Engine as ExecutionEngine

    Note over N1,Engine: データ処理完了
    N1->>Cache: SetData(nodeId1, filteredData)
    N1->>Engine: NotifyOutputChanged(nodeId1)
    
    Engine->>Engine: FindDownstreamNodes(nodeId1)
    Engine->>N2: InvalidateInputs()
    N2->>N2: setState(Dirty)
    
    Note over N1,Engine: 下流ノード実行
    Engine->>N2: Execute()
    N2->>Cache: GetData(nodeId1)
    Cache-->>N2: filteredData
    N2->>N2: ProcessData(filteredData)
    N2->>Cache: SetData(nodeId2, sortedData)
    N2->>Engine: NotifyOutputChanged(nodeId2)
    
    Engine->>N3: InvalidateInputs()
    Engine->>N3: Execute()
    N3->>Cache: GetData(nodeId2)
    Cache-->>N3: sortedData
    N3->>N3: WriteCSVFile(sortedData)
    N3->>Engine: ExecutionComplete(nodeId3)
```

## メモリ管理フロー

```mermaid
flowchart TD
    A[データ生成/更新] --> B[DataCacheManager.SetData]
    B --> C[メモリ使用量計算]
    C --> D{制限超過?}
    
    D -->|No| E[キャッシュに保存]
    D -->|Yes| F[LRU削除対象選択]
    F --> G[古いデータ削除]
    G --> H[メモリ使用量更新]
    H --> I{十分な空き?}
    I -->|No| F
    I -->|Yes| E
    
    E --> J[アクセス時刻更新]
    J --> K[メモリ統計更新]
    
    subgraph "データアクセス"
        L[GetData要求] --> M{キャッシュ存在?}
        M -->|Yes| N[アクセス時刻更新]
        M -->|No| O[再計算が必要]
        N --> P[データ返却]
        O --> Q[上流ノード実行]
        Q --> P
    end
```

## ストリーミング処理フロー

```mermaid
sequenceDiagram
    participant User as ユーザー
    participant Node as 大容量CSVノード
    participant Stream as StreamingReader
    participant Processor as DataProcessor
    participant UI as ProgressUI

    User->>Node: 大容量ファイル指定
    Node->>Stream: StreamingCSVReader作成
    Stream->>Stream: ファイルサイズ取得
    
    loop チャンク単位で処理
        Node->>Stream: ReadNextChunk()
        Stream-->>Node: データチャンク
        Node->>Processor: ProcessChunk(chunk)
        Processor-->>Node: 処理済みチャンク
        Node->>Node: 結果を累積
        
        Node->>UI: UpdateProgress(processed/total)
        UI->>UI: プログレスバー更新
        
        Node->>Node: CheckCancellation()
        alt キャンセル要求
            Node->>Node: 処理中断
            break
        end
    end
    
    Node->>Node: 最終結果生成
    Node-->>User: 処理完了通知
```

## エラーハンドリングフロー

```mermaid
flowchart TD
    A[ノードでエラー発生] --> B{エラー種類判定}
    
    B -->|Input Error| C[入力エラー処理]
    B -->|Processing Error| D[処理エラー処理]
    B -->|System Error| E[システムエラー処理]
    
    C --> F[ノード状態をError設定]
    D --> F
    E --> G[クリティカルエラー報告]
    
    F --> H[エラーメッセージ表示]
    H --> I[下流ノード実行停止]
    I --> J[ユーザー通知]
    
    G --> K[システム保護処理]
    K --> L[データ保存]
    L --> M[安全な状態で停止]
    
    subgraph "エラー回復"
        N[ユーザーが設定修正] --> O[ノードリセット]
        O --> P[再実行可能状態]
    end
    
    J --> N
```

## プロジェクト保存/読み込みフロー

```mermaid
sequenceDiagram
    participant User as ユーザー
    participant PM as ProjectManager
    participant Tab as EditorTabManager
    participant Graph as NodeGraph
    participant FS as FileSystem

    Note over User,FS: プロジェクト保存
    User->>PM: SaveProject()
    PM->>Tab: GetAllTabs()
    Tab-->>PM: タブ一覧
    
    loop 各タブを処理
        PM->>Graph: Serialize()
        Graph->>Graph: ノード情報シリアライズ
        Graph->>Graph: 接続情報シリアライズ
        Graph-->>PM: JSON データ
    end
    
    PM->>PM: プロジェクト情報統合
    PM->>FS: WriteFile(projectPath, jsonData)
    FS-->>PM: 保存完了
    PM-->>User: 保存成功通知
    
    Note over User,FS: プロジェクト読み込み
    User->>PM: OpenProject(filePath)
    PM->>FS: ReadFile(filePath)
    FS-->>PM: JSON データ
    PM->>PM: ValidateProjectFile()
    
    alt 有効なファイル
        PM->>Tab: ClearAllTabs()
        loop 各タブデータ
            PM->>Tab: CreateTab()
            PM->>Graph: Deserialize(tabData)
            Graph->>Graph: ノード復元
            Graph->>Graph: 接続復元
        end
        PM-->>User: 読み込み成功
    else 無効なファイル
        PM-->>User: エラー通知
    end
```

## ノード接続管理フロー

```mermaid
flowchart TD
    A[ユーザーがノード接続開始] --> B[出力ピン選択]
    B --> C[ドラッグ開始]
    C --> D[接続線表示]
    D --> E[入力ピン上でドロップ]
    
    E --> F[接続妥当性検証]
    F --> G{接続可能?}
    
    G -->|Yes| H[Connection作成]
    G -->|No| I[エラーメッセージ表示]
    
    H --> J[NodeGraphに接続追加]
    J --> K[データ型チェック]
    K --> L{型互換性OK?}
    
    L -->|Yes| M[接続確定]
    L -->|No| N[警告表示]
    
    M --> O[下流ノード無効化]
    O --> P[UI更新]
    
    I --> Q[接続キャンセル]
    N --> R[ユーザー選択待ち]
    R --> S{強制接続?}
    S -->|Yes| M
    S -->|No| Q
    
    subgraph "接続削除"
        T[ユーザーが接続線クリック] --> U[接続選択]
        U --> V[Deleteキー押下]
        V --> W[接続削除]
        W --> X[下流ノード無効化]
        X --> Y[UI更新]
    end
```

## データプレビュー更新フロー

```mermaid
sequenceDiagram
    participant User as ユーザー
    participant Preview as DataPreview
    participant Node as SelectedNode
    participant Cache as DataCacheManager
    participant UI as PreviewUI

    User->>Preview: ノード選択
    Preview->>Node: GetOutputData()
    Node->>Cache: GetData(nodeId)
    
    alt データ存在
        Cache-->>Node: CSVData
        Node-->>Preview: CSVData
        Preview->>Preview: CreatePreviewData()
        
        alt 大量データ
            Preview->>Preview: SampleData(10000rows)
            Preview->>UI: DisplaySampledData()
            Preview->>UI: ShowDataSizeWarning()
        else 少量データ
            Preview->>UI: DisplayFullData()
        end
        
    else データなし
        Cache-->>Node: null
        Node-->>Preview: null
        Preview->>UI: DisplayNoDataMessage()
    end
    
    Note over Preview,UI: リアルタイム更新
    loop データ変更監視
        Preview->>Node: CheckDataVersion()
        alt データ更新あり
            Preview->>Preview: RefreshPreview()
        end
    end
```

## パフォーマンス監視フロー

```mermaid
flowchart LR
    subgraph "実行監視"
        A[ノード実行開始] --> B[開始時刻記録]
        B --> C[メモリ使用量記録]
        C --> D[処理実行]
        D --> E[終了時刻記録]
        E --> F[メモリ使用量記録]
        F --> G[統計情報更新]
    end
    
    subgraph "統計表示"
        H[統計UI更新] --> I[実行時間表示]
        I --> J[メモリ使用量表示]
        J --> K[スループット表示]
        K --> L[ボトルネック検出]
    end
    
    G --> H
    
    subgraph "最適化提案"
        M[パフォーマンス分析] --> N{ボトルネック検出}
        N -->|メモリ| O[キャッシュ最適化提案]
        N -->|CPU| P[並列処理提案]
        N -->|I/O| Q[ストリーミング提案]
    end
    
    L --> M
```

## undo/Redo 操作フロー

```mermaid
sequenceDiagram
    participant User as ユーザー
    participant History as HistoryManager
    participant Graph as NodeGraph
    participant UI as EditorUI

    Note over User,UI: 操作実行
    User->>Graph: ノード作成
    Graph->>History: RecordAction(CreateNodeAction)
    History->>History: PushToUndoStack()
    History->>History: ClearRedoStack()
    Graph-->>User: 操作完了
    
    Note over User,UI: Undo実行
    User->>History: Undo()
    History->>History: PopFromUndoStack()
    History->>Graph: ReverseAction()
    Graph->>Graph: ノード削除
    History->>History: PushToRedoStack()
    Graph->>UI: UpdateDisplay()
    
    Note over User,UI: Redo実行
    User->>History: Redo()
    History->>History: PopFromRedoStack()
    History->>Graph: ReplayAction()
    Graph->>Graph: ノード復元
    History->>History: PushToUndoStack()
    Graph->>UI: UpdateDisplay()
```

## 並列処理フロー

```mermaid
flowchart TD
    A[大量データ処理開始] --> B[データ分割]
    B --> C[ワーカースレッド数決定]
    C --> D[タスクキューにチャンク追加]
    
    D --> E[ワーカースレッド起動]
    
    subgraph "並列実行"
        F[Worker1: チャンク処理] --> I[結果1]
        G[Worker2: チャンク処理] --> J[結果2]
        H[Worker3: チャンク処理] --> K[結果3]
    end
    
    E --> F
    E --> G
    E --> H
    
    I --> L[結果統合]
    J --> L
    K --> L
    
    L --> M[最終結果生成]
    M --> N[メインスレッドに返却]
    
    subgraph "進捗管理"
        O[各ワーカーから進捗報告] --> P[全体進捗計算]
        P --> Q[UIに進捗表示]
    end
    
    F --> O
    G --> O
    H --> O
```