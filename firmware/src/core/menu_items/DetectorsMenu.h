#ifndef __DETECTORS_MENU_H__
#define __DETECTORS_MENU_H__

#include <MenuItemInterface.h>

class DetectorsMenu : public MenuItemInterface {
public:
    DetectorsMenu() : MenuItemInterface("Detectors") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    String themePath() { return ""; }
};

#endif
