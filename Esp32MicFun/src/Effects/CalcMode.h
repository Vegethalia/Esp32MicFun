
// variables globals de la calculadora
#define NUM1 1
#define NUM2 2
#define NUM3 3
#define NUM4 4
// u8g2_font_eventhrees_tr 3x3
// u8g2_font_tiny_gk_tr 4x4 no monospaced
// u8g2_font_fourmat_tr 4x3 no monospaced, es llegeix bé

#define NUMBERS_FONT_3x5 u8g2_font_tom_thumb_4x6_mr
#define NUMBERS_FONT_3x3 u8g2_font_eventhrees_tr
#define NUMBERS_FONT_HEIGHT 5
#define NUMBERS_FONT_WIDTH 4
#define DIGITS_PER_LINE ((THE_PANEL_WIDTH / NUMBERS_FONT_WIDTH) - 1)
#define MARGIN_TILE_VERT (8 - NUMBERS_FONT_HEIGHT - 1) // quin espai queda per sota de la línia de números a un tile de 8x8

std::string _CalcNum1;
std::string _CalcNum2;
std::string _CalcNum3;
std::string _CalcNum4;
bool _Calculat = false; // si es true, el resultat ha estat calculat i es pot pintar "maco"
int32_t _Result = 0; // el resultat de la operació si _Calculat és true
uint8_t _NumDigitsResShown = 0; // quants dígits del resultat s'han de mostrar
bool _LeftClock = false;

enum CALC_OPERATION {
    NONE,
    SUMA,
    RESTA,
    MULT,
    DIV
};
CALC_OPERATION _CalcOp1;
CALC_OPERATION _CalcOp2;
CALC_OPERATION _CalcOp3;
uint8_t _currentNum = NUM1;

void ResetCalcul()
{
    _CalcNum1 = _CalcNum2 = _CalcNum3 = _CalcNum4 = "";
    _CalcOp1 = _CalcOp2 = _CalcOp3 = CALC_OPERATION::NONE;
    _currentNum = NUM1;
    _Calculat = false;
    _LeftClock = false;
}

/// @brief processa la key passada
/// @param theKey
/// @returns true si s'ha apretat ENTER
bool ProcessNewKey(GEN_KEY_PRESS theKey)
{
    bool retValue = false;

    if (_Calculat && theKey != GEN_KEY_PRESS::KEY_RIGHT && theKey != GEN_KEY_PRESS::KEY_LEFT && theKey != GEN_KEY_PRESS::KEY_ENTER) {
        ResetCalcul();
    }

    std::string* pNum = &_CalcNum1;
    CALC_OPERATION aux;
    CALC_OPERATION* pOp = &aux;
    switch (_currentNum) {
    case NUM1:
        pNum = &_CalcNum1;
        pOp = &_CalcOp1;
        break;
    case NUM2:
        pNum = &_CalcNum2;
        pOp = &_CalcOp2;
        break;
    case NUM3:
        pNum = &_CalcNum3;
        pOp = &_CalcOp3;
        break;
    case NUM4:
        pNum = &_CalcNum4;
        pOp = &_CalcOp3;
        break;
    }

    switch (theKey) {
    case GEN_KEY_PRESS::KEY_0:
        pNum->append("0");
        break;
    case GEN_KEY_PRESS::KEY_1:
        pNum->append("1");
        break;
    case GEN_KEY_PRESS::KEY_2:
        pNum->append("2");
        break;
    case GEN_KEY_PRESS::KEY_3:
        pNum->append("3");
        break;
    case GEN_KEY_PRESS::KEY_4:
        pNum->append("4");
        break;
    case GEN_KEY_PRESS::KEY_5:
        pNum->append("5");
        break;
    case GEN_KEY_PRESS::KEY_6:
        pNum->append("6");
        break;
    case GEN_KEY_PRESS::KEY_7:
        pNum->append("7");
        break;
    case GEN_KEY_PRESS::KEY_8:
        pNum->append("8");
        break;
    case GEN_KEY_PRESS::KEY_9:
        pNum->append("9");
        break;
    case GEN_KEY_PRESS::KEY_PLUS:
        *pOp = CALC_OPERATION::SUMA;
        _currentNum = _currentNum < NUM4 ? _currentNum + 1 : _currentNum;
        break;
    case GEN_KEY_PRESS::KEY_MINUS:
        *pOp = CALC_OPERATION::RESTA;
        _currentNum = _currentNum < NUM4 ? _currentNum + 1 : _currentNum;
        break;
    case GEN_KEY_PRESS::KEY_PROD:
        *pOp = CALC_OPERATION::MULT;
        _currentNum = _currentNum < NUM4 ? _currentNum + 1 : _currentNum;
        break;
    case GEN_KEY_PRESS::KEY_DIV:
        *pOp = CALC_OPERATION::DIV;
        _currentNum = _currentNum < NUM4 ? _currentNum + 1 : _currentNum;
        break;
    case GEN_KEY_PRESS::KEY_ENTER:
        retValue = true;
        break;
    case GEN_KEY_PRESS::KEY_REFRESH:
        if (_Calculat) {
            ResetCalcul();
        } else {
            switch (_currentNum) {
            case NUM1:
                _CalcNum1 = "";
                break;
            case NUM2:
                _CalcNum2 = "";
                break;
            case NUM3:
                _CalcNum3 = "";
                break;
            case NUM4:
                _CalcNum4 = "";
                break;
            }
        }
    case GEN_KEY_PRESS::KEY_LEFT:
        _NumDigitsResShown = _Calculat ? _NumDigitsResShown + 1 : 0;
        break;
    case GEN_KEY_PRESS::KEY_RIGHT:
        _NumDigitsResShown = _Calculat && _NumDigitsResShown > 0 ? _NumDigitsResShown - 1 : _NumDigitsResShown;
        break;
    }
    std::string msg = Utils::string_format("pNum=%s pOp=%d currentNum=%d", pNum->c_str(), (int)*pOp, (int)_currentNum);
    _ThePubSub.publish(TOPIC_DEBUG, msg.c_str(), false);

    return retValue;
}

char GetSymbolFromOp(CALC_OPERATION calcOp)
{
    switch (calcOp) {
    case CALC_OPERATION::SUMA:
        return '+';
        break;
    case CALC_OPERATION::RESTA:
        return '-';
        break;
    case CALC_OPERATION::MULT:
        return 'x';
        break;
    case CALC_OPERATION::DIV:
        return '/';
        break;
    }

    return '?';
}

int32_t SolveOperation(int32_t num1, int32_t num2, CALC_OPERATION op)
{
    int32_t res = 0;
    switch (op) {
    case CALC_OPERATION::SUMA:
        res = num1 + num2;
        break;
    case CALC_OPERATION::RESTA:
        res = num1 - num2;
        break;
    case CALC_OPERATION::MULT:
        res = num1 * num2;
        break;
    case CALC_OPERATION::DIV:
        res = num1 / num2;
        break;
    }
    return res;
}

void Calculate()
{
    int32_t num1 = atoi(_CalcNum1.c_str());
    int32_t num2 = atoi(_CalcNum2.c_str());
    int32_t num3 = atoi(_CalcNum3.c_str());
    int32_t num4 = atoi(_CalcNum4.c_str());
    int32_t res = num1;

    if (_CalcOp1 != CALC_OPERATION::NONE) {
        res = SolveOperation(num1, num2, _CalcOp1);
    }
    if (_CalcOp2 != CALC_OPERATION::NONE) {
        res = SolveOperation(res, num3, _CalcOp2);
    }
    if (_CalcOp3 != CALC_OPERATION::NONE) {
        res = SolveOperation(res, num4, _CalcOp3);
    }
    _Result = res;
    _Calculat = true;
    // _CalcNum1 = std::to_string(num1);
    // _CalcNum2 = std::to_string(num2);
    // _CalcNum3 = std::to_string(num3);
    // _CalcNum4 = std::to_string(num4);
    // _CalcOp1 = _CalcOp2 = _CalcOp3 = CALC_OPERATION::NONE;
    // _currentNum = NUM1;
}
void drawSumaResta(uint8_t varIntens)
{
    std::string sRes = std::to_string(_Result);

    if (sRes.length() > _NumDigitsResShown) {
        sRes = sRes.substr(sRes.length() - _NumDigitsResShown, DIGITS_PER_LINE);
    }

    uint8_t vmarge = 0;
    int8_t topMarge = 2;
    uint8_t lenNum1 = _u8g2.getStrWidth(_CalcNum1.c_str());
    uint8_t lenNum2 = _u8g2.getStrWidth(_CalcNum2.c_str());
    uint8_t lenNum3 = _u8g2.getStrWidth(_CalcNum3.c_str());
    uint8_t lenNum4 = _u8g2.getStrWidth(_CalcNum4.c_str());
    uint8_t lenRes = _u8g2.getStrWidth(sRes.c_str());
    uint8_t maxLine = 0;
    uint8_t lenLastNum = 0;
    uint8_t maxWidth = 0;

    std::string op;
    op += GetSymbolFromOp(_CalcOp1);
    uint8_t lenOp1 = _u8g2.getStrWidth(op.c_str()) + 2;

    _u8g2.clearBuffer();
    if (_CalcOp2 == CALC_OPERATION::NONE) {
        vmarge = 1;
    }
    if (_CalcOp3 != CALC_OPERATION::NONE) {
        topMarge = -NUMBERS_FONT_HEIGHT + 1;
        _LeftClock = true;
    }
    _u8g2.drawStr(0, NUMBERS_FONT_HEIGHT * 2 + topMarge, _CalcNum1.c_str());
    _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 0, 3,
        THE_PANEL_WIDTH - lenNum1 - 2, 0, HSVHue::HUE_PURPLE, 128, false, 164);

    _u8g2.clearBuffer();
    maxLine = NUMBERS_FONT_HEIGHT * 3 + topMarge + vmarge;
    lenLastNum = lenNum2;
    maxWidth = std::max((uint8_t)lenNum1, (uint8_t)(lenNum2 + lenOp1));
    _u8g2.drawStr(0, maxLine, _CalcNum2.c_str());
    _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 0, 3,
        THE_PANEL_WIDTH - lenNum2 - 2, 0, HSVHue::HUE_AQUA, 128, false, 164);

    if (_CalcOp2 != CALC_OPERATION::NONE) { // he de pintar el 3er núm
        _u8g2.clearBuffer();
        maxLine = NUMBERS_FONT_HEIGHT * 4 + topMarge + vmarge;
        lenLastNum = lenNum3;
        if (_CalcOp3 != CALC_OPERATION::NONE) {
            maxWidth = (uint8_t)std::max((uint8_t)maxWidth, (uint8_t)lenNum3);
        } else {
            maxWidth = (uint8_t)std::max((uint8_t)maxWidth, (uint8_t)(lenNum3 + lenOp1));
        }

        _u8g2.drawStr(0, maxLine, _CalcNum3.c_str());
        _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 0, 3,
            THE_PANEL_WIDTH - lenNum3 - 2, 0, HSVHue::HUE_PINK, 128, false, 164);
    }
    if (_CalcOp3 != CALC_OPERATION::NONE) { // he de pintar el 3er núm
        _u8g2.clearBuffer();
        maxLine = NUMBERS_FONT_HEIGHT * 5 + topMarge + vmarge;
        lenLastNum = lenNum4;
        maxWidth = (uint8_t)std::max((uint8_t)maxWidth, (uint8_t)(lenNum4 + lenOp1));
        _u8g2.drawStr(0, maxLine, _CalcNum4.c_str());
        _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 0, 3,
            THE_PANEL_WIDTH - lenNum4 - 2, 0, HSVHue::HUE_ORANGE, 128, false, 164);
    }

    // pintem el símbol i els puntets
    _u8g2.clearBuffer();
    _u8g2.drawStr(0, maxLine, op.c_str());
    _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 0, 3,
        THE_PANEL_WIDTH - lenLastNum - lenOp1, 0, HSVHue::HUE_RED, varIntens, false, 164);

    maxWidth = (uint8_t)std::max((uint8_t)maxWidth, lenRes) + 2;
    for (uint8_t i = 0; i < maxWidth; i++) {
        _TheLeds[_TheMapping.XY(THE_PANEL_WIDTH - i - 2, maxLine)] = CHSV(HSVHue::HUE_RED, 128, varIntens);
    }

    _u8g2.clearBuffer();
    _u8g2.drawStr(0, maxLine + NUMBERS_FONT_HEIGHT + 1, sRes.c_str());
    _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 0, 3,
        THE_PANEL_WIDTH - lenRes - 2, 0, HSVHue::HUE_YELLOW, 128, false, 64);
}
void DrawCalculator(MsgAudio2Draw& mad)
{
    static uint8_t __hue = 0;
    static int16_t __xTitlePos = THE_PANEL_WIDTH;
    uint8_t varIntens = std::max((uint8_t)(100), _1stBarValue);

    if (_StartedCalcMode <= 0) {
        ResetCalcul();
        _StartedCalcMode = millis();
        __hue = random8();
        //_u8g2.setFont("u8g2_font_tom_thumb_4x6_mn"); // u8g2_font_tom_thumb_4x6_mn); u8g2_font_princess_tr
    }
    _u8g2.clearBuffer();
    if (millis() - _StartedCalcMode < CALC_MODE_SHOWTITLE_MS) {
        //        _u8g2.clearBuffer();
        _u8g2.setFont(u8g2_font_5x8_tr); // u8g2_font_tom_thumb_4x6_mn); u8g2_font_princess_tr
        ShowSmallTitle("Super Calculadora", __hue, varIntens, __xTitlePos--, -1);
        if (_TheFrameNumber % 2 != 0) {
            //__xTitlePos--;
            __hue++;
        }
        if (__xTitlePos < -(THE_PANEL_WIDTH * 2)) {
            __xTitlePos = THE_PANEL_WIDTH;
        }
    } else {
        _u8g2.setFont(u8g2_font_5x8_tr);
        DrawClock(NUMBERS_FONT_HEIGHT + 1, _LeftClock ? 0 : (THE_PANEL_WIDTH - (8 * NUMBERS_FONT_WIDTH)) / 2);
    }

    if (_TheLastKey != GEN_KEY_PRESS::KEY_NONE) {
        bool solve = ProcessNewKey(_TheLastKey);
        if (solve) {
            std::string sRes = std::to_string(_Result);
            _NumDigitsResShown = _Calculat ? sRes.length() : 0; // el 1er cop no mostrem res, el segon mostrem tot
            Calculate();
            std::string msg = Utils::string_format("NumDigitsShown=%d", _NumDigitsResShown);
            _ThePubSub.publish(TOPIC_DEBUG, msg.c_str(), false);
        }

        _TheLastKey = GEN_KEY_PRESS::KEY_NONE;
    }
    std::string numsStr = _CalcNum1;
    std::string opStr;
    std::string cursor;
    if (_currentNum >= 1) {
        opStr.append(_CalcNum1.length(), ' ');
    }
    if (_currentNum >= 2) {
        opStr += GetSymbolFromOp(_CalcOp1);
        numsStr += ' ';
        numsStr += _CalcNum2;
        opStr.append(_CalcNum2.length(), ' ');
    }
    if (_currentNum >= 3) {
        opStr += GetSymbolFromOp(_CalcOp2);
        numsStr += ' ';
        numsStr += _CalcNum3;
        opStr.append(_CalcNum3.length(), ' ');
    }
    if (_currentNum >= 4) {
        opStr += GetSymbolFromOp(_CalcOp3);
        numsStr += ' ';
        numsStr += _CalcNum4;
        opStr.append(_CalcNum4.length(), ' ');
    }
    if (_Calculat) {
        opStr += "=";
    } else {
        cursor.append(opStr.length(), ' ');
        cursor += "_";
    }
    bool same1OpsSuma = _CalcOp2 == CALC_OPERATION::NONE && (_CalcOp1 == CALC_OPERATION::SUMA || _CalcOp1 == CALC_OPERATION::RESTA);
    bool same2OpsSuma = (_CalcOp1 == CALC_OPERATION::SUMA && _CalcOp2 == CALC_OPERATION::SUMA) || (_CalcOp1 == CALC_OPERATION::RESTA && _CalcOp2 == CALC_OPERATION::RESTA);
    bool same3OpsSuma = (_CalcOp1 == CALC_OPERATION::SUMA && _CalcOp2 == CALC_OPERATION::SUMA && _CalcOp3 == CALC_OPERATION::SUMA);
    same3OpsSuma != (_CalcOp1 == CALC_OPERATION::RESTA && _CalcOp2 == CALC_OPERATION::RESTA && _CalcOp3 == CALC_OPERATION::RESTA);

    _u8g2.setFont(NUMBERS_FONT_3x5);
    if (!_Calculat || (opStr.length() + _CalcNum2.length()) < DIGITS_PER_LINE - 1) {
        _u8g2.drawStr(0, 15 - MARGIN_TILE_VERT, numsStr.c_str());
        _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 1, 1, 2, NUMBERS_FONT_HEIGHT * 2 + 2,
            HSVHue::HUE_YELLOW, 128, false, 164);
        _u8g2.drawStr(0, 23 - MARGIN_TILE_VERT, opStr.c_str());
        _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 2, 2, 2, NUMBERS_FONT_HEIGHT * 2 + 2,
            HSVHue::HUE_GREEN, varIntens, false, 128);
    }
    if (_Calculat) {
        if (same1OpsSuma) {
            drawSumaResta(varIntens);
        } else if (same2OpsSuma || same3OpsSuma) {
            drawSumaResta(varIntens);
        } else {
            std::string sRes = std::to_string(_Result);
            if (opStr.length() + sRes.length() > DIGITS_PER_LINE) { // pintar el resultat en la següent línia
                _u8g2.drawStr(0, 31 - MARGIN_TILE_VERT, sRes.c_str());
                uint8_t marge = 2 + (DIGITS_PER_LINE - sRes.length()) * NUMBERS_FONT_WIDTH;
                _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 3, 3, marge, NUMBERS_FONT_HEIGHT * 3 + 2,
                    HSVHue::HUE_AQUA, 128, false, 164);
            } else { // pintar el resultat en la mateixa línia
                numsStr = "";
                numsStr.append(opStr.length(), ' ');
                numsStr += sRes;
                _u8g2.drawStr(0, 31 - MARGIN_TILE_VERT, numsStr.c_str());
                _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 3, 3, 2, NUMBERS_FONT_HEIGHT * 2 + 2, HSVHue::HUE_AQUA, 128, false, 164);
            }
        }
    } else {
        _u8g2.drawStr(0, 31 - MARGIN_TILE_VERT, cursor.c_str());
        _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 3, 3, 2, NUMBERS_FONT_HEIGHT * 2 + 2,
            HSVHue::HUE_GREEN, (_TheFrameNumber % 10) < 6 ? varIntens : 32, false, 164);
    }
}
