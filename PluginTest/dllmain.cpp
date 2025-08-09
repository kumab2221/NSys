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



// 描画関数のエクスポート
extern "C" __declspec(dllexport) void run(ImGuiContext* shared_context, const void*inputs, void *outputs)
{
    // ImGuiのコンテキストをセット
    ImGui::SetCurrentContext(shared_context);

    // DLL内で描画するウィジェット
    static bool show_window = true;
    static float slider_value = 0.0f;

    ImGui::Begin("DLL Window");
    ImGui::Text("Hello from DLL!");
    ImGui::SliderFloat("Slider", &slider_value, 0.0f, 1.0f);
    ImGui::Checkbox("Show Window", &show_window);
    ImGui::End();
}

