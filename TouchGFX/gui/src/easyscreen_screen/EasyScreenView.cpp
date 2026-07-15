


#include <gui/easyscreen_screen/EasyScreenView.hpp>
#include "main.h"
#include "cmsis_os.h"
#include "joystick_task.h"
#include <cmath>

// Handle hàng đợi tin nhắn FreeRTOS cho các lệnh từ joystick
extern osMessageQueueId_t joystickQueueHandle;

// Hàm khởi tạo của lớp EasyScreenView
EasyScreenView::EasyScreenView() : ballX(160), ballY(80), ballVelX(1.4f), ballVelY(1.4f),
                                   waitingForServe(false), serveDelayTicks(0), servingPlayer(0),
                                   score1(0), score2(0), gameOver(false),
                                   buzzerBeepCounter(0), buzzerBeepState(false),
                                   desiredBallVelY1(0.0f), desiredBallVelY2(0.0f),
                                   lineAngle1(0.0f), lineAngle2(0.0f)
{
}

// Phương thức thiết lập màn hình khi bắt đầu
void EasyScreenView::setupScreen()
{
    EasyScreenViewBase::setupScreen();
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

        // Khởi tạo hiển thị điểm số
        Unicode::snprintf(EasyScreenViewBase::score1Buffer, EasyScreenViewBase::SCORE1_SIZE, "%d", score1);
        EasyScreenViewBase::score1.invalidate();
        Unicode::snprintf(EasyScreenViewBase::score2Buffer, EasyScreenViewBase::SCORE2_SIZE, "%d", score2);
        EasyScreenViewBase::score2.invalidate();

}

// Phương thức dọn dẹp khi thoát màn hình
void EasyScreenView::tearDownScreen()
{
    EasyScreenViewBase::tearDownScreen();
}

// Phương thức xử lý sự kiện tick (gọi định kỳ, thường 60 FPS)
void EasyScreenView::handleTickEvent()
{
    if (gameOver) {
        line1.setVisible(false);
        line1_1.setVisible(false);
        line1.invalidate();
        line1_1.invalidate();
        return;
    }

    // Xử lý âm thanh buzzer
    if (buzzerBeepCounter > 0) {
        if (buzzerBeepCounter % 6 == 0) {
            buzzerBeepState = !buzzerBeepState;
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, buzzerBeepState ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        buzzerBeepCounter--;
        if (buzzerBeepCounter == 0) {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
        }
    }

    // Xử lý các lệnh joystick từ hàng đợi
    JoystickCommand_t command;
    while (osMessageQueueGet(joystickQueueHandle, &command, NULL, 0) == osOK) {
        switch (command) {
            case JOY1_LEFT:
            {
                int16_t newY = paddle1.getY() + 2;
                if (newY > 198) newY = 198;
                if (newY != paddle1.getY()) {
                    paddle1.invalidate();
                    paddle1.moveTo(paddle1.getX(), newY);
                    paddle1.invalidate();
                }
            }
            break;

            case JOY1_RIGHT:
            {
                int16_t newY = paddle1.getY() - 2;
                if (newY < 2) newY = 2;
                if (newY != paddle1.getY()) {
                    paddle1.invalidate();
                    paddle1.moveTo(paddle1.getX(), newY);
                    paddle1.invalidate();
                }
            }
            break;

            case JOY2_LEFT:
            {
                int16_t newY = paddle2.getY() + 2;
                if (newY > 198) newY = 198;
                if (newY != paddle2.getY()) {
                    paddle2.invalidate();
                    paddle2.moveTo(paddle2.getX(), newY);
                    paddle2.invalidate();
                }
            }
            break;

            case JOY2_RIGHT:
            {
                int16_t newY = paddle2.getY() - 2;
                if (newY < 2) newY = 2;
                if (newY != paddle2.getY()) {
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
                    lineAngle1 = atan2f(desiredBallVelY1, 2.0f) * 180.0f / M_PI;//tính góc phát bóng thoe độ từ giá trị radian của hàm  atan2f
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
                    lineAngle1 = atan2f(desiredBallVelY1, 2.0f) * 180.0f / M_PI;//đổi từ radian sang độ
                    // Cập nhật đường dẫn với tâm bóng
                    line1.invalidate();
                                        float ballCenterX = ballX + ball.getWidth() / 2.0f;
                                        float ballCenterY = ballY + ball.getHeight() / 2.0f;
                                        line1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);//khung hiển thị hướng bóng
                                        line1.setStart(16, 16);
                                        line1.setEnd(16 + 30 * cosf(lineAngle1 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle1 * M_PI / 180.0f));//
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
                    lineAngle2 = atan2f(desiredBallVelY2, -2.0f) * 180.0f / M_PI;//tính góc phát bóng theo độ, hàm atan2f trả về góc theo radian
                    // Cập nhật đường dẫn với tâm bóng
                    line1_1.invalidate();
                                        float ballCenterX = ballX + ball.getWidth() / 2.0f;
                                        float ballCenterY = ballY + ball.getHeight() / 2.0f;
                                        line1_1.setPosition(ballCenterX - 16, ballCenterY - 16, 33, 33);//khung hiển thị hướng bóng
                                        line1_1.setStart(16, 16);//tọa độ điểm bắt đầu
                                        line1_1.setEnd(16 + 30 * cosf(lineAngle2 * M_PI / 180.0f), 16 + 30 * sinf(lineAngle2 * M_PI / 180.0f));//tính tọa độ điểm kết thúc từ cos, sin của góc
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

    // Cập nhật vị trí bóng nếu không chờ phát
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

        // Xử lý va chạm với paddle1
        if (ballX <= paddle1.getX() + paddle1.getWidth() &&
            ballX >= paddle1.getX() &&
            ballY + ball.getHeight() >= paddle1.getY() &&
            ballY <= paddle1.getY() + paddle1.getHeight()) {
            ballX = paddle1.getX() + paddle1.getWidth();
            ballVelX = -ballVelX;
        }

        // Xử lý va chạm với paddle2
        if (ballX + ball.getWidth() >= paddle2.getX() &&
            ballX + ball.getWidth() <= paddle2.getX() + paddle2.getWidth() &&
            ballY + ball.getHeight() >= paddle2.getY() &&
            ballY <= paddle2.getY() + paddle2.getHeight()) {
            ballX = paddle2.getX() - ball.getWidth();
            ballVelX = -ballVelX;
        }

        // Xử lý ghi điểm
        if (ballX < 0) {
            score2++;
            Unicode::snprintf(EasyScreenViewBase::score2Buffer, EasyScreenViewBase::SCORE2_SIZE, "%d", score2);
            EasyScreenViewBase::score2.invalidate();
            buzzerBeepCounter = 18;
            buzzerBeepState = true;
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);

            if (score2 >= 11) {
                gameOver = true;
                presenter->goToEndScreen(2);
            }

            ball.setVisible(false);
            ball.invalidate();
            waitingForServe = true;
            serveDelayTicks = 60;
            servingPlayer = 2;
            ballX = paddle2.getX() - ball.getWidth() - 10;
            ballY = paddle2.getY() + paddle2.getHeight()/2 - ball.getHeight()/2;
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
        } else if (ballX + ball.getWidth() > 320) {
            score1++;
            Unicode::snprintf(EasyScreenViewBase::score1Buffer, EasyScreenViewBase::SCORE1_SIZE, "%d", score1);
            EasyScreenViewBase::score1.invalidate();
            buzzerBeepCounter = 18;
            buzzerBeepState = true;
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);

            if (score1 >= 11) {
                gameOver = true;
                presenter->goToEndScreen(1);
            }

            ball.setVisible(false);
            ball.invalidate();
            waitingForServe = true;
            serveDelayTicks = 60;
            servingPlayer = 1;
            ballX = paddle1.getX() + paddle1.getWidth() + 10;
            ballY = paddle1.getY() + paddle1.getHeight()/2 - ball.getHeight()/2;
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
        }

        ball.moveTo(ballX, ballY);
        ball.invalidate();
    } else {
        // Xử lý khi chờ phát bóng
        if (serveDelayTicks > 0) {
            serveDelayTicks--;
            if (serveDelayTicks == 0) {
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
                       float ballCenterX = ballX + ball.getWidth() / 2.0f;//xác định tọa độ tâm bóng
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
