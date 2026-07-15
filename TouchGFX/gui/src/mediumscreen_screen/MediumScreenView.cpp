#include <gui/mediumscreen_screen/MediumScreenView.hpp>
#include "main.h"
#include "cmsis_os.h"
#include "joystick_task.h"
#include <cmath>

extern osMessageQueueId_t joystickQueueHandle;

MediumScreenView::MediumScreenView():ballX(160), ballY(120), ballVelX(1.4f), ballVelY(1.4f),
								  waitingForServe(false), serveDelayTicks(0), servingPlayer(0),
								  score1(0), score2(0),gameOver(false),buzzerBeepCounter(0), buzzerBeepState(false),
								  desiredBallVelY1(0.0f), desiredBallVelY2(0.0f),lineAngle1(0.0f), lineAngle2(0.0f)
{

}

void MediumScreenView::setupScreen()
{
    MediumScreenViewBase::setupScreen();
    score1 = 0;
	score2 = 0;
	gameOver = false;
	ballX = 160;
	ballY = 120;
	ballVelX = 1.4f;
	ballVelY = 1.4f;
	waitingForServe = false;
	serveDelayTicks = 0;
	servingPlayer = 0;
    buzzerBeepCounter = 0;
    buzzerBeepState = false;
    // Đặt vị trí ban đầu của bóng

    desiredBallVelY1 = 0.0f;
    desiredBallVelY2 = 0.0f;
    lineAngle1 = 0.0f;
    lineAngle2 = 0.0f;

    // Ẩn các đường ban đầu
    line1.setVisible(false);
    line1_1.setVisible(false);
    line1.invalidate();
    line1_1.invalidate();

	ball.invalidate();
	ball.moveTo(ballX, ballY);
	ball.invalidate();
    // Khởi tạo điểm số
	Unicode::snprintf(MediumScreenViewBase::score1Buffer, MediumScreenViewBase::SCORE1_SIZE, "%d", score1);
	MediumScreenViewBase::score1.invalidate();
	Unicode::snprintf(MediumScreenViewBase::score2Buffer, MediumScreenViewBase::SCORE2_SIZE, "%d", score2);
	MediumScreenViewBase::score2.invalidate();
}

void MediumScreenView::tearDownScreen()
{
    MediumScreenViewBase::tearDownScreen();
}

void MediumScreenView::handleTickEvent()
{

    if (gameOver) {
    	line1.setVisible(false);
		line1_1.setVisible(false);
		line1.invalidate();
		line1_1.invalidate();
        return;
    }
    // Xử lý bíp buzzer
	if (buzzerBeepCounter > 0) {
		if (buzzerBeepCounter % 6 == 0) { // Bíp mỗi 100ms (6 ticks tại 60 FPS)
			buzzerBeepState = !buzzerBeepState;
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, buzzerBeepState ? GPIO_PIN_SET : GPIO_PIN_RESET);
		}
		buzzerBeepCounter--;
		if (buzzerBeepCounter == 0) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET); // Tắt buzzer sau khi bíp xong
		}
	}

    JoystickCommand_t command;
    while (osMessageQueueGet(joystickQueueHandle, &command, NULL, 0) == osOK) {
        switch (command) {
            case JOY1_LEFT:
				{
					int16_t newY=paddle1.getY()+2;
					if (newY >198) {
						newY = 198; // Dừng tại cạnh dưới
					}
                    if (newY != paddle1.getY()) { // Chỉ cập nhật nếu có thay đổi
                        paddle1.invalidate();
                        paddle1.moveTo(paddle1.getX(), newY);
                        paddle1.invalidate();
                    }
				}
                break;

            case JOY1_RIGHT:
				{
					int16_t newY = paddle1.getY() - 2;
					if (newY < 2) {
						newY = 2; // Dừng tại cạnh trên
					}
					if (newY != paddle1.getY()) { // Chỉ cập nhật nếu có thay đổi
						paddle1.invalidate();
						paddle1.moveTo(paddle1.getX(), newY);
						paddle1.invalidate();
					}
				}
                break;
            case JOY2_LEFT:
				{
					int16_t newY=paddle2.getY()+2;
					if (newY >198) {
						newY = 198; // Dừng tại cạnh dưới
					}
					if (newY != paddle2.getY()) { // Chỉ cập nhật nếu có thay đổi
						paddle2.invalidate();
						paddle2.moveTo(paddle2.getX(), newY);
						paddle2.invalidate();
					}
				}
                break;
            case JOY2_RIGHT:
				{
					int16_t newY = paddle2.getY() - 2;
					if (newY < 2) {
						newY = 2; // Dừng tại cạnh trên
					}
					if (newY != paddle2.getY()) { // Chỉ cập nhật nếu có thay đổi
						paddle2.invalidate();
						paddle2.moveTo(paddle2.getX(), newY);
						paddle2.invalidate();
					}
				}
				break;

            case JOY1_UP:
				if (waitingForServe && servingPlayer == 1) {
					desiredBallVelY1 -= 0.2f;
					if (desiredBallVelY1 < -2.0f) desiredBallVelY1 = -2.0f;
					lineAngle1 = atan2f(desiredBallVelY1, 2.0f) * 180.0f / M_PI;
					// Cập nhật đường dẫn với tâm bóng
					line1.invalidate();
										float ballCenterX = ballX + ball.getWidth() / 2.0f;
										float ballCenterY = ballY + ball.getHeight() / 2.0f;
										line1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33); // Kích thước vùng bao quanh
										line1.setStart(16, 16); // Tâm tương đối trong vùng
										line1.setEnd(16 + 30 * cosf(lineAngle1 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle1 * M_PI / 180.0f));
										line1.setVisible(true);
										line1.invalidate();
				}
				break;

			case JOY1_DOWN:
				if (waitingForServe && servingPlayer == 1) {
					desiredBallVelY1 += 0.2f;
					if (desiredBallVelY1 > 2.0f) desiredBallVelY1 = 2.0f;
					lineAngle1 = atan2f(desiredBallVelY1, 2.0f) * 180.0f / M_PI;
					// Cập nhật đường dẫn với tâm bóng
					line1.invalidate();
										float ballCenterX = ballX + ball.getWidth() / 2.0f;
										float ballCenterY = ballY + ball.getHeight() / 2.0f;
										line1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);
										line1.setStart(16, 16);
										line1.setEnd(16 + 30 * cosf(lineAngle1 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle1 * M_PI / 180.0f));
										line1.setVisible(true);
										line1.invalidate();
				}
				break;

			case JOY2_UP:
				if (waitingForServe && servingPlayer == 2) {
					desiredBallVelY2 -= 0.2f;
					if (desiredBallVelY2 < -2.0f) desiredBallVelY2 = -2.0f;
					lineAngle2 = atan2f(desiredBallVelY2, -2.0f) * 180.0f / M_PI;
					// Cập nhật đường dẫn với tâm bóng
					line1_1.invalidate();
									   float ballCenterX = ballX + ball.getWidth() / 2.0f;
									   float ballCenterY = ballY + ball.getHeight() / 2.0f;
									   line1_1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);
									   line1_1.setStart(16, 16);
									   line1_1.setEnd(16 + 30 * cosf(lineAngle2 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle2 * M_PI / 180.0f));
									   line1_1.setVisible(true);
									   line1_1.invalidate();
				}
				break;

			case JOY2_DOWN:
				if (waitingForServe && servingPlayer == 2) {
					desiredBallVelY2 += 0.2f;
					if (desiredBallVelY2 > 2.0f) desiredBallVelY2 = 2.0f;
					lineAngle2 = atan2f(desiredBallVelY2, -2.0f) * 180.0f / M_PI;
					// Cập nhật đường dẫn với tâm bóng
					line1_1.invalidate();
										float ballCenterX = ballX + ball.getWidth() / 2.0f;
										float ballCenterY = ballY + ball.getHeight() / 2.0f;
										line1_1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);
										line1_1.setStart(16, 16);
										line1_1.setEnd(16 + 30 * cosf(lineAngle2 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle2 * M_PI / 180.0f));
										line1_1.setVisible(true);
										line1_1.invalidate();
				}
				break;

            case JOY1_BUTTON:
                if (waitingForServe && servingPlayer == 1) {
                    waitingForServe = false;
                    float speed = 2.0f;
                    ballVelX = sqrt(speed * speed - desiredBallVelY1 * desiredBallVelY1);
                    ballVelY = desiredBallVelY1;
                    if (ballVelX < 0.5f) {
                        ballVelX = 0.5f;
                        ballVelY = (desiredBallVelY1 >= 0) ? sqrt(speed * speed - ballVelX * ballVelX)
                                                          : -sqrt(speed * speed - ballVelX * ballVelX);
                    }
                    ball.setVisible(true);
                    ball.invalidate();
                    line1.setVisible(false);
                    line1.invalidate();
                }
                break;

            case JOY2_BUTTON:
                if (waitingForServe && servingPlayer == 2) {
                    waitingForServe = false;
                    float speed = 2.0f;
                    ballVelX = -sqrt(speed * speed - desiredBallVelY2 * desiredBallVelY2);
                    ballVelY = desiredBallVelY2;
                    if (ballVelX > -0.5f) {
                        ballVelX = -0.5f;
                        ballVelY = (desiredBallVelY2 >= 0) ? sqrt(speed * speed - ballVelX * ballVelX)
                                                          : -sqrt(speed * speed - ballVelX * ballVelX);
                    }
                    ball.setVisible(true);
                    ball.invalidate();
                    line1_1.setVisible(false);
                    line1_1.invalidate();
                }
                break;
        }
    }

    // Cập nhật bóng nếu không chờ phát
        if (!waitingForServe) {
            ball.invalidate();
            ballX += ballVelX;
            ballY += ballVelY;

            // Xử lý va chạm với cạnh trên/dưới
            if (ballY < 0) {
                ballY = 0;
                ballVelY = -ballVelY;
            } else if (ballY + ball.getHeight() > 240) {
                ballY = 240 - ball.getHeight();
                ballVelY = -ballVelY;
            }

            // Xử lý va chạm với paddle1 (bên trái)
            if (ballX <= paddle1.getX() + paddle1.getWidth() &&
                ballX >= paddle1.getX() &&
                ballY + ball.getHeight() >= paddle1.getY() &&
                ballY <= paddle1.getY() + paddle1.getHeight()) {
                ballX = paddle1.getX() + paddle1.getWidth();
                ballVelX = -ballVelX;
            }

            // Xử lý va chạm với paddle2 (bên phải)
            if (ballX + ball.getWidth() >= paddle2.getX() &&
                ballX + ball.getWidth() <= paddle2.getX() + paddle2.getWidth() &&
                ballY + ball.getHeight() >= paddle2.getY() &&
                ballY <= paddle2.getY() + paddle2.getHeight()) {
                ballX = paddle2.getX() - ball.getWidth();
                ballVelX = -ballVelX;
            }

            // Xử lý va chạm với bên trái (ballX < 1)
			if (ballX >319) {
				if (ballY > 60 && ballY < 181) {
                        // Bóng trong vùng goal2, cộng điểm cho người chơi 1
                        score1++;
                        Unicode::snprintf(MediumScreenViewBase::score1Buffer, MediumScreenViewBase::SCORE1_SIZE, "%d", score1);
                        MediumScreenViewBase::score1.invalidate();
                        buzzerBeepCounter = 18; // 3 tiếng bíp (18 ticks = 3 * 6 ticks tại 60 FPS)
						buzzerBeepState = true;
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
                        if (score1 >= 11) {
                            gameOver = true;
                            presenter->goToEndScreen(1);
                        }
                        ball.setVisible(false);
                        ball.invalidate();
                        waitingForServe = true;
                        serveDelayTicks = 60; // 1 giây với 60 FPS
                        servingPlayer = 1; // Người chơi 1 phát bóng
                        ballX = paddle1.getX() + paddle1.getWidth() + 10; // Gần paddle1
                        ballY = paddle1.getY() + paddle1.getHeight()/2 - ball.getHeight()/2; // Căn giữa paddle
                        desiredBallVelY1 = 0.0f;
					    lineAngle1 = 0.0f;
					    // Cập nhật đường dẫn với tâm bóng
					    line1.invalidate();
								   float ballCenterX = ballX + ball.getWidth() / 2.0f;
								   float ballCenterY = ballY + ball.getHeight() / 2.0f;
								   line1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);
								   line1.setStart(16, 16);
								   line1.setEnd(16 + 30 * cosf(lineAngle1 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle1 * M_PI / 180.0f));
								   line1.setVisible(true);
								   line1.invalidate();
								   ball.moveTo(ballX, ballY);
                        ball.invalidate();
                        return; // Thoát để không xử lý thêm
                    } else {
                        // Bóng ngoài vùng goal, phản xạ
                        ballX = 319;
                        ballVelX = -ballVelX;
                    }
                }

                // Xử lý va chạm với bên phải (ballX > 319)
                if (ballX <1) {
                    if (ballY > 60 && ballY < 181) {
                        // Bóng trong vùng goal1, cộng điểm cho người chơi 2
                        score2++;
                        Unicode::snprintf(MediumScreenViewBase::score2Buffer, MediumScreenViewBase::SCORE2_SIZE, "%d", score2);
                        MediumScreenViewBase::score2.invalidate();
                        buzzerBeepCounter = 18; // 3 tiếng bíp (18 ticks = 3 * 6 ticks tại 60 FPS)
						buzzerBeepState = true;
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
                        if (score2 >= 11) {
                            gameOver = true;
                            presenter->goToEndScreen(2);
                        }
                        ball.setVisible(false);
                        ball.invalidate();
                        waitingForServe = true;
                        serveDelayTicks = 60; // 1 giây với 60 FPS
                        servingPlayer = 2; // Người chơi 2 phát bóng
                        ballX = paddle2.getX() - ball.getWidth() - 10; // Gần paddle2
                        ballY = paddle2.getY() + paddle2.getHeight()/2 - ball.getHeight()/2; // Căn giữa paddle
                        desiredBallVelY2 = 0.0f;
						lineAngle2 = 0.0f;
						// Cập nhật đường dẫn với tâm bóng
						line1_1.invalidate();
									float ballCenterX = ballX + ball.getWidth() / 2.0f;
									float ballCenterY = ballY + ball.getHeight() / 2.0f;
									line1_1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);
									line1_1.setStart(16, 16);
									line1_1.setEnd(16 + 30 * cosf(lineAngle2 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle2 * M_PI / 180.0f));
									line1_1.setVisible(true);
									line1_1.invalidate();
									ball.moveTo(ballX, ballY);
                        ball.invalidate();
                        return; // Thoát để không xử lý thêm
                    } else {
                        // Bóng ngoài vùng goal, phản xạ
                        ballX = 1;
                        ballVelX = -ballVelX;
                    }
                }


            // Di chuyển bóng
            ball.moveTo(ballX, ballY);
            ball.invalidate();
        } else {
            // Đếm thời gian chờ 1 giây
            if (serveDelayTicks > 0) {
                serveDelayTicks--;
                if (serveDelayTicks == 0) {
                    // Hiện bóng sau 1 giây
                    ball.setVisible(true);
                    ball.invalidate();
                }
            }
            // Cập nhật vị trí bóng theo paddle
            if (servingPlayer == 1) {
                ballY = paddle1.getY() + paddle1.getHeight()/2 - ball.getHeight()/2;
                // Cập nhật đường dẫn với tâm bóng
                line1.invalidate();
                            float ballCenterX = ballX + ball.getWidth() / 2.0f;
                            float ballCenterY = ballY + ball.getHeight() / 2.0f;
                            line1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);
                            line1.setStart(16, 16);
                            line1.setEnd(16 + 30 * cosf(lineAngle1 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle1 * M_PI / 180.0f));
                            line1.setVisible(true);
                            line1.invalidate();
            } else if (servingPlayer == 2) {
                ballY = paddle2.getY() + paddle2.getHeight()/2 - ball.getHeight()/2;
                // Cập nhật đường dẫn với tâm bóng
                line1_1.invalidate();
                           float ballCenterX = ballX + ball.getWidth() / 2.0f;
                           float ballCenterY = ballY + ball.getHeight() / 2.0f;
                           line1_1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);
                           line1_1.setStart(16, 16);
                           line1_1.setEnd(16 + 30 * cosf(lineAngle2 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle2 * M_PI / 180.0f));
                           line1_1.setVisible(true);
                           line1_1.invalidate();
            }
            ball.moveTo(ballX, ballY);
            ball.invalidate();
        }
}





