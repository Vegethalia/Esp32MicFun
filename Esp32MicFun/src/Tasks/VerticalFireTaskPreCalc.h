

/// @brief Task de fons que fa el scroll i el fade del buffer de foc (uint8_t, row-major).
///        Rep un uint8_t** per tal que sempre llegeixi l'adreça actual del buffer
///        fins i tot després de cicles alloc/free.
void vTaskVertFire(void *pvParameters) {
  uint8_t **ppFireBuffer = (uint8_t **)pvParameters;

  while (true) {
    EventBits_t bits = xEventGroupWaitBits(
        _xEventVertFireNewLine,
        BIT_0,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);

    if (bits & BIT_0) {
      uint8_t *pFire = *ppFireBuffer;
      if (pFire == nullptr) continue;

      // Doom-style upward diffusion: each pixel gets the weighted avg of the 3 pixels
      // directly below it minus a small random decay. This naturally propagates the
      // audio energy (bottom row) upward with lateral spread, producing organic flames.
      // No memmove needed — the top-to-bottom loop writes row y from row y+1 (unmodified).
      for (int y = 0; y < THE_PANEL_HEIGHT - 1; y++) {
        const uint8_t *rowBelow = pFire + (y + 1) * THE_PANEL_WIDTH;
        uint8_t       *rowCur   = pFire +  y      * THE_PANEL_WIDTH;
        for (int x = 0; x < THE_PANEL_WIDTH; x++) {
          const int xl = x > 0                    ? x - 1 : 0;
          const int xr = x < THE_PANEL_WIDTH - 1  ? x + 1 : THE_PANEL_WIDTH - 1;
          const uint16_t avg = ((uint16_t)rowBelow[xl] +
                                (uint16_t)rowBelow[x] * 2 +
                                (uint16_t)rowBelow[xr]) >> 2;
          const uint8_t decay = random8(1, 5); // 1..4
          rowCur[x] = avg > decay ? (uint8_t)(avg - decay) : 0;
        }
      }
    }
  }
}
