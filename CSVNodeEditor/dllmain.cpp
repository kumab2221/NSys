#include <Windows.h>
#include "imgui.h"
#include "CSVNodeEditor.h"

// �O���[�o���ϐ�
static CSVNodeEditor* g_csvNodeEditor = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // �v���O�C��������
        g_csvNodeEditor = new CSVNodeEditor();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // �v���O�C���I������
        if (g_csvNodeEditor)
        {
            delete g_csvNodeEditor;
            g_csvNodeEditor = nullptr;
        }
        break;
    }
    return TRUE;
}

// �v���O�C���̃��C���`��֐����G�N�X�|�[�g
extern "C" __declspec(dllexport) void run(ImGuiContext* shared_context, const void* inputs, void* outputs)
{
    // ImGui�̃R���e�L�X�g���Z�b�g
    ImGui::SetCurrentContext(shared_context);

    // CSVNodeEditor������������Ă��邩�`�F�b�N
    if (g_csvNodeEditor)
    {
        // �v���O�C���̃��C���`�揈��
        g_csvNodeEditor->Render();
    }
}
