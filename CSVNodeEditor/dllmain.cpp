#include <Windows.h>
#include "imgui.h"
#include "CSVNodeEditor.h"

// グローバル変数
static CSVNodeEditor* g_csvNodeEditor = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // プラグイン初期化
        g_csvNodeEditor = new CSVNodeEditor();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // プラグイン終了処理
        if (g_csvNodeEditor)
        {
            delete g_csvNodeEditor;
            g_csvNodeEditor = nullptr;
        }
        break;
    }
    return TRUE;
}

// プラグインのメイン描画関数をエクスポート
extern "C" __declspec(dllexport) void run(ImGuiContext* shared_context, const void* inputs, void* outputs)
{
    // ImGuiのコンテキストをセット
    ImGui::SetCurrentContext(shared_context);

    // CSVNodeEditorが初期化されているかチェック
    if (g_csvNodeEditor)
    {
        // プラグインのメイン描画処理
        g_csvNodeEditor->Render();
    }
}
