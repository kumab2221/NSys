#pragma once

#include "imgui.h"
#include "imnodes.h"
#include "implot.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

// プラグインのメインクラス
class CSVNodeEditor
{
public:
    CSVNodeEditor();
    ~CSVNodeEditor();

    // メイン描画関数
    void Render();

private:
    // タブ管理
    struct TabData
    {
        std::string name;
        bool isOpen;
        std::unique_ptr<class NodeEditor> nodeEditor;
        std::unique_ptr<class CSVData> csvData;
    };

    std::vector<TabData> tabs;
    int currentTab;

    // UI状態
    bool showNodePalette;
    bool showProperties;
    bool showDataPreview;
    bool showLog;

    // ファイル操作
    void OpenCSVFile();
    void SaveCSVFile();
    void NewTab();
    void CloseTab(int index);

    // ノードパレット
    void RenderNodePalette();
    void RenderProperties();
    void RenderDataPreview();
    void RenderLog();

    // メニューバー
    void RenderMenuBar();
};
