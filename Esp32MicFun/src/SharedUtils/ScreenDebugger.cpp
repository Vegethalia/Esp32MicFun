#include <string>
#include "ScreenDebugger.h"

#ifdef USE_SCREENDEBUGGER

ScreenDebugger::ScreenDebugger(U8G2* pTheScreen, uint16_t linesBeforeDelay, uint16_t delayAfterLineMS, bool showLineNumber)
: _pTheScreen(pTheScreen)
, _TheDelay(delayAfterLineMS)
, _LinesBeforeWait(linesBeforeDelay)
, _LinesAddedNoWait(0)
, _ShowLineNumber(showLineNumber)
, _LinesAdded(0)
{

}

void ScreenDebugger::SetFont(FONTS theFont)
{
	if(!_pTheScreen) {
		return;
	}

	switch(theFont) {
		case FONTS::MINSIZE:
			_pTheScreen->setFont(u8g2_font_tom_thumb_4x6_mf);
		break;
		case FONTS::SIZE1:
			_pTheScreen->setFont(u8g2_font_5x8_mf);
			break;
		case FONTS::SIZE2:
			_pTheScreen->setFont(u8g2_font_6x12_mf);
			break;
		case FONTS::SIZE3:
			_pTheScreen->setFont(u8g2_font_7x13_mf);
			break;
		case FONTS::SIZE4:
			_pTheScreen->setFont(u8g2_font_9x15_mf);
			break;
		case FONTS::SIZE5:
			_pTheScreen->setFont(u8g2_font_10x20_mf);
			break;
		case FONTS::MAXSIZE:
			_pTheScreen->setFont(u8g2_font_inr16_mf);
			break;
	}
}

void ScreenDebugger::NewLine(const char* pTheLine, bool repaint)
{
	std::string theLine(pTheLine);
	NewLine(theLine, repaint);
}

void ScreenDebugger::NewLine(std::string& theLine, bool repaint)
{
	uint16_t charH = _pTheScreen->getMaxCharHeight() == 0 ? 1 : _pTheScreen->getMaxCharHeight();
	uint16_t h = _pTheScreen->getHeight();
	uint16_t maxLines=h/charH;

	_LinesAdded++;
	char buff[10];
	snprintf(buff, sizeof(buff), "%03d.", _LinesAdded);
	std::string line(buff+theLine);

	if(_ThePhrases.size()<maxLines) {
		_ThePhrases.push_back(line);
	}
	else if(_ThePhrases.size() > maxLines) {
		do {
			if(_FirstLine>0) {
				_ThePhrases.erase(_ThePhrases.begin());
				_FirstLine--;
			}
			else {
				_ThePhrases.pop_back();
			}
		} while(_ThePhrases.size()>maxLines);
	}
	else { //_ThePhrases.size() == maxLines
		_ThePhrases[_FirstLine]=line;
		_FirstLine = (_FirstLine + 1) % maxLines;
	}

	if(repaint) {
		DrawLines();
	}

	if(_TheDelay > 0)  {
		_LinesAddedNoWait++;
		if(_LinesAddedNoWait >= _LinesBeforeWait) {
			delay(_TheDelay);
			_LinesAddedNoWait=0;
		}
	}
}

void ScreenDebugger::NewLines(std::list<std::string>& lines, bool repaint)
{
	auto iter=lines.begin();
	int i=0;

	while(iter != lines.end()) {
		NewLine(*iter, i == (lines.size()-1)?repaint:false); //only repaint on the last line
		i++;
		iter++;
	}
}

void ScreenDebugger::DrawLines()
{
	if(!_pTheScreen) {
		return;
	}

	uint16_t charH = _pTheScreen->getMaxCharHeight() == 0 ? 10 : _pTheScreen->getMaxCharHeight();
	uint16_t maxHeight = _pTheScreen->getHeight();
	uint16_t j = charH;
	auto iter = _ThePhrases.begin();

	_pTheScreen->firstPage();
	do {
		j=charH;
		iter = _ThePhrases.begin();
		while(iter != _ThePhrases.end() && j <= maxHeight) {
			_pTheScreen->setCursor(0, j);
			_pTheScreen->print((*iter).c_str());
			j+=charH;
			iter++;
		}
	} while(_pTheScreen->nextPage());
}
#endif
