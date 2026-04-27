
// StarWarp – audio-reactive hyperspace starfield.
//
// Stars spawn at the panel centre (vanishing point) and shoot radially
// outward, simulating hyperspace flight at warp speed.
// A per-pixel brightness-persistence buffer creates short glowing trails.
// The whole field rotates slowly (spiral galaxy feel); bass accelerates the spin.
//
// Audio mapping:
//   Bass (pDBs[0-5])      → warp factor [1.0..3.5] + faster galaxy spin.
//   Bin index at birth    → hue: bass bins warm (red/orange), treble cold (blue/purple).
//   Volume (_1stBarValue) → base brightness (60..140). Colours always fully saturated.
//   Total energy          → spawn rate (0..STARWARP_MAX_SPAWN stars per frame).
//
// Memory: ~12.2 KB heap (2×W×H uint8_t persistence/hue + 100 stars × 18 B).
// CPU: O(MAX_STARS) move/rotate + O(W×H) fade/render per frame.

#define STARWARP_MAX_STARS  100
#define STARWARP_MAX_SPAWN   10   // max new stars born per frame

struct StarWarpStar {
  float   d;      // distance from centre (scalar, grows each frame)
  float   nx, ny; // direction unit vector (rotated slightly each frame → spiral path)
  float   speed;  // base speed in px/frame (further boosted by warp & distance)
  uint8_t hue;    // colour from birth frequency bin
  bool    alive;
};

struct StarWarpData {
  StarWarpStar stars  [STARWARP_MAX_STARS];
  uint8_t      persist[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];  // brightness trail
  uint8_t      hueBuf [THE_PANEL_WIDTH * THE_PANEL_HEIGHT];  // colour trail
  float        sWarp;       // smoothed warp multiplier [1.0 .. 3.5]
  uint8_t      spawnPhase;  // pseudo-random seed (advances each spawn)
  uint8_t      rotOffset;   // global spawn-angle rotation offset (256 = full turn)
};

StarWarpData* _starWarpData = nullptr;

void CleanupStarWarp() {
  delete _starWarpData;
  _starWarpData = nullptr;
}

void DrawStarWarp(MsgAudio2Draw& mad) {
  const uint8_t W  = THE_PANEL_WIDTH;
  const uint8_t H  = THE_PANEL_HEIGHT;
  const float   cx = (float)(W >> 1);  // centre x (48 for 96-wide panel)
  const float   cy = (float)(H >> 1);  // centre y (27 for 54-tall panel)
  // Stars die when distance exceeds the longest panel dimension / 2 + margin
  const float maxD = (cx > cy ? cx : cy) + 4.0f;

  // ---- Init on first call ----
  if (!_starWarpData) {
    _starWarpData = new (std::nothrow) StarWarpData();
    if (!_starWarpData) {
      log_e("StarWarp: alloc failed");
      ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true);
      return;
    }
    memset(_starWarpData->persist, 0, sizeof(_starWarpData->persist));
    memset(_starWarpData->hueBuf,  0, sizeof(_starWarpData->hueBuf));
    _starWarpData->sWarp      = 1.0f;
    _starWarpData->spawnPhase = 71;
    _starWarpData->rotOffset  = 0;
    for (uint8_t i = 0; i < STARWARP_MAX_STARS; i++) {
      _starWarpData->stars[i].alive = false;
    }
  }

  // ---- 1. Audio bands ----
  float bass = 0.0f;
  for (uint8_t i = 0; i < 6; i++) bass += mad.pDBs[i];
  bass /= (6.0f * 255.0f);  // normalised 0..1

  float energy = 0.0f;
  for (uint8_t i = 0; i < W; i++) energy += mad.pDBs[i];
  energy /= ((float)W * 255.0f);  // normalised 0..1

  // ---- 2. Smooth warp factor toward bass target (asymmetric: fast up, slow down) ----
  // Range 0.002..3.5: in silence stars are nearly frozen; full bass they shoot out fast.
  const float targetWarp = 0.002f + bass * 3.498f;
  // Acceleration reacts quickly to bass hits (α=0.20); deceleration is gradual (α=0.05)
  const float alpha = (targetWarp > _starWarpData->sWarp) ? 0.20f : 0.05f;
  _starWarpData->sWarp += alpha * (targetWarp - _starWarpData->sWarp);
  const float warp = _starWarpData->sWarp;

  // ---- 3. Global rotation: advance spawn-angle offset each frame ----
  // Base: ~1 full rotation every 13 s. Bass doubles the speed.
  _starWarpData->rotOffset += (uint8_t)(1u + (uint8_t)(bass * 2.0f));

  // Per-frame delta angle for rotating existing star direction vectors (spiral paths).
  // 0.025 rad/frame base ≈ full turn every ~13 s; bass pushes up to ~0.065 rad/frame.
  const float rotDelta = 0.025f + bass * 0.04f;
  const float rc = cosf(rotDelta);  // precomputed once for all stars this frame
  const float rs = sinf(rotDelta);

  // ---- 4. Fade persistence buffer (~×0.86/frame → ~1 s trail at 52 ms/frame) ----
  const uint16_t TOTAL = (uint16_t)W * H;
  for (uint16_t k = 0; k < TOTAL; k++) {
    uint8_t v = _starWarpData->persist[k];
    if      (v > 6) _starWarpData->persist[k] = (uint8_t)(((uint16_t)v * 220u) >> 8);
    else if (v)     _starWarpData->persist[k]--;
  }

  // ---- 5. Move & rotate existing stars, paint trails into persist buffer ----
  for (uint8_t i = 0; i < STARWARP_MAX_STARS; i++) {
    StarWarpStar& s = _starWarpData->stars[i];
    if (!s.alive) continue;

    // Rotate direction vector → star follows a spiral outward path
    const float new_nx = s.nx * rc - s.ny * rs;
    const float new_ny = s.nx * rs + s.ny * rc;
    s.nx = new_nx;
    s.ny = new_ny;

    // Advance: base speed boosted by warp and by distance (parallax acceleration)
    s.d += s.speed * warp * (1.0f + s.d * 0.08f);

    if (s.d >= maxD) { s.alive = false; continue; }

    const int16_t px = (int16_t)(cx + s.nx * s.d + 0.5f);
    const int16_t py = (int16_t)(cy + s.ny * s.d + 0.5f);

    if ((uint16_t)px >= W || (uint16_t)py >= H) { s.alive = false; continue; }

    // Brightness grows with distance (parallax depth illusion)
    const uint8_t br = (uint8_t)((s.d / maxD) * 230.0f) + 15u;

    const uint16_t idx  = (uint16_t)py * W + (uint16_t)px;
    const uint16_t newV = (uint16_t)_starWarpData->persist[idx] + br;
    _starWarpData->persist[idx] = (newV > 255u) ? 255u : (uint8_t)newV;
    _starWarpData->hueBuf[idx]  = s.hue;
  }

  // ---- 6. Spawn new stars ----
  uint8_t toSpawn = (uint8_t)(energy * 6.0f + bass * 5.0f + 0.5f);
  if (toSpawn > STARWARP_MAX_SPAWN) toSpawn = STARWARP_MAX_SPAWN;
  if (toSpawn == 0 && energy > 0.05f) toSpawn = 1;

  uint8_t spawned = 0;
  for (uint8_t i = 0; i < STARWARP_MAX_STARS && spawned < toSpawn; i++) {
    StarWarpStar& s = _starWarpData->stars[i];
    if (s.alive) continue;

    _starWarpData->spawnPhase += 83;
    uint8_t bin = (uint8_t)(((uint16_t)_starWarpData->spawnPhase * W) >> 8);
    if (mad.pDBs[bin] < 40) {
      _starWarpData->spawnPhase += 53;
      uint8_t bin2 = (uint8_t)(((uint16_t)_starWarpData->spawnPhase * W) >> 8);
      if (mad.pDBs[bin2] > mad.pDBs[bin]) bin = bin2;
    }
    if (mad.pDBs[bin] < 20) { spawned++; continue; }

    // Direction: pseudo-random angle + global rotation offset (born into the rotating frame)
    _starWarpData->spawnPhase += 37;
    const uint8_t angle = _starWarpData->spawnPhase + _starWarpData->rotOffset;
    float nx = (float)((int16_t)cos8(angle) - 128) / 127.0f;
    float ny = (float)((int16_t)sin8(angle) - 128) / 127.0f;
    float len = sqrtf(nx * nx + ny * ny);
    if (len > 0.01f) { nx /= len; ny /= len; }
    else              { nx = 1.0f; ny = 0.0f; }

    const float speed = 0.3f + (mad.pDBs[bin] / 255.0f) * 0.4f;  // 0.3..0.7 px/frame base

    s.d     = 0.5f;
    s.nx    = nx;
    s.ny    = ny;
    s.speed = speed;
    s.hue   = (uint8_t)((uint16_t)bin * 220u / W);
    s.alive = true;
    spawned++;
  }

  // ---- 7. Render all pixels ----
  // Volume-driven overall brightness (60..140). Saturation always 255 — no white-out.
  const uint8_t brightness = (uint8_t)(60u + (uint16_t)_1stBarValue * 80u / 255u);

  for (uint8_t py = 0; py < H; py++) {
    for (uint8_t px = 0; px < W; px++) {
      const uint16_t idx = (uint16_t)py * W + (uint16_t)px;
      const uint8_t  br  = _starWarpData->persist[idx];
      if (br > 8) {
        _TheLeds[_TheMapping.XY(px, py)] = CHSV(_starWarpData->hueBuf[idx], 255, scale8(br, brightness));
      } else {
        _TheLeds[_TheMapping.XY(px, py)] = CRGB::Black;
      }
    }
  }
}
