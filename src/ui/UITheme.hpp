#ifndef UITHEME_HPP
#define UITHEME_HPP

#include <SFML/Graphics.hpp>
#include <cmath>

namespace UI {
    // --- Surgical Cyberpunk tokens (DESIGN.md + Stitch HTML) ---
    const sf::Color CyberBg(5, 5, 5);
    const sf::Color CyberSurface(19, 19, 19);
    const sf::Color CyberSurfaceDim(19, 19, 19);
    const sf::Color CyberSurfaceLowest(14, 14, 14);
    const sf::Color CyberSurfaceLow(28, 27, 27);
    const sf::Color CyberSurfaceContainer(32, 31, 31);
    const sf::Color CyberSurfaceContainerHigh(42, 42, 42);
    const sf::Color CyberPrimary(57, 255, 20);
    const sf::Color CyberPrimaryFixed(121, 255, 91);
    const sf::Color CyberPrimaryContainer(57, 255, 20);
    const sf::Color CyberWarning(255, 240, 0);
    const sf::Color CyberSecondaryFixed(245, 231, 0);
    const sf::Color CyberSecondaryFixedDim(215, 202, 0);
    const sf::Color CyberNeutral(0, 209, 255);
    const sf::Color CyberTertiaryFixedDim(76, 214, 255);
    const sf::Color CyberText(229, 226, 225);
    const sf::Color CyberOutline(133, 150, 124);
    const sf::Color CyberOutlineVariant(60, 75, 53);
    const sf::Color CyberOutlineActive(121, 255, 91);
    const sf::Color CyberGrid(255, 255, 255, 10);
    const sf::Color CyberOnPrimary(5, 57, 0);

    const sf::Color CyberTeal = CyberPrimaryFixed;
    const sf::Color NeonPink(255, 46, 99);
    const sf::Color MutedText(186, 204, 176);
    const sf::Color DeepNavy = CyberBg;
    const sf::Color PanelBg = CyberSurface;

    const float HeaderH = 32.0f;
    const float FooterH = 40.0f;
    const float SidebarW = 152.0f;
    /** Right tactical panel width during PLAYING (wider than legacy 228). */
    const float PlayHudW = 304.0f;
    const float MarginSm = 8.0f;
    const float MarginMd = 16.0f;
    const float ContainerPad = 12.0f;

    // Piece sprites are generated at this pixel size (see main.cpp).
    const unsigned PieceTexPx = 56;

    const float PlayLabelBand = 26.0f;
    const float PlayGridMarginX = 10.0f;
    const float PlayGridMarginY = 8.0f;
    const float PlayCellInset = 5.0f;

    inline float uiScaleFrom(float winW, float winH) {
        float s = std::min(winW / 1280.0f, winH / 720.0f);
        return std::max(0.9f, std::min(1.4f, s));
    }

    inline float playCellSize(float ui) { return 54.0f * ui; }
    inline float pieceSpacing(float ui) { return playCellSize(ui); }
    inline float piecePlayScale(float ui) {
        return (playCellSize(ui) - 2.0f * PlayCellInset) / (float)PieceTexPx;
    }
    inline float pieceGridScale(float ui) { return piecePlayScale(ui); }
    inline float pieceNextScale(float ui) { return 0.68f * ui; }
    inline float pieceHudMiniScale(float ui) { return 0.58f * ui; }

    inline void playGridOrigin(float playX, float playY, float &ox, float &oy) {
        ox = playX + PlayGridMarginX;
        oy = playY + PlayLabelBand + PlayGridMarginY;
    }

    enum class CodeSize { XS, SM, MD, LG, XL, BrandMenu, BrandAuth };

    inline unsigned codeSizePx(CodeSize s) {
        switch (s) {
            case CodeSize::XS: return 10;
            case CodeSize::SM: return 12;
            case CodeSize::MD: return 14;
            case CodeSize::LG: return 18;
            case CodeSize::XL: return 24;
            case CodeSize::BrandMenu: return 56;
            case CodeSize::BrandAuth: return 48;
            default: return 14;
        }
    }

    inline sf::RectangleShape createPanel(sf::Vector2f size, sf::Vector2f pos, bool active = false) {
        sf::RectangleShape rect(size);
        rect.setPosition(pos);
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineThickness(1.0f);
        rect.setOutlineColor(active ? CyberOutlineActive : CyberOutlineVariant);
        return rect;
    }

    inline void centerText(sf::Text& text, sf::Vector2f pos) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(std::floor(bounds.left + bounds.width / 2.0f), std::floor(bounds.top + bounds.height / 2.0f));
        text.setPosition(pos);
    }

    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

    inline sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
        return sf::Color(
            (sf::Uint8)lerp(a.r, b.r, t),
            (sf::Uint8)lerp(a.g, b.g, t),
            (sf::Uint8)lerp(a.b, b.b, t),
            (sf::Uint8)lerp(a.a, b.a, t)
        );
    }
}

#endif
