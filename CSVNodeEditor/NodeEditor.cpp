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
    // ImNodesの初期化
    ImNodes::CreateContext();
}

NodeEditor::~NodeEditor()
{
    // ImNodesの終了処理
    ImNodes::DestroyContext();
}

void NodeEditor::Render()
{
    // ImNodesの開始
    ImNodes::BeginNodeEditor();

    // ノードを描画
    RenderNodes();

    // ImNodesの終了
    ImNodes::EndNodeEditor();

    // ノードの作成・削除処理
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
        // ノードの開始
        ImNodes::BeginNode(node->GetID());

        // ノードのタイトル
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(node->GetName().c_str());
        ImNodes::EndNodeTitleBar();

        // ノードの内容を描画
        node->Render();

        // ノードの終了
        ImNodes::EndNode();
    }
}

void NodeEditor::HandleNodeCreation()
{
    // 右クリックメニューでノードを作成
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup("NodeCreationMenu");
    }

    if (ImGui::BeginPopup("NodeCreationMenu"))
    {
        if (ImGui::MenuItem("CSV読み込み"))
        {
            // CSV読み込みノードを作成
            // 実際の実装では、NodeTypes.hで定義されたクラスを使用
        }
        if (ImGui::MenuItem("フィルター"))
        {
            // フィルターノードを作成
        }
        if (ImGui::MenuItem("ソート"))
        {
            // ソートノードを作成
        }
        if (ImGui::MenuItem("集計"))
        {
            // 集計ノードを作成
        }
        if (ImGui::MenuItem("結合"))
        {
            // 結合ノードを作成
        }
        if (ImGui::MenuItem("CSV出力"))
        {
            // 出力ノードを作成
        }
        ImGui::EndPopup();
    }
}

void NodeEditor::HandleNodeDeletion()
{
    // 選択されたノードを削除（API変更対応）
    for (const auto& node : nodes) {
        int nodeId = node->GetID();
        if (ImNodes::IsNodeSelected(nodeId)) {
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                RemoveNode(nodeId);
                break; // 1つだけ削除
            }
        }
    }
}
