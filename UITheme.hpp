#ifndef UITHEME_HPP
#define UITHEME_HPP

#include <SFML/Graphics.hpp>

namespace UI {
    // New Design Palette from Stitch
    const sf::Color MatteBlack(15, 15, 15);
    const sf::Color DeepCharcoal(26, 26, 29);
    const sf::Color CyberTeal(0, 173, 181);
    const sf::Color NeonPink(255, 46, 99);
    const sf::Color DarkSlate(57, 62, 70);
    const sf::Color MintGreen(78, 204, 163);
    const sf::Color MatrixGreen(0, 255, 65);
    const sf::Color PureWhite(238, 238, 238);
    const sf::Color MutedText(150, 150, 160);

    // Legacy Aliases for Compatibility
    const sf::Color NeonGreen = MintGreen;
    const sf::Color NeonMagenta = NeonPink;
    const sf::Color DeepNavy = MatteBlack;
    const sf::Color PanelBg(37, 37, 41, 200); // Cards color from design
    const sf::Color GlassBorder(57, 62, 70, 100); // DarkSlate border

    // Helpers
    inline sf::RectangleShape createPanel(sf::Vector2f size, sf::Vector2f pos) {
        sf::RectangleShape rect(size);
        rect.setPosition(pos);
        rect.setFillColor(PanelBg);
        rect.setOutlineThickness(1.0f);
        rect.setOutlineColor(GlassBorder);
        return rect;
    }

    inline void centerText(sf::Text& text, sf::Vector2f pos) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        text.setPosition(pos);
    }

    inline float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

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
