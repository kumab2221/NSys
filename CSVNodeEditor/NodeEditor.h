#pragma once


#include "imgui.h"
#include "imnodes.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

// ノードの基本クラス
class BaseNode
{
public:
    BaseNode(int id, const std::string& name);
    virtual ~BaseNode() = default;

    virtual void Render() = 0;
    virtual void Process() = 0;
    virtual void SaveState() = 0;
    virtual void LoadState() = 0;

    int GetID() const { return nodeId; }
    const std::string& GetName() const { return nodeName; }

protected:
    int nodeId;
    std::string nodeName;
    ImVec2 position;
    bool isSelected;
};

// ノードエディタクラス
class NodeEditor
{
public:
    NodeEditor();
    ~NodeEditor();

    void Render();
    void AddNode(std::unique_ptr<BaseNode> node);
    void RemoveNode(int nodeId);
    void Clear();

private:
    std::vector<std::unique_ptr<BaseNode>> nodes;
    std::unordered_map<int, BaseNode*> nodeMap;
    int nextNodeId;

    void RenderNodes();
    void HandleNodeCreation();
    void HandleNodeDeletion();
};
