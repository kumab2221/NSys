#include "NodeEditor.h"
#include "imgui.h"
#include "imnodes.h"
#include <algorithm>

BaseNode::BaseNode(int id, const std::string& name)
    : nodeId(id)
    , nodeName(name)
    , position(0, 0)
    , isSelected(false)
{
}

NodeEditor::NodeEditor()
    : nextNodeId(1)
{
    // ImNodes�̏�����
    ImNodes::CreateContext();
}

NodeEditor::~NodeEditor()
{
    // ImNodes�̏I������
    ImNodes::DestroyContext();
}

void NodeEditor::Render()
{
    // ImNodes�̊J�n
    ImNodes::BeginNodeEditor();

    // �m�[�h��`��
    RenderNodes();

    // ImNodes�̏I��
    ImNodes::EndNodeEditor();

    // �m�[�h�̍쐬�E�폜����
    HandleNodeCreation();
    HandleNodeDeletion();
}

void NodeEditor::AddNode(std::unique_ptr<BaseNode> node)
{
    if (node)
    {
        nodeMap[node->GetID()] = node.get();
        nodes.push_back(std::move(node));
    }
}

void NodeEditor::RemoveNode(int nodeId)
{
    auto it = std::find_if(nodes.begin(), nodes.end(),
        [nodeId](const std::unique_ptr<BaseNode>& node) {
            return node->GetID() == nodeId;
        });

    if (it != nodes.end())
    {
        nodeMap.erase(nodeId);
        nodes.erase(it);
    }
}

void NodeEditor::Clear()
{
    nodes.clear();
    nodeMap.clear();
}

void NodeEditor::RenderNodes()
{
    for (const auto& node : nodes)
    {
        // �m�[�h�̊J�n
        ImNodes::BeginNode(node->GetID());

        // �m�[�h�̃^�C�g��
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(node->GetName().c_str());
        ImNodes::EndNodeTitleBar();

        // �m�[�h�̓��e��`��
        node->Render();

        // �m�[�h�̏I��
        ImNodes::EndNode();
    }
}

void NodeEditor::HandleNodeCreation()
{
    // �E�N���b�N���j���[�Ńm�[�h���쐬
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup("NodeCreationMenu");
    }

    if (ImGui::BeginPopup("NodeCreationMenu"))
    {
        if (ImGui::MenuItem("CSV�ǂݍ���"))
        {
            // CSV�ǂݍ��݃m�[�h���쐬
            // ���ۂ̎����ł́ANodeTypes.h�Œ�`���ꂽ�N���X���g�p
        }
        if (ImGui::MenuItem("�t�B���^�["))
        {
            // �t�B���^�[�m�[�h���쐬
        }
        if (ImGui::MenuItem("�\�[�g"))
        {
            // �\�[�g�m�[�h���쐬
        }
        if (ImGui::MenuItem("�W�v"))
        {
            // �W�v�m�[�h���쐬
        }
        if (ImGui::MenuItem("����"))
        {
            // �����m�[�h���쐬
        }
        if (ImGui::MenuItem("CSV�o��"))
        {
            // �o�̓m�[�h���쐬
        }
        ImGui::EndPopup();
    }
}

void NodeEditor::HandleNodeDeletion()
{
    // �I�����ꂽ�m�[�h���폜�iAPI�ύX�Ή��j
    for (const auto& node : nodes) {
        int nodeId = node->GetID();
        if (ImNodes::IsNodeSelected(nodeId)) {
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                RemoveNode(nodeId);
                break; // 1�����폜
            }
        }
    }
}
