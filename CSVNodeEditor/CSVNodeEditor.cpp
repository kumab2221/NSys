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
    // �����^�u���쐬
    NewTab();
}

CSVNodeEditor::~CSVNodeEditor()
{
}

void CSVNodeEditor::Render()
{
    // ���j���[�o�[��`��
    RenderMenuBar();

    // �^�u�o�[��`��
    if (ImGui::BeginTabBar("CSVNodeEditorTabs"))
    {
        for (int i = 0; i < tabs.size(); ++i)
        {
            if (ImGui::BeginTabItem(tabs[i].name.c_str(), &tabs[i].isOpen))
            {
                currentTab = i;
                
                // ���C���G�f�B�^�G���A
                ImGui::BeginChild("NodeEditorArea", ImVec2(0, 0), true);
                
                // �m�[�h�G�f�B�^��`��
                if (tabs[i].nodeEditor)
                {
                    tabs[i].nodeEditor->Render();
                }
                
                ImGui::EndChild();
                
                ImGui::EndTabItem();
            }
            
            // �^�u������ꂽ�ꍇ�̏���
            if (!tabs[i].isOpen)
            {
                CloseTab(i);
                break;
            }
        }
        
        // �V�����^�u��ǉ�����{�^��
        if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
        {
            NewTab();
        }
        
        ImGui::EndTabBar();
    }

    // �T�C�h�p�l����`��
    if (showNodePalette)
    {
        ImGui::Begin("�m�[�h�p���b�g", &showNodePalette);
        RenderNodePalette();
        ImGui::End();
    }

    if (showProperties)
    {
        ImGui::Begin("�v���p�e�B", &showProperties);
        RenderProperties();
        ImGui::End();
    }

    if (showDataPreview)
    {
        ImGui::Begin("�f�[�^�v���r���[", &showDataPreview);
        RenderDataPreview();
        ImGui::End();
    }

    if (showLog)
    {
        ImGui::Begin("���O", &showLog);
        RenderLog();
        ImGui::End();
    }
}

void CSVNodeEditor::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("�t�@�C��"))
        {
            if (ImGui::MenuItem("CSV���J��", "Ctrl+O"))
            {
                OpenCSVFile();
            }
            if (ImGui::MenuItem("CSV��ۑ�", "Ctrl+S"))
            {
                SaveCSVFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("�V�K�^�u", "Ctrl+T"))
            {
                NewTab();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("�\��"))
        {
            ImGui::MenuItem("�m�[�h�p���b�g", nullptr, &showNodePalette);
            ImGui::MenuItem("�v���p�e�B", nullptr, &showProperties);
            ImGui::MenuItem("�f�[�^�v���r���[", nullptr, &showDataPreview);
            ImGui::MenuItem("���O", nullptr, &showLog);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("���s"))
        {
            if (ImGui::MenuItem("���s", "F5"))
            {
                // ���݂̃^�u�̃m�[�h�G�f�B�^�����s
                if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].nodeEditor)
                {
                    // ���s�����������Ɏ���
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void CSVNodeEditor::RenderNodePalette()
{
    ImGui::Text("���p�\�ȃm�[�h:");
    ImGui::Separator();

    if (ImGui::TreeNode("�f�[�^����"))
    {
        if (ImGui::Button("CSV�ǂݍ���"))
        {
            // ���݂̃^�u��CSV�ǂݍ��݃m�[�h��ǉ�
            if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].nodeEditor)
            {
                // �m�[�h�ǉ������������Ɏ���
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("�f�[�^����"))
    {
        if (ImGui::Button("�t�B���^�["))
        {
            // �t�B���^�[�m�[�h��ǉ�
        }
        if (ImGui::Button("�\�[�g"))
        {
            // �\�[�g�m�[�h��ǉ�
        }
        if (ImGui::Button("�W�v"))
        {
            // �W�v�m�[�h��ǉ�
        }
        if (ImGui::Button("����"))
        {
            // �����m�[�h��ǉ�
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("�f�[�^�o��"))
    {
        if (ImGui::Button("CSV�o��"))
        {
            // �o�̓m�[�h��ǉ�
        }
        ImGui::TreePop();
    }
}

void CSVNodeEditor::RenderProperties()
{
    if (currentTab >= 0 && currentTab < tabs.size())
    {
        ImGui::Text("�^�u: %s", tabs[currentTab].name.c_str());
        ImGui::Separator();
        
        // �I�����ꂽ�m�[�h�̃v���p�e�B��\��
        ImGui::Text("�I�����ꂽ�m�[�h�̃v���p�e�B");
        // �����Ƀv���p�e�B�ҏWUI������
    }
}

void CSVNodeEditor::RenderDataPreview()
{
    if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].csvData)
    {
        const auto& data = tabs[currentTab].csvData;
        ImGui::Text("�f�[�^�v���r���[ (%zu �s, %zu ��)", 
                   data->GetRowCount(), data->GetColumnCount());
        
        ImGui::Separator();
        
        // �w�b�_�[��\��
        const auto& headers = data->GetHeaders();
        if (!headers.empty())
        {
            ImGui::Text("�w�b�_�[:");
            for (size_t i = 0; i < headers.size(); ++i)
            {
                ImGui::SameLine();
                ImGui::Text("%s", headers[i].c_str());
                if (i < headers.size() - 1) ImGui::SameLine();
            }
        }
        
        // �ŏ��̐��s��\��
        const auto& rows = data->GetRows();
        if (!rows.empty())
        {
            ImGui::Text("�f�[�^ (�ŏ���10�s):");
            for (size_t i = 0; i < std::min(rows.size(), size_t(10)); ++i)
            {
                ImGui::Text("�s %zu:", i + 1);
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
        ImGui::Text("�f�[�^���ǂݍ��܂�Ă��܂���");
    }
}

void CSVNodeEditor::RenderLog()
{
    ImGui::Text("���O:");
    ImGui::Separator();
    
    // ���O���b�Z�[�W��\��
    ImGui::Text("CSVNodeEditor�v���O�C��������ɓǂݍ��܂�܂���");
    ImGui::Text("���݂̃^�u: %d", currentTab);
    
    if (currentTab >= 0 && currentTab < tabs.size())
    {
        ImGui::Text("�^�u��: %s", tabs[currentTab].name.c_str());
    }
}

void CSVNodeEditor::OpenCSVFile()
{
    // �t�@�C���I���_�C�A���O������
    // ���݂͊ȈՎ���
    if (currentTab >= 0 && currentTab < tabs.size())
    {
        // CSV�t�@�C����ǂݍ��ޏ����������Ɏ���
        ImGui::OpenPopup("�t�@�C���I��");
    }
}

void CSVNodeEditor::SaveCSVFile()
{
    // �t�@�C���ۑ��_�C�A���O������
    if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].csvData)
    {
        // CSV�t�@�C����ۑ����鏈���������Ɏ���
    }
}

void CSVNodeEditor::NewTab()
{
    TabData newTab;
    newTab.name = "�^�u " + std::to_string(tabs.size() + 1);
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
