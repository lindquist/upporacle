#include "App.h"

void SendButton::RightDown(Point p, dword keyflags)
{
    if (IsEnabled()) {
        App().StatusMessage("On");
    }
    else {
        App().StatusMessage("Off");
    }
}
