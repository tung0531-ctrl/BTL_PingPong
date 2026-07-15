#include <gui/mediumscreen_screen/MediumScreenView.hpp>
#include <gui/mediumscreen_screen/MediumScreenPresenter.hpp>

MediumScreenPresenter::MediumScreenPresenter(MediumScreenView& v)
    : view(v)
{

}

void MediumScreenPresenter::activate()
{
	model->resetGame();
}

void MediumScreenPresenter::deactivate()
{

}
void MediumScreenPresenter::handleTickEvent()
{

}
void MediumScreenPresenter::goToEndScreen(int winner){
	model->setWinner(winner);
	static_cast<FrontendApplication*>(touchgfx::Application::getInstance())->gotoEndScreenScreenNoTransition();
}
void MediumScreenPresenter::resetGame(){
	model->resetGame();
}
