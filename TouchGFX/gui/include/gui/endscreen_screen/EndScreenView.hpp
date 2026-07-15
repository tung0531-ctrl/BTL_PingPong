#ifndef ENDSCREENVIEW_HPP
#define ENDSCREENVIEW_HPP

#include <gui_generated/endscreen_screen/EndScreenViewBase.hpp>
#include <gui/endscreen_screen/EndScreenPresenter.hpp>

class EndScreenView : public EndScreenViewBase
{
public:
    EndScreenView();
    virtual ~EndScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void updateWinner();
protected:
};

#endif // ENDSCREENVIEW_HPP
