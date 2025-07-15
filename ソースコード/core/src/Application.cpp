#include <chrono>
#include <thread>
#include "Application.h"
#include "fpscontrol.h"
#include "Renderer.h"
#include "SceneDemo.h"
#include "DirectInput.h"
#include "DebugUI.h"
#include "DLSSManager.h"

const auto ClassName = TEXT("ウィンドウクラス名"); // ウィンドウクラス名.
const auto WindowName = TEXT("RePixeler");         // ウィンドウ名.

// 静的変数
HINSTANCE  Application::m_hInst;    // インスタンスハンドル
HWND       Application::m_hWnd;     // ウィンドウハンドル
uint32_t   Application::m_Width;    // ウィンドウの横幅
uint32_t   Application::m_Height;   // ウィンドウの縦幅

unsigned int  Application::m_DrawNum;     // ドローコール回数
unsigned int  Application::m_PolygonNum;  // 描画されているポリゴン数


// ImGuiのWin32プロシージャハンドラ(マウス対応)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// コンストラクタ
Application::Application(uint32_t width, uint32_t height)
{ 
    m_Height = height;
    m_Width = width;

    timeBeginPeriod(1);
}

// デストラクタ
Application::~Application()
{ 
    timeEndPeriod(1);
}

// アプリケーションの実行
void Application::Run()
{
    if (InitApp())
    { 
        MainLoop(); 
    }

    TermApp();
}

// アプリケーションの初期化
bool Application::InitApp()
{
    // ウィンドウの初期化.
    if (!InitWnd())
    { 
        return false; 
    }

    // 正常終了.
    return true;
}

// アプリケーションの終了
void Application::TermApp()
{
    // ウィンドウの終了処理.
    TermWnd();
}

// ウィンドウの初期化
bool Application::InitWnd()
{
    // インスタンスハンドルを取得.
    auto hInst = GetModuleHandle(nullptr);
    if (hInst == nullptr)
    { 
        return false; 
    }

    // ウィンドウの設定.
    WNDCLASSEX wc = {};
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProc;
    wc.hIcon            = LoadIcon(hInst, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(hInst, IDC_ARROW);
    wc.hbrBackground    = GetSysColorBrush(COLOR_BACKGROUND);
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = ClassName;
    wc.hIconSm          = LoadIcon(hInst, IDI_APPLICATION);

    // ウィンドウの登録.
    if (!RegisterClassEx(&wc))
    { return false; }

    // インスタンスハンドル設定.
    m_hInst = hInst;

    // ウィンドウのサイズを設定.
    RECT rc = {};
    rc.right  = static_cast<LONG>(m_Width);
    rc.bottom = static_cast<LONG>(m_Height);

    // ウィンドウサイズを調整.
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // ウィンドウを生成.
    m_hWnd = CreateWindowEx(
        0,
        //        WS_EX_TOPMOST,
        ClassName,
        WindowName,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right  - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInst,
        nullptr);

    if (m_hWnd == nullptr)
    { return false; }
 
    // ウィンドウを表示.
    ShowWindow(m_hWnd, SW_SHOWNORMAL);

    // ウィンドウを更新.
    UpdateWindow(m_hWnd);

    // ウィンドウにフォーカスを設定.
    SetFocus(m_hWnd);

    // 正常終了.
    return true;
}

// ウィンドウの終了
void Application::TermWnd()
{
    // ウィンドウの登録を解除.
    if (m_hInst != nullptr)
    { UnregisterClass(ClassName, m_hInst); }

    m_hInst = nullptr;
    m_hWnd  = nullptr;
}

// メインループ
void Application::MainLoop()
{
    MSG msg = {};

    // FPS調整クラス
    FPS fpsrate(60);

    // シーン環境生成
    SceneDemo scene;

    // 描画初期化
    Renderer::Init();

    // デバッグUI
    DebugUI::Init(Renderer::GetDevice(), Renderer::GetDeviceContext());

    // 入力初期化
    DirectInput::GetInstance().Init(m_hInst, m_hWnd, m_Width, m_Height);

    // DLSS初期化
    if (Renderer::GetIsAbleNVIDIA()) {
         DLSSManager::GetInstance().InitializeNGX(L".");
    }

    // シーン生成
    scene.SceneInit();

    while(WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {

            uint64_t delta_time = 0;

            // デルタタイムを計算
            delta_time = fpsrate.CalcDelta();

            // 入力データ取得
            DirectInput::GetInstance().GetKeyBuffer();
            DirectInput::GetInstance().GetMouseState();

			// シーン更新    
            scene.SceneUpdate();

            // 描画前処理
            Renderer::Begin();

			// シーン描画
            scene.SceneDraw();

			// デバッグUI
            DebugUI::Render();

            // 描画後処理
            Renderer::End();

            // 規定時間までWAIT
            fpsrate.Wait();
        }
    }

    // ImGuiの破棄
    DebugUI::DisposeUI();

    // シーンの破棄
    scene.SceneDispose();

    // DLSS及びNGXの解放
    DLSSManager::GetInstance().ShutdownNGX();

    // 描画終了処理
    Renderer::Uninit();
}

// ウィンドウプロシージャ
LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
        return true;

    switch(msg)
    {
        case WM_DESTROY:
            { 
                PostQuitMessage(0); 
            }
            break;

        default:
            { /* DO_NOTHING */ }
            break;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
}
