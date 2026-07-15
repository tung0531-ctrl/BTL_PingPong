#ifndef MODEL_HPP
#define MODEL_HPP

//#include <mvp/Model.hpp>
#include <touchgfx/Unicode.hpp> // Include for Unicode::UnicodeChar
#include <cstdint>

class ModelListener;


class Model
{
    
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
    // Gọi từ joystick_task.c
//    static void movePaddle1Left();
//    static void movePaddle1Right();
//
//    static void movePaddle2Left();
//    static void movePaddle2Right();
//    static void paddle1Action();
//    static void paddle2Action();
    void setWinner(int winnerId);
	int getWinner(int playerId);
	void resetGame();
	ModelListener* modelListener;
    
protected:
	static ModelListener* staticListener;
    int winner; // Thay từ char winner[] thành int
	int player1Score;
	int player2Score;
	bool gameOver;
};



#endif // MODEL_HPP
