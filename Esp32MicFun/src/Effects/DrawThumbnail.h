
// Decode one RGB565 pixel from the raw buffer to CRGB (90% brightness scale)
static inline CRGB ThumbPixel(uint16_t pixIdx) {
  const uint16_t p = ((uint16_t)_ThumbnailRaw[pixIdx * 2] << 8) | _ThumbnailRaw[pixIdx * 2 + 1];
  return CRGB(
    (uint8_t)((((p >> 11) & 0x1F) * 9u * 8u) / 10u),   // R 5-bit → 8-bit × 0.9
    (uint8_t)((((p >>  5) & 0x3F) * 9u * 4u) / 10u),   // G 6-bit → 8-bit × 0.9
    (uint8_t)(( (p        & 0x1F) * 9u * 8u) / 10u)    // B 5-bit → 8-bit × 0.9
  );
}

void DrawThumbnail() {
  if (!_ThumbnailReady || !_ThumbnailRaw) {
    ChangeDrawStyle(_ThumbnailPrevStyle, true);
    log_w("Thumbnail not ready yet. Returning to previous style [%d]", (int)_ThumbnailPrevStyle);
    return;
  }
  if (millis() - _TimeThumbnailReceived > 20000) {
    FastLED.setBrightness(_ThumbnailPrevIntensity);
    ChangeDrawStyle(_ThumbnailPrevStyle, true);
    log_w("Thumbnail expired. Returning to previous style [%d]", (int)_ThumbnailPrevStyle);
    return;
  }

  // Scroll state — reset to top whenever a new thumbnail arrives
  static int16_t  scrollRow     = 0;
  static int8_t   scrollDir     = 1;
  static uint32_t lastThumbTime = 0;
  static uint8_t  frameCount    = 0;
  if (_TimeThumbnailReceived != lastThumbTime) {
    lastThumbTime = _TimeThumbnailReceived;
    scrollRow  = 0;
    scrollDir  = 1;
    frameCount = 0;
  }

  FastLED.setBrightness(THUMBNAIL_MILLIS);

  const uint8_t imgW      = THUMBNAIL_WIDTH;
  const uint8_t panW      = THE_PANEL_WIDTH;
  const uint8_t panH      = THE_PANEL_HEIGHT;
  const uint8_t startX    = (uint8_t)((panW - imgW) / 2);
  const int16_t maxScroll = (int16_t)THUMBNAIL_HEIGHT - (int16_t)panH;

  // Draw visible window [scrollRow .. scrollRow+panH-1], decode RGB565 on the fly
  for (uint8_t j = 0; j < panH; j++) {
    const uint16_t srcRow  = (uint16_t)(scrollRow + j);
    const uint16_t rowBase = (uint16_t)(j * panW) + startX;
    const uint16_t imgOff  = srcRow * imgW;
    for (uint8_t i = 0; i < imgW; i++) {
      _TheLeds[LedIndexFlat(rowBase + i)] = ThumbPixel(imgOff + i);
    }
  }

  // Advance scroll by 1 line every 2 frames and bounce
  if (++frameCount >= 2) {
    frameCount = 0;
    scrollRow += scrollDir;
    if (scrollRow >= maxScroll) {
      scrollRow = maxScroll;
      scrollDir = -1;
    } else if (scrollRow <= 0) {
      scrollRow = 0;
      scrollDir = 1;
    }
  }
}
