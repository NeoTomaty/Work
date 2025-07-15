#include    "main.h"
#include    "Application.h"
#include    <locale>
#include    <iostream>


int main(void)
{

    // wcoutで日本語が出力されるための処理
    std::locale::global(std::locale("ja-JP"));
    std::wcout.imbue(std::locale("ja-JP"));

#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else
    HWND consoleWindow = GetConsoleWindow(); // コンソールウィンドウのハンドルを取得
    ShowWindow(consoleWindow, SW_HIDE);     // コンソールウィンドウを非表示にする
#endif//defined(DEBUG) || defined(_DEBUG)

    // アプリケーション実行
    Application app(SCREEN_WIDTH,SCREEN_HEIGHT);
    app.Run();

    return 0;
}