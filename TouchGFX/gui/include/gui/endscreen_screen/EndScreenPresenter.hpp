#ifndef ENDSCREENPRESENTER_HPP
#define ENDSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class EndScreenView;

class EndScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    EndScreenPresenter(EndScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();
    int getWinner(int playerId);
    void handlePlayAgain();


    virtual ~EndScreenPresenter() {}

private:
    EndScreenPresenter();

    EndScreenView& view;
};

#endif // ENDSCREENPRESENTER_HPP
