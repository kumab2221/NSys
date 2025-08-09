#pragma once

#include "NodeEditor.h"
#include "CSVData.h"
#include <string>

// CSV読み込みノード
class CSVLoadNode : public BaseNode
{
public:
    CSVLoadNode(int id);
    void Render() override;
    void Process() override;
    void SaveState() override;
    void LoadState() override;

private:
    std::string filePath;
    bool fileLoaded;
    std::shared_ptr<CSVData> outputData;
};

// フィルターノード
class FilterNode : public BaseNode
{
public:
    FilterNode(int id);
    void Render() override;
    void Process() override;
    void SaveState() override;
    void LoadState() override;

private:
    std::string filterColumn;
    std::string filterValue;
    std::string filterOperator; // "==", "!=", ">", "<", ">=", "<=", "contains"
    std::shared_ptr<CSVData> inputData;
    std::shared_ptr<CSVData> outputData;
};

// ソートノード
class SortNode : public BaseNode
{
public:
    SortNode(int id);
    void Render() override;
    void Process() override;
    void SaveState() override;
    void LoadState() override;

private:
    std::string sortColumn;
    bool ascending;
    std::shared_ptr<CSVData> inputData;
    std::shared_ptr<CSVData> outputData;
};

// 集計ノード
class AggregateNode : public BaseNode
{
public:
    AggregateNode(int id);
    void Render() override;
    void Process() override;
    void SaveState() override;
    void LoadState() override;

private:
    std::string groupColumn;
    std::string aggregateColumn;
    std::string aggregateFunction; // "sum", "average", "count", "min", "max"
    std::shared_ptr<CSVData> inputData;
    std::shared_ptr<CSVData> outputData;
};

// 結合ノード
class JoinNode : public BaseNode
{
public:
    JoinNode(int id);
    void Render() override;
    void Process() override;
    void SaveState() override;
    void LoadState() override;

private:
    std::string leftJoinColumn;
    std::string rightJoinColumn;
    std::string joinType; // "inner", "left", "right", "outer"
    std::shared_ptr<CSVData> leftInputData;
    std::shared_ptr<CSVData> rightInputData;
    std::shared_ptr<CSVData> outputData;
};

// 出力ノード
class OutputNode : public BaseNode
{
public:
    OutputNode(int id);
    void Render() override;
    void Process() override;
    void SaveState() override;
    void LoadState() override;

private:
    std::string outputPath;
    std::shared_ptr<CSVData> inputData;
};
