// 2020 - November - Jordi Carrión
// This class uses an initialized instance of a U8G2 screen and uses it to easily display debug messages on it.
// Adding new "debug lines" will scroll the previous ones when needed.
// The U8G2 object can be configured outside this class (for instance the Font) or you might use
// this class methods to choose the font, size etc.
// It is possible to add a default delay after adding a new line, just to be sure that they can be read.

#ifdef USE_SCREENDEBUGGER

#include <U8g2lib.h>
#include <list>

class ScreenDebugger
{
public:
	enum FONTS { MINSIZE, SIZE1, SIZE2, SIZE3, SIZE4, SIZE5, MAXSIZE};

public:
	//Constructor. if pTheScreen is null, do not forget to call Init before using the class.
	//The passed object will be shared with the caller, do not free it before this class, or call Init(null) to eliminate the dependency.
	//linesBeforeDelay is the number of lines to print before waiting the delayAfterLineMS time. Ignored if delayAfterLinesMS is 0.
	//if showLineNumber is true, each line will show the order in which was inserted
	ScreenDebugger(U8G2* pTheScreen = nullptr, uint16_t linesBeforeDelay = 4, uint16_t delayAfterLinesMS = 100, bool showLineNumber = true);

public:
	//Replaces the Screen object by a new one. Pass null to reset the class.
	//linesBeforeDelay is the number of lines to print before waiting the delayAfterLineMS time. Ignored if delayAfterLinesMS is 0.
	//if showLineNumber is true, each line will show the order in which was inserted
	void Init(U8G2* pTheScreen, uint16_t linesBeforeDelay = 4, uint16_t delayAfterLineMS = 100, bool showLineNumber = true) {
		_pTheScreen = pTheScreen; _LinesBeforeWait = linesBeforeDelay; _TheDelay = delayAfterLineMS; _ShowLineNumber=showLineNumber;
	}
	//Configures the screen with a predefined font
	void SetFont(FONTS theFont);
	//Adds a new line and repaints the screen if repaint is true. Then waits the configured delay when appropiate.
	//std::string version
	void NewLine(std::string &line, bool repaint=true);
	//Adds a new line and repaints the screen if repaint is true. Then waits the configured delay when appropiate.
	//char* version.
	void NewLine(const char *pTheLine, bool repaint = true);
	//Adds some lines and repaints the screen if repaint is true. Then waits the configured delay when appropiate.
	//Repaint will only happen once all lines have been added.
	void NewLines(std::list<std::string> &lines, bool repaint=true);

protected:
	//Draw all lines in the screen.
	void DrawLines();

private:
	U8G2 *_pTheScreen;  //the screen object. Shared with the caller.
	uint16_t _TheDelay; //delay applied after each call to "NewLine"
	uint16_t _LinesBeforeWait;  //Number of lines to print before waiting the _TheDelay time. Ignored if _TheDelay is 0.
	uint16_t _LinesAddedNoWait; //Number of lines added without waiting. When equal to _LinesBeforeWait, the Delay will be executed
	bool     _ShowLineNumber;   //if showLineNumber is true, each line will show the order in which was inserted

	std::vector<std::string> _ThePhrases; //Contains all the sentences displayed. Used as a Circular buffer.
	uint16_t _FirstLine; //Index of _ThePhrases with the 1st line to show.
	uint32_t _LinesAdded; //Keeps a counter of the number of lines inserted
};

#endif
