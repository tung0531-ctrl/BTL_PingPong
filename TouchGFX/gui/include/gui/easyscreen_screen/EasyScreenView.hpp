#ifndef EASYSCREENVIEW_HPP
#define EASYSCREENVIEW_HPP

#include <gui_generated/easyscreen_screen/EasyScreenViewBase.hpp>
#include <gui/easyscreen_screen/EasyScreenPresenter.hpp>

class EasyScreenView : public EasyScreenViewBase
{
public:
    EasyScreenView();
    virtual ~EasyScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();
protected:


    float ballX;      // Vị trí X của bóng
	float ballY;      // Vị trí Y của bóng
	float ballVelX;   // Vận tốc X của bóng
	float ballVelY;   // Vận tốc Y của bóng
	bool waitingForServe; // Trạng thái chờ phát bóng
	int serveDelayTicks;  // Đếm tick để delay 1 giây
	int servingPlayer;    // Người chơi phát bóng (1: paddle1, 2: paddle2)
    int score1;           // Điểm số người chơi 1
    int score2;           // Điểm số người chơi 2
    bool gameOver;        // Trạng thái trò chơi
    int buzzerBeepCounter;      // Bộ đếm số lần bíp của buzzer
    bool buzzerBeepState;
    float desiredBallVelY1;         // Vận tốc Y mong muốn cho người chơi 1
	float desiredBallVelY2;         // Vận tốc Y mong muốn cho người chơi 2
	float lineAngle1, lineAngle2;    // Góc của đường line1 và line1_1
};

#endif // EASYSCREENVIEW_HPP
