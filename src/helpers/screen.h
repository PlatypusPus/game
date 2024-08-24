#ifndef SCREEN_H
#define SCREEN_H
#include <raylib.h>

void ToggleFullScreen(int screenWidth, int screenHeight)
{
    if (IsWindowFullscreen())
    {
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    }
    else
    {
        ToggleFullscreen();
        SetWindowSize(screenWidth, screenHeight);
    }
}

#endif