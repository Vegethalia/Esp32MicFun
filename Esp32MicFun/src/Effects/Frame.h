
/// @brief Dibuixa un marc al voltant del panell.
/// @param hue El color del marc
/// @param intensity La intensitat dels píxels
void DrawFrame(uint8_t hue, uint8_t intensity)
{
    CHSV frame = CHSV(hue, 255, intensity);
    CHSV frameLight = CHSV(hue, 128, intensity);
    const uint16_t topRowBase = 0;
    const uint16_t bottomRowBase = (THE_PANEL_HEIGHT - 1) * THE_PANEL_WIDTH;

    for (int i = 0; i < THE_PANEL_WIDTH; i++) {
        CHSV color = i % 8 == 0 ? frameLight : frame;
        _TheLeds[LedIndexFlat(topRowBase + i)] = color;
        _TheLeds[LedIndexFlat(bottomRowBase + i)] = color;
    }
    for (int i = 0; i < THE_PANEL_HEIGHT; i++) {
        CHSV color = i % 8 == 0 ? frameLight : frame;
        uint16_t rowBase = i * THE_PANEL_WIDTH;
        _TheLeds[LedIndexFlat(rowBase)] = color;
        _TheLeds[LedIndexFlat(rowBase + (THE_PANEL_WIDTH - 1))] = color;
    }
}
