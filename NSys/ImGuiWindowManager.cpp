#include "ImGuiWindowManager.h"

void ShowHelloWorldWindow(bool& show_demo_window, bool& show_another_window, ImVec4& clear_color)
{
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void ShowMainMenuBar()
{
    // メニューバーの作成
    if (ImGui::BeginMainMenuBar())
    {
        // ファイルメニュー
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Open処理 */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Save処理 */ }
            if (ImGui::MenuItem("Exit", "Ctrl+Q")) { /* Exit処理 */ }
            ImGui::EndMenu();
        }

        // 編集メニュー
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Undo処理 */ }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) { /* Redo処理 (無効) */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) { /* Cut処理 */ }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) { /* Copy処理 */ }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) { /* Paste処理 */ }
            ImGui::EndMenu();
        }

        // ヘルプメニュー
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About")) { /* About処理 */ }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
