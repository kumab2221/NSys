// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "imgui.h"
#include <d3d12.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}



// �`��֐��̃G�N�X�|�[�g
extern "C" __declspec(dllexport) void DrawImGui(ImGuiContext* shared_context, ID3D12Device* device, ID3D12CommandQueue* command_queue)
{
    // ImGui�̃R���e�L�X�g���Z�b�g
    ImGui::SetCurrentContext(shared_context);

    // DLL���ŕ`�悷��E�B�W�F�b�g
    static bool show_window = true;
    static float slider_value = 0.0f;

    ImGui::Begin("DLL Window");
    ImGui::Text("Hello from DLL!");
    ImGui::SliderFloat("Slider", &slider_value, 0.0f, 1.0f);
    ImGui::Checkbox("Show Window", &show_window);
    ImGui::End();
}

