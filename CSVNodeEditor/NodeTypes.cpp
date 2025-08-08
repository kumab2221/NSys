#include "NodeTypes.h"
#include "imgui.h"
#include "imnodes.h"
#include <imgui.h>

// CSV�ǂݍ��݃m�[�h
CSVLoadNode::CSVLoadNode(int id)
    : BaseNode(id, "CSV�ǂݍ���")
    , fileLoaded(false)
{
    outputData = std::make_shared<CSVData>();
}

void CSVLoadNode::Render()
{
    // ���̓s���i�Ȃ��j
    
    // �o�̓s��
    ImNodes::BeginOutputAttribute(nodeId * 100 + 1);
    ImGui::Text("�o��");
    ImNodes::EndOutputAttribute();
    
    // �t�@�C���p�X����
    ImGui::Text("�t�@�C���p�X:");
    static char filePathBuffer[256] = "";
    if (ImGui::InputText("##FilePath", filePathBuffer, sizeof(filePathBuffer)))
    {
        filePath = filePathBuffer;
    }
    
    // �t�@�C���ǂݍ��݃{�^��
    if (ImGui::Button("�t�@�C����ǂݍ���"))
    {
        if (outputData->LoadFromFile(filePath))
        {
            fileLoaded = true;
        }
    }
    
    // ��ԕ\��
    if (fileLoaded)
    {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "? �ǂݍ��݊���");
        ImGui::Text("�s��: %zu", outputData->GetRowCount());
        ImGui::Text("��: %zu", outputData->GetColumnCount());
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "? ���ǂݍ���");
    }
}

void CSVLoadNode::Process()
{
    // �t�@�C�����ǂݍ��܂�Ă��Ȃ��ꍇ�͓ǂݍ��݂����s
    if (!fileLoaded && !filePath.empty())
    {
        fileLoaded = outputData->LoadFromFile(filePath);
    }
}

void CSVLoadNode::SaveState()
{
    // ��ԕۑ������������Ɏ���
}

void CSVLoadNode::LoadState()
{
    // ��ԓǂݍ��ݏ����������Ɏ���
}

// �t�B���^�[�m�[�h
FilterNode::FilterNode(int id)
    : BaseNode(id, "�t�B���^�[")
    , filterOperator("==")
{
    inputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void FilterNode::Render()
{
    // ���̓s��
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("����");
    ImNodes::EndInputAttribute();
    
    // �o�̓s��
    ImNodes::BeginOutputAttribute(nodeId * 100 + 2);
    ImGui::Text("�o��");
    ImNodes::EndOutputAttribute();
    
    // �t�B���^�[�ݒ�
    ImGui::Text("�t�B���^�[�ݒ�:");
    
    // ��I��
    static char columnBuffer[128] = "";
    if (ImGui::InputText("��", columnBuffer, sizeof(columnBuffer)))
    {
        filterColumn = columnBuffer;
    }
    
    // ���Z�q�I��
    const char* operators[] = { "==", "!=", ">", "<", ">=", "<=", "contains" };
    if (ImGui::BeginCombo("���Z�q", filterOperator.c_str()))
    {
        for (const char* op : operators)
        {
            if (ImGui::Selectable(op, filterOperator == op))
            {
                filterOperator = op;
            }
        }
        ImGui::EndCombo();
    }
    
    // �l����
    static char valueBuffer[128] = "";
    if (ImGui::InputText("�l", valueBuffer, sizeof(valueBuffer)))
    {
        filterValue = valueBuffer;
    }
    
    // �t�B���^�[���s�{�^��
    if (ImGui::Button("�t�B���^�[���s"))
    {
        Process();
    }
    
    // ���ʕ\��
    if (!outputData->GetRows().empty())
    {
        ImGui::Text("�t�B���^�[����: %zu �s", outputData->GetRowCount());
    }
}

void FilterNode::Process()
{
    if (inputData && !filterColumn.empty() && !filterValue.empty())
    {
        // �t�B���^�[����������
        auto filteredRows = inputData->FilterRows(filterColumn, filterValue);
        
        // ���ʂ��o�̓f�[�^�ɐݒ�
        outputData->Clear();
        outputData->AddRow(inputData->GetHeaders()); // �w�b�_�[��ǉ�
        for (const auto& row : filteredRows)
        {
            outputData->AddRow(row);
        }
    }
}

void FilterNode::SaveState()
{
    // ��ԕۑ������������Ɏ���
}

void FilterNode::LoadState()
{
    // ��ԓǂݍ��ݏ����������Ɏ���
}

// �\�[�g�m�[�h
SortNode::SortNode(int id)
    : BaseNode(id, "�\�[�g")
    , ascending(true)
{
    inputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void SortNode::Render()
{
    // ���̓s��
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("����");
    ImNodes::EndInputAttribute();
    
    // �o�̓s��
    ImNodes::BeginOutputAttribute(nodeId * 100 + 2);
    ImGui::Text("�o��");
    ImNodes::EndOutputAttribute();
    
    // �\�[�g�ݒ�
    ImGui::Text("�\�[�g�ݒ�:");
    
    // ��I��
    static char columnBuffer[128] = "";
    if (ImGui::InputText("��", columnBuffer, sizeof(columnBuffer)))
    {
        sortColumn = columnBuffer;
    }
    
    // ����/�~���I��
    ImGui::Checkbox("����", &ascending);
    
    // �\�[�g���s�{�^��
    if (ImGui::Button("�\�[�g���s"))
    {
        Process();
    }
}

void SortNode::Process()
{
    if (inputData && !sortColumn.empty())
    {
        // �\�[�g����������
        *outputData = *inputData; // �f�[�^���R�s�[
        outputData->SortByColumn(sortColumn, ascending);
    }
}

void SortNode::SaveState()
{
    // ��ԕۑ������������Ɏ���
}

void SortNode::LoadState()
{
    // ��ԓǂݍ��ݏ����������Ɏ���
}

// �W�v�m�[�h
AggregateNode::AggregateNode(int id)
    : BaseNode(id, "�W�v")
    , aggregateFunction("sum")
{
    inputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void AggregateNode::Render()
{
    // ���̓s��
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("����");
    ImNodes::EndInputAttribute();
    
    // �o�̓s��
    ImNodes::BeginOutputAttribute(nodeId * 100 + 2);
    ImGui::Text("�o��");
    ImNodes::EndOutputAttribute();
    
    // �W�v�ݒ�
    ImGui::Text("�W�v�ݒ�:");
    
    // �O���[�v����
    static char groupColumnBuffer[128] = "";
    if (ImGui::InputText("�O���[�v����", groupColumnBuffer, sizeof(groupColumnBuffer)))
    {
        groupColumn = groupColumnBuffer;
    }
    
    // �W�v��
    static char aggregateColumnBuffer[128] = "";
    if (ImGui::InputText("�W�v��", aggregateColumnBuffer, sizeof(aggregateColumnBuffer)))
    {
        aggregateColumn = aggregateColumnBuffer;
    }
    
    // �W�v�֐��I��
    const char* functions[] = { "sum", "average", "count", "min", "max" };
    if (ImGui::BeginCombo("�W�v�֐�", aggregateFunction.c_str()))
    {
        for (const char* func : functions)
        {
            if (ImGui::Selectable(func, aggregateFunction == func))
            {
                aggregateFunction = func;
            }
        }
        ImGui::EndCombo();
    }
    
    // �W�v���s�{�^��
    if (ImGui::Button("�W�v���s"))
    {
        Process();
    }
}

void AggregateNode::Process()
{
    if (inputData && !groupColumn.empty() && !aggregateColumn.empty())
    {
        // �W�v����������
        // �ȈՎ����̂��߁A��{�I�ȏW�v�̂�
        if (aggregateFunction == "sum")
        {
            double sum = inputData->GetColumnSum(aggregateColumn);
            // ���ʂ��o�̓f�[�^�ɐݒ�
        }
        else if (aggregateFunction == "average")
        {
            double avg = inputData->GetColumnAverage(aggregateColumn);
            // ���ʂ��o�̓f�[�^�ɐݒ�
        }
    }
}

void AggregateNode::SaveState()
{
    // ��ԕۑ������������Ɏ���
}

void AggregateNode::LoadState()
{
    // ��ԓǂݍ��ݏ����������Ɏ���
}

// �����m�[�h
JoinNode::JoinNode(int id)
    : BaseNode(id, "����")
    , joinType("inner")
{
    leftInputData = std::make_shared<CSVData>();
    rightInputData = std::make_shared<CSVData>();
    outputData = std::make_shared<CSVData>();
}

void JoinNode::Render()
{
    // �����̓s��
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("������");
    ImNodes::EndInputAttribute();
    
    // �E���̓s��
    ImNodes::BeginInputAttribute(nodeId * 100 + 2);
    ImGui::Text("�E����");
    ImNodes::EndInputAttribute();
    
    // �o�̓s��
    ImNodes::BeginOutputAttribute(nodeId * 100 + 3);
    ImGui::Text("�o��");
    ImNodes::EndOutputAttribute();
    
    // �����ݒ�
    ImGui::Text("�����ݒ�:");
    
    // ��������
    static char leftColumnBuffer[128] = "";
    if (ImGui::InputText("��������", leftColumnBuffer, sizeof(leftColumnBuffer)))
    {
        leftJoinColumn = leftColumnBuffer;
    }
    
    // �E������
    static char rightColumnBuffer[128] = "";
    if (ImGui::InputText("�E������", rightColumnBuffer, sizeof(rightColumnBuffer)))
    {
        rightJoinColumn = rightColumnBuffer;
    }
    
    // �����^�C�v�I��
    const char* joinTypes[] = { "inner", "left", "right", "outer" };
    if (ImGui::BeginCombo("�����^�C�v", joinType.c_str()))
    {
        for (const char* type : joinTypes)
        {
            if (ImGui::Selectable(type, joinType == type))
            {
                joinType = type;
            }
        }
        ImGui::EndCombo();
    }
    
    // �������s�{�^��
    if (ImGui::Button("�������s"))
    {
        Process();
    }
}

void JoinNode::Process()
{
    if (leftInputData && rightInputData && !leftJoinColumn.empty() && !rightJoinColumn.empty())
    {
        // ��������������
        // �ȈՎ����̂��߁A��{�I�Ȍ����̂�
    }
}

void JoinNode::SaveState()
{
    // ��ԕۑ������������Ɏ���
}

void JoinNode::LoadState()
{
    // ��ԓǂݍ��ݏ����������Ɏ���
}

// �o�̓m�[�h
OutputNode::OutputNode(int id)
    : BaseNode(id, "CSV�o��")
{
    inputData = std::make_shared<CSVData>();
}

void OutputNode::Render()
{
    // ���̓s��
    ImNodes::BeginInputAttribute(nodeId * 100 + 1);
    ImGui::Text("����");
    ImNodes::EndInputAttribute();
    
    // �o�͐ݒ�
    ImGui::Text("�o�͐ݒ�:");
    
    // �o�̓p�X
    static char outputPathBuffer[256] = "";
    if (ImGui::InputText("�o�̓p�X", outputPathBuffer, sizeof(outputPathBuffer)))
    {
        outputPath = outputPathBuffer;
    }
    
    // �ۑ��{�^��
    if (ImGui::Button("CSV�ۑ�"))
    {
        Process();
    }
    
    // ��ԕ\��
    if (!inputData->GetRows().empty())
    {
        ImGui::Text("�ۑ��Ώ�: %zu �s", inputData->GetRowCount());
    }
}

void OutputNode::Process()
{
    if (inputData && !outputPath.empty())
    {
        inputData->SaveToFile(outputPath);
    }
}

void OutputNode::SaveState()
{
    // ��ԕۑ������������Ɏ���
}

void OutputNode::LoadState()
{
    // ��ԓǂݍ��ݏ����������Ɏ���
}
