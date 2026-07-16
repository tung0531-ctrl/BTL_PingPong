#include <gui/hardscreen_screen/HardScreenView.hpp>
#include "buzzer_music.h"
#include "cmsis_os.h"
#include "joystick_task.h"
#include <cmath>

extern osMessageQueueId_t joystickQueueHandle;

namespace
{
float clampImpact(float value)
{
    if (value < -1.0f)
    {
        return -1.0f;
    }
    if (value > 1.0f)
    {
        return 1.0f;
    }
    return value;
}

void applyPaddleBounce(float& ballX, float& ballY, float& ballVelX, float& ballVelY, int ballWidth, int ballHeight,
                       const touchgfx::Box& paddle, bool bounceRight)
{
    const float speed = sqrtf(ballVelX * ballVelX + ballVelY * ballVelY);
    const float paddleCenterY = paddle.getY() + paddle.getHeight() * 0.5f;
    const float ballCenterY = ballY + ballHeight * 0.5f;
    const float halfPaddleHeight = paddle.getHeight() * 0.5f;
    float impact = 0.0f;

    if (halfPaddleHeight > 0.0f)
    {
        impact = clampImpact((ballCenterY - paddleCenterY) / halfPaddleHeight);
    }

    const float maxVerticalSpeed = speed * 0.85f;
    ballVelY = impact * maxVerticalSpeed;

    float horizontalSpeed = sqrtf(speed * speed - ballVelY * ballVelY);
    if (horizontalSpeed < 0.5f)
    {
        horizontalSpeed = 0.5f;
        const float remainingVertical = speed * speed - horizontalSpeed * horizontalSpeed;
        ballVelY = (ballVelY >= 0.0f ? 1.0f : -1.0f) * sqrtf(remainingVertical > 0.0f ? remainingVertical : 0.0f);
    }

    ballVelX = bounceRight ? horizontalSpeed : -horizontalSpeed;
    ballX = bounceRight ? paddle.getX() + paddle.getWidth() : paddle.getX() - ballWidth;
}
}

HardScreenView::HardScreenView():ballX(160), ballY(120), ballVelX(1.6f), ballVelY(1.6f),
								  waitingForServe(false), serveDelayTicks(0), servingPlayer(0),
								  score1(0), score2(0),gameOver(false),buzzerBeepCounter(0), buzzerBeepState(false),
								  desiredBallVelY1(0.0f), desiredBallVelY2(0.0f),lineAngle1(0.0f), lineAngle2(0.0f)
{

}

void HardScreenView::invalidateAbsoluteArea(const touchgfx::Rect& before, const touchgfx::Rect& after)
{
    touchgfx::Rect dirty = before;
    dirty.expandToFit(after);
    touchgfx::Rect screenBounds(0, 0, 320, 240);
    dirty &= screenBounds;
    if (dirty.width > 0 && dirty.height > 0)
    {
        invalidateRect(dirty);
        refreshStaticBars(dirty);
    }
}

void HardScreenView::refreshStaticBars(const touchgfx::Rect& dirty)
{
    if (dirty.intersect(goal1.getAbsoluteRect()))
    {
        goal1.invalidate();
    }
    if (dirty.intersect(goal2.getAbsoluteRect()))
    {
        goal2.invalidate();
    }
    if (dirty.intersect(image1.getAbsoluteRect()))
    {
        image1.invalidate();
    }
    if (dirty.intersect(image1_1.getAbsoluteRect()))
    {
        image1_1.invalidate();
    }
    if (dirty.intersect(image2.getAbsoluteRect()))
    {
        image2.invalidate();
    }
    if (dirty.intersect(image2_2.getAbsoluteRect()))
    {
        image2_2.invalidate();
    }
}

void HardScreenView::refreshStaticScene()
{
    __background.invalidate();
    box3.invalidate();
    box1.invalidate();
    box2.invalidate();
    goal1.invalidate();
    goal2.invalidate();
    image1.invalidate();
    image1_1.invalidate();
    image2.invalidate();
    image2_2.invalidate();
}

void HardScreenView::moveBallTo(int16_t x, int16_t y)
{
    touchgfx::Rect before = ball.getAbsoluteRect();
    ball.moveTo(x, y);
    touchgfx::Rect after = ball.getAbsoluteRect();
    invalidateAbsoluteArea(before, after);
}

void HardScreenView::updateAimLine(touchgfx::Line& line, float centerX, float centerY, float angleDeg, bool visible)
{
    touchgfx::Rect before = line.getAbsoluteRect();
    line.setPosition(centerX - 16, centerY - 16, 33, 33);
    line.setStart(16, 16);
    line.setEnd(16 + 30 * cosf(angleDeg * M_PI / 180.0f), 16 + 30 * sinf(angleDeg * M_PI / 180.0f));
    line.setVisible(visible);
    touchgfx::Rect after = line.getAbsoluteRect();
    invalidateAbsoluteArea(before, after);
}

void HardScreenView::hideAimLines()
{
    const float ballCenterX = ballX + ball.getWidth() / 2.0f;
    const float ballCenterY = ballY + ball.getHeight() / 2.0f;

    updateAimLine(line1, ballCenterX, ballCenterY, lineAngle1, false);
    updateAimLine(line1_1, ballCenterX, ballCenterY, lineAngle2, false);
}

void HardScreenView::showAimLineForPlayer(int player)
{
    hideAimLines();

    const float ballCenterX = ballX + ball.getWidth() / 2.0f;
    const float ballCenterY = ballY + ball.getHeight() / 2.0f;

    if (player == 1)
    {
        updateAimLine(line1, ballCenterX, ballCenterY, lineAngle1, true);
    }
    else if (player == 2)
    {
        updateAimLine(line1_1, ballCenterX, ballCenterY, lineAngle2, true);
    }
}

void HardScreenView::setupScreen()
{
    HardScreenViewBase::setupScreen();
    BuzzerMusic_StartGameLoop();
    score1 = 0;
	score2 = 0;
	gameOver = false;
	ballX = 160;
	ballY = 120;

	ballVelX = 1.6f;
	ballVelY = 1.6f;
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
    hideAimLines();
	ball.invalidate();
    moveBallTo(ballX, ballY);
	ball.invalidate();
    // Khởi tạo điểm số
	Unicode::snprintf(HardScreenViewBase::score1Buffer, HardScreenViewBase::SCORE1_SIZE, "%d", score1);
	HardScreenViewBase::score1.invalidate();
	Unicode::snprintf(HardScreenViewBase::score2Buffer, HardScreenViewBase::SCORE2_SIZE, "%d", score2);
	HardScreenViewBase::score2.invalidate();
}

void HardScreenView::tearDownScreen()
{
    BuzzerMusic_Stop();
    HardScreenViewBase::tearDownScreen();
}

void HardScreenView::handleTickEvent()
{
    invalidate();
    refreshStaticScene();
    BuzzerMusic_Update();


    if (gameOver) {
        BuzzerMusic_Stop();
        hideAimLines();
        return; // Không xử lý nếu trò chơi kết thúc
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
                    showAimLineForPlayer(1);
				}
				break;

			case JOY1_DOWN:
				if (waitingForServe && servingPlayer == 1) {
					desiredBallVelY1 += 0.2f;
					if (desiredBallVelY1 > 2.0f) desiredBallVelY1 = 2.0f;
					lineAngle1 = atan2f(desiredBallVelY1, 2.0f) * 180.0f / M_PI;
                    showAimLineForPlayer(1);
				}
				break;

			case JOY2_UP:
				if (waitingForServe && servingPlayer == 2) {
					desiredBallVelY2 -= 0.2f;
					if (desiredBallVelY2 < -2.0f) desiredBallVelY2 = -2.0f;
					lineAngle2 = atan2f(desiredBallVelY2, -2.0f) * 180.0f / M_PI;
                    showAimLineForPlayer(2);
				}
				break;

			case JOY2_DOWN:
				if (waitingForServe && servingPlayer == 2) {
					desiredBallVelY2 += 0.2f;
					if (desiredBallVelY2 > 2.0f) desiredBallVelY2 = 2.0f;
					lineAngle2 = atan2f(desiredBallVelY2, -2.0f) * 180.0f / M_PI;
                    showAimLineForPlayer(2);
				}
				break;

            case JOY1_BUTTON:
                if (waitingForServe && servingPlayer == 1) {
                    waitingForServe = false;
                    float speed = 2.3f;
                    ballVelX = sqrt(speed * speed - desiredBallVelY1 * desiredBallVelY1);
                    ballVelY = desiredBallVelY1;
                    if (ballVelX < 0.5f) {
                        ballVelX = 0.5f;
                        ballVelY = (desiredBallVelY1 >= 0) ? sqrt(speed * speed - ballVelX * ballVelX)
                                                          : -sqrt(speed * speed - ballVelX * ballVelX);
                    }
                    ball.setVisible(true);
                    ball.invalidate();
                    hideAimLines();
                }
                break;

            case JOY2_BUTTON:
                if (waitingForServe && servingPlayer == 2) {
                    waitingForServe = false;
                    float speed = 2.3f;
                    ballVelX = -sqrt(speed * speed - desiredBallVelY2 * desiredBallVelY2);
                    ballVelY = desiredBallVelY2;
                    if (ballVelX > -0.5f) {
                        ballVelX = -0.5f;
                        ballVelY = (desiredBallVelY2 >= 0) ? sqrt(speed * speed - ballVelX * ballVelX)
                                                          : -sqrt(speed * speed - ballVelX * ballVelX);
                    }
                    ball.setVisible(true);
                    ball.invalidate();
                    hideAimLines();
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
                BuzzerMusic_WallBounce();
            } else if (ballY + ball.getHeight() > 240) {
                ballY = 240 - ball.getHeight();
                ballVelY = -ballVelY;
                BuzzerMusic_WallBounce();
            }

            // Xử lý va chạm với paddle1 (bên trái)
            if (ballX <= paddle1.getX() + paddle1.getWidth() &&
                ballX >= paddle1.getX() &&
                ballY + ball.getHeight() >= paddle1.getY() &&
                ballY <= paddle1.getY() + paddle1.getHeight()) {
                applyPaddleBounce(ballX, ballY, ballVelX, ballVelY, ball.getWidth(), ball.getHeight(), paddle1, true);
                BuzzerMusic_PaddleBounce();
            }

            // Xử lý va chạm với paddle2 (bên phải)
            if (ballX + ball.getWidth() >= paddle2.getX() &&
                ballX + ball.getWidth() <= paddle2.getX() + paddle2.getWidth() &&
                ballY + ball.getHeight() >= paddle2.getY() &&
                ballY <= paddle2.getY() + paddle2.getHeight()) {
                applyPaddleBounce(ballX, ballY, ballVelX, ballVelY, ball.getWidth(), ball.getHeight(), paddle2, false);
                BuzzerMusic_PaddleBounce();
            }

            // Xử lý va chạm với các image
                   bool imageCollision = false;
                   int newServingPlayer = 0;
                   // Kiểm tra va chạm với image1
                   if (ballX + ball.getWidth() >= image1.getX() &&
                       ballX <= image1.getX() + image1.getWidth() &&
                       ballY + ball.getHeight() >= image1.getY() &&
                       ballY <= image1.getY() + image1.getHeight()) {
                       imageCollision = true;
                       newServingPlayer = ballVelX > 0 ? 2 : 1; // Nếu bóng đi sang phải, người chơi 1 phát; ngược lại, người chơi 2
                   }
                   // Kiểm tra va chạm với image1_1
                   if (ballX + ball.getWidth() >= image1_1.getX() &&
                       ballX <= image1_1.getX() + image1_1.getWidth() &&
                       ballY + ball.getHeight() >= image1_1.getY() &&
                       ballY <= image1_1.getY() + image1_1.getHeight()) {
                       imageCollision = true;
                       newServingPlayer = ballVelX > 0 ? 2 : 1;
                   }
                   // Kiểm tra va chạm với image2
                   if (ballX + ball.getWidth() >= image2.getX() &&
                       ballX <= image2.getX() + image2.getWidth() &&
                       ballY + ball.getHeight() >= image2.getY() &&
                       ballY <= image2.getY() + image2.getHeight()) {
                       imageCollision = true;
                       newServingPlayer = ballVelX > 0 ? 2 : 1;
                   }
                   // Kiểm tra va chạm với image2_2
                   if (ballX + ball.getWidth() >= image2_2.getX() &&
                       ballX <= image2_2.getX() + image2_2.getWidth() &&
                       ballY + ball.getHeight() >= image2_2.getY() &&
                       ballY <= image2_2.getY() + image2_2.getHeight()) {
                       imageCollision = true;
                       newServingPlayer = ballVelX > 0 ? 2 : 1;
                   }

                   if (imageCollision) {
                       // Cộng điểm cho người chơi còn lại
                       if (newServingPlayer == 2) {
                           score2++;
                           Unicode::snprintf(HardScreenViewBase::score2Buffer, HardScreenViewBase::SCORE2_SIZE, "%d", score2);
                           HardScreenViewBase::score2.invalidate();
                           BuzzerMusic_Accent();
                           if (score2 >= 5) {
                               gameOver = true;
                               presenter->goToEndScreen(2);
                           }
                       } else {
                           score1++;
                           Unicode::snprintf(HardScreenViewBase::score1Buffer, HardScreenViewBase::SCORE1_SIZE, "%d", score1);
                           HardScreenViewBase::score1.invalidate();
                           BuzzerMusic_Accent();
                           if (score1 >= 5) {
                               gameOver = true;
                               presenter->goToEndScreen(1);
                           }
                       }
                       // Thiết lập lại trạng thái phát bóng
                       ball.setVisible(false);
                       ball.invalidate();
                       waitingForServe = true;
                       serveDelayTicks = 60; // 1 giây với 60 FPS
                       servingPlayer = newServingPlayer;
                       if (newServingPlayer == 1) {
                           ballX = paddle1.getX() + paddle1.getWidth() + 10; // Gần paddle1
                           ballY = paddle1.getY() + paddle1.getHeight()/2 - ball.getHeight()/2; // Căn giữa paddle
                           desiredBallVelY1 = 0.0f;
							lineAngle1 = 0.0f;
							// Cập nhật đường dẫn với tâm bóng
                        showAimLineForPlayer(1);
                       } else {
                           ballX = paddle2.getX() - ball.getWidth() - 10; // Gần paddle2
                           ballY = paddle2.getY() + paddle2.getHeight()/2 - ball.getHeight()/2; // Căn giữa paddle
                           desiredBallVelY2 = 0.0f;
							lineAngle2 = 0.0f;
							// Cập nhật đường dẫn với tâm bóng
                        showAimLineForPlayer(2);
                       }
                       moveBallTo(ballX, ballY);
                       ball.invalidate();
                       return; // Thoát để không xử lý thêm
                   }

            // Xử lý va chạm với bên trái (ballX < 1)
                if (ballX >319) {
                    if (ballY > 60 && ballY < 181) {
                        // Bóng trong vùng goal2, cộng điểm cho người chơi 1
                        score1++;
                        Unicode::snprintf(HardScreenViewBase::score1Buffer, HardScreenViewBase::SCORE1_SIZE, "%d", score1);
                        HardScreenViewBase::score1.invalidate();

                        BuzzerMusic_Accent();
                        if (score1 >= 5) {
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
                        showAimLineForPlayer(1);
                       moveBallTo(ballX, ballY);
                        ball.invalidate();
                        return; // Thoát để không xử lý thêm
                    } else {
                        // Bóng ngoài vùng goal, phản xạ
                        ballX = 319;
                        ballVelX = -ballVelX;
                        BuzzerMusic_WallBounce();
                    }
                }

                // Xử lý va chạm với bên phải (ballX > 319)
                if (ballX <1) {
                    if (ballY > 60 && ballY < 181) {
                        // Bóng trong vùng goal1, cộng điểm cho người chơi 2
                        score2++;
                        Unicode::snprintf(HardScreenViewBase::score2Buffer, HardScreenViewBase::SCORE2_SIZE, "%d", score2);
                        HardScreenViewBase::score2.invalidate();
                        BuzzerMusic_Accent();
                        if (score2 >= 5) {
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
                        showAimLineForPlayer(2);
                       moveBallTo(ballX, ballY);
                        ball.invalidate();
                        return; // Thoát để không xử lý thêm
                    } else {
                        // Bóng ngoài vùng goal, phản xạ
                        ballX = 1;
                        ballVelX = -ballVelX;
                        BuzzerMusic_WallBounce();
                    }
                }


            // Di chuyển bóng
            moveBallTo(ballX, ballY);
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
                showAimLineForPlayer(1);
            } else if (servingPlayer == 2) {
                ballY = paddle2.getY() + paddle2.getHeight()/2 - ball.getHeight()/2;
                showAimLineForPlayer(2);
            }
            moveBallTo(ballX, ballY);
            ball.invalidate();
        }
}





