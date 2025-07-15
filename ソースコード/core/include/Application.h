#pragma once

#include    <Windows.h>
#include    <cstdint>
#include    "NonCopyable.h"


// アプリケーションクラス
class Application : NonCopyable
{

public:

    // メンバ関数
    Application(uint32_t width, uint32_t height);
    ~Application();
    void Run();

    // 幅を取得
    static uint32_t GetWidth() 
    {
        return m_Width;
    }

    // 高さを取得
    static uint32_t GetHeight() 
    {
        return m_Height;
    }

    // ウインドウハンドルを返す
    static HWND GetWindow() 
    {
        return m_hWnd;
    }

    // ドローコールの回数を取得
    static unsigned int GetDrawNum()
    {
        return m_DrawNum;
    }

    // 描画されているポリゴン数を取得
    static unsigned int GetPolygonNum()
    {
        return m_PolygonNum;
    }

    // 描画情報をカウント
    static void CountDrawInfo(unsigned int polygonNum ,bool isShadow)
    {
        m_DrawNum++;

        if (!isShadow)
        m_PolygonNum += polygonNum;
    }

    // 描画情報をリセット
    static void ResetDrawInfo()
    {
        m_DrawNum = 0;
        m_PolygonNum = 0;
    }

private:

    static bool InitApp();
    static void TermApp();
    static bool InitWnd();
    static void TermWnd();
    static void MainLoop();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);


private:
    // メンバ変数
    static HINSTANCE   m_hInst;        // インスタンスハンドル
    static HWND        m_hWnd;         // ウィンドウハンドル
    static uint32_t    m_Width;        // ウィンドウの横幅
    static uint32_t    m_Height;       // ウィンドウの縦幅


    static unsigned int m_DrawNum;     // ドローコール回数
    static unsigned int m_PolygonNum;  // 描画されているポリゴン数

};