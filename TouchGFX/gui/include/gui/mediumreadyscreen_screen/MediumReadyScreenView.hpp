#ifndef MEDIUMREADYSCREENVIEW_HPP
#define MEDIUMREADYSCREENVIEW_HPP

#include <gui_generated/mediumreadyscreen_screen/MediumReadyScreenViewBase.hpp>
#include <gui/mediumreadyscreen_screen/MediumReadyScreenPresenter.hpp>

class MediumReadyScreenView : public MediumReadyScreenViewBase
{
public:
    MediumReadyScreenView();
    virtual ~MediumReadyScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
	virtual void updateCounter(int value); // Hàm cập nhật đếm ngược
protected:
	//int countdownValue;
	int tickCounter=180;
};

#endif // MEDIUMREADYSCREENVIEW_HPP
