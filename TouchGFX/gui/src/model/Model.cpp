#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#include <cstring>
#include <cstdio>

ModelListener* Model::staticListener = nullptr;
Model::Model() : modelListener(0), winner(0), player1Score(0), player2Score(0), gameOver(false)
{
}

void Model::tick()
{
    // Tick xử lý định kỳ nếu cần
    staticListener = modelListener; // gán mỗi lần để sync
}

//void Model::movePaddle1Left() {
//    if (staticListener) staticListener->movePaddle1Left();
//}
//
//void Model::movePaddle1Right() {
//    if (staticListener) staticListener->movePaddle1Right();
//}
//
//
//void Model::movePaddle2Left() {
//    if (staticListener) staticListener->movePaddle2Left();
//}
//
//void Model::movePaddle2Right() {
//    if (staticListener) staticListener->movePaddle2Right();
//}
//
//void Model::paddle1Action() {
//    if (staticListener) staticListener->paddle1Action();
//}
//
//void Model::paddle2Action() {
//    if (staticListener) staticListener->paddle2Action();
//}
void Model::setWinner(int winnerId)
{
    winner = winnerId;
    printf("Model::setWinner: %d\n", winner);
}

int Model::getWinner(int playerId)
{
    int result = (winner == playerId) ? 1 : 0;
    printf("Model::getWinner: playerId=%d, result=%d\n", playerId, result);
    return result;
}

void Model::resetGame()
{

    player1Score = 0;
    player2Score = 0;
    gameOver = false;
    winner = 0; // Đặt lại winner về 0
}
