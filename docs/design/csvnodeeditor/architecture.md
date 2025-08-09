# CSVNodeEditor アーキテクチャ設計

## システム概要

CSVNodeEditorは、NSysプラットフォーム上で動作するノードベースのCSVデータ処理プラグインです。ImGui、ImNodes、ImPlotを使用して、直感的なビジュアルプログラミング環境でCSVデータの変換・分析・可視化を可能にします。

## アーキテクチャパターン

### 採用パターン: モデル・ビュー・コントローラー + ノードベース処理

- **パターン**: MVC + Node-Based Processing Pipeline
- **理由**: 
  - UI（View）とデータ処理（Model）の分離
  - ノード間の処理フローの管理
  - 複雑なデータ変換パイプラインの構造化
  - 再利用可能なノードコンポーネントの実現

### 処理モデル

- **データフロープログラミング**: ノード間のデータ伝播による処理実行
- **遅延評価**: 必要な時点でのみデータ処理を実行
- **キャッシュ機能**: 計算済み結果の再利用によるパフォーマンス向上

## システム構成

### 全体アーキテクチャ

```
┌─────────────────────────────────────────────────────────────┐
│                    CSVNodeEditor Plugin                    │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │ User Interface  │  │   Workflow      │  │    Node      │ │
│  │     Layer       │  │   Manager       │  │   Palette    │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   Node Graph    │  │   Data Flow     │  │  Execution   │ │
│  │    Manager      │  │    Engine       │  │   Engine     │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   Data Model    │  │     Node        │  │     CSV      │ │
│  │     Layer       │  │  Implementations │  │    Parser    │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   File I/O      │  │    Memory       │  │    Cache     │ │
│  │    Manager      │  │    Manager      │  │   Manager    │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## コンポーネント構成

### User Interface Layer (UI層)

#### MainWindow
- **EditorTabManager**: 複数タブでのワークフロー管理
- **NodeCanvas**: ImNodesを使用したノードグラフ描画
- **PropertyPanel**: 選択ノードの設定パネル
- **DataPreview**: CSVデータの表形式プレビュー
- **NodePalette**: 使用可能ノードの一覧表示
- **LogPanel**: 処理状況とエラー情報表示

```cpp
class CSVNodeEditorWindow : public NSys::IWindowProvider {
private:
    std::unique_ptr<EditorTabManager> m_tabManager;
    std::unique_ptr<NodeCanvas> m_nodeCanvas;
    std::unique_ptr<PropertyPanel> m_propertyPanel;
    std::unique_ptr<DataPreview> m_dataPreview;
    std::unique_ptr<NodePalette> m_nodePalette;
    std::unique_ptr<LogPanel> m_logPanel;
    
public:
    void RenderWindow(const std::string& windowId) override;
    void UpdateLayout();
    void HandleInput();
};
```

#### Tab Management
```cpp
class EditorTab {
private:
    std::string m_name;
    std::unique_ptr<NodeGraph> m_nodeGraph;
    std::unique_ptr<WorkflowState> m_workflowState;
    bool m_modified = false;
    
public:
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name);
    bool IsModified() const { return m_modified; }
    void SetModified(bool modified);
    
    NodeGraph* GetNodeGraph() { return m_nodeGraph.get(); }
    WorkflowState* GetWorkflowState() { return m_workflowState.get(); }
};

class EditorTabManager {
private:
    std::vector<std::unique_ptr<EditorTab>> m_tabs;
    int m_activeTabIndex = 0;
    
public:
    EditorTab* CreateTab(const std::string& name);
    void CloseTab(int index);
    void SetActiveTab(int index);
    EditorTab* GetActiveTab();
    int GetTabCount() const { return static_cast<int>(m_tabs.size()); }
};
```

### Node Graph Management (ノードグラフ管理)

#### Node Base Classes
```cpp
class NodeBase {
protected:
    NodeId m_id;
    std::string m_name;
    ImVec2 m_position;
    std::vector<InputPin> m_inputPins;
    std::vector<OutputPin> m_outputPins;
    NodeState m_state = NodeState::Idle;
    std::string m_errorMessage;
    
public:
    virtual ~NodeBase() = default;
    
    // Node lifecycle
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual bool Execute() = 0;
    virtual void Reset() = 0;
    
    // UI rendering
    virtual void RenderNode() = 0;
    virtual void RenderProperties() = 0;
    
    // Data flow
    virtual bool ValidateInputs() = 0;
    virtual void OnInputChanged(PinId pinId) = 0;
    
    // Serialization
    virtual nlohmann::json Serialize() const = 0;
    virtual bool Deserialize(const nlohmann::json& data) = 0;
};
```

#### Node Graph
```cpp
class NodeGraph {
private:
    std::unordered_map<NodeId, std::unique_ptr<NodeBase>> m_nodes;
    std::vector<Connection> m_connections;
    NodeId m_nextNodeId = 1;
    
public:
    // Node management
    NodeId CreateNode(const std::string& nodeType);
    bool DeleteNode(NodeId nodeId);
    NodeBase* FindNode(NodeId nodeId);
    
    // Connection management
    bool CreateConnection(NodeId fromNode, PinId fromPin, NodeId toNode, PinId toPin);
    bool DeleteConnection(NodeId fromNode, PinId fromPin, NodeId toNode, PinId toPin);
    std::vector<Connection> GetConnections(NodeId nodeId) const;
    
    // Graph operations
    bool ValidateGraph();
    std::vector<NodeId> GetExecutionOrder();
    bool HasCycle();
    
    // Serialization
    nlohmann::json Serialize() const;
    bool Deserialize(const nlohmann::json& data);
};
```

### Data Model Layer (データモデル層)

#### CSV Data Structure
```cpp
class CSVData {
private:
    std::vector<std::string> m_headers;
    std::vector<std::vector<std::string>> m_rows;
    std::vector<ColumnType> m_columnTypes;
    std::string m_filePath;
    std::string m_encoding = "UTF-8";
    char m_delimiter = ',';
    bool m_hasHeader = true;
    
public:
    // Data access
    size_t GetRowCount() const { return m_rows.size(); }
    size_t GetColumnCount() const { return m_headers.size(); }
    const std::vector<std::string>& GetHeaders() const { return m_headers; }
    const std::vector<std::string>& GetRow(size_t index) const;
    std::string GetCell(size_t row, size_t column) const;
    
    // Data modification
    void SetCell(size_t row, size_t column, const std::string& value);
    void AddRow(const std::vector<std::string>& row);
    void RemoveRow(size_t index);
    void AddColumn(const std::string& header, ColumnType type = ColumnType::String);
    void RemoveColumn(size_t index);
    
    // Type system
    ColumnType GetColumnType(size_t column) const;
    void SetColumnType(size_t column, ColumnType type);
    bool TryConvertCell(size_t row, size_t column, ColumnType targetType, std::string& result);
    
    // Statistics
    DataStatistics GetColumnStatistics(size_t column) const;
    
    // Serialization for caching
    std::vector<uint8_t> SerializeBinary() const;
    bool DeserializeBinary(const std::vector<uint8_t>& data);
};

enum class ColumnType {
    String,
    Integer,
    Float,
    Boolean,
    Date,
    DateTime
};

struct DataStatistics {
    size_t count = 0;
    size_t nullCount = 0;
    std::string min;
    std::string max;
    double numericMin = 0.0;
    double numericMax = 0.0;
    double average = 0.0;
    std::unordered_map<std::string, size_t> valueFrequency;
};
```

#### Data Processing Pipeline
```cpp
class DataProcessor {
public:
    virtual ~DataProcessor() = default;
    virtual std::unique_ptr<CSVData> Process(const CSVData& input) = 0;
    virtual bool CanProcess(const CSVData& input) = 0;
    virtual std::string GetProcessorName() const = 0;
};

class ProcessingPipeline {
private:
    std::vector<std::unique_ptr<DataProcessor>> m_processors;
    
public:
    void AddProcessor(std::unique_ptr<DataProcessor> processor);
    std::unique_ptr<CSVData> Execute(const CSVData& input);
    bool Validate(const CSVData& input);
    void Clear();
};
```

### Node Implementations (ノード実装)

#### Input Nodes
```cpp
class CSVReaderNode : public NodeBase {
private:
    std::string m_filePath;
    std::string m_encoding = "UTF-8";
    char m_delimiter = ',';
    bool m_hasHeader = true;
    std::unique_ptr<CSVData> m_cachedData;
    std::filesystem::file_time_type m_lastModified;
    
public:
    bool Execute() override;
    void RenderNode() override;
    void RenderProperties() override;
    
private:
    bool LoadCSVFile();
    bool DetectEncoding(const std::string& filePath);
    char DetectDelimiter(const std::string& sample);
    bool DetectHeader(const std::vector<std::string>& firstRow);
};
```

#### Processing Nodes
```cpp
class FilterNode : public NodeBase {
private:
    size_t m_targetColumn = 0;
    FilterOperator m_operator = FilterOperator::Equals;
    std::string m_filterValue;
    bool m_caseSensitive = false;
    
public:
    bool Execute() override;
    void RenderNode() override;
    void RenderProperties() override;
    
private:
    bool ApplyFilter(const std::string& value, const std::string& filterValue);
};

enum class FilterOperator {
    Equals,
    NotEquals,
    Contains,
    StartsWith,
    EndsWith,
    GreaterThan,
    LessThan,
    GreaterEqual,
    LessEqual,
    IsEmpty,
    IsNotEmpty,
    Regex
};

class SortNode : public NodeBase {
private:
    size_t m_sortColumn = 0;
    SortOrder m_sortOrder = SortOrder::Ascending;
    bool m_stableSort = true;
    
public:
    bool Execute() override;
    void RenderNode() override;
    void RenderProperties() override;
    
private:
    bool CompareRows(const std::vector<std::string>& a, const std::vector<std::string>& b, ColumnType columnType);
};

class AggregateNode : public NodeBase {
private:
    std::vector<size_t> m_groupByColumns;
    std::vector<AggregateOperation> m_operations;
    
public:
    bool Execute() override;
    void RenderNode() override;
    void RenderProperties() override;
    
private:
    std::unique_ptr<CSVData> GroupAndAggregate(const CSVData& input);
};

struct AggregateOperation {
    size_t targetColumn;
    AggregateFunction function;
    std::string outputName;
};

enum class AggregateFunction {
    Count,
    Sum,
    Average,
    Min,
    Max,
    StdDev,
    Variance
};
```

#### Output Nodes
```cpp
class CSVWriterNode : public NodeBase {
private:
    std::string m_outputPath;
    std::string m_encoding = "UTF-8";
    char m_delimiter = ',';
    bool m_writeHeader = true;
    bool m_appendMode = false;
    
public:
    bool Execute() override;
    void RenderNode() override;
    void RenderProperties() override;
    
private:
    bool WriteCSVFile(const CSVData& data);
    std::string EscapeCSVField(const std::string& field, char delimiter);
};
```

### Execution Engine (実行エンジン)

```cpp
class ExecutionEngine {
private:
    NodeGraph* m_nodeGraph;
    std::unordered_map<NodeId, std::unique_ptr<CSVData>> m_nodeOutputCache;
    std::unordered_set<NodeId> m_dirtyNodes;
    bool m_isExecuting = false;
    std::atomic<bool> m_shouldCancel = false;
    
public:
    explicit ExecutionEngine(NodeGraph* nodeGraph);
    
    // Execution control
    bool ExecuteGraph();
    bool ExecuteNode(NodeId nodeId);
    void CancelExecution();
    bool IsExecuting() const { return m_isExecuting; }
    
    // Cache management
    void InvalidateNode(NodeId nodeId);
    void InvalidateDownstream(NodeId nodeId);
    void ClearCache();
    
    // Progress reporting
    void SetProgressCallback(std::function<void(float, const std::string&)> callback);
    
private:
    std::vector<NodeId> GetExecutionOrder();
    bool ExecuteNodeInternal(NodeId nodeId);
    void PropagateData(NodeId fromNode, NodeId toNode);
    void UpdateNodeStates();
};

enum class NodeState {
    Idle,
    Executing,
    Completed,
    Error,
    Cancelled
};
```

### Memory Management (メモリ管理)

```cpp
class DataCacheManager {
private:
    struct CacheEntry {
        std::unique_ptr<CSVData> data;
        std::chrono::steady_clock::time_point lastAccess;
        size_t memorySize;
        NodeId nodeId;
    };
    
    std::unordered_map<NodeId, CacheEntry> m_cache;
    size_t m_maxMemoryUsage = 500 * 1024 * 1024; // 500MB
    size_t m_currentMemoryUsage = 0;
    
public:
    void SetData(NodeId nodeId, std::unique_ptr<CSVData> data);
    CSVData* GetData(NodeId nodeId);
    void RemoveData(NodeId nodeId);
    void ClearAll();
    
    void SetMaxMemoryUsage(size_t maxBytes) { m_maxMemoryUsage = maxBytes; }
    size_t GetCurrentMemoryUsage() const { return m_currentMemoryUsage; }
    size_t GetMaxMemoryUsage() const { return m_maxMemoryUsage; }
    
private:
    void EvictLRU();
    size_t CalculateDataSize(const CSVData& data);
};
```

### File I/O Management (ファイルI/O管理)

```cpp
class CSVParser {
private:
    struct ParseOptions {
        std::string encoding = "UTF-8";
        char delimiter = ',';
        char quote = '"';
        char escape = '\\';
        bool hasHeader = true;
        bool skipEmptyLines = true;
        size_t maxRowsToRead = 0; // 0 = unlimited
    };
    
public:
    static std::unique_ptr<CSVData> ParseFile(const std::string& filePath, const ParseOptions& options = ParseOptions{});
    static std::unique_ptr<CSVData> ParseString(const std::string& csvContent, const ParseOptions& options = ParseOptions{});
    static bool WriteFile(const std::string& filePath, const CSVData& data, const ParseOptions& options = ParseOptions{});
    
    // Utility functions
    static std::string DetectEncoding(const std::string& filePath);
    static char DetectDelimiter(const std::string& sample);
    static bool DetectHeader(const std::vector<std::string>& firstRow);
    static std::vector<ColumnType> InferColumnTypes(const CSVData& data, size_t sampleSize = 1000);
    
private:
    static std::string ConvertEncoding(const std::string& input, const std::string& fromEncoding, const std::string& toEncoding);
    static std::vector<std::string> ParseCSVLine(const std::string& line, char delimiter, char quote, char escape);
    static std::string EscapeCSVField(const std::string& field, char delimiter, char quote);
};
```

### Serialization and Project Management (シリアライゼーションとプロジェクト管理)

```cpp
class ProjectManager {
private:
    std::string m_currentProjectPath;
    bool m_projectModified = false;
    
public:
    // Project operations
    bool NewProject();
    bool OpenProject(const std::string& filePath);
    bool SaveProject();
    bool SaveProjectAs(const std::string& filePath);
    bool CloseProject();
    
    // Project state
    bool IsProjectOpen() const { return !m_currentProjectPath.empty(); }
    const std::string& GetCurrentProjectPath() const { return m_currentProjectPath; }
    bool IsProjectModified() const { return m_projectModified; }
    void SetProjectModified(bool modified) { m_projectModified = modified; }
    
    // Serialization
    nlohmann::json SerializeWorkspace(const EditorTabManager& tabManager);
    bool DeserializeWorkspace(const nlohmann::json& data, EditorTabManager& tabManager);
    
private:
    static constexpr const char* PROJECT_EXTENSION = ".csvproj";
    static constexpr int PROJECT_VERSION = 1;
    
    bool ValidateProjectFile(const nlohmann::json& data);
};
```

## パフォーマンス設計

### 大量データ処理対応
```cpp
class StreamingCSVReader {
private:
    std::ifstream m_fileStream;
    std::string m_buffer;
    size_t m_bufferSize = 64 * 1024; // 64KB buffer
    size_t m_currentRow = 0;
    ParseOptions m_options;
    
public:
    explicit StreamingCSVReader(const std::string& filePath, const ParseOptions& options);
    
    bool HasNextRow();
    std::vector<std::string> ReadNextRow();
    void Reset();
    size_t GetCurrentRowNumber() const { return m_currentRow; }
    
    // For progress reporting
    size_t GetFileSize() const;
    size_t GetCurrentPosition() const;
    float GetProgress() const;
};
```

### 並列処理対応
```cpp
class ParallelDataProcessor {
private:
    size_t m_threadCount;
    std::vector<std::thread> m_workers;
    
public:
    explicit ParallelDataProcessor(size_t threadCount = std::thread::hardware_concurrency());
    
    template<typename ProcessorFunc>
    std::unique_ptr<CSVData> ProcessInParallel(const CSVData& input, ProcessorFunc processor);
    
    void SetThreadCount(size_t count) { m_threadCount = count; }
    size_t GetThreadCount() const { return m_threadCount; }
};
```

## エラーハンドリング設計

```cpp
class NodeErrorHandler {
public:
    enum class ErrorSeverity {
        Warning,
        Error,
        Critical
    };
    
    struct NodeError {
        NodeId nodeId;
        std::string message;
        ErrorSeverity severity;
        std::chrono::steady_clock::time_point timestamp;
        std::string details;
    };
    
    static void ReportError(NodeId nodeId, const std::string& message, ErrorSeverity severity, const std::string& details = "");
    static std::vector<NodeError> GetNodeErrors(NodeId nodeId);
    static void ClearNodeErrors(NodeId nodeId);
    static void ClearAllErrors();
    
private:
    static std::vector<NodeError> s_errors;
    static std::mutex s_errorMutex;
};
```

## 拡張性設計

### カスタムノード作成フレームワーク
```cpp
class CustomNodeFactory {
public:
    using CreateNodeFunc = std::function<std::unique_ptr<NodeBase>()>;
    
    static void RegisterNodeType(const std::string& typeName, CreateNodeFunc creator, const std::string& category = "Custom");
    static std::unique_ptr<NodeBase> CreateNode(const std::string& typeName);
    static std::vector<std::string> GetAvailableNodeTypes();
    static std::vector<std::string> GetNodeCategories();
    static std::vector<std::string> GetNodesByCategory(const std::string& category);
    
private:
    struct NodeTypeInfo {
        CreateNodeFunc creator;
        std::string category;
    };
    
    static std::unordered_map<std::string, NodeTypeInfo> s_nodeTypes;
};

// Custom node registration macro
#define REGISTER_CUSTOM_NODE(ClassName, TypeName, Category) \
    static bool s_##ClassName##Registered = []() { \
        CustomNodeFactory::RegisterNodeType(TypeName, []() -> std::unique_ptr<NodeBase> { \
            return std::make_unique<ClassName>(); \
        }, Category); \
        return true; \
    }();
```

## 設定管理

```cpp
struct CSVNodeEditorConfig {
    // UI settings
    struct UI {
        bool showNodeIds = false;
        bool showPerformanceStats = false;
        float nodeSnapDistance = 10.0f;
        ImVec4 nodeColors[static_cast<int>(NodeCategory::Count)] = {
            {0.5f, 0.5f, 1.0f, 1.0f}, // Input
            {1.0f, 0.5f, 0.5f, 1.0f}, // Process
            {0.5f, 1.0f, 0.5f, 1.0f}  // Output
        };
    } ui;
    
    // Performance settings
    struct Performance {
        size_t maxCacheMemory = 500 * 1024 * 1024; // 500MB
        size_t maxRowsForPreview = 10000;
        bool enableParallelProcessing = true;
        size_t maxThreadCount = std::thread::hardware_concurrency();
        bool enableProgressReporting = true;
    } performance;
    
    // CSV parsing defaults
    struct CSVDefaults {
        std::string encoding = "UTF-8";
        char delimiter = ',';
        bool hasHeader = true;
        bool autoDetectDelimiter = true;
        bool autoDetectEncoding = true;
    } csvDefaults;
    
    void LoadFromConfig(NSys::IConfigurationService* config);
    void SaveToConfig(NSys::IConfigurationService* config);
};
```