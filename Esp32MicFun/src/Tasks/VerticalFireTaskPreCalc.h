

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

      constexpr uint8_t baseFade = 4;

      // Desplacem totes les files cap amunt (files 1..H-1 → files 0..H-2)
      memmove(pFire, pFire + THE_PANEL_WIDTH, (THE_PANEL_HEIGHT - 1) * THE_PANEL_WIDTH);

      // Apliquem el fade a les files desplaçades (la fila H-1 la sobreescriurà PushLineWithPrecalcFire)
      const int numPixels = (THE_PANEL_HEIGHT - 1) * THE_PANEL_WIDTH;
      for (int i = 0; i < numPixels; i++) {
        pFire[i] = pFire[i] > baseFade ? pFire[i] - baseFade : 0;
      }
    }
  }
}
