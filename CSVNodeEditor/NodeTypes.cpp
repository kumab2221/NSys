#include "NodeTypes.h"
#include "imgui.h"
#include "imnodes.h"
#include <imgui.h>

// CSV読み込みノード
CSVLoadNode::CSVLoadNode(int id)
    : BaseNode(id, "CSV読み込み")
    , fileLoaded(false)
{
    outputData = std::make_shared<CSVData>();
}

void CSVLoadNode::Render()
{
    // 入力ピン（なし）
    
    // 出力ピン
    ImNodes::BeginOutputAttribute(nodeId * 100 + 1);
    ImGui::Text("出力");
    ImNodes::EndOutputAttribute();
    
    // ファイルパス入力
    ImGui::Text("ファイルパス:");
    static char filePathBuffer[256] = "";
    if (ImGui::InputText("##FilePath", filePathBuffer, sizeof(filePathBuffer)))
    {
        filePath = filePathBuffer;
    }
    
    // ファイル読み込みボタン
    if (ImGui::Button("ファイルを読み込み"))
    {
        if (outputData->LoadFromFile(filePath))
        {
            fileLoaded = true;
        }
    }
    
    // 状態表示
    if (fileLoaded)
    {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "? 読み込み完了");
        ImGui::Text("行数: %zu", outputData->GetRowCount());
        ImGui::Text("列数: %zu", outputData->GetColumnCount());
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "? 未読み込み");
    }
}

void CSVLoadNode::Process()
{
    // ファイルが読み込まれていない場合は読み込みを実行
    if (!fileLoaded && !filePath.empty())
    {
        fileLoaded = outputData->LoadFromFile(filePath);
    }
}

void CSVLoadNode::SaveState()
{
    // 状態保存処理をここに実装
}

void CSVLoadNode::LoadState()
{
    // 状態読み込み処理をここに実装
}

// フィルターノード
FilterNode::FilterNode(int id)
    : BaseNode(id, "フィルター")
    , filterOperator("==")
{
    inputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void FilterNode::Render()
{
    // 入力ピン
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("入力");
    ImNodes::EndInputAttribute();
    
    // 出力ピン
    ImNodes::BeginOutputAttribute(nodeId * 100 + 2);
    ImGui::Text("出力");
    ImNodes::EndOutputAttribute();
    
    // フィルター設定
    ImGui::Text("フィルター設定:");
    
    // 列選択
    static char columnBuffer[128] = "";
    if (ImGui::InputText("列名", columnBuffer, sizeof(columnBuffer)))
    {
        filterColumn = columnBuffer;
    }
    
    // 演算子選択
    const char* operators[] = { "==", "!=", ">", "<", ">=", "<=", "contains" };
    if (ImGui::BeginCombo("演算子", filterOperator.c_str()))
    {
        for (const char* op : operators)
        {
            if (ImGui::Selectable(op, filterOperator == op))
            {
                filterOperator = op;
            }
        }
        ImGui::EndCombo();
    }
    
    // 値入力
    static char valueBuffer[128] = "";
    if (ImGui::InputText("値", valueBuffer, sizeof(valueBuffer)))
    {
        filterValue = valueBuffer;
    }
    
    // フィルター実行ボタン
    if (ImGui::Button("フィルター実行"))
    {
        Process();
    }
    
    // 結果表示
    if (!outputData->GetRows().empty())
    {
        ImGui::Text("フィルター結果: %zu 行", outputData->GetRowCount());
    }
}

void FilterNode::Process()
{
    if (inputData && !filterColumn.empty() && !filterValue.empty())
    {
        // フィルター処理を実装
        auto filteredRows = inputData->FilterRows(filterColumn, filterValue);
        
        // 結果を出力データに設定
        outputData->Clear();
        outputData->AddRow(inputData->GetHeaders()); // ヘッダーを追加
        for (const auto& row : filteredRows)
        {
            outputData->AddRow(row);
        }
    }
}

void FilterNode::SaveState()
{
    // 状態保存処理をここに実装
}

void FilterNode::LoadState()
{
    // 状態読み込み処理をここに実装
}

// ソートノード
SortNode::SortNode(int id)
    : BaseNode(id, "ソート")
    , ascending(true)
{
    inputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void SortNode::Render()
{
    // 入力ピン
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("入力");
    ImNodes::EndInputAttribute();
    
    // 出力ピン
    ImNodes::BeginOutputAttribute(nodeId * 100 + 2);
    ImGui::Text("出力");
    ImNodes::EndOutputAttribute();
    
    // ソート設定
    ImGui::Text("ソート設定:");
    
    // 列選択
    static char columnBuffer[128] = "";
    if (ImGui::InputText("列名", columnBuffer, sizeof(columnBuffer)))
    {
        sortColumn = columnBuffer;
    }
    
    // 昇順/降順選択
    ImGui::Checkbox("昇順", &ascending);
    
    // ソート実行ボタン
    if (ImGui::Button("ソート実行"))
    {
        Process();
    }
}

void SortNode::Process()
{
    if (inputData && !sortColumn.empty())
    {
        // ソート処理を実装
        *outputData = *inputData; // データをコピー
        outputData->SortByColumn(sortColumn, ascending);
    }
}

void SortNode::SaveState()
{
    // 状態保存処理をここに実装
}

void SortNode::LoadState()
{
    // 状態読み込み処理をここに実装
}

// 集計ノード
AggregateNode::AggregateNode(int id)
    : BaseNode(id, "集計")
    , aggregateFunction("sum")
{
    inputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void AggregateNode::Render()
{
    // 入力ピン
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("入力");
    ImNodes::EndInputAttribute();
    
    // 出力ピン
    ImNodes::BeginOutputAttribute(nodeId * 100 + 2);
    ImGui::Text("出力");
    ImNodes::EndOutputAttribute();
    
    // 集計設定
    ImGui::Text("集計設定:");
    
    // グループ化列
    static char groupColumnBuffer[128] = "";
    if (ImGui::InputText("グループ化列", groupColumnBuffer, sizeof(groupColumnBuffer)))
    {
        groupColumn = groupColumnBuffer;
    }
    
    // 集計列
    static char aggregateColumnBuffer[128] = "";
    if (ImGui::InputText("集計列", aggregateColumnBuffer, sizeof(aggregateColumnBuffer)))
    {
        aggregateColumn = aggregateColumnBuffer;
    }
    
    // 集計関数選択
    const char* functions[] = { "sum", "average", "count", "min", "max" };
    if (ImGui::BeginCombo("集計関数", aggregateFunction.c_str()))
    {
        for (const char* func : functions)
        {
            if (ImGui::Selectable(func, aggregateFunction == func))
            {
                aggregateFunction = func;
            }
        }
        ImGui::EndCombo();
    }
    
    // 集計実行ボタン
    if (ImGui::Button("集計実行"))
    {
        Process();
    }
}

void AggregateNode::Process()
{
    if (inputData && !groupColumn.empty() && !aggregateColumn.empty())
    {
        // 集計処理を実装
        // 簡易実装のため、基本的な集計のみ
        if (aggregateFunction == "sum")
        {
            double sum = inputData->GetColumnSum(aggregateColumn);
            // 結果を出力データに設定
        }
        else if (aggregateFunction == "average")
        {
            double avg = inputData->GetColumnAverage(aggregateColumn);
            // 結果を出力データに設定
        }
    }
}

void AggregateNode::SaveState()
{
    // 状態保存処理をここに実装
}

void AggregateNode::LoadState()
{
    // 状態読み込み処理をここに実装
}

// 結合ノード
JoinNode::JoinNode(int id)
    : BaseNode(id, "結合")
    , joinType("inner")
{
    leftInputData = std::make_shared<CSVData>();
    rightInputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void JoinNode::Render()
{
    // 左入力ピン
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("左入力");
    ImNodes::EndInputAttribute();
    
    // 右入力ピン
    ImNodes::BeginInputAttribute(nodeId * 100 + 2);
    ImGui::Text("右入力");
    ImNodes::EndInputAttribute();
    
    // 出力ピン
    ImNodes::BeginOutputAttribute(nodeId * 100 + 3);
    ImGui::Text("出力");
    ImNodes::EndOutputAttribute();
    
    // 結合設定
    ImGui::Text("結合設定:");
    
    // 左結合列
    static char leftColumnBuffer[128] = "";
    if (ImGui::InputText("左結合列", leftColumnBuffer, sizeof(leftColumnBuffer)))
    {
        leftJoinColumn = leftColumnBuffer;
    }
    
    // 右結合列
    static char rightColumnBuffer[128] = "";
    if (ImGui::InputText("右結合列", rightColumnBuffer, sizeof(rightColumnBuffer)))
    {
        rightJoinColumn = rightColumnBuffer;
    }
    
    // 結合タイプ選択
    const char* joinTypes[] = { "inner", "left", "right", "outer" };
    if (ImGui::BeginCombo("結合タイプ", joinType.c_str()))
    {
        for (const char* type : joinTypes)
        {
            if (ImGui::Selectable(type, joinType == type))
            {
                joinType = type;
            }
        }
        ImGui::EndCombo();
    }
    
    // 結合実行ボタン
    if (ImGui::Button("結合実行"))
    {
        Process();
    }
}

void JoinNode::Process()
{
    if (leftInputData && rightInputData && !leftJoinColumn.empty() && !rightJoinColumn.empty())
    {
        // 結合処理を実装
        // 簡易実装のため、基本的な結合のみ
    }
}

void JoinNode::SaveState()
{
    // 状態保存処理をここに実装
}

void JoinNode::LoadState()
{
    // 状態読み込み処理をここに実装
}

// 出力ノード
OutputNode::OutputNode(int id)
    : BaseNode(id, "CSV出力")
{
    inputData = std::make_shared<CSVData>();
}

void OutputNode::Render()
{
    // 入力ピン
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("入力");
    ImNodes::EndInputAttribute();
    
    // 出力設定
    ImGui::Text("出力設定:");
    
    // 出力パス
    static char outputPathBuffer[256] = "";
    if (ImGui::InputText("出力パス", outputPathBuffer, sizeof(outputPathBuffer)))
    {
        outputPath = outputPathBuffer;
    }
    
    // 保存ボタン
    if (ImGui::Button("CSV保存"))
    {
        Process();
    }
    
    // 状態表示
    if (!inputData->GetRows().empty())
    {
        ImGui::Text("保存対象: %zu 行", inputData->GetRowCount());
    }
}

void OutputNode::Process()
{
    if (inputData && !outputPath.empty())
    {
        inputData->SaveToFile(outputPath);
    }
}

void OutputNode::SaveState()
{
    // 状態保存処理をここに実装
}

void OutputNode::LoadState()
{
    // 状態読み込み処理をここに実装
}
