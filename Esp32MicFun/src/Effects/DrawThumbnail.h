

void DrawThumbnail() {
  if (!_ThumbnailReady || _ThumbnailImg.size() < (THUMBNAIL_WIDTH * THUMBNAIL_WIDTH)) {
    _TheDrawStyle = _ThumbnailPrevStyle;
    log_w("Thumbnail not ready yet. Returning to previous style [%d]", (int)_TheDrawStyle);
    return;
  }
  if (millis() - _TimeThumbnailReceived > 20000) {
    _TheDrawStyle = _ThumbnailPrevStyle;  // return to previous style after 10 seconds
    log_w("Thumbnail expired. Returning to previous style [%d]", (int)_TheDrawStyle);
    return;
  }

  // Draw the thumbnail image
  uint8_t startX = (THE_PANEL_WIDTH - THUMBNAIL_WIDTH) / 2;  // center the thumbnail horizontally
  uint16_t k = THUMBNAIL_WIDTH;                              // index for the thumbnail image. Ens saltem la 1era línia, no sé pq te soroll....
  for (uint8_t j = 1; j < THE_PANEL_HEIGHT; j++) { //ens saltem la primera fila. No se pq te soroll....
    for (uint8_t i = 0; i < THUMBNAIL_WIDTH; i++, k++) {
      _TheLeds[_TheMapping.XY(startX + i, j)] = _ThumbnailImg[k];  // draw the thumbnail pixel
    }
  }
}
