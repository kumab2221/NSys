#pragma once

#include <vector>
#include <string>
#include <memory>
#include <fstream>

// CSV�f�[�^���i�[����N���X
class CSVData
{
public:
    CSVData();
    ~CSVData();

    // �t�@�C������
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename);

    // �f�[�^�A�N�Z�X
    const std::vector<std::string>& GetHeaders() const { return headers; }
    const std::vector<std::vector<std::string>>& GetRows() const { return rows; }
    
    // �f�[�^����
    void AddRow(const std::vector<std::string>& row);
    void RemoveRow(size_t index);
    void Clear();

    // ���v���
    size_t GetRowCount() const { return rows.size(); }
    size_t GetColumnCount() const { return headers.size(); }

    // �f�[�^�t�B���^�����O
    std::vector<std::vector<std::string>> FilterRows(const std::string& column, const std::string& value);
    
    // �f�[�^�\�[�g
    void SortByColumn(const std::string& column, bool ascending = true);

    // �f�[�^�W�v
    double GetColumnSum(const std::string& column);
    double GetColumnAverage(const std::string& column);
    std::string GetColumnMin(const std::string& column);
    std::string GetColumnMax(const std::string& column);

private:
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;

    // �w���p�[�֐�
    std::vector<std::string> ParseCSVLine(const std::string& line);
    int GetColumnIndex(const std::string& column) const;
    bool IsNumeric(const std::string& value) const;
    double StringToDouble(const std::string& value) const;
};
