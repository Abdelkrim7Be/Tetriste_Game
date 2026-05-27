#ifndef UITHEME_HPP
#define UITHEME_HPP

#include <SFML/Graphics.hpp>

namespace UI {
    // Cyberpunk Palette
    const sf::Color NeonGreen(57, 255, 139);
    const sf::Color NeonMagenta(255, 0, 255);
    const sf::Color DeepNavy(10, 20, 40);
    const sf::Color PanelBg(20, 30, 50, 200);
    const sf::Color GlassBorder(57, 255, 139, 100);
    const sf::Color MutedText(150, 150, 160);

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
            (sf::Uint8)lerp(a.a, b.b, t)
        );
    }
}

#endif
