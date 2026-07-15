#include <gui/endscreen_screen/EndScreenView.hpp>
#include <cstdio>

EndScreenView::EndScreenView()
{

}

void EndScreenView::setupScreen()
{
    EndScreenViewBase::setupScreen();

}


void EndScreenView::tearDownScreen()
{
    EndScreenViewBase::tearDownScreen();
}

void EndScreenView::updateWinner()
{
    // Gán số vào playerwinBuffer
    int valueToDisplay = 0;
    if (presenter->getWinner(1)) {
        valueToDisplay = 1;
    } else if (presenter->getWinner(2)) {
        valueToDisplay = 2;
    }
    Unicode::snprintf(playerwinBuffer, PLAYERWIN_SIZE, "%d", valueToDisplay);
    playerwinBuffer[PLAYERWIN_SIZE - 1] = 0; // Đảm bảo null terminator
    playerwin.setWildcard(playerwinBuffer); // Liên kết lại buffer
    playerwin.resizeToCurrentText(); // Điều chỉnh kích thước
    playerwin.setVisible(true); // Đảm bảo hiển thị
    playerwin.invalidate(); // Yêu cầu vẽ lại
}
