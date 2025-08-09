#include "CSVNodeEditor.h"
#include "NodeEditor.h"
#include "CSVData.h"
#include "NodeTypes.h"
#include <imgui.h>
#include <imnodes.h>
#include <implot.h>
#include <filesystem>

CSVNodeEditor::CSVNodeEditor()
    : currentTab(0)
    , showNodePalette(true)
    , showProperties(true)
    , showDataPreview(true)
    , showLog(true)
{
    // 初期タブを作成
    NewTab();
}

CSVNodeEditor::~CSVNodeEditor()
{
}

void CSVNodeEditor::Render()
{
    // メニューバーを描画
    RenderMenuBar();

    // タブバーを描画
    if (ImGui::BeginTabBar("CSVNodeEditorTabs"))
    {
        for (int i = 0; i < tabs.size(); ++i)
        {
            if (ImGui::BeginTabItem(tabs[i].name.c_str(), &tabs[i].isOpen))
            {
                currentTab = i;
                
                // メインエディタエリア
                ImGui::BeginChild("NodeEditorArea", ImVec2(0, 0), true);
                
                // ノードエディタを描画
                if (tabs[i].nodeEditor)
                {
                    tabs[i].nodeEditor->Render();
                }
                
                ImGui::EndChild();
                
                ImGui::EndTabItem();
            }
            
            // タブが閉じられた場合の処理
            if (!tabs[i].isOpen)
            {
                CloseTab(i);
                break;
            }
        }
        
        // 新しいタブを追加するボタン
        if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
        {
            NewTab();
        }
        
        ImGui::EndTabBar();
    }

    // サイドパネルを描画
    if (showNodePalette)
    {
        ImGui::Begin("ノードパレット", &showNodePalette);
        RenderNodePalette();
        ImGui::End();
    }

    if (showProperties)
    {
        ImGui::Begin("プロパティ", &showProperties);
        RenderProperties();
        ImGui::End();
    }

    if (showDataPreview)
    {
        ImGui::Begin("データプレビュー", &showDataPreview);
        RenderDataPreview();
        ImGui::End();
    }

    if (showLog)
    {
        ImGui::Begin("ログ", &showLog);
        RenderLog();
        ImGui::End();
    }
}

void CSVNodeEditor::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("ファイル"))
        {
            if (ImGui::MenuItem("CSVを開く", "Ctrl+O"))
            {
                OpenCSVFile();
            }
            if (ImGui::MenuItem("CSVを保存", "Ctrl+S"))
            {
                SaveCSVFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("新規タブ", "Ctrl+T"))
            {
                NewTab();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("表示"))
        {
            ImGui::MenuItem("ノードパレット", nullptr, &showNodePalette);
            ImGui::MenuItem("プロパティ", nullptr, &showProperties);
            ImGui::MenuItem("データプレビュー", nullptr, &showDataPreview);
            ImGui::MenuItem("ログ", nullptr, &showLog);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("実行"))
        {
            if (ImGui::MenuItem("実行", "F5"))
            {
                // 現在のタブのノードエディタを実行
                if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].nodeEditor)
                {
                    // 実行処理をここに実装
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void CSVNodeEditor::RenderNodePalette()
{
    ImGui::Text("利用可能なノード:");
    ImGui::Separator();

    if (ImGui::TreeNode("データ入力"))
    {
        if (ImGui::Button("CSV読み込み"))
        {
            // 現在のタブにCSV読み込みノードを追加
            if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].nodeEditor)
            {
                // ノード追加処理をここに実装
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("データ処理"))
    {
        if (ImGui::Button("フィルター"))
        {
            // フィルターノードを追加
        }
        if (ImGui::Button("ソート"))
        {
            // ソートノードを追加
        }
        if (ImGui::Button("集計"))
        {
            // 集計ノードを追加
        }
        if (ImGui::Button("結合"))
        {
            // 結合ノードを追加
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("データ出力"))
    {
        if (ImGui::Button("CSV出力"))
        {
            // 出力ノードを追加
        }
        ImGui::TreePop();
    }
}

void CSVNodeEditor::RenderProperties()
{
    if (currentTab >= 0 && currentTab < tabs.size())
    {
        ImGui::Text("タブ: %s", tabs[currentTab].name.c_str());
        ImGui::Separator();
        
        // 選択されたノードのプロパティを表示
        ImGui::Text("選択されたノードのプロパティ");
        // ここにプロパティ編集UIを実装
    }
}

void CSVNodeEditor::RenderDataPreview()
{
    if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].csvData)
    {
        const auto& data = tabs[currentTab].csvData;
        ImGui::Text("データプレビュー (%zu 行, %zu 列)", 
                   data->GetRowCount(), data->GetColumnCount());
        
        ImGui::Separator();
        
        // ヘッダーを表示
        const auto& headers = data->GetHeaders();
        if (!headers.empty())
        {
            ImGui::Text("ヘッダー:");
            for (size_t i = 0; i < headers.size(); ++i)
            {
                ImGui::SameLine();
                ImGui::Text("%s", headers[i].c_str());
                if (i < headers.size() - 1) ImGui::SameLine();
            }
        }
        
        // 最初の数行を表示
        const auto& rows = data->GetRows();
        if (!rows.empty())
        {
            ImGui::Text("データ (最初の10行):");
            for (size_t i = 0; i < std::min(rows.size(), size_t(10)); ++i)
            {
                ImGui::Text("行 %zu:", i + 1);
                for (size_t j = 0; j < rows[i].size(); ++j)
                {
                    ImGui::SameLine();
                    ImGui::Text("%s", rows[i][j].c_str());
                }
            }
        }
    }
    else
    {
        ImGui::Text("データが読み込まれていません");
    }
}

void CSVNodeEditor::RenderLog()
{
    ImGui::Text("ログ:");
    ImGui::Separator();
    
    // ログメッセージを表示
    ImGui::Text("CSVNodeEditorプラグインが正常に読み込まれました");
    ImGui::Text("現在のタブ: %d", currentTab);
    
    if (currentTab >= 0 && currentTab < tabs.size())
    {
        ImGui::Text("タブ名: %s", tabs[currentTab].name.c_str());
    }
}

void CSVNodeEditor::OpenCSVFile()
{
    // ファイル選択ダイアログを実装
    // 現在は簡易実装
    if (currentTab >= 0 && currentTab < tabs.size())
    {
        // CSVファイルを読み込む処理をここに実装
        ImGui::OpenPopup("ファイル選択");
    }
}

void CSVNodeEditor::SaveCSVFile()
{
    // ファイル保存ダイアログを実装
    if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].csvData)
    {
        // CSVファイルを保存する処理をここに実装
    }
}

void CSVNodeEditor::NewTab()
{
    TabData newTab;
    newTab.name = "タブ " + std::to_string(tabs.size() + 1);
    newTab.isOpen = true;
    newTab.nodeEditor = std::make_unique<NodeEditor>();
    newTab.csvData = std::make_unique<CSVData>();
    
    tabs.push_back(std::move(newTab));
    currentTab = tabs.size() - 1;
}

void CSVNodeEditor::CloseTab(int index)
{
    if (index >= 0 && index < tabs.size())
    {
        tabs.erase(tabs.begin() + index);
        if (currentTab >= tabs.size())
        {
            currentTab = tabs.size() - 1;
        }
    }
}
