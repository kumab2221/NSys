#pragma once

#include "imgui.h"
#include "imnodes.h"
#include "implot.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

// �v���O�C���̃��C���N���X
class CSVNodeEditor
{
public:
    CSVNodeEditor();
    ~CSVNodeEditor();

    // ���C���`��֐�
    void Render();

private:
    // �^�u�Ǘ�
    struct TabData
    {
        std::string name;
        bool isOpen;
        std::unique_ptr<class NodeEditor> nodeEditor;
        std::unique_ptr<class CSVData> csvData;
    };

    std::vector<TabData> tabs;
    int currentTab;

    // UI���
    bool showNodePalette;
    bool showProperties;
    bool showDataPreview;
    bool showLog;

    // �t�@�C������
    void OpenCSVFile();
    void SaveCSVFile();
    void NewTab();
    void CloseTab(int index);

    // �m�[�h�p���b�g
    void RenderNodePalette();
    void RenderProperties();
    void RenderDataPreview();
    void RenderLog();

    // ���j���[�o�[
    void RenderMenuBar();
};
