
/// @brief Dibuixa un marc al voltant del panell.
/// @param hue El color del marc
/// @param intensity La intensitat dels píxels
void DrawFrame(uint8_t hue, uint8_t intensity)
{
    CHSV frame = CHSV(hue, 255, intensity);
    CHSV frameLight = CHSV(hue, 128, intensity);

    for (int i = 0; i < THE_PANEL_WIDTH; i++) {
        CHSV color = i % 8 == 0 ? frameLight : frame;
        _TheLeds[_TheMapping.XY(i, 0)] = color;
        _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1)] = color;
    }
    for (int i = 0; i < THE_PANEL_HEIGHT; i++) {
        CHSV color = i % 8 == 0 ? frameLight : frame;
        _TheLeds[_TheMapping.XY(0, i)] = color;
        _TheLeds[_TheMapping.XY(THE_PANEL_WIDTH - 1, i)] = color;
    }
}
