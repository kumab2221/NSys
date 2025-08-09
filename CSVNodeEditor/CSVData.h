#pragma once

#include <vector>
#include <string>
#include <memory>
#include <fstream>

// CSVデータを格納するクラス
class CSVData
{
public:
    CSVData();
    ~CSVData();

    // ファイル操作
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename);

    // データアクセス
    const std::vector<std::string>& GetHeaders() const { return headers; }
    const std::vector<std::vector<std::string>>& GetRows() const { return rows; }
    
    // データ操作
    void AddRow(const std::vector<std::string>& row);
    void RemoveRow(size_t index);
    void Clear();

    // 統計情報
    size_t GetRowCount() const { return rows.size(); }
    size_t GetColumnCount() const { return headers.size(); }

    // データフィルタリング
    std::vector<std::vector<std::string>> FilterRows(const std::string& column, const std::string& value);
    
    // データソート
    void SortByColumn(const std::string& column, bool ascending = true);

    // データ集計
    double GetColumnSum(const std::string& column);
    double GetColumnAverage(const std::string& column);
    std::string GetColumnMin(const std::string& column);
    std::string GetColumnMax(const std::string& column);

private:
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;

    // ヘルパー関数
    std::vector<std::string> ParseCSVLine(const std::string& line);
    int GetColumnIndex(const std::string& column) const;
    bool IsNumeric(const std::string& value) const;
    double StringToDouble(const std::string& value) const;
};
