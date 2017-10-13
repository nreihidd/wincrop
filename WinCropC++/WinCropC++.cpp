// WinCropC++.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WinCropC++.h"
#include <map>

#define MAX_LOADSTRING 100

struct ThumbInfo {
	HWND original;
	HWND thumbWindow;
	HTHUMBNAIL thumbnail;
	RECT view;
	double scale;
};

// Global Variables:
int lastMouseX = 0;
int lastMouseY = 0;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
std::map<HWND, ThumbInfo> activeThumbs;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
ATOM				MyRegisterThumbClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	ThumbProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
#if DEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINCROPC, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	MyRegisterThumbClass(hInstance);

	// Perform application initialization:
	HWND hWnd = InitInstance(hInstance, nCmdShow);
	if (hWnd == NULL)
	{
		return FALSE;
	}

	if (!RegisterHotKey(hWnd, 1, MOD_NOREPEAT | MOD_WIN, 'A')) {
		return FALSE;
	}
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINCROPC));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINCROPC));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINCROPC);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

ATOM MyRegisterThumbClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ThumbProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINCROPC));
	wcex.hCursor = NULL; // LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0)); // (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL; // MAKEINTRESOURCE(IDC_WINCROPC);
	wcex.lpszClassName = _T("WINCROPTHUMB");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx(NULL, szWindowClass, szTitle, NULL, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, HWND_MESSAGE, NULL, hInstance, NULL);

   if (hWnd == NULL)
   {
      return NULL;
   }

   // ShowWindow(hWnd, nCmdShow);
   // UpdateWindow(hWnd);

   return hWnd;
}

enum Corner {
	TOPLEFT,
	BOTTOMLEFT,
	TOPRIGHT,
	BOTTOMRIGHT
};
BOOL isLeft(Corner c) {
	return c <= 1;
}
BOOL isTop(Corner c) {
	return (c & 1) == 0;
}

long DwmFindWidth(long height, long owidth, long oheight) {
	return round(height * (double)owidth / oheight);
}
long DwmFindHeight(long width, long owidth, long oheight) {
	return round(width * (double)oheight / owidth);
}
void DwmFindBiggest(long *width, long *height, long owidth, long oheight) {
	long cheight = DwmFindHeight(*width, owidth, oheight);
	if (cheight <= *height) {
		*height = cheight;
		return;
	}
	long cwidth = DwmFindWidth(*height, owidth, oheight);
	if (cwidth <= *width) {
		*width = cwidth;
		return;
	}
	printf("Uh, WTF?\n");
}

void ResizeRect(RECT *r, long width, long height, Corner anchor) {
	if (isLeft(anchor)) {
		r->right = r->left + width;
	}
	else {
		r->left = r->right - width;
	}
	if (isTop(anchor)) {
		r->bottom = r->top + height;
	}
	else {
		r->top = r->bottom - height;
	}
}

Corner SizingAnchor(WPARAM wParam) {
	switch (wParam) {
	case WMSZ_RIGHT:
	case WMSZ_BOTTOMRIGHT:
	case WMSZ_BOTTOM:
		return Corner::TOPLEFT;
	case WMSZ_TOP:
	case WMSZ_TOPRIGHT:
		return Corner::BOTTOMLEFT;
	case WMSZ_LEFT:
	case WMSZ_BOTTOMLEFT:
		return Corner::TOPRIGHT;
	case WMSZ_TOPLEFT:
		return Corner::BOTTOMRIGHT;
	default:
		return Corner::TOPLEFT;
	}
}

#define RESIZE_BORDER 10
#define VISIBLE_BORDER 1
void ManipulateBorder(RECT *r, int border) {
	r->left -= border;
	r->right += border;
	r->top -= border;
	r->bottom += border;
}
int RectWidth(const RECT &r) {
	return r.right - r.left;
}
int RectHeight(const RECT &r) {
	return r.bottom - r.top;
}

void UpdateDestinationRect(RECT *dst, const RECT &rect) {
	dst->left = VISIBLE_BORDER;
	dst->top = VISIBLE_BORDER;
	dst->right = RectWidth(rect) - VISIBLE_BORDER;
	dst->bottom = RectHeight(rect) - VISIBLE_BORDER;
}

void ScaleThumb(HWND hWnd, WPARAM wParam, RECT *rect) {
	auto iter = activeThumbs.find(hWnd);
	if (iter == activeThumbs.end()) return;
	ThumbInfo &info = iter->second;
	RECT &dst = *rect;
	ManipulateBorder(&dst, -VISIBLE_BORDER);
	long width = RectWidth(dst),
		height = RectHeight(dst),
		owidth = RectWidth(info.view),
		oheight = RectHeight(info.view);
	switch (wParam) {
	case WMSZ_RIGHT:
	case WMSZ_LEFT:
		height = DwmFindHeight(width, owidth, oheight);
		break;
	case WMSZ_TOP:
	case WMSZ_BOTTOM:
		width = DwmFindWidth(height, owidth, oheight);
		break;
	default:
		DwmFindBiggest(&width, &height, owidth, oheight);
		break;
	}
	ResizeRect(&dst, width, height, SizingAnchor(wParam));
	ManipulateBorder(&dst, VISIBLE_BORDER);

	DWM_THUMBNAIL_PROPERTIES properties;
	properties.dwFlags = DWM_TNP_RECTDESTINATION;
	UpdateDestinationRect(&properties.rcDestination, *rect);
	DwmUpdateThumbnailProperties(info.thumbnail, &properties);

	info.scale = width / (double)owidth;
	// printf("Scale: %f\n", info.scale);
}

void IntersectRect(const RECT &a, const RECT &b, RECT *dst) {
	dst->left = max(a.left, b.left);
	dst->right = min(a.right, b.right);
	dst->top = max(a.top, b.top);
	dst->bottom = min(a.bottom, b.bottom);
}

void DwmQueryThumbnailSourceRect(HTHUMBNAIL thumbnail, RECT *rect) {
	SIZE size;
	DwmQueryThumbnailSourceSize(thumbnail, &size);
	rect->left = 0;
	rect->top = 0;
	rect->right = size.cx;
	rect->bottom = size.cy;
}

void CropThumb(HWND hWnd, WPARAM wParam, RECT *rect) {
	auto iter = activeThumbs.find(hWnd);
	if (iter == activeThumbs.end()) return;
	ThumbInfo &info = iter->second;
	RECT &dst = *rect;
	ManipulateBorder(&dst, -VISIBLE_BORDER);
	ResizeRect(&info.view, round(RectWidth(dst) / info.scale), round(RectHeight(dst) / info.scale), SizingAnchor(wParam));
	RECT bounds;
	DwmQueryThumbnailSourceRect(info.thumbnail, &bounds);
	IntersectRect(info.view, bounds, &info.view);
	ResizeRect(&dst, round(RectWidth(info.view) * info.scale), round(RectHeight(info.view) * info.scale), SizingAnchor(wParam));
	ManipulateBorder(&dst, VISIBLE_BORDER);

	DWM_THUMBNAIL_PROPERTIES properties;
	properties.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE;
	UpdateDestinationRect(&properties.rcDestination, *rect);
	properties.rcSource = info.view;
	DwmUpdateThumbnailProperties(info.thumbnail, &properties);
}

void TranslateRectInsideRect(RECT *translating, const RECT bounds, int dx, int dy) {
	dx = max(bounds.left - translating->left, min(bounds.right - translating->right, dx));
	dy = max(bounds.top - translating->top, min(bounds.bottom - translating->bottom, dy));
	translating->left += dx;
	translating->right += dx;
	translating->top += dy;
	translating->bottom += dy;
}

void PanThumb(HWND hWnd, int deltaX, int deltaY) {
	auto iter = activeThumbs.find(hWnd);
	if (iter == activeThumbs.end()) return;
	ThumbInfo &info = iter->second;
	int deltaXV = round(deltaX / info.scale),
		deltaYV = round(deltaY / info.scale);
	RECT bounds;
	DwmQueryThumbnailSourceRect(info.thumbnail, &bounds);
	TranslateRectInsideRect(&info.view, bounds, deltaXV, deltaYV);

	DWM_THUMBNAIL_PROPERTIES properties;
	properties.dwFlags = DWM_TNP_RECTSOURCE;
	properties.rcSource = info.view;
	DwmUpdateThumbnailProperties(info.thumbnail, &properties);
}

void ZoomThumb(HWND hWnd, double deltaZ, int posX, int posY) {
	auto iter = activeThumbs.find(hWnd);
	if (iter == activeThumbs.end()) return;
	ThumbInfo &info = iter->second;
	RECT thumbBounds;
	GetWindowRect(info.thumbWindow, &thumbBounds);
	ManipulateBorder(&thumbBounds, -VISIBLE_BORDER);
	RECT bounds;
	DwmQueryThumbnailSourceRect(info.thumbnail, &bounds);
	// Only change the info.view/info.scale in response to a mouse wheel
	// finish this
	double percentX = (posX - thumbBounds.left) / (double)RectWidth(thumbBounds),
		percentY = (posY - thumbBounds.top) / (double)RectHeight(thumbBounds);
	int	viewX = info.view.left + RectWidth(info.view) * percentX,
		viewY = info.view.top + RectHeight(info.view) * percentY;
	double minScale = max(RectWidth(thumbBounds) / (double)RectWidth(bounds), RectHeight(thumbBounds) / (double)RectHeight(bounds));
	info.scale = max(min(info.scale + deltaZ / 10.0, 2.5), minScale);
	ResizeRect(&info.view, round(RectWidth(thumbBounds) / info.scale), round(RectHeight(thumbBounds) / info.scale), Corner::TOPLEFT);
	int	viewXp = info.view.left + RectWidth(info.view) * percentX,
		viewYp = info.view.top + RectHeight(info.view) * percentY;
	TranslateRectInsideRect(&info.view, bounds, viewX - viewXp, viewY - viewYp);

	DWM_THUMBNAIL_PROPERTIES properties;
	properties.dwFlags = DWM_TNP_RECTSOURCE;
	properties.rcSource = info.view;
	DwmUpdateThumbnailProperties(info.thumbnail, &properties);
}

void ToggleOriginal(HWND hWnd) {
	auto iter = activeThumbs.find(hWnd);
	if (iter == activeThumbs.end()) return;
	ThumbInfo &info = iter->second;
	RECT rect;
	if (!GetWindowRect(info.original, &rect)) return;
	if (rect.top < -5000) {
		MoveWindow(info.original, rect.left, rect.top + 10000, RectWidth(rect), RectHeight(rect), FALSE);
	}
	else {
		MoveWindow(info.original, rect.left, rect.top - 10000, RectWidth(rect), RectHeight(rect), FALSE);
	}
}

void SetScaleToOne(HWND hWnd) {
	auto iter = activeThumbs.find(hWnd);
	if (iter == activeThumbs.end()) return;
	ThumbInfo &info = iter->second;

	// Change the thumbWindow's size to match 1:1 with the already chosen area of the original window (info.view)
	RECT desiredWindowSize = info.view;
	ManipulateBorder(&desiredWindowSize, VISIBLE_BORDER);

	// Resize the thumbWindow but keep its center in place
	RECT currentPosition;
	GetWindowRect(hWnd, &currentPosition);
	int cx = (currentPosition.left + currentPosition.right) / 2,
		cy = (currentPosition.top + currentPosition.bottom) / 2;
	MoveWindow(hWnd, cx - RectWidth(desiredWindowSize) / 2, cy - RectHeight(desiredWindowSize) / 2, RectWidth(desiredWindowSize), RectHeight(desiredWindowSize), TRUE);

	// Update the info struct by setting scale to 1
	info.scale = 1;

	// Finally, update the thumbnail destination to fit the resized window
	DWM_THUMBNAIL_PROPERTIES properties;
	properties.dwFlags = DWM_TNP_RECTDESTINATION;
	UpdateDestinationRect(&properties.rcDestination, desiredWindowSize);
	DwmUpdateThumbnailProperties(info.thumbnail, &properties);
}

void DestroyThumb(HWND hWnd) {
	auto iter = activeThumbs.find(hWnd);
	if (iter == activeThumbs.end()) {
		DestroyWindow(hWnd);
		return;
	}
	ThumbInfo &info = iter->second;
	DwmUnregisterThumbnail(info.thumbnail);
	RECT rect;
	if (GetWindowRect(info.original, &rect)) {
		if (rect.top < -5000) {
			MoveWindow(info.original, rect.left, rect.top + 10000, RectWidth(rect), RectHeight(rect), FALSE);
		}
	}
	activeThumbs.erase(iter);
	DestroyWindow(hWnd);
}

LRESULT CALLBACK ThumbProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	// TODO: try a mode giving focus to the original; try translating and passing along mouse messages to the original
	int x, y;
	RECT rect;
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
			SetScaleToOne(hWnd);
			break;
		case VK_RETURN:
			ToggleOriginal(hWnd);
			break;
		case VK_ESCAPE:
			DestroyThumb(hWnd);
			break;
		case VK_DELETE:
			if (MessageBox(hWnd, _T("Are you sure you want to quit?"), _T("WinCrop"), MB_OKCANCEL) == IDOK) {
				PostQuitMessage(0);
			}
			break;
		}
	case WM_NCHITTEST:
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		// printf("%i x %i\n", x, y);
		if (GetWindowRect(hWnd, &rect)) {
			ManipulateBorder(&rect, -RESIZE_BORDER);
			BOOL left = x < rect.left,
				right = x > rect.right,
				top = y < rect.top,
				bottom = y > rect.bottom;

			if (right) {
				if (bottom) return HTBOTTOMRIGHT;
				if (top) return HTTOPRIGHT;
				return HTRIGHT;
			}
			if (left) {
				if (bottom) return HTBOTTOMLEFT;
				if (top) return HTTOPLEFT;
				return HTLEFT;
			}
			if (top) return HTTOP;
			if (bottom) return HTBOTTOM;
		}
		return HTCAPTION;
	case WM_SIZING:
		if (GetKeyState(VK_SHIFT) & 0x7000) {
			ScaleThumb(hWnd, wParam, (RECT*)lParam);
		}
		else {
			CropThumb(hWnd, wParam, (RECT*)lParam);
		}
		return TRUE;
	case WM_MOUSEWHEEL:
		{
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			// printf("wheel %d\n", (int)zDelta);
			ZoomThumb(hWnd, zDelta / 120.0, xPos, yPos);
		}
		break;
	case WM_MBUTTONUP:
		if (!(GetKeyState(VK_MBUTTON) & 0x7000)) {
			// printf("releasing\n");
			SetCursor(NULL);
			ReleaseCapture();
		}
		break;
	case WM_NCMBUTTONDOWN:
		if (GetKeyState(VK_MBUTTON) & 0x7000) {
			RECT rect;
			GetWindowRect(hWnd, &rect);
			lastMouseX = GET_X_LPARAM(lParam) - rect.left;
			lastMouseY = GET_Y_LPARAM(lParam) - rect.top;
			// printf("capturing %d, %d\n", lastMouseX, lastMouseY);
			SetCapture(hWnd);
			SetCursor(LoadCursor(NULL, IDC_SIZEALL));
		}
		break;
	case WM_MOUSEMOVE:
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int xDelta = xPos - lastMouseX;
			int yDelta = yPos - lastMouseY;
			lastMouseX = xPos;
			lastMouseY = yPos;
			if (GetKeyState(VK_MBUTTON) & 0x7000) {
				PanThumb(hWnd, -xDelta, -yDelta);
			}
			// printf("move %d, %d\n", xPos, yPos);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void PrintRect(RECT &r) {
	printf("%i, %i, %i, %i(%i x %i)", r.left, r.right, r.top, r.bottom, r.right - r.left, r.bottom - r.top);
}

void CreateThumbWindow(HWND hTarget) {
	printf("Making new thumb window\n");
	ThumbInfo info;
	info.original = hTarget;
	info.thumbWindow = CreateWindowEx(/* WS_EX_NOACTIVATE | */ WS_EX_TOPMOST, _T("WINCROPTHUMB"), _T("Dunno"), WS_VISIBLE | WS_POPUP, 0, 0, 350, 350, NULL, NULL, NULL, NULL);
		
	if (info.thumbWindow == NULL) {
		return;
	}

	HTHUMBNAIL thumbnail = NULL;
	if (!SUCCEEDED(DwmRegisterThumbnail(info.thumbWindow, info.original, &thumbnail))) {
		DestroyWindow(info.thumbWindow);
		return;
	}
	info.thumbnail = thumbnail;

	DwmQueryThumbnailSourceRect(thumbnail, &info.view);
	printf("SIZE: %i x %i\n", RectWidth(info.view), RectHeight(info.view));

	RECT targetRect;
	GetWindowRect(info.original, &targetRect);
	printf("RECT: ");
	PrintRect(targetRect);
	printf("\n");

	activeThumbs[info.thumbWindow] = info;

	RECT rect;
	GetWindowRect(info.thumbWindow, &rect);
	ScaleThumb(info.thumbWindow, WMSZ_RIGHT, &rect);
	MoveWindow(info.thumbWindow, (targetRect.left + targetRect.right - RectWidth(rect)) / 2, (targetRect.top + targetRect.bottom - RectHeight(rect)) / 2, RectWidth(rect), RectHeight(rect), TRUE);
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_HOTKEY:
		if (wParam == 1) {
			HWND foregroundHWnd = GetForegroundWindow();
			CreateThumbWindow(foregroundHWnd);
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}