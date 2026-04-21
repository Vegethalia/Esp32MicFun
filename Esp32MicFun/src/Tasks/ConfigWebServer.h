#pragma once

#include <WebServer.h>
#include <WiFi.h>

namespace ConfigWebServer {
namespace {
WebServer _server(80);
bool _started = false;

struct HueOption {
  int16_t value;
  const char* label;
};

const HueOption _hueOptions[] = {
    {-1, "Auto"},
    {HSVHue::HUE_RED, "HUE_RED"},
    {HSVHue::HUE_ORANGE, "HUE_ORANGE"},
    {HSVHue::HUE_YELLOW, "HUE_YELLOW"},
    {HSVHue::HUE_GREEN, "HUE_GREEN"},
    {HSVHue::HUE_AQUA, "HUE_AQUA"},
    {HSVHue::HUE_BLUE, "HUE_BLUE"},
    {HSVHue::HUE_PURPLE, "HUE_PURPLE"},
    {HSVHue::HUE_PINK, "HUE_PINK"},
};

const char* GetStyleName(DRAW_STYLE style) {
  switch (style) {
    case DRAW_STYLE::BARS_WITH_TOP:
      return "Bars with top";
    case DRAW_STYLE::VERT_FIRE:
      return "Vert fire";
    case DRAW_STYLE::HORIZ_FIRE:
      return "Horiz fire";
    case DRAW_STYLE::VISUAL_CURRENT:
      return "Visual current";
    case DRAW_STYLE::MATRIX_FFT:
      return "Matrix FFT";
    case DRAW_STYLE::DISCO_LIGHTS:
      return "Disco lights";
    case DRAW_STYLE::ANALOG_CLOCK:
      return "Analog clock";
    case DRAW_STYLE::DRAW_THUMBNAIL:
      return "Thumbnail";
    case DRAW_STYLE::CALC_MODE:
      return "Calc mode";
    default:
      return "Unknown";
  }
}

const char* GetBarAlterDrawName(ALTERDRAW alterDraw) {
  switch (alterDraw) {
    case ALTERDRAW::NO_ALTER:
      return "NO_ALTER";
    case ALTERDRAW::ODD:
      return "ODD";
    case ALTERDRAW::ALTERNATE:
      return "ALTERNATE";
    default:
      return "UNKNOWN";
  }
}

DRAW_STYLE GetDisplayableStyle() {
  return _TheDrawStyle <= DRAW_STYLE::ANALOG_CLOCK ? _TheDrawStyle : DRAW_STYLE::DEFAULT_STYLE;
}

uint8_t GetDisplayableIntensity() {
  return (uint8_t)max(min((int)_MAX_MILLIS, 128), 1);
}

String GetStatusMessageFromQuery() {
  if (_server.hasArg("status")) {
    String status = _server.arg("status");
    if (status == "saved") {
      return "Configuracio guardada.";
    }
  }
  return "";
}

bool IsNamedHue(int16_t hue) {
  for (const auto& option : _hueOptions) {
    if (option.value == hue) {
      return true;
    }
  }
  return false;
}

void AppendSelectOption(String& html, int value, const char* label, int selectedValue) {
  html += F("<option value='");
  html += String(value);
  html += "'";
  if (value == selectedValue) {
    html += F(" selected");
  }
  html += ">";
  html += label;
  html += F("</option>");
}

String BuildStateJson() {
  String json;
  json.reserve(256);
  json += F("{\"style\":");
  json += String((int)GetDisplayableStyle());
  json += F(",\"hue\":");
  json += String(_TheDesiredHue);
  json += F(",\"intensity\":");
  json += String((int)GetDisplayableIntensity());
  json += F(",\"piano\":");
  json += (_pianoMode ? F("true") : F("false"));
  json += F(",\"night\":");
  json += (_NightMode ? F("true") : F("false"));
  json += F(",\"shazam\":");
  json += (_ShazamSongs ? F("true") : F("false"));
  json += F(",\"binGrouping\":");
  json += String((int)_binGrouping);
  json += F(",\"barSpacing\":");
  json += String((int)_barSpacing);
  json += F(",\"barSpacingMax\":");
  json += String((int)GetMaxBarSpacingForGrouping(_binGrouping));
  json += F(",\"barAlterDraw\":");
  json += String((int)_barAlterDraw);
  json += F(",\"fadingWave\":");
  json += (_FadingWaveMode ? F("true") : F("false"));
  json += F(",\"clockZebra\":");
  json += (_ClockZebraMode ? F("true") : F("false"));
  json += F("}");
  return json;
}

void SendJsonState() {
  _server.sendHeader("Cache-Control", "no-store");
  _server.send(200, "application/json", BuildStateJson());
}

int ClampWebIntensity(long value) {
  if (value < 1) {
    return 1;
  }
  if (value > 128) {
    return 128;
  }
  return (int)value;
}

bool ApplyPreviewControl(const String& control, const String& value) {
  if (control == "style") {
    ChangeDrawStyle(ClampDrawStyle(value.toInt(), DRAW_STYLE::ANALOG_CLOCK), false, false);
    return true;
  }
  if (control == "hue") {
    ApplyDesiredHueSetting(value.toInt(), false);
    return true;
  }
  if (control == "intensity") {
    ApplyIntensitySetting(ClampWebIntensity(value.toInt()), false);
    return true;
  }
  if (control == "piano") {
    ApplyPianoModeSetting(value == "1", false);
    return true;
  }
  if (control == "night") {
    ApplyNightModeSetting(value == "1", false);
    return true;
  }
  if (control == "shazam") {
    ApplyShazamModeSetting(value == "1");
    return true;
  }
  if (control == "binGrouping") {
    ApplyBinGroupingSetting(value.toInt());
    return true;
  }
  if (control == "barSpacing") {
    ApplyBarSpacingSetting(value.toInt());
    return true;
  }
  if (control == "barAlterDraw") {
    ApplyBarAlterDrawSetting(value.toInt());
    return true;
  }
  if (control == "fadingWave") {
    ApplyFadingWaveModeSetting(value == "1");
    return true;
  }
  if (control == "clockZebra") {
    _ClockZebraMode = (value == "1");
    return true;
  }
  return false;
}

void PersistRequestValues() {
  bool nightEnabled = _server.hasArg("night");

  if (!nightEnabled && _NightMode) {
    ApplyNightModeSetting(false, true);
  }

  if (_server.hasArg("style")) {
    ChangeDrawStyle(ClampDrawStyle(_server.arg("style").toInt(), DRAW_STYLE::ANALOG_CLOCK), false, true);
  }
  if (_server.hasArg("hue")) {
    ApplyDesiredHueSetting(_server.arg("hue").toInt(), true);
  }
  if (_server.hasArg("intensity") && !nightEnabled) {
    ApplyIntensitySetting(ClampWebIntensity(_server.arg("intensity").toInt()), true);
  }
  ApplyPianoModeSetting(_server.hasArg("piano"), true);

  if (nightEnabled) {
    ApplyNightModeSetting(true, true);
  }
}

String FormatDetectionTime(time_t t) {
  if (t == 0) return "--:--:--";
  struct tm* ti = localtime(&t);
  char buf[10];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", ti->tm_hour, ti->tm_min, ti->tm_sec);
  return String(buf);
}

String SanitizeSongName(const std::string& name) {
  String s(name.c_str());
  s.replace("<", "[");
  s.replace(">", "]");
  return s;
}

String BuildSongHistoryHtml() {
  if (_SongHistory.empty()) return "";

  String html;
  html.reserve(2048);

  bool nowPlaying = (millis() - _SongHistory[0].detectionMillis) < 3 * 60 * 1000UL;
  bool hasThumbnail = _ThumbnailReady && (int)_ThumbnailImg.size() >= THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT;

  html += F("<h2 style='margin-top:32px;'>Can&#231;ons reconegudes</h2>");

  if (nowPlaying) {
    const SongEntry& s = _SongHistory[0];
    html += F("<div class='nowplaying-box'>");
    html += F("<div class='nowplaying-title'>&#127925; Est&agrave; sonant</div>");
    html += F("<div class='nowplaying-layout'>");

    // Columna esquerra: nom, hora i imatge
    html += F("<div class='nowplaying-left'>");
    html += "<div class='nowplaying-name'>" + SanitizeSongName(s.name) + "</div>";
    html += "<div class='nowplaying-time'>" + FormatDetectionTime(s.detectionWallTime) + "</div>";
    if (hasThumbnail) {
      html += F("<img src='/thumbnail.bmp' class='nowplaying-img' alt='portada'>");
    }
    html += F("</div>");

    // Columna dreta: llista de cançons anteriors
    if (_SongHistory.size() > 1) {
      html += F("<div class='nowplaying-right'><ul class='song-history-inline'>");
      for (uint8_t i = 1; i < (uint8_t)_SongHistory.size(); i++) {
        html += F("<li><span class='song-time'>");
        html += FormatDetectionTime(_SongHistory[i].detectionWallTime);
        html += F("</span><span class='song-name'>");
        html += SanitizeSongName(_SongHistory[i].name);
        html += F("</span></li>");
      }
      html += F("</ul></div>");
    }

    html += F("</div></div>");  // tanca nowplaying-layout + nowplaying-box
  } else {
    // Sense "ara sonant": llista simple de totes les cançons
    html += F("<ul class='song-history'>");
    for (uint8_t i = 0; i < (uint8_t)_SongHistory.size(); i++) {
      html += F("<li><span class='song-time'>");
      html += FormatDetectionTime(_SongHistory[i].detectionWallTime);
      html += F("</span><span class='song-name'>");
      html += SanitizeSongName(_SongHistory[i].name);
      html += F("</span></li>");
    }
    html += F("</ul>");
  }
  return html;
}

void HandleThumbnail() {
  if (!_ThumbnailReady || (int)_ThumbnailImg.size() < THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT) {
    _server.send(404, "text/plain", "No thumbnail");
    return;
  }

  const int rowStride = THUMBNAIL_WIDTH * 3;  // 72*3=216 and 52*3=156 are both multiples of 4
  const uint32_t pixelDataSize = (uint32_t)rowStride * THUMBNAIL_HEIGHT;
  const uint32_t fileSize = 54 + pixelDataSize;

  auto le32 = [](uint8_t* p, uint32_t v) {
    p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; p[2] = (v >> 16) & 0xFF; p[3] = (v >> 24) & 0xFF;
  };

  uint8_t bmpHeader[54] = {};
  bmpHeader[0] = 'B'; bmpHeader[1] = 'M';
  le32(&bmpHeader[2], fileSize);
  bmpHeader[10] = 54;  // pixel data offset
  bmpHeader[14] = 40;  // DIB header size
  le32(&bmpHeader[18], (uint32_t)THUMBNAIL_WIDTH);
  le32(&bmpHeader[22], (uint32_t)THUMBNAIL_HEIGHT);
  bmpHeader[26] = 1;   // color planes
  bmpHeader[28] = 24;  // bits per pixel (RGB888)
  le32(&bmpHeader[34], pixelDataSize);

  _server.sendHeader("Cache-Control", "no-store");
  _server.setContentLength(fileSize);
  _server.send(200, "image/bmp", "");
  _server.sendContent(reinterpret_cast<const char*>(bmpHeader), 54);

  std::vector<uint8_t> rowBuf(rowStride);
  for (int row = THUMBNAIL_HEIGHT - 1; row >= 0; row--) {
    for (int col = 0; col < THUMBNAIL_WIDTH; col++) {
      const CRGB& px = _ThumbnailImg[row * THUMBNAIL_WIDTH + col];
      rowBuf[col * 3 + 0] = px.b;
      rowBuf[col * 3 + 1] = px.g;
      rowBuf[col * 3 + 2] = px.r;
    }
    _server.sendContent(reinterpret_cast<const char*>(rowBuf.data()), rowStride);
  }
}

String BuildPage(const String& statusMessage = "") {
  String html;
  html.reserve(13000);
  html += F(
      "<!doctype html><html><head><meta charset='utf-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>MicFun config</title>"
      "<style>"
      "body{font-family:Arial,sans-serif;max-width:760px;margin:0 auto;padding:20px;background:#111;color:#f5f5f5;}"
      "h1{margin-bottom:6px;}form{background:#1d1d1d;padding:18px;border-radius:12px;}"
      "label{display:block;margin-top:14px;font-weight:bold;}"
      "select,input{width:100%;margin-top:6px;padding:10px;border-radius:8px;border:1px solid #444;background:#222;color:#f5f5f5;box-sizing:border-box;}"
      ".checks{display:flex;gap:18px;flex-wrap:wrap;margin-top:14px;}"
      ".checks label{display:flex;align-items:center;gap:8px;margin:0;font-weight:normal;}"
      ".checks input{width:auto;margin:0;}"
      ".actions{display:flex;gap:12px;flex-wrap:wrap;margin-top:18px;}"
      "button{margin-top:18px;padding:12px 18px;border:0;border-radius:8px;background:#3b82f6;color:#fff;font-weight:bold;cursor:pointer;}"
      ".actions button{margin-top:0;}"
      "#recognizeAsapBtn{background:#7c3aed;}"
      "#reloadConfigBtn{background:#4b5563;}"
      ".status{background:#173b22;color:#c7ffd8;padding:10px 12px;border-radius:8px;margin-bottom:16px;}"
      ".nowplaying-box{background:#1e1b4b;border:1px solid #6d28d9;border-radius:12px;padding:18px;margin-top:24px;}"
      ".nowplaying-title{color:#a78bfa;font-weight:bold;font-size:1.05rem;margin-bottom:10px;}"
      ".nowplaying-name{font-size:1.3rem;font-weight:bold;margin-bottom:4px;word-break:break-word;}"
      ".nowplaying-time{color:#bbb;font-size:.9rem;margin-bottom:4px;}"
      ".nowplaying-img{display:block;width:100%;border-radius:6px;image-rendering:pixelated;margin-top:12px;}"
      ".nowplaying-layout{display:flex;gap:18px;align-items:flex-start;margin-top:10px;}"
      ".nowplaying-left{flex:0 0 50%;min-width:0;box-sizing:border-box;}"
      ".nowplaying-right{flex:1;min-width:0;}"
      ".song-history-inline{list-style:none;padding:0;margin:0;}"
      ".song-history-inline li{display:flex;gap:10px;padding:6px 0;border-bottom:1px solid #3a3a5c;align-items:baseline;}"
      ".song-history-inline li:last-child{border-bottom:none;}"
      ".song-history{list-style:none;padding:0;margin:14px 0 0 0;}"
      ".song-history li{display:flex;gap:14px;padding:8px 2px;border-bottom:1px solid #2a2a2a;}"
      ".song-history li:last-child{border-bottom:none;}"
      ".song-time{color:#888;font-size:.9rem;white-space:nowrap;min-width:68px;}"
      ".song-name{color:#e5e5e5;}"
      ".muted{color:#bbb;font-size:.95rem;}"
      ".value{color:#93c5fd;font-weight:normal;}"
      "</style></head><body><h1>MicFun config</h1>");

  html += F("<p class='muted'>IP actual: ");
  html += WiFi.localIP().toString();
  html += F("</p>");

  if (statusMessage.length() > 0) {
    html += F("<div class='status'>");
    html += statusMessage;
    html += F("</div>");
  }

  html += F("<div id='liveStatus' class='status' style='display:none;'></div>");
  html += F("<form id='configForm' method='post' action='/save'>");

  html += F("<label for='style'>Estil</label><select id='style' name='style'>");
  AppendSelectOption(html, (int)DRAW_STYLE::BARS_WITH_TOP, GetStyleName(DRAW_STYLE::BARS_WITH_TOP), (int)GetDisplayableStyle());
  AppendSelectOption(html, (int)DRAW_STYLE::VERT_FIRE, GetStyleName(DRAW_STYLE::VERT_FIRE), (int)GetDisplayableStyle());
  AppendSelectOption(html, (int)DRAW_STYLE::HORIZ_FIRE, GetStyleName(DRAW_STYLE::HORIZ_FIRE), (int)GetDisplayableStyle());
  AppendSelectOption(html, (int)DRAW_STYLE::VISUAL_CURRENT, GetStyleName(DRAW_STYLE::VISUAL_CURRENT), (int)GetDisplayableStyle());
  AppendSelectOption(html, (int)DRAW_STYLE::MATRIX_FFT, GetStyleName(DRAW_STYLE::MATRIX_FFT), (int)GetDisplayableStyle());
  AppendSelectOption(html, (int)DRAW_STYLE::DISCO_LIGHTS, GetStyleName(DRAW_STYLE::DISCO_LIGHTS), (int)GetDisplayableStyle());
  AppendSelectOption(html, (int)DRAW_STYLE::ANALOG_CLOCK, GetStyleName(DRAW_STYLE::ANALOG_CLOCK), (int)GetDisplayableStyle());
  html += F("</select>");

  html += F("<label for='hue'>Basic Hue</label><select id='hue' name='hue'>");
  for (const auto& option : _hueOptions) {
    AppendSelectOption(html, option.value, option.label, _TheDesiredHue);
  }
  if (!IsNamedHue(_TheDesiredHue)) {
    String customLabel = String("CUSTOM_") + String(_TheDesiredHue);
    AppendSelectOption(html, _TheDesiredHue, customLabel.c_str(), _TheDesiredHue);
  }
  html += F("</select>");

  html += F("<label for='intensity'>Intensity</label><select id='intensity' name='intensity'>");
  {
    const int intensityVals[] = {1, 2, 4, 8, 16, 32, 64, 128};
    for (int v : intensityVals) {
      String s = String(v);
      AppendSelectOption(html, v, s.c_str(), (int)GetDisplayableIntensity());
    }
  }
  html += F("</select>");

  html += F("<label for='binGrouping'>Bin grouping</label><select id='binGrouping'>");
  for (int v = 1; v <= 8; v++) {
    String s = String(v);
    AppendSelectOption(html, v, s.c_str(), (int)_binGrouping);
  }
  html += F("</select>");

  html += F("<label for='barSpacing'>Bar spacing</label><select id='barSpacing'>");
  for (int v = 0; v <= 7; v++) {
    String s = String(v);
    AppendSelectOption(html, v, s.c_str(), (int)_barSpacing);
  }
  html += F("</select>");

  html += F("<label for='barAlterDraw'>Bar alter draw</label><select id='barAlterDraw'>");
  AppendSelectOption(html, (int)ALTERDRAW::NO_ALTER, GetBarAlterDrawName(ALTERDRAW::NO_ALTER), (int)_barAlterDraw);
  AppendSelectOption(html, (int)ALTERDRAW::ODD, GetBarAlterDrawName(ALTERDRAW::ODD), (int)_barAlterDraw);
  AppendSelectOption(html, (int)ALTERDRAW::ALTERNATE, GetBarAlterDrawName(ALTERDRAW::ALTERNATE), (int)_barAlterDraw);
  html += F("</select>");

  html += F("<div class='checks'>");
  html += F("<label for='piano'><input id='piano' name='piano' type='checkbox'");
  if (_pianoMode) {
    html += F(" checked");
  }
  html += F(">Piano mode</label>");
  html += F("<label for='night'><input id='night' name='night' type='checkbox'");
  if (_NightMode) {
    html += F(" checked");
  }
  html += F(">Night mode</label>");
  html += F("<label for='shazam'><input id='shazam' type='checkbox'");
  if (_ShazamSongs) {
    html += F(" checked");
  }
  html += F(">Shazam mode</label>");
  html += F("<label for='fadingWave'><input id='fadingWave' type='checkbox'");
  if (_FadingWaveMode) {
    html += F(" checked");
  }
  html += F(">Fading wave mode</label>");
  html += F("<label for='clockZebra'><input id='clockZebra' type='checkbox'");
  if (_ClockZebraMode) {
    html += F(" checked");
  }
  html += F(">Clock zebra mode</label>");
  html += F("</div>");

  html += F("<p class='muted'>Els canvis s'apliquen al moment al control modificat. El boto Guardar nomes persisteix estil, hue, intensity, piano i night mode.</p>");
  html += F(
      "<div class='actions'>"
      "<button id='reloadConfigBtn' type='button'>Rellegeix configuracio</button>"
      "<button id='recognizeAsapBtn' type='button'>Shazam recognize ASAP</button>"
      "<button type='submit'>Guardar</button>"
      "</div></form>");

  html += BuildSongHistoryHtml();

  html += F(
      "<script>"
      "const liveStatus=document.getElementById('liveStatus');"
      "const form=document.getElementById('configForm');"
      "const styleInput=document.getElementById('style');"
      "const hueInput=document.getElementById('hue');"
      "const intensityInput=document.getElementById('intensity');"
      "const binGroupingInput=document.getElementById('binGrouping');"
      "const barSpacingInput=document.getElementById('barSpacing');"
      "const barAlterDrawInput=document.getElementById('barAlterDraw');"
      "const pianoInput=document.getElementById('piano');"
      "const nightInput=document.getElementById('night');"
      "const shazamInput=document.getElementById('shazam');"
      "const fadingWaveInput=document.getElementById('fadingWave');"
      "const clockZebraInput=document.getElementById('clockZebra');"
      "function showLiveStatus(msg){liveStatus.textContent=msg;liveStatus.style.display='block';}"
      "function syncBarSpacing(maxVal){"
        "const sel=barSpacingInput;"
        "for(let i=0;i<sel.options.length;i++){const v=Number(sel.options[i].value);sel.options[i].disabled=v>maxVal;sel.options[i].hidden=v>maxVal;}"
        "if(Number(sel.value)>maxVal){sel.value=String(maxVal);}"
        "sel.disabled=maxVal===0;"
      "}"
      "function syncUi(){"
        "const maxSpacing=Math.max(Number(binGroupingInput.value)-1,0);"
        "syncBarSpacing(maxSpacing);"
        "intensityInput.disabled=nightInput.checked;"
      "}"
      "function applyState(state){"
        "styleInput.value=String(state.style);"
        "hueInput.value=String(state.hue);"
        "intensityInput.value=String(state.intensity);"
        "pianoInput.checked=!!state.piano;"
        "nightInput.checked=!!state.night;"
        "shazamInput.checked=!!state.shazam;"
        "binGroupingInput.value=String(state.binGrouping);"
        "syncBarSpacing(state.barSpacingMax);"
        "barSpacingInput.value=String(state.barSpacing);"
        "barAlterDrawInput.value=String(state.barAlterDraw);"
        "fadingWaveInput.checked=!!state.fadingWave;"
        "clockZebraInput.checked=!!state.clockZebra;"
        "syncUi();"
      "}"
      "async function fetchState(){const res=await fetch('/state',{cache:'no-store'});if(!res.ok)throw new Error('state');applyState(await res.json());}"
      "async function previewControl(control,value){const body=new URLSearchParams();body.set('control',control);body.set('value',value);const res=await fetch('/preview',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded;charset=UTF-8'},body});if(!res.ok)throw new Error('preview');applyState(await res.json());showLiveStatus('Canvi aplicat en viu.');}"
      "const noErr=()=>showLiveStatus('No s\\'ha pogut aplicar el canvi.');"
      "styleInput.addEventListener('change',()=>previewControl('style',styleInput.value).catch(noErr));"
      "hueInput.addEventListener('change',()=>previewControl('hue',hueInput.value).catch(noErr));"
      "intensityInput.addEventListener('change',()=>previewControl('intensity',intensityInput.value).catch(noErr));"
      "pianoInput.addEventListener('change',()=>previewControl('piano',pianoInput.checked?'1':'0').catch(noErr));"
      "nightInput.addEventListener('change',()=>previewControl('night',nightInput.checked?'1':'0').catch(noErr));"
      "shazamInput.addEventListener('change',()=>previewControl('shazam',shazamInput.checked?'1':'0').catch(noErr));"
      "binGroupingInput.addEventListener('change',()=>{syncUi();previewControl('binGrouping',binGroupingInput.value).catch(noErr);});"
      "barSpacingInput.addEventListener('change',()=>previewControl('barSpacing',barSpacingInput.value).catch(noErr));"
      "barAlterDrawInput.addEventListener('change',()=>previewControl('barAlterDraw',barAlterDrawInput.value).catch(noErr));"
      "fadingWaveInput.addEventListener('change',()=>previewControl('fadingWave',fadingWaveInput.checked?'1':'0').catch(noErr));"
      "clockZebraInput.addEventListener('change',()=>previewControl('clockZebra',clockZebraInput.checked?'1':'0').catch(noErr));"
      "document.getElementById('reloadConfigBtn').addEventListener('click',async()=>{try{await fetchState();showLiveStatus('Configuracio rellegida.');}catch{showLiveStatus('No s\\'ha pogut rellegir la configuracio.');}});"
      "document.getElementById('recognizeAsapBtn').addEventListener('click',async()=>{try{const res=await fetch('/shazam-asap',{method:'POST'});if(!res.ok)throw new Error('asap');applyState(await res.json());showLiveStatus('Shazam ASAP enviat.');}catch{showLiveStatus('No s\\'ha pogut enviar el Shazam ASAP.');}});"
      "syncUi();"
      "</script></body></html>");

  return html;
}

void SendPage(const String& statusMessage = "") {
  _server.sendHeader("Cache-Control", "no-store");
  _server.send(200, "text/html", BuildPage(statusMessage));
}

void HandlePageRequest() {
  SendPage(GetStatusMessageFromQuery());
}

void HandlePreview() {
  if (!_server.hasArg("control") || !_server.hasArg("value")) {
    _server.send(400, "text/plain", "Missing control/value");
    return;
  }

  if (!ApplyPreviewControl(_server.arg("control"), _server.arg("value"))) {
    _server.send(400, "text/plain", "Unknown control");
    return;
  }
  SendJsonState();
}

void HandleSave() {
  PersistRequestValues();
  _server.sendHeader("Location", "/?status=saved", true);
  _server.send(303, "text/plain", "");
}

void HandleShazamRecognizeAsap() {
  TriggerShazamRecognizeAsap();
  SendJsonState();
}
}  // namespace

inline void SetupIfNeeded() {
  if (_started) {
    return;
  }

  _server.on("/", HTTP_GET, []() { HandlePageRequest(); });
  _server.on("/config", HTTP_GET, []() { HandlePageRequest(); });
  _server.on("/state", HTTP_GET, []() { SendJsonState(); });
  _server.on("/preview", HTTP_POST, []() { HandlePreview(); });
  _server.on("/save", HTTP_POST, []() { HandleSave(); });
  _server.on("/shazam-asap", HTTP_POST, []() { HandleShazamRecognizeAsap(); });
  _server.on("/thumbnail.bmp", HTTP_GET, []() { HandleThumbnail(); });
  _server.on("/favicon.ico", HTTP_GET, []() { _server.send(204); });
  _server.onNotFound([]() {
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });
  _server.begin();
  _started = true;
  SendDebugMessage(Utils::string_format("Config web server ready on http://%s/", WiFi.localIP().toString().c_str()).c_str());
}

inline void HandleClient() {
  if (_started) {
    _server.handleClient();
  }
}
}  // namespace ConfigWebServer
