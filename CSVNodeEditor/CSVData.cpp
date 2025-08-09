#include "CSVData.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

CSVData::CSVData()
{
}

CSVData::~CSVData()
{
}

bool CSVData::LoadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    Clear();
    std::string line;
    
    // ヘッダー行を読み込み
    if (std::getline(file, line))
    {
        headers = ParseCSVLine(line);
    }

    // データ行を読み込み
    while (std::getline(file, line))
    {
        if (!line.empty())
        {
            rows.push_back(ParseCSVLine(line));
        }
    }

    file.close();
    return true;
}

bool CSVData::SaveToFile(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    // ヘッダーを書き込み
    for (size_t i = 0; i < headers.size(); ++i)
    {
        if (i > 0) file << ",";
        file << headers[i];
    }
    file << std::endl;

    // データ行を書き込み
    for (const auto& row : rows)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            if (i > 0) file << ",";
            file << row[i];
        }
        file << std::endl;
    }

    file.close();
    return true;
}

void CSVData::AddRow(const std::vector<std::string>& row)
{
    rows.push_back(row);
}

void CSVData::RemoveRow(size_t index)
{
    if (index < rows.size())
    {
        rows.erase(rows.begin() + index);
    }
}

void CSVData::Clear()
{
    headers.clear();
    rows.clear();
}

std::vector<std::vector<std::string>> CSVData::FilterRows(const std::string& column, const std::string& value)
{
    std::vector<std::vector<std::string>> filteredRows;
    int columnIndex = GetColumnIndex(column);
    
    if (columnIndex >= 0)
    {
        for (const auto& row : rows)
        {
            if (columnIndex < static_cast<int>(row.size()) && row[columnIndex] == value)
            {
                filteredRows.push_back(row);
            }
        }
    }
    
    return filteredRows;
}

void CSVData::SortByColumn(const std::string& column, bool ascending)
{
    int columnIndex = GetColumnIndex(column);
    if (columnIndex >= 0)
    {
        std::sort(rows.begin(), rows.end(),
            [columnIndex, ascending](const std::vector<std::string>& a, const std::vector<std::string>& b) {
                if (columnIndex >= static_cast<int>(a.size()) || columnIndex >= static_cast<int>(b.size()))
                    return false;
                
                if (ascending)
                    return a[columnIndex] < b[columnIndex];
                else
                    return a[columnIndex] > b[columnIndex];
            });
    }
}

double CSVData::GetColumnSum(const std::string& column)
{
    int columnIndex = GetColumnIndex(column);
    double sum = 0.0;
    
    if (columnIndex >= 0)
    {
        for (const auto& row : rows)
        {
            if (columnIndex < static_cast<int>(row.size()) && IsNumeric(row[columnIndex]))
            {
                sum += StringToDouble(row[columnIndex]);
            }
        }
    }
    
    return sum;
}

double CSVData::GetColumnAverage(const std::string& column)
{
    int columnIndex = GetColumnIndex(column);
    double sum = 0.0;
    int count = 0;
    
    if (columnIndex >= 0)
    {
        for (const auto& row : rows)
        {
            if (columnIndex < static_cast<int>(row.size()) && IsNumeric(row[columnIndex]))
            {
                sum += StringToDouble(row[columnIndex]);
                count++;
            }
        }
    }
    
    return count > 0 ? sum / count : 0.0;
}

std::string CSVData::GetColumnMin(const std::string& column)
{
    int columnIndex = GetColumnIndex(column);
    std::string minValue;
    
    if (columnIndex >= 0 && !rows.empty())
    {
        minValue = rows[0][columnIndex];
        for (const auto& row : rows)
        {
            if (columnIndex < static_cast<int>(row.size()) && row[columnIndex] < minValue)
            {
                minValue = row[columnIndex];
            }
        }
    }
    
    return minValue;
}

std::string CSVData::GetColumnMax(const std::string& column)
{
    int columnIndex = GetColumnIndex(column);
    std::string maxValue;
    
    if (columnIndex >= 0 && !rows.empty())
    {
        maxValue = rows[0][columnIndex];
        for (const auto& row : rows)
        {
            if (columnIndex < static_cast<int>(row.size()) && row[columnIndex] > maxValue)
            {
                maxValue = row[columnIndex];
            }
        }
    }
    
    return maxValue;
}

std::vector<std::string> CSVData::ParseCSVLine(const std::string& line)
{
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string field;
    
    while (std::getline(ss, field, ','))
    {
        // 前後の空白を削除
        field.erase(0, field.find_first_not_of(" \t\r\n"));
        field.erase(field.find_last_not_of(" \t\r\n") + 1);
        result.push_back(field);
    }
    
    return result;
}

int CSVData::GetColumnIndex(const std::string& column) const
{
    for (size_t i = 0; i < headers.size(); ++i)
    {
        if (headers[i] == column)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool CSVData::IsNumeric(const std::string& value) const
{
    if (value.empty()) return false;
    
    try
    {
        std::stod(value);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

double CSVData::StringToDouble(const std::string& value) const
{
    try
    {
        return std::stod(value);
    }
    catch (const std::exception&)
    {
        return 0.0;
    }
}
