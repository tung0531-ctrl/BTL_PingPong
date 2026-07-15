#include <gui/hardscreen_screen/HardScreenView.hpp>
#include <gui/hardscreen_screen/HardScreenPresenter.hpp>

HardScreenPresenter::HardScreenPresenter(HardScreenView& v)
    : view(v)
{

}

void HardScreenPresenter::activate()
{

}

void HardScreenPresenter::deactivate()
{

}

void HardScreenPresenter::handleTickEvent()
{

}
void HardScreenPresenter::goToEndScreen(int winner)
{
    model->setWinner(winner);
    static_cast<FrontendApplication*>(touchgfx::Application::getInstance())->gotoEndScreenScreenNoTransition();
}
void HardScreenPresenter::resetGame()
{
    model->resetGame();
}
