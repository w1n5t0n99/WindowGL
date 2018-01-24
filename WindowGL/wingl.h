#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <memory>
#include <string>
#include <bitset>


namespace wingl
{

	enum class WndFlags
	{
		KFullscreen,
		kFullscreenBorderless,
		KMaximized,
		KMinimized,
		KResizable,
		KShown,
		KHidden,
		KBorderless,
		KHideMouse
	};

	static const std::bitset<16> KWndFullscreen(1 << static_cast<int>(WndFlags::KFullscreen));
	static const std::bitset<16> KWndFullscreenBorderless(1 << static_cast<int>(WndFlags::kFullscreenBorderless));
	static const std::bitset<16> KWndMaximized(1 << static_cast<int>(WndFlags::KMaximized));


}
