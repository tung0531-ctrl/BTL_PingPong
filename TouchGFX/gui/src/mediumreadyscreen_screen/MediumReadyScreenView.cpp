#include <gui/mediumreadyscreen_screen/MediumReadyScreenView.hpp>

MediumReadyScreenView::MediumReadyScreenView()
{

}

void MediumReadyScreenView::setupScreen()
{
    MediumReadyScreenViewBase::setupScreen();
    updateCounter(tickCounter);
}

void MediumReadyScreenView::tearDownScreen()
{
    MediumReadyScreenViewBase::tearDownScreen();
}
void MediumReadyScreenView::handleTickEvent()
{

	    if (tickCounter > 0)
	    {
	        tickCounter--;
	        updateCounter(tickCounter);

	    }else{
	    	application().gotoMediumScreenScreenNoTransition();
	    }

}
void MediumReadyScreenView::updateCounter(int value)
{
	 	int countdownDisplay = 3;
	    if (value > 120)
	        countdownDisplay = 3;
	    else if (value > 60)
	        countdownDisplay = 2;
	    else if (value > 0)
	        countdownDisplay = 1;
	    else
	        countdownDisplay = 0;

	    Unicode::snprintf(counter1Buffer, COUNTER1_SIZE, "%d", countdownDisplay);
	    Unicode::snprintf(counter2Buffer, COUNTER2_SIZE, "%d", countdownDisplay);
	    counter1.setWildcard(counter1Buffer);
	    counter2.setWildcard(counter2Buffer);
	    counter1.invalidate();  // Vẽ lại TextArea
	    counter2.invalidate();  // Vẽ lại TextArea
}
