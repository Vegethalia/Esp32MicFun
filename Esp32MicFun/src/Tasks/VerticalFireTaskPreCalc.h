

/// @brief This task processes in background the vertical scrolling of the Vertical fire effect.
/// @param pvParameters  Pointer to CRGB* (double-pointer) so the task always
///                      sees the current allocation after alloc/free cycles.
void vTaskVertFire(void *pvParameters) {
  CRGB **ppAuxLeds = (CRGB **)pvParameters;

  while (true) {
    EventBits_t bits = xEventGroupWaitBits(
        _xEventVertFireNewLine,  // Grup d'esdeveniments
        BIT_0,                   // Espera els bits 0
        pdTRUE,                  // Esborra els bits després de llegir
        pdTRUE,                  // Espera tots els bits
        portMAX_DELAY);

    if (bits & BIT_0) {
      CRGB *pLeds = *ppAuxLeds;
      if (pLeds == nullptr) continue;
      int ledDest = 0;
      uint8_t baseFade = 5;  //_pianoMode ? 1 : 5; // 5
#if defined(PANEL_SIZE_96x54)
      baseFade = 4;  // 2
#endif

      // shift all columns to the top
      for (int iLine = 0; iLine < THE_PANEL_HEIGHT - 1; iLine++) {
        for (int x = 0; x < THE_PANEL_WIDTH; x++) {
          ledDest = _TheMapping.XY(x, iLine);

          pLeds[ledDest] = pLeds[_TheMapping.XY(x, iLine + 1)];
          pLeds[ledDest].subtractFromRGB(baseFade);
        }
      }
    }
  }
}
