#ifndef EASYSCREENPRESENTER_HPP
#define EASYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class EasyScreenView;

class EasyScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    EasyScreenPresenter(EasyScreenView& v);

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
    void handleTickEvent();
    void goToEndScreen(int winner);
    void resetGame();
    virtual ~EasyScreenPresenter() {}

private:
    EasyScreenPresenter();

    EasyScreenView& view;
};

#endif // EASYSCREENPRESENTER_HPP
