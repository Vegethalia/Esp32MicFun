
/// @brief Dibuixa un títol al panell _ThePanel. Fa servir la instància global _u8g2 per dibuixar el text.
/// La font ja ha d'haver estat configurada abans de cridar aquesta funció.
/// Si cal netejar el buffer abans de dibuixar el text, s'ha de fer abans de cridar aquesta funció amb _u8g2.clearBuffer();
/// La mida del text no pot ser més de 8 pixels d'alt, aquesta funció només dibuixa 1 línia/tile de 8.
/// @param msg Text a mostrar
/// @param hue Color del text
/// @param intensity Intensitat dels píxels
/// @param xPos Columna on es mostrarà el text
/// @param yPos Línia on es mostrarà el text
void ShowSmallTitle(std::string msg, uint8_t hue, uint8_t intensity, int16_t xPos, int16_t yPos)
{
    _u8g2.drawStr(0, 7, msg.c_str());
    _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 0, 0, xPos, yPos, hue, intensity);
}