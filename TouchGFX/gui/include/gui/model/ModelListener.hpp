#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>

class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}
//    virtual void movePaddle1Left() {}
//    virtual void movePaddle1Right() {}
//    virtual void movePaddle2Left() {}
//    virtual void movePaddle2Right() {}
//    virtual void paddle1Action() {}
//    virtual void paddle2Action() {}
    void bind(Model* m)
    {
        model = m;
    }
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
