#include "DebugUI.h"
#include "Application.h"

std::vector<std::function<void(void)>> DebugUI::m_debugfunction;

void DebugUI::Init(ID3D11Device* device, ID3D11DeviceContext* context) 
{

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls


    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(Application::GetWindow());
    ImGui_ImplDX11_Init(device, context);
}

void DebugUI::DisposeUI() {
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

// デバッグ表示関数の登録
void DebugUI::RedistDebugFunction(std::function<void(void)> f) {
    m_debugfunction.push_back(std::move(f));
}

void DebugUI::Render() {
    // ImGuiの新しいフレームを開始
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ウィンドウとデバッグ情報の描画
    ImGui::Begin("Rendering Information");
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    // ImGuiの表示サイズをウィンドウサイズに設定
    io.DisplaySize = ImVec2(static_cast<float>(Application::GetWidth()), static_cast<float>(Application::GetHeight()));

    unsigned int drawcall = Application::GetDrawNum();
    unsigned int polygonnum = Application::GetPolygonNum();

    ImGui::Text("Draw Calls: %d", drawcall);
    ImGui::Text("Rendered Triangles: %d", polygonnum);


    // デバッグ関数の実行
    for (auto& f : m_debugfunction)
    {
        f();
    }


    ImGui::End();

    // フレームのレンダリングを完了
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
