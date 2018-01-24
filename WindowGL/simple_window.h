#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <memory>
#include <string>

// set application instance for windows 
void SimpleSetInstance(HINSTANCE hinstance);

std::pair<HWND, HGLRC> SimpleCreateWindow(int width, int height, std::string title);

void SimpleDestroyContext(std::pair<HWND, HGLRC> window);

std::pair<int, int> SimpleGetWindowSize(const HWND& hwnd);
