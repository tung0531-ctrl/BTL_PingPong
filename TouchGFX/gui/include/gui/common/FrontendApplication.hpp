#ifndef FRONTENDAPPLICATION_HPP
#define FRONTENDAPPLICATION_HPP

#include <gui_generated/common/FrontendApplicationBase.hpp>
#include "buzzer_music.h"

class FrontendHeap;

using namespace touchgfx;

class FrontendApplication : public FrontendApplicationBase
{
public:
    FrontendApplication(Model& m, FrontendHeap& heap);
    virtual ~FrontendApplication() { }

    virtual void handleTickEvent()
    {
        model.tick();
        // Advance the buzzer's note/effect state machine on every single tick
        // regardless of which screen is currently shown (Start/Ready/Play/End).
        // Previously this was only called from inside the 3 play screens'
        // handleTickEvent(), so the background tune got stuck on its very
        // first note whenever the app was on any other screen.
        BuzzerMusic_Update();
        FrontendApplicationBase::handleTickEvent();
    }
private:
};

#endif // FRONTENDAPPLICATION_HPP
