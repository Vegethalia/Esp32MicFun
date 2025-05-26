
#if defined(PANEL_SIZE_96x48)
#define PORT_STREAM_AUDIO 4444
#define PORT_SHAZAM_SERVICE 4445
#elif defined(PANEL_SIZE_64x32) 
#define PORT_STREAM_AUDIO 4448
#define PORT_SHAZAM_SERVICE 4449
#endif

WiFiUDP _myUdpSendAudio;
int16_t* _pBuffAudio2Send = nullptr;

void SendAudio(MsgAudio2Draw& mad, uint16_t port) {
  const uint32_t ipAddres = 192 << 24 | 168 << 16 | 1 << 8 | 140;
  uint32_t numSamples = mad.audioLenInSamples;

// #if defined(PANEL_SIZE_96x48)
  // 1sec of audio
  const int16_t buffSizeAudio2Send = TARGET_SAMPLE_RATE / 4;  // 250ms of audio
  static int16_t index = 0;
  if (!_pBuffAudio2Send) {
    _pBuffAudio2Send = (int16_t*)malloc(buffSizeAudio2Send * sizeof(int16_t));
    if (!_pBuffAudio2Send) {
      log_e("Error allocating memory for audio buffer");
      return;
    }
  }

  if (index + numSamples <= buffSizeAudio2Send) {
    memcpy(_pBuffAudio2Send + index, mad.pAudio, numSamples * sizeof(int16_t));
    index += numSamples;
  } else {
    if (WiFi.isConnected()) {
      //_bSending = true;
      int err = _myUdpSendAudio.beginPacket(IPAddress(192, 168, 1, 140), port);

      _myUdpSendAudio.write((uint8_t*)_pBuffAudio2Send, index * sizeof(int16_t));
      // log_d("JustSent %d samples", index);

      _myUdpSendAudio.endPacket();
    }
    index = 0;
    memcpy(_pBuffAudio2Send, mad.pAudio, numSamples * sizeof(int16_t));
    index += numSamples;
  }
  //_bSending = false;

// #elif defined(PANEL_SIZE_64x32)
//   if (_Connected2Wifi) {
//     int err = _myUdpSendAudio.beginPacket(IPAddress(ipAddres), port);

//     _myUdpSendAudio.write((uint8_t*)mad.pAudio, numSamples * sizeof(int16_t));
//     _myUdpSendAudio.endPacket();
//   }
// #endif
}
