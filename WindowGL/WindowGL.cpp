#include <iostream>
#include <fstream>
#include "error.h"
#include <assert.h>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "scene.h"
#include "renderer.h"
#include "preamble.glsl"
#include "packed_freelist.h"
#include "shader_system.h"
#include "simple_window.h"
#include "NanoLog.hpp"

const int KWIDTH = 1200;
const int KHEIGHT = 800;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	nanolog::initialize(nanolog::GuaranteedLogger(), " ", "logs", 1);
  
	SimpleSetInstance(hInstance);
	auto window = SimpleCreateWindow(KWIDTH, KHEIGHT, "LearnOpenGL");
	auto current_window_width = KWIDTH;
	auto current_window_height = KHEIGHT;


	if (window.first == NULL)
		LOG_INFO << "Window handle null";

	ShowWindow(window.first, nCmdShow);
	UpdateWindow(window.first);

	Scene scene;
	scene.Init();

	Renderer renderer;
	renderer.Init(&scene, KWIDTH, KHEIGHT);

	auto hdc = GetWindowDC(window.first);

	bool quit_game = false;
	while (!quit_game)
	{
		MSG msg;
		switch (::MsgWaitForMultipleObjectsEx(0, NULL, 12, QS_ALLINPUT, 0))
		{
		case WAIT_OBJECT_0:
			while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					quit_game = true;// return;
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		case WAIT_TIMEOUT:

			//check for windows resize
			auto sz = SimpleGetWindowSize(window.first);
			if (sz.first != current_window_width || sz.second != current_window_height)
			{
				current_window_width = sz.first;
				current_window_height = sz.second;
				if(!quit_game )
					renderer.Resize(current_window_width, current_window_height);
			}

			renderer.Paint();
			::SwapBuffers(hdc);
		}
	}


	ReleaseDC(window.first, hdc);
	SimpleDestroyContext(window);
	return 0;

}



