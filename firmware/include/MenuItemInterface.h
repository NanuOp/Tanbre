#ifndef __MENU_ITEM_INTERFACE_H__
#define __MENU_ITEM_INTERFACE_H__

#include "core/display.h"
#include <globals.h>

class MenuItemInterface {
public:
    virtual ~MenuItemInterface() = default;
    virtual void optionsMenu(void) = 0;
    virtual void drawIcon(float scale = 1) = 0;
    virtual void drawIconImg() {
        drawImg(
            *bruceConfig.themeFS(),
            bruceConfig.getThemeItemImg(themePath()),
            0,
            imgCenterY,
            true,
            bruceConfig.theme.gifDuration,
            false
        );
    }
    virtual bool hasTheme() = 0;
    virtual String themePath() = 0;

    bool checkTheme() { return hasTheme() && themePath() != ""; }
    String getName() const { return _name; }

    void draw(float scale = 1) {
        if (rotation != bruceConfigPins.rotation) resetCoordinates();
        if (!checkTheme()) {
            tft.fillRect(0, 27, tftWidth, tftHeight - 27, bruceConfig.bgColor);
#if !defined(TANBRE_WATCH)
            drawIcon(scale);
#endif
            drawArrows(scale);
            drawTitle(scale);
        } else {
            if (bruceConfig.theme.label)
                drawTitle(scale); // If using .GIF, labels are draw after complete, which takes some time
            drawIconImg();
            if (bruceConfig.theme.label) drawTitle(scale); // Makes sure to draw over the image
        }
        drawStatusBar();
    }

    void drawArrows(float scale = 1) {
#if defined(TANBRE_WATCH)
        // Clear arrow areas on left and right
        tft.fillRect(0, 27, 20, tftHeight - 27, bruceConfig.bgColor);
        tft.fillRect(tftWidth - 20, 27, 20, tftHeight - 27, bruceConfig.bgColor);

        // Center vertically at Y = 45.
        int arrowY = 45;
        int arrowXLeft = 6;
        int arrowXRight = tftWidth - 6;

        // Draw Left Arrow (<)
        tft.drawWideLine(arrowXLeft, arrowY, arrowXLeft + 5, arrowY - 5, 2, bruceConfig.priColor, bruceConfig.bgColor);
        tft.drawWideLine(arrowXLeft, arrowY, arrowXLeft + 5, arrowY + 5, 2, bruceConfig.priColor, bruceConfig.bgColor);

        // Draw Right Arrow (>)
        tft.drawWideLine(arrowXRight, arrowY, arrowXRight - 5, arrowY - 5, 2, bruceConfig.priColor, bruceConfig.bgColor);
        tft.drawWideLine(arrowXRight, arrowY, arrowXRight - 5, arrowY + 5, 2, bruceConfig.priColor, bruceConfig.bgColor);
#else
        tft.fillRect(arrowAreaX, iconAreaY, arrowAreaW, iconAreaH, bruceConfig.bgColor);
        tft.fillRect(
            tftWidth - arrowAreaX - arrowAreaW, iconAreaY, arrowAreaW, iconAreaH, bruceConfig.bgColor
        );

        int arrowSize = scale * 10;
        int lineWidth = scale * 3;

        int arrowX = BORDER_PAD_X + 1.5 * arrowSize;
        int arrowY = iconCenterY + 1.5 * arrowSize;

        // Left Arrow
        tft.drawWideLine(
            arrowX,
            arrowY,
            arrowX + arrowSize,
            arrowY + arrowSize,
            lineWidth,
            bruceConfig.priColor,
            bruceConfig.bgColor
        );
        tft.drawWideLine(
            arrowX,
            arrowY,
            arrowX + arrowSize,
            arrowY - arrowSize,
            lineWidth,
            bruceConfig.priColor,
            bruceConfig.bgColor
        );

        // Right Arrow
        tft.drawWideLine(
            tftWidth - arrowX,
            arrowY,
            tftWidth - arrowX - arrowSize,
            arrowY + arrowSize,
            lineWidth,
            bruceConfig.priColor,
            bruceConfig.bgColor
        );
        tft.drawWideLine(
            tftWidth - arrowX,
            arrowY,
            tftWidth - arrowX - arrowSize,
            arrowY - arrowSize,
            lineWidth,
            bruceConfig.priColor,
            bruceConfig.bgColor
        );
#endif
    }

    void drawTitle(float scale = 1) {
#if defined(TANBRE_WATCH)
        // Center text vertically in the client area (Y = 27 to 64)
        // Center Y is 45. Using text size FM (8px tall), Y starts at 45 - 4 = 41.
        int titleY = 41;
        tft.setTextSize(FM);
        tft.drawPixel(0, 0, 0);
        // Clear text area (X = 20 to tftWidth - 20)
        tft.fillRect(20, 27, tftWidth - 40, tftHeight - 27, bruceConfig.bgColor);
        tft.drawCentreString(getName(), tftWidth / 2, titleY, 1);
#else
        int titleY = iconCenterY + iconAreaH / 2 + FG;

        tft.setTextSize(FM);
        tft.drawPixel(0, 0, 0);
        tft.fillRect(arrowAreaX, titleY, tftWidth - 2 * arrowAreaX, LH * FM, bruceConfig.bgColor);
        int nchars = (tftWidth - 16) / (LW * FM);
        tft.drawCentreString(getName().substring(0, nchars), iconCenterX, titleY, 1);
#endif
    }

protected:
    String _name = "";
    uint8_t rotation = 255;

    int iconAreaH =
        ((tftHeight - 2 * BORDER_PAD_Y) % 2 == 0 ? tftHeight - 2 * BORDER_PAD_Y
                                                 : tftHeight - 2 * BORDER_PAD_Y + 1);
    int iconAreaW = iconAreaH;

    int iconCenterX = tftWidth / 2;
    int iconCenterY = tftHeight / 2;
    int imgCenterY = 13;

    int iconAreaX = iconCenterX - iconAreaW / 2;
    int iconAreaY = iconCenterY - iconAreaH / 2;

    int arrowAreaX = BORDER_PAD_X;
    int arrowAreaW = iconAreaX - arrowAreaX;

    MenuItemInterface(const String &name) : _name(name) {}

    void clearIconArea(void) {
        tft.fillRect(iconAreaX, iconAreaY, iconAreaW, iconAreaH, bruceConfig.bgColor);
    }
    void clearImgArea(void) { tft.fillRect(7, 27, tftWidth - 14, tftHeight - 34, bruceConfig.bgColor); }
    void resetCoordinates(void) {
        // Recalculate Center and ared due to portrait/landscape changings
        if (tftWidth > tftHeight) {
            iconAreaH =
                ((tftHeight - 2 * BORDER_PAD_Y) % 2 == 0 ? tftHeight - 2 * BORDER_PAD_Y
                                                         : tftHeight - 2 * BORDER_PAD_Y + 1);
        } else {
            iconAreaH =
                ((tftWidth - 2 * BORDER_PAD_Y) % 2 == 0 ? tftWidth - 2 * BORDER_PAD_Y
                                                        : tftWidth - 2 * BORDER_PAD_Y + 1);
        }

        iconAreaW = iconAreaH;

        iconCenterX = tftWidth / 2;
        iconCenterY = tftHeight / 2;

        iconAreaX = iconCenterX - iconAreaW / 2;
        iconAreaY = iconCenterY - iconAreaH / 2;

        arrowAreaX = BORDER_PAD_X;
        arrowAreaW = iconAreaX - arrowAreaX;

        rotation = bruceConfigPins.rotation;
    }

private:
};

#endif // __MENU_ITEM_INTERFACE_H__
