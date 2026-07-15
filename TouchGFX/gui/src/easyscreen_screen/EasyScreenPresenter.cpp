#include <gui/easyscreen_screen/EasyScreenView.hpp>
#include <gui/easyscreen_screen/EasyScreenPresenter.hpp>
#include <gui/model/Model.hpp>

EasyScreenPresenter::EasyScreenPresenter(EasyScreenView& v)
    : view(v)
{

}

void EasyScreenPresenter::activate()
{
	model->resetGame();
}

void EasyScreenPresenter::deactivate()
{

}


void  EasyScreenPresenter::handleTickEvent()
{

}
void EasyScreenPresenter::goToEndScreen(int winner)
{
    // Gửi thông tin người chiến thắng đến Model
    model->setWinner(winner);
    // Yêu cầu chuyển đến EndScreen
    static_cast<FrontendApplication*>(touchgfx::Application::getInstance())->gotoEndScreenScreenNoTransition();
}
void EasyScreenPresenter::resetGame()
{
    model->resetGame();
}
