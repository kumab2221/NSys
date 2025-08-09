#pragma once

#include "../nsys/interfaces.h"
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <chrono>
#include <filesystem>

// Third-party dependencies
#include "json/json.hpp" // nlohmann::json for serialization
#include "imgui/imgui.h"
#include "imnodes/imnodes.h"
#include "implot/implot.h"

namespace CSVNodeEditor {

// ==================== Type Definitions ====================

using NodeId = uint32_t;
using PinId = uint32_t;
using ConnectionId = uint32_t;

constexpr NodeId INVALID_NODE_ID = 0;
constexpr PinId INVALID_PIN_ID = 0;
constexpr ConnectionId INVALID_CONNECTION_ID = 0;

// ==================== Data Types ====================

/**
 * @brief CSV列のデータ型
 */
enum class ColumnType {
    String = 0,
    Integer = 1,
    Float = 2,
    Boolean = 3,
    Date = 4,
    DateTime = 5
};

/**
 * @brief ノードカテゴリ
 */
enum class NodeCategory {
    Input = 0,
    Process = 1,
    Output = 2,
    Custom = 3,
    Count = 4  // カテゴリ数
};

/**
 * @brief ノード状態
 */
enum class NodeState {
    Idle = 0,
    Executing = 1,
    Completed = 2,
    Error = 3,
    Cancelled = 4
};

/**
 * @brief データ統計情報
 */
struct DataStatistics {
    size_t totalRows = 0;
    size_t totalColumns = 0;
    size_t nullCount = 0;
    std::string minValue;
    std::string maxValue;
    double numericMin = 0.0;
    double numericMax = 0.0;
    double average = 0.0;
    double standardDeviation = 0.0;
    std::unordered_map<std::string, size_t> valueFrequency;
    
    // Serialization
    nlohmann::json ToJson() const;
    void FromJson(const nlohmann::json& json);
};

/**
 * @brief CSV解析オプション
 */
struct ParseOptions {
    std::string encoding = "UTF-8";
    char delimiter = ',';
    char quote = '"';
    char escape = '\\';
    bool hasHeader = true;
    bool skipEmptyLines = true;
    bool trimWhitespace = true;
    size_t maxRowsToRead = 0; // 0 = unlimited
    size_t bufferSize = 64 * 1024; // 64KB
    
    // Serialization
    nlohmann::json ToJson() const;
    void FromJson(const nlohmann::json& json);
};

// ==================== Core Data Classes ====================

/**
 * @brief CSVデータの主要インターフェース
 */
class ICSVData {
public:
    virtual ~ICSVData() = default;
    
    // Basic data access
    virtual size_t GetRowCount() const = 0;
    virtual size_t GetColumnCount() const = 0;
    virtual const std::vector<std::string>& GetHeaders() const = 0;
    virtual const std::vector<std::string>& GetRow(size_t index) const = 0;
    virtual std::string GetCell(size_t row, size_t column) const = 0;
    
    // Data modification
    virtual void SetCell(size_t row, size_t column, const std::string& value) = 0;
    virtual void AddRow(const std::vector<std::string>& row) = 0;
    virtual void RemoveRow(size_t index) = 0;
    virtual void AddColumn(const std::string& header, ColumnType type = ColumnType::String) = 0;
    virtual void RemoveColumn(size_t index) = 0;
    
    // Type system
    virtual ColumnType GetColumnType(size_t column) const = 0;
    virtual void SetColumnType(size_t column, ColumnType type) = 0;
    virtual bool TryConvertCell(size_t row, size_t column, ColumnType targetType, std::string& result) const = 0;
    
    // Statistics and analysis
    virtual DataStatistics GetColumnStatistics(size_t column) const = 0;
    virtual DataStatistics GetGlobalStatistics() const = 0;
    
    // Memory management
    virtual size_t GetMemoryFootprint() const = 0;
    virtual void OptimizeMemory() = 0;
    
    // Serialization
    virtual std::vector<uint8_t> SerializeBinary() const = 0;
    virtual bool DeserializeBinary(const std::vector<uint8_t>& data) = 0;
    virtual nlohmann::json SerializeJson() const = 0;
    virtual bool DeserializeJson(const nlohmann::json& json) = 0;
    
    // Clone and copy
    virtual std::unique_ptr<ICSVData> Clone() const = 0;
    virtual std::unique_ptr<ICSVData> Sample(size_t maxRows) const = 0;
};

/**
 * @brief CSVデータの標準実装
 */
class CSVData : public ICSVData {
private:
    std::vector<std::string> m_headers;
    std::vector<std::vector<std::string>> m_rows;
    std::vector<ColumnType> m_columnTypes;
    std::string m_filePath;
    ParseOptions m_parseOptions;
    mutable std::unordered_map<size_t, DataStatistics> m_columnStatsCache;
    mutable std::mutex m_dataMutex;
    
public:
    explicit CSVData(const ParseOptions& options = ParseOptions{});
    CSVData(const CSVData& other);
    CSVData(CSVData&& other) noexcept;
    CSVData& operator=(const CSVData& other);
    CSVData& operator=(CSVData&& other) noexcept;
    
    // ICSVData implementation
    size_t GetRowCount() const override;
    size_t GetColumnCount() const override;
    const std::vector<std::string>& GetHeaders() const override;
    const std::vector<std::string>& GetRow(size_t index) const override;
    std::string GetCell(size_t row, size_t column) const override;
    
    void SetCell(size_t row, size_t column, const std::string& value) override;
    void AddRow(const std::vector<std::string>& row) override;
    void RemoveRow(size_t index) override;
    void AddColumn(const std::string& header, ColumnType type = ColumnType::String) override;
    void RemoveColumn(size_t index) override;
    
    ColumnType GetColumnType(size_t column) const override;
    void SetColumnType(size_t column, ColumnType type) override;
    bool TryConvertCell(size_t row, size_t column, ColumnType targetType, std::string& result) const override;
    
    DataStatistics GetColumnStatistics(size_t column) const override;
    DataStatistics GetGlobalStatistics() const override;
    
    size_t GetMemoryFootprint() const override;
    void OptimizeMemory() override;
    
    std::vector<uint8_t> SerializeBinary() const override;
    bool DeserializeBinary(const std::vector<uint8_t>& data) override;
    nlohmann::json SerializeJson() const override;
    bool DeserializeJson(const nlohmann::json& json) override;
    
    std::unique_ptr<ICSVData> Clone() const override;
    std::unique_ptr<ICSVData> Sample(size_t maxRows) const override;
    
    // Additional functionality
    const std::string& GetFilePath() const { return m_filePath; }
    void SetFilePath(const std::string& path) { m_filePath = path; }
    const ParseOptions& GetParseOptions() const { return m_parseOptions; }
    void SetParseOptions(const ParseOptions& options) { m_parseOptions = options; }
    
private:
    void InvalidateStatsCache();
    DataStatistics CalculateColumnStatistics(size_t column) const;
    void InferColumnTypes();
};

// ==================== Node System ====================

/**
 * @brief ノードピン（入力/出力）
 */
struct NodePin {
    PinId id = INVALID_PIN_ID;
    std::string name;
    std::string description;
    bool isInput = true;
    bool isConnected = false;
    std::string dataType = "CSVData";
    
    NodePin() = default;
    NodePin(PinId pinId, const std::string& pinName, bool input = true)
        : id(pinId), name(pinName), isInput(input) {}
    
    nlohmann::json ToJson() const;
    void FromJson(const nlohmann::json& json);
};

/**
 * @brief ノード間接続
 */
struct Connection {
    ConnectionId id = INVALID_CONNECTION_ID;
    NodeId fromNode = INVALID_NODE_ID;
    PinId fromPin = INVALID_PIN_ID;
    NodeId toNode = INVALID_NODE_ID;
    PinId toPin = INVALID_PIN_ID;
    
    Connection() = default;
    Connection(NodeId from, PinId fromP, NodeId to, PinId toP)
        : fromNode(from), fromPin(fromP), toNode(to), toPin(toP) {}
    
    bool IsValid() const {
        return fromNode != INVALID_NODE_ID && toNode != INVALID_NODE_ID &&
               fromPin != INVALID_PIN_ID && toPin != INVALID_PIN_ID;
    }
    
    nlohmann::json ToJson() const;
    void FromJson(const nlohmann::json& json);
};

/**
 * @brief ノードの基底インターフェース
 */
class INode {
public:
    virtual ~INode() = default;
    
    // Node identification
    virtual NodeId GetId() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetTypeName() const = 0;
    virtual NodeCategory GetCategory() const = 0;
    virtual std::string GetDescription() const = 0;
    
    // Position and UI
    virtual ImVec2 GetPosition() const = 0;
    virtual void SetPosition(const ImVec2& position) = 0;
    virtual ImVec2 GetSize() const = 0;
    virtual ImVec4 GetColor() const = 0;
    
    // Pins
    virtual const std::vector<NodePin>& GetInputPins() const = 0;
    virtual const std::vector<NodePin>& GetOutputPins() const = 0;
    virtual NodePin* FindPin(PinId pinId) = 0;
    virtual bool CanConnectPin(PinId pinId, const INode* otherNode, PinId otherPinId) const = 0;
    
    // State management
    virtual NodeState GetState() const = 0;
    virtual std::string GetErrorMessage() const = 0;
    virtual float GetProgress() const = 0;
    
    // Execution
    virtual bool ValidateInputs() const = 0;
    virtual bool Execute() = 0;
    virtual void Reset() = 0;
    virtual void Cancel() = 0;
    
    // Data flow
    virtual void SetInputData(PinId pinId, std::shared_ptr<ICSVData> data) = 0;
    virtual std::shared_ptr<ICSVData> GetOutputData(PinId pinId) const = 0;
    virtual void OnInputChanged(PinId pinId) = 0;
    virtual void OnConnectionAdded(PinId pinId, NodeId connectedNodeId, PinId connectedPinId) = 0;
    virtual void OnConnectionRemoved(PinId pinId, NodeId connectedNodeId, PinId connectedPinId) = 0;
    
    // UI rendering
    virtual void RenderNode() = 0;
    virtual void RenderProperties() = 0;
    virtual void RenderTooltip() = 0;
    
    // Serialization
    virtual nlohmann::json Serialize() const = 0;
    virtual bool Deserialize(const nlohmann::json& data) = 0;
    
    // Events
    virtual void OnCreated() {}
    virtual void OnDestroyed() {}
    virtual void OnSelected() {}
    virtual void OnDeselected() {}
};

/**
 * @brief ノードの基底実装クラス
 */
class NodeBase : public INode {
protected:
    NodeId m_id = INVALID_NODE_ID;
    std::string m_name;
    std::string m_typeName;
    NodeCategory m_category = NodeCategory::Custom;
    std::string m_description;
    ImVec2 m_position = {0, 0};
    ImVec2 m_size = {200, 100};
    ImVec4 m_color = {0.5f, 0.5f, 0.5f, 1.0f};
    
    std::vector<NodePin> m_inputPins;
    std::vector<NodePin> m_outputPins;
    
    std::atomic<NodeState> m_state{NodeState::Idle};
    std::string m_errorMessage;
    std::atomic<float> m_progress{0.0f};
    
    std::unordered_map<PinId, std::shared_ptr<ICSVData>> m_inputData;
    std::unordered_map<PinId, std::shared_ptr<ICSVData>> m_outputData;
    
    mutable std::mutex m_dataMutex;
    
public:
    NodeBase(NodeId id, const std::string& typeName, NodeCategory category);
    virtual ~NodeBase() = default;
    
    // INode implementation
    NodeId GetId() const override { return m_id; }
    std::string GetName() const override { return m_name; }
    std::string GetTypeName() const override { return m_typeName; }
    NodeCategory GetCategory() const override { return m_category; }
    std::string GetDescription() const override { return m_description; }
    
    ImVec2 GetPosition() const override { return m_position; }
    void SetPosition(const ImVec2& position) override { m_position = position; }
    ImVec2 GetSize() const override { return m_size; }
    ImVec4 GetColor() const override { return m_color; }
    
    const std::vector<NodePin>& GetInputPins() const override { return m_inputPins; }
    const std::vector<NodePin>& GetOutputPins() const override { return m_outputPins; }
    NodePin* FindPin(PinId pinId) override;
    bool CanConnectPin(PinId pinId, const INode* otherNode, PinId otherPinId) const override;
    
    NodeState GetState() const override { return m_state.load(); }
    std::string GetErrorMessage() const override { return m_errorMessage; }
    float GetProgress() const override { return m_progress.load(); }
    
    bool ValidateInputs() const override;
    bool Execute() override;
    void Reset() override;
    void Cancel() override;
    
    void SetInputData(PinId pinId, std::shared_ptr<ICSVData> data) override;
    std::shared_ptr<ICSVData> GetOutputData(PinId pinId) const override;
    void OnInputChanged(PinId pinId) override {}
    void OnConnectionAdded(PinId pinId, NodeId connectedNodeId, PinId connectedPinId) override {}
    void OnConnectionRemoved(PinId pinId, NodeId connectedNodeId, PinId connectedPinId) override {}
    
    void RenderNode() override;
    void RenderProperties() override;
    void RenderTooltip() override;
    
    nlohmann::json Serialize() const override;
    bool Deserialize(const nlohmann::json& data) override;
    
protected:
    // Template methods for derived classes
    virtual bool OnExecute() = 0;
    virtual void OnRenderProperties() {}
    virtual nlohmann::json OnSerialize() const { return nlohmann::json::object(); }
    virtual bool OnDeserialize(const nlohmann::json& data) { return true; }
    
    // Utility methods
    void SetState(NodeState state);
    void SetError(const std::string& message);
    void ClearError();
    void SetProgress(float progress);
    
    PinId AddInputPin(const std::string& name, const std::string& description = "");
    PinId AddOutputPin(const std::string& name, const std::string& description = "");
    
private:
    static std::atomic<PinId> s_nextPinId;
};

// ==================== Built-in Node Types ====================

/**
 * @brief CSV読み込みノード
 */
class CSVReaderNode : public NodeBase {
private:
    std::string m_filePath;
    ParseOptions m_parseOptions;
    std::filesystem::file_time_type m_lastModified{};
    std::shared_ptr<ICSVData> m_cachedData;
    
public:
    CSVReaderNode(NodeId id);
    
    void SetFilePath(const std::string& path);
    const std::string& GetFilePath() const { return m_filePath; }
    
    void SetParseOptions(const ParseOptions& options) { m_parseOptions = options; }
    const ParseOptions& GetParseOptions() const { return m_parseOptions; }
    
protected:
    bool OnExecute() override;
    void OnRenderProperties() override;
    nlohmann::json OnSerialize() const override;
    bool OnDeserialize(const nlohmann::json& data) override;
    
private:
    bool LoadCSVFile();
    bool IsFileModified() const;
    std::string DetectEncoding(const std::string& filePath) const;
    char DetectDelimiter(const std::string& sample) const;
    bool DetectHeader(const std::vector<std::string>& firstRow) const;
};

/**
 * @brief フィルターノード
 */
class FilterNode : public NodeBase {
public:
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
    
private:
    size_t m_targetColumn = 0;
    FilterOperator m_operator = FilterOperator::Equals;
    std::string m_filterValue;
    bool m_caseSensitive = false;
    bool m_useRegex = false;
    
public:
    FilterNode(NodeId id);
    
    void SetTargetColumn(size_t column) { m_targetColumn = column; }
    size_t GetTargetColumn() const { return m_targetColumn; }
    
    void SetOperator(FilterOperator op) { m_operator = op; }
    FilterOperator GetOperator() const { return m_operator; }
    
    void SetFilterValue(const std::string& value) { m_filterValue = value; }
    const std::string& GetFilterValue() const { return m_filterValue; }
    
    void SetCaseSensitive(bool sensitive) { m_caseSensitive = sensitive; }
    bool IsCaseSensitive() const { return m_caseSensitive; }
    
protected:
    bool OnExecute() override;
    void OnRenderProperties() override;
    nlohmann::json OnSerialize() const override;
    bool OnDeserialize(const nlohmann::json& data) override;
    
private:
    bool ApplyFilter(const std::string& cellValue) const;
    static const char* GetOperatorName(FilterOperator op);
};

/**
 * @brief ソートノード
 */
class SortNode : public NodeBase {
public:
    enum class SortOrder {
        Ascending,
        Descending
    };
    
private:
    size_t m_sortColumn = 0;
    SortOrder m_sortOrder = SortOrder::Ascending;
    bool m_stableSort = true;
    
public:
    SortNode(NodeId id);
    
    void SetSortColumn(size_t column) { m_sortColumn = column; }
    size_t GetSortColumn() const { return m_sortColumn; }
    
    void SetSortOrder(SortOrder order) { m_sortOrder = order; }
    SortOrder GetSortOrder() const { return m_sortOrder; }
    
    void SetStableSort(bool stable) { m_stableSort = stable; }
    bool IsStableSort() const { return m_stableSort; }
    
protected:
    bool OnExecute() override;
    void OnRenderProperties() override;
    nlohmann::json OnSerialize() const override;
    bool OnDeserialize(const nlohmann::json& data) override;
    
private:
    bool CompareRows(const std::vector<std::string>& a, const std::vector<std::string>& b) const;
    static const char* GetSortOrderName(SortOrder order);
};

/**
 * @brief 集計ノード
 */
class AggregateNode : public NodeBase {
public:
    enum class AggregateFunction {
        Count,
        Sum,
        Average,
        Min,
        Max,
        StdDev,
        Variance,
        First,
        Last
    };
    
    struct AggregateOperation {
        size_t targetColumn = 0;
        AggregateFunction function = AggregateFunction::Count;
        std::string outputName;
        
        nlohmann::json ToJson() const;
        void FromJson(const nlohmann::json& json);
    };
    
private:
    std::vector<size_t> m_groupByColumns;
    std::vector<AggregateOperation> m_operations;
    
public:
    AggregateNode(NodeId id);
    
    void SetGroupByColumns(const std::vector<size_t>& columns) { m_groupByColumns = columns; }
    const std::vector<size_t>& GetGroupByColumns() const { return m_groupByColumns; }
    
    void SetOperations(const std::vector<AggregateOperation>& operations) { m_operations = operations; }
    const std::vector<AggregateOperation>& GetOperations() const { return m_operations; }
    
    void AddOperation(const AggregateOperation& operation) { m_operations.push_back(operation); }
    void RemoveOperation(size_t index);
    
protected:
    bool OnExecute() override;
    void OnRenderProperties() override;
    nlohmann::json OnSerialize() const override;
    bool OnDeserialize(const nlohmann::json& data) override;
    
private:
    std::unique_ptr<ICSVData> GroupAndAggregate(const ICSVData& input) const;
    double ApplyAggregateFunction(AggregateFunction func, const std::vector<std::string>& values) const;
    static const char* GetFunctionName(AggregateFunction func);
};

/**
 * @brief CSV出力ノード
 */
class CSVWriterNode : public NodeBase {
private:
    std::string m_outputPath;
    ParseOptions m_writeOptions;
    bool m_appendMode = false;
    
public:
    CSVWriterNode(NodeId id);
    
    void SetOutputPath(const std::string& path) { m_outputPath = path; }
    const std::string& GetOutputPath() const { return m_outputPath; }
    
    void SetWriteOptions(const ParseOptions& options) { m_writeOptions = options; }
    const ParseOptions& GetWriteOptions() const { return m_writeOptions; }
    
    void SetAppendMode(bool append) { m_appendMode = append; }
    bool IsAppendMode() const { return m_appendMode; }
    
protected:
    bool OnExecute() override;
    void OnRenderProperties() override;
    nlohmann::json OnSerialize() const override;
    bool OnDeserialize(const nlohmann::json& data) override;
    
private:
    bool WriteCSVFile(const ICSVData& data);
    std::string EscapeCSVField(const std::string& field) const;
};

// ==================== Graph Management ====================

/**
 * @brief ノードグラフ管理インターフェース
 */
class INodeGraph {
public:
    virtual ~INodeGraph() = default;
    
    // Node management
    virtual NodeId CreateNode(const std::string& nodeType) = 0;
    virtual bool DeleteNode(NodeId nodeId) = 0;
    virtual INode* FindNode(NodeId nodeId) = 0;
    virtual std::vector<INode*> GetAllNodes() = 0;
    virtual std::vector<NodeId> GetNodeIds() const = 0;
    
    // Connection management
    virtual ConnectionId CreateConnection(NodeId fromNode, PinId fromPin, NodeId toNode, PinId toPin) = 0;
    virtual bool DeleteConnection(ConnectionId connectionId) = 0;
    virtual bool DeleteConnection(NodeId fromNode, PinId fromPin, NodeId toNode, PinId toPin) = 0;
    virtual std::vector<Connection> GetConnections() const = 0;
    virtual std::vector<Connection> GetNodeConnections(NodeId nodeId) const = 0;
    
    // Graph validation
    virtual bool ValidateGraph() = 0;
    virtual bool HasCycle() = 0;
    virtual std::vector<NodeId> GetExecutionOrder() = 0;
    virtual std::vector<NodeId> GetDependentNodes(NodeId nodeId) = 0;
    virtual std::vector<NodeId> GetDependencyNodes(NodeId nodeId) = 0;
    
    // Serialization
    virtual nlohmann::json Serialize() const = 0;
    virtual bool Deserialize(const nlohmann::json& data) = 0;
    virtual void Clear() = 0;
    
    // Events
    virtual void SetNodeCreatedCallback(std::function<void(NodeId)> callback) = 0;
    virtual void SetNodeDeletedCallback(std::function<void(NodeId)> callback) = 0;
    virtual void SetConnectionCreatedCallback(std::function<void(ConnectionId)> callback) = 0;
    virtual void SetConnectionDeletedCallback(std::function<void(ConnectionId)> callback) = 0;
};

/**
 * @brief ノードグラフの標準実装
 */
class NodeGraph : public INodeGraph {
private:
    std::unordered_map<NodeId, std::unique_ptr<INode>> m_nodes;
    std::unordered_map<ConnectionId, Connection> m_connections;
    
    std::atomic<NodeId> m_nextNodeId{1};
    std::atomic<ConnectionId> m_nextConnectionId{1};
    
    mutable std::shared_mutex m_graphMutex;
    
    // Callbacks
    std::function<void(NodeId)> m_nodeCreatedCallback;
    std::function<void(NodeId)> m_nodeDeletedCallback;
    std::function<void(ConnectionId)> m_connectionCreatedCallback;
    std::function<void(ConnectionId)> m_connectionDeletedCallback;
    
public:
    NodeGraph();
    ~NodeGraph() = default;
    
    // INodeGraph implementation
    NodeId CreateNode(const std::string& nodeType) override;
    bool DeleteNode(NodeId nodeId) override;
    INode* FindNode(NodeId nodeId) override;
    std::vector<INode*> GetAllNodes() override;
    std::vector<NodeId> GetNodeIds() const override;
    
    ConnectionId CreateConnection(NodeId fromNode, PinId fromPin, NodeId toNode, PinId toPin) override;
    bool DeleteConnection(ConnectionId connectionId) override;
    bool DeleteConnection(NodeId fromNode, PinId fromPin, NodeId toNode, PinId toPin) override;
    std::vector<Connection> GetConnections() const override;
    std::vector<Connection> GetNodeConnections(NodeId nodeId) const override;
    
    bool ValidateGraph() override;
    bool HasCycle() override;
    std::vector<NodeId> GetExecutionOrder() override;
    std::vector<NodeId> GetDependentNodes(NodeId nodeId) override;
    std::vector<NodeId> GetDependencyNodes(NodeId nodeId) override;
    
    nlohmann::json Serialize() const override;
    bool Deserialize(const nlohmann::json& data) override;
    void Clear() override;
    
    void SetNodeCreatedCallback(std::function<void(NodeId)> callback) override { m_nodeCreatedCallback = callback; }
    void SetNodeDeletedCallback(std::function<void(NodeId)> callback) override { m_nodeDeletedCallback = callback; }
    void SetConnectionCreatedCallback(std::function<void(ConnectionId)> callback) override { m_connectionCreatedCallback = callback; }
    void SetConnectionDeletedCallback(std::function<void(ConnectionId)> callback) override { m_connectionDeletedCallback = callback; }
    
private:
    bool HasCycleInternal(NodeId nodeId, std::unordered_set<NodeId>& visited, std::unordered_set<NodeId>& recursionStack) const;
    void GetExecutionOrderInternal(NodeId nodeId, std::unordered_set<NodeId>& visited, std::vector<NodeId>& result) const;
};

// ==================== Node Factory ====================

/**
 * @brief ノードファクトリー
 */
class NodeFactory {
public:
    using CreateNodeFunc = std::function<std::unique_ptr<INode>(NodeId)>;
    
    struct NodeTypeInfo {
        std::string typeName;
        std::string displayName;
        std::string description;
        NodeCategory category;
        CreateNodeFunc creator;
    };
    
    static void RegisterNodeType(const NodeTypeInfo& info);
    static std::unique_ptr<INode> CreateNode(const std::string& typeName, NodeId nodeId);
    static std::vector<std::string> GetAvailableNodeTypes();
    static std::vector<NodeTypeInfo> GetNodeTypesByCategory(NodeCategory category);
    static NodeTypeInfo* GetNodeTypeInfo(const std::string& typeName);
    static void RegisterBuiltinNodes();
    
private:
    static std::unordered_map<std::string, NodeTypeInfo> s_nodeTypes;
    static std::mutex s_registrationMutex;
};

// Node registration helper macro
#define REGISTER_NODE_TYPE(TypeName, ClassName, Category, DisplayName, Description) \
    static bool s_##ClassName##_registered = []() { \
        NodeFactory::RegisterNodeType({ \
            TypeName, \
            DisplayName, \
            Description, \
            Category, \
            [](NodeId id) -> std::unique_ptr<INode> { \
                return std::make_unique<ClassName>(id); \
            } \
        }); \
        return true; \
    }();

// ==================== Execution Engine ====================

/**
 * @brief ノードグラフ実行エンジン
 */
class IExecutionEngine {
public:
    virtual ~IExecutionEngine() = default;
    
    // Execution control
    virtual bool ExecuteGraph() = 0;
    virtual bool ExecuteNode(NodeId nodeId) = 0;
    virtual bool ExecuteSubgraph(const std::vector<NodeId>& nodeIds) = 0;
    virtual void CancelExecution() = 0;
    virtual bool IsExecuting() const = 0;
    
    // Cache management
    virtual void InvalidateNode(NodeId nodeId) = 0;
    virtual void InvalidateDownstream(NodeId nodeId) = 0;
    virtual void ClearCache() = 0;
    virtual std::shared_ptr<ICSVData> GetCachedData(NodeId nodeId, PinId pinId) = 0;
    
    // Progress reporting
    virtual void SetProgressCallback(std::function<void(float, const std::string&)> callback) = 0;
    virtual float GetOverallProgress() const = 0;
    
    // Statistics
    virtual std::chrono::milliseconds GetLastExecutionTime() const = 0;
    virtual size_t GetCacheMemoryUsage() const = 0;
    virtual void GetExecutionStatistics(nlohmann::json& stats) const = 0;
};

/**
 * @brief 実行エンジンの標準実装
 */
class ExecutionEngine : public IExecutionEngine {
private:
    INodeGraph* m_nodeGraph;
    std::unordered_map<NodeId, std::unordered_map<PinId, std::shared_ptr<ICSVData>>> m_cache;
    std::unordered_set<NodeId> m_dirtyNodes;
    
    std::atomic<bool> m_isExecuting{false};
    std::atomic<bool> m_shouldCancel{false};
    std::atomic<float> m_overallProgress{0.0f};
    
    std::function<void(float, const std::string&)> m_progressCallback;
    
    std::chrono::steady_clock::time_point m_executionStartTime;
    std::chrono::milliseconds m_lastExecutionTime{0};
    
    mutable std::shared_mutex m_cacheMutex;
    mutable std::mutex m_executionMutex;
    
public:
    explicit ExecutionEngine(INodeGraph* nodeGraph);
    ~ExecutionEngine() = default;
    
    // IExecutionEngine implementation
    bool ExecuteGraph() override;
    bool ExecuteNode(NodeId nodeId) override;
    bool ExecuteSubgraph(const std::vector<NodeId>& nodeIds) override;
    void CancelExecution() override;
    bool IsExecuting() const override { return m_isExecuting.load(); }
    
    void InvalidateNode(NodeId nodeId) override;
    void InvalidateDownstream(NodeId nodeId) override;
    void ClearCache() override;
    std::shared_ptr<ICSVData> GetCachedData(NodeId nodeId, PinId pinId) override;
    
    void SetProgressCallback(std::function<void(float, const std::string&)> callback) override;
    float GetOverallProgress() const override { return m_overallProgress.load(); }
    
    std::chrono::milliseconds GetLastExecutionTime() const override { return m_lastExecutionTime; }
    size_t GetCacheMemoryUsage() const override;
    void GetExecutionStatistics(nlohmann::json& stats) const override;
    
private:
    bool ExecuteNodeInternal(INode* node);
    void PropagateData(NodeId nodeId);
    void UpdateProgress(const std::string& message);
    void InvalidateDownstreamRecursive(NodeId nodeId, std::unordered_set<NodeId>& visited);
};

// ==================== CSV Parser ====================

/**
 * @brief CSV解析ユーティリティ
 */
class CSVParser {
public:
    static std::unique_ptr<ICSVData> ParseFile(const std::string& filePath, const ParseOptions& options = ParseOptions{});
    static std::unique_ptr<ICSVData> ParseString(const std::string& csvContent, const ParseOptions& options = ParseOptions{});
    static bool WriteFile(const std::string& filePath, const ICSVData& data, const ParseOptions& options = ParseOptions{});
    
    // Analysis utilities
    static std::string DetectEncoding(const std::string& filePath);
    static char DetectDelimiter(const std::string& sample);
    static bool DetectHeader(const std::vector<std::string>& firstRow);
    static std::vector<ColumnType> InferColumnTypes(const ICSVData& data, size_t sampleSize = 1000);
    
    // Validation
    static bool ValidateCSVFile(const std::string& filePath, std::string& errorMessage);
    static bool ValidateCSVContent(const std::string& content, const ParseOptions& options, std::string& errorMessage);
    
private:
    static std::string ConvertEncoding(const std::string& input, const std::string& fromEncoding, const std::string& toEncoding);
    static std::vector<std::string> ParseCSVLine(const std::string& line, const ParseOptions& options);
    static std::string EscapeCSVField(const std::string& field, const ParseOptions& options);
    static ColumnType InferColumnType(const std::vector<std::string>& values);
    static bool IsNumeric(const std::string& value);
    static bool IsDate(const std::string& value);
    static bool IsBoolean(const std::string& value);
};

// ==================== Streaming Support ====================

/**
 * @brief 大容量ファイル用ストリーミングリーダー
 */
class StreamingCSVReader {
private:
    std::ifstream m_fileStream;
    ParseOptions m_options;
    size_t m_currentRow = 0;
    size_t m_totalSize = 0;
    size_t m_currentPosition = 0;
    std::vector<std::string> m_headers;
    
public:
    explicit StreamingCSVReader(const std::string& filePath, const ParseOptions& options = ParseOptions{});
    ~StreamingCSVReader();
    
    bool IsOpen() const { return m_fileStream.is_open(); }
    bool HasNextRow();
    std::vector<std::string> ReadNextRow();
    void Reset();
    
    size_t GetCurrentRowNumber() const { return m_currentRow; }
    size_t GetFileSize() const { return m_totalSize; }
    size_t GetCurrentPosition() const { return m_currentPosition; }
    float GetProgress() const;
    
    const std::vector<std::string>& GetHeaders() const { return m_headers; }
    
private:
    void ReadHeaders();
};

} // namespace CSVNodeEditor