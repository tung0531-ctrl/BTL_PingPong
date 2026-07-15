#ifndef MEDIUMREADYSCREENPRESENTER_HPP
#define MEDIUMREADYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class MediumReadyScreenView;

class MediumReadyScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    MediumReadyScreenPresenter(MediumReadyScreenView& v);

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

    virtual ~MediumReadyScreenPresenter() {}

private:
    MediumReadyScreenPresenter();

    MediumReadyScreenView& view;
};

#endif // MEDIUMREADYSCREENPRESENTER_HPP
