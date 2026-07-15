#ifndef EASYREADYSCREENVIEW_HPP
#define EASYREADYSCREENVIEW_HPP

#include <gui_generated/easyreadyscreen_screen/EasyReadyScreenViewBase.hpp>
#include <gui/easyreadyscreen_screen/EasyReadyScreenPresenter.hpp>

class EasyReadyScreenView : public EasyReadyScreenViewBase
{
public:
    EasyReadyScreenView();
    virtual ~EasyReadyScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
	virtual void updateCounter(int value); // Hàm cập nhật đếm ngược
protected:
	//int countdownValue;
	int tickCounter=180;
};

#endif // EASYREADYSCREENVIEW_HPP
