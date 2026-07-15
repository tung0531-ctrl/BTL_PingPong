#include <gui/endscreen_screen/EndScreenView.hpp>
#include <gui/endscreen_screen/EndScreenPresenter.hpp>

EndScreenPresenter::EndScreenPresenter(EndScreenView& v)
    : view(v)
{

}

void EndScreenPresenter::activate()
{
	view.updateWinner();
}

void EndScreenPresenter::deactivate()
{

}
int EndScreenPresenter::getWinner(int playerId)
{
    return model->getWinner(playerId);
}

void EndScreenPresenter::handlePlayAgain()
{
    model->resetGame();
    static_cast<FrontendApplication*>(touchgfx::Application::getInstance())->gotoStartScreenScreenNoTransition();
}

