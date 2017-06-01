#include <windows.h> 
#include <gdiplus.h> 

#pragma comment(lib,"gdiplus.lib")

using namespace Gdiplus;

WCHAR szClassName[ ] = L"PNGDialog";
WCHAR szWindowName[] = L"St2-045 检测工具";

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

typedef enum _BUTTON_STATUS
{
    BS_NORMAL,
    BS_OVER,
    BS_DOWN

} BUTTON_STATUS, *PBUTTON_STATUS;

BUTTON_STATUS Button1_status;
INT Button1_Width;
INT Button1_Height;
INT Button1_x;
INT Button1_y;
ULONG Button1BkgLen;
Bitmap *Button1_BitMap;
PointF *pt1;

HDC g_screenDC;
HDC memDC;
HWND hWnd;
SIZE wndSize;
HBITMAP memBitmap;
BLENDFUNCTION g_blendFunction;
Graphics *pGraphics;
Image *pButtonNormal;
Image *pButtonOver;
Image *pButtonDown;
SolidBrush *pBrush;
Font *pFont;

//pGraphics->Clear(Color(255, 0, 0, 0));
//SolidBrush blackBrush(Color(255, 0, 0, 0));     // ARGB
//pGraphics->FillRectangle(&blackBrush, (int)x, y, Width, Height);  // 清空区域
 
int WINAPI WinMain (HINSTANCE hThisInstance, 
                     HINSTANCE hPrevInstance, 
                     LPSTR lpszArgument, 
                     int nCmdShow) 
{
    ULONG_PTR gdiplusStartupToken;
    GdiplusStartupInput gdiInput; 
    GdiplusStartup(&gdiplusStartupToken,&gdiInput,NULL);

    WNDCLASSEXW wincl;
    //
    wincl.hInstance = hThisInstance; 
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style   = CS_DBLCLKS;                /* CS_DBLCLKS == Catch double-clicks */ 
    wincl.cbSize  = sizeof (WNDCLASSEX); 
    wincl.hIcon   = LoadIcon (NULL, IDI_APPLICATION); 
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION); 
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW); 
    wincl.lpszMenuName = NULL;                 /* No menu */ 
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */ 
    wincl.cbWndExtra = 0;                      /* structure or the window instance */ 
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND; 
    //
    if (!RegisterClassExW (&wincl)) 
        return -1;
 
    //======================================================
    //     WCHAR *strFileName = L"d:\\1.png"; 
    //     Image *m_pImage = Image::FromFile(strFileName);  // 带路径的

    UINT Width;
    UINT Height;
    Image *pImageBkg = new Image(L"bkg.png");
    UINT Width1  = pImageBkg->GetWidth();
    UINT Height1 = pImageBkg->GetHeight();

    Image *pImage2 = new Image(L"bkg1.png");
    UINT Width2  = pImage2->GetWidth();
    UINT Height2 = pImage2->GetHeight();

    Height = Height1 + Height2 - 80;
    Width  = Width1 + Width2 - 60;
    //
    RECT rScreen;
    SystemParametersInfo( SPI_GETWORKAREA, sizeof(RECT), &rScreen, 0 );
    //
    hWnd = CreateWindowExW ( 
           WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,       // WS_EX_TOPMOST == 全局置顶; WS_EX_TOOLWINDOW == 不在任务栏显示
           szClassName,
           szWindowName, 
           WS_POPUP,            // 没有标题栏和边框
           (rScreen.right-Width)   / 2,
           (rScreen.bottom-Height) / 2,
           Width,
           Height, 
           HWND_DESKTOP,        /* The window is a child-window to desktop */ 
           NULL,                /* No menu */ 
           hThisInstance,       /* Program Instance handler */ 
           NULL                 /* No Window Creation data */ 
           ); 
 
    /* Make the window visible on the screen */ 
    ShowWindow (hWnd, nCmdShow);
 
    /******************************************** 
    *   step 1. 
    *   Using Gdiplus to load the image 
    ********************************************/ 

    RECT wndRect; 
    GetWindowRect(hWnd,&wndRect); 
    wndSize.cx = wndRect.right-wndRect.left;
    wndSize.cy = wndRect.bottom-wndRect.top; 
    HDC hdc = GetDC(hWnd); 
    memDC = CreateCompatibleDC(hdc);
    memBitmap = CreateCompatibleBitmap(hdc,wndSize.cx,wndSize.cy); 
    HBITMAP OldBitmap = (HBITMAP)SelectObject(memDC,memBitmap);

    pGraphics = new Graphics(memDC); 
    pGraphics->DrawImage(pImageBkg, 0, 0, Width1, Height1);
    pGraphics->DrawImage(pImage2, (int)Width1-60, Height1-80, Width2, Height2); 
    delete pImage2;
    pImage2 = NULL;

    // Initialize arguments.
    pFont = new Font(L"微软雅黑"/*"Arial"*/, 13);
    PointF pt(12, 10);
    pBrush = new SolidBrush(Color(255, 255, 255, 255));  // ARGB

    // Draw string.
    pGraphics->DrawString(
        szWindowName,
        (sizeof(szWindowName)-2)/2,
        pFont,
        pt,
        pBrush);

    pButtonNormal = new Image(L"btn_nor.png");
    Button1_Width  = pButtonNormal->GetWidth()  + 60;
    Button1_Height = pButtonNormal->GetHeight() + 18;
    Button1_x = (Width1-Button1_Width)/2;
    Button1_y = Height1-60;

    //================================
    PixelFormat pf = pImageBkg->GetPixelFormat();
    Rect rc(Button1_x, Button1_y, Button1_Width, Button1_Height); 
    Button1_BitMap = ((Bitmap *)pImageBkg)->Clone(rc, pf);
    
    delete pImageBkg;
    pImageBkg = NULL;

    pGraphics->DrawImage(pButtonNormal, (int)Button1_x, Button1_y, Button1_Width, Button1_Height);

    pt1 = new PointF(192, 217);
    pBrush->SetColor(0xFF000000);
    pGraphics->DrawString(L"检测", 2, pFont, *pt1, pBrush);

    pButtonOver = new Image(L"btn_over.png");
    pButtonDown = new Image(L"btn_down.png");

    /********************************************* 
    *   step 3. 
    *   Use UpdateLayeredWindow to Draw the Window 
    *********************************************/

    // get screen dc 
    g_screenDC = GetDC(NULL);
    //
    RECT Rect;
    GetWindowRect(hWnd,&Rect);
    POINT ptSrc = {0,0};
    POINT ptDst = {Rect.left, Rect.top};
    g_blendFunction.AlphaFormat = AC_SRC_ALPHA; 
    g_blendFunction.BlendFlags = 0; 
    g_blendFunction.BlendOp = AC_SRC_OVER; 
    g_blendFunction.SourceConstantAlpha = 255; 
    UpdateLayeredWindow(hWnd,g_screenDC,&ptDst,&wndSize,memDC,&ptSrc,0,&g_blendFunction,ULW_ALPHA);
 
    /* Run the message loop. It will run until GetMessage() returns 0 */
    MSG messages;
    while (GetMessage (&messages, NULL, 0, 0)) 
    { 
        TranslateMessage(&messages); 
        DispatchMessage(&messages); 
    }

    delete pt1;
    pt1 = NULL;
    delete pFont;
    pFont = NULL;
    delete pGraphics;
    pGraphics = NULL;
    delete pButtonNormal;
    pButtonNormal = NULL;
    delete pButtonOver;
    pButtonOver = NULL;
    delete pButtonDown;
    pButtonDown = NULL;

    if (g_screenDC)
        ReleaseDC(NULL, g_screenDC);
    SelectObject(memDC, OldBitmap);
    DeleteObject(memBitmap);
    if (memDC)
        DeleteDC(memDC);
    ReleaseDC(hWnd, hdc);

    GdiplusShutdown(gdiplusStartupToken); 
    return (int)messages.wParam;  /* The program return-value is 0 - The value that PostQuitMessage() gave */
} 
 
 
/*  This function is called bY the Windows function DispatchMessage()  */ 
 
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    switch (message)                  /* handle the messages */ 
    {
        case WM_CREATE:
            break;
        case WM_DESTROY: 
            PostQuitMessage (0);      /* send a WM_QUIT to the message queue */ 
            break; 
        case WM_LBUTTONDOWN:
            {
                if (Button1_status != BS_DOWN)
                {
                    Button1_status = BS_DOWN;
                    SolidBrush blackBrush(Color(255, 0, 0, 0));     // ARGB
                    pGraphics->FillRectangle(&blackBrush, Button1_x, Button1_y, Button1_Width, Button1_Height);  // 清空区域
                    pGraphics->DrawImage(Button1_BitMap,  Button1_x, Button1_y, Button1_Width, Button1_Height);
                    pGraphics->DrawImage(pButtonDown,     Button1_x, Button1_y, Button1_Width, Button1_Height);
                    pGraphics->DrawString(L"检测", 2, pFont, *pt1, pBrush);
                    POINT ptSrc = {0,0};
                    UpdateLayeredWindow(hWnd,g_screenDC,NULL,&wndSize,memDC,&ptSrc,0,&g_blendFunction,ULW_ALPHA);
                }
            }
            break;
        case WM_LBUTTONUP:
            {
                if (Button1_status = BS_DOWN)
                {
                    Button1_status = BS_OVER;
                    SolidBrush blackBrush(Color(255, 0, 0, 0));     // ARGB
                    pGraphics->FillRectangle(&blackBrush, Button1_x, Button1_y, Button1_Width, Button1_Height);  // 清空区域
                    pGraphics->DrawImage(Button1_BitMap,  Button1_x, Button1_y, Button1_Width, Button1_Height);
                    pGraphics->DrawImage(pButtonOver,     Button1_x, Button1_y, Button1_Width, Button1_Height);
                    pGraphics->DrawString(L"检测", 2, pFont, *pt1, pBrush);
                    POINT ptSrc = {0,0};
                    UpdateLayeredWindow(hWnd,g_screenDC,NULL,&wndSize,memDC,&ptSrc,0,&g_blendFunction,ULW_ALPHA);
                }
            }
            break;
        case WM_NCHITTEST:           // 优先捕获点击
            {
                INT x = LOWORD(lParam);
                INT y = HIWORD(lParam);

                RECT Rect;
                GetWindowRect(hWnd,&Rect);

                x -= Rect.left;
                y -= Rect.top;

                if ( (x >= Button1_x && x <= Button1_x + Button1_Width) &&
                     (y >= Button1_y && y <= Button1_y + Button1_Height) )
                {
                    if (Button1_status != BS_OVER && Button1_status != BS_DOWN)
                    {
                        Button1_status = BS_OVER;
                        SolidBrush blackBrush(Color(255, 0, 0, 0));     // ARGB
                        pGraphics->FillRectangle(&blackBrush, Button1_x, Button1_y, Button1_Width, Button1_Height);  // 清空区域
                        pGraphics->DrawImage(Button1_BitMap,  Button1_x, Button1_y, Button1_Width, Button1_Height);
                        pGraphics->DrawImage(pButtonOver,     Button1_x, Button1_y, Button1_Width, Button1_Height);
                        pGraphics->DrawString(L"检测", 2, pFont, *pt1, pBrush);
                        POINT ptSrc = {0,0};
                        UpdateLayeredWindow(hWnd,g_screenDC,NULL,&wndSize,memDC,&ptSrc,0,&g_blendFunction,ULW_ALPHA);
                    }

                    return DefWindowProc (hwnd, message, wParam, lParam);
                }
                else
                {
                    if (Button1_status != BS_NORMAL)
                    {
                        Button1_status = BS_NORMAL;
                        SolidBrush blackBrush(Color(255, 0, 0, 0));     // ARGB
                        pGraphics->FillRectangle(&blackBrush, Button1_x, Button1_y, Button1_Width, Button1_Height);  // 清空区域
                        pGraphics->DrawImage(Button1_BitMap,  Button1_x, Button1_y, Button1_Width, Button1_Height);
                        pGraphics->DrawImage(pButtonNormal,   Button1_x, Button1_y, Button1_Width, Button1_Height);
                        pGraphics->DrawString(L"检测", 2, pFont, *pt1, pBrush);
                        POINT ptSrc = {0,0};
                        UpdateLayeredWindow(hWnd,g_screenDC,NULL,&wndSize,memDC,&ptSrc,0,&g_blendFunction,ULW_ALPHA);
                    }

                    return HTCAPTION;         // 响应窗口移动
                }
            }
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam); 
    } 
    return 0; 
} 

// LockBits()
//BYTE *Button1Bkg;
BYTE *GetButtonBkg(Bitmap *Bkg, PULONG BkgLen, UINT bX, UINT bY, UINT bW, UINT bH)
{
    *BkgLen = 0;
    PixelFormat pf = Bkg->GetPixelFormat();
    ULONG PixLen = (((ULONG)pf & 0xFF00)>>8)>>3;  // 每像素长度(Bytes)

    BitmapData Bitdata; 
    Rect rc(bX, bY, bW, bH); 
    Button1_BitMap = Bkg->Clone(rc, pf);
    Bkg->LockBits(&rc, ImageLockModeRead, pf, &Bitdata); 
    
    BYTE *p = (BYTE *)Bitdata.Scan0;

    ULONG ButtonBkgLen = bW * bH * PixLen;
    BYTE *ButtonBkgBuf = new BYTE[ ButtonBkgLen ];
    if (ButtonBkgBuf)
    {
        memcpy( ButtonBkgBuf, p, ButtonBkgLen );
        *BkgLen = ButtonBkgLen;
    }

    Bkg->UnlockBits(&Bitdata); 
    return ButtonBkgBuf;
}

