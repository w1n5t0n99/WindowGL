#include "simple_window.h"
#include "opengl.h"
#include "NanoLog.hpp"

#include <codecvt>

namespace
{
	HINSTANCE KHInstance = NULL;
	int KNumberOfWindows = 0;

	static LRESULT OnDestory()
	{
		KNumberOfWindows--;

		if (KNumberOfWindows <= 0)
		{
			PostQuitMessage(0);
		}

		return 0;
	}

	LRESULT CALLBACK SimpleWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_DESTROY:
			OnDestory();
			return 0;

		case WM_CREATE:
			KNumberOfWindows++;

		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}

}

void SimpleSetInstance(HINSTANCE hinstance)
{
	KHInstance = hinstance;
}


std::pair<HWND, HGLRC> SimpleCreateWindow(int width, int height, std::string title)
{
	if (!KHInstance)
	{
		LOG_CRIT << "HINSTANCE null";
		return { NULL, NULL };
	}

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = SimpleWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = TEXT("OPENGL-WINDOW");
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);	// Window background brush color.
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Window cursor
	RegisterClass(&wc);

	/* Create a temporaray context to get address of wgl extensions. */
	HWND hwnd_tmp = CreateWindowEx(WS_EX_APPWINDOW,
		wc.lpszClassName,
		TEXT("TEMP-WINDOW"),
		WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL,
		KHInstance,
		NULL);
	if (!hwnd_tmp)
	{
		LOG_CRIT << "temp HWND null";
		return { NULL, NULL };
	}

	HDC dc_tmp = GetDC(hwnd_tmp);
	if (!dc_tmp)
	{
		LOG_CRIT << "tmp JDC null";
		DestroyWindow(hwnd_tmp);
		return { NULL, NULL };
	}

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.iLayerType = PFD_MAIN_PLANE;

	HGLRC hglrc_tmp;
	int iPF;
	if ((!(iPF = ChoosePixelFormat(dc_tmp, &pfd)) || !SetPixelFormat(dc_tmp, iPF, &pfd)) || 
		(!(hglrc_tmp = wglCreateContext(dc_tmp)) || !wglMakeCurrent(dc_tmp, hglrc_tmp)))
	{
		LOG_CRIT << "temp HGLRC null";
		ReleaseDC(hwnd_tmp, dc_tmp);
		DestroyWindow(hwnd_tmp);
		return { NULL, NULL };
	}
	
	/* Like all extensions in Win32, the function pointers returned by wglGetProcAddress are tied
	* to the render context they were obtained with. Since this is a temporary context, we
	* place those function pointers in automatic storage of the window and context creation function. */
	WGL_Init();

	HWND hwnd;
	HGLRC hglrc;
	HDC hdc;

	if (wglChoosePixelFormatARB && wglCreateContextAttribsARB)
	{
		/* good we have access to extended pixelformat and context attributes */
		hwnd = CreateWindowEx(WS_EX_APPWINDOW,
			wc.lpszClassName,
			TEXT("MAIN-WINDOW"),
			WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			0, 0, width, height,
			NULL, NULL,
			KHInstance,
			NULL);

		if (!hwnd)
		{
			LOG_CRIT << "main HWND null";
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc_tmp);
			ReleaseDC(hwnd_tmp, dc_tmp);
			DestroyWindow(hwnd_tmp);
			return { NULL, NULL };
		}

		hdc = GetDC(hwnd);
		if (!hdc)
		{
			LOG_CRIT << "main HDC null";
			DestroyWindow(hwnd);

			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc_tmp);
			ReleaseDC(hwnd_tmp, dc_tmp);
			DestroyWindow(hwnd_tmp);
			return { NULL, NULL };
		}

		int attribs[] = 
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_SAMPLE_BUFFERS_ARB, 1, // Number of buffers (must be 1 at time of writing)
			WGL_SAMPLES_ARB, 4,        // Number of samples
			0
		};

		UINT num_formats_choosen;
		BOOL choose_pf_success = wglChoosePixelFormatARB(hdc, attribs, NULL, 1, &iPF, &num_formats_choosen);
		if (!choose_pf_success)
			LOG_CRIT << "choose main PIXELFORMAT failed";
		else
			LOG_INFO << "choose main PIXELFORMAT succeeded";


		/* now this is a kludge; we need to pass something in the PIXELFORMATDESCRIPTOR
		* to SetPixelFormat; it will be ignored, mostly. OTOH we want to send something
		* sane, we're nice people after all - it doesn't hurt if this fails. */
		DescribePixelFormat(hdc, iPF, sizeof(pfd), &pfd);

		if (!(choose_pf_success &&
			num_formats_choosen >= 1 &&
			SetPixelFormat(hdc, iPF, &pfd)))
		{
			LOG_CRIT << "main PIXELFORMAT creation failed";
			ReleaseDC(hwnd, hdc);
			DestroyWindow(hwnd);

			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc_tmp);
			ReleaseDC(hwnd_tmp, dc_tmp);
			DestroyWindow(hwnd_tmp);
			return { NULL, NULL };
		}
		else
		{
			LOG_INFO << "main PIXELFORMAT creation succeeded";
		}

		/* we request a OpenGL-4 compatibility profile */
		int otherBits = 0;
		int context_attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 5,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | otherBits,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		hglrc = wglCreateContextAttribsARB(hdc, NULL, context_attribs);
		if (!hglrc)
		{
			LOG_CRIT << "main HGLRC null";
			ReleaseDC(hwnd, hdc);
			DestroyWindow(hwnd);

			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc_tmp);
			ReleaseDC(hwnd_tmp, dc_tmp);
			DestroyWindow(hwnd_tmp);
			return { NULL, NULL };
		}
		else
		{
			LOG_INFO << "creating main HGLRC context succeeded";
		}

		if (!wglMakeCurrent(hdc, hglrc))
			LOG_CRIT << "making main HGLRC current context failed";
		else
			LOG_INFO << "making main HGLRC current context succeeded";

		OpenGL_Init();

		/* now that we've created the proper window, DC and RC
		* we can delete the temporaries */
		//wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc_tmp);
		ReleaseDC(hwnd_tmp, dc_tmp);
		DestroyWindow(hwnd_tmp);
	}
	else
	{
		/* extended pixelformats and context attributes not supported
		* => use temporary window and context as the proper ones */
		hwnd = hwnd_tmp;
		hdc = dc_tmp;
		hglrc = hglrc_tmp;
	}

	LOG_INFO << "Version: " << (char*)glGetString(GL_VERSION) << " VENDOR: " << (char*)glGetString(GL_VENDOR);

	ReleaseDC(hwnd, hdc);
	return { hwnd, hglrc };
}

void SimpleDestroyContext(std::pair<HWND, HGLRC> window)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(window.second);
	wglDeleteContext(window.second);
	PostQuitMessage(0);
}