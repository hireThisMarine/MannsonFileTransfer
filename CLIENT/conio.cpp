/*! ============================================================================
  @file conio.cpp

  @author Team LikeIt||! (Maximilian Manndorff & Jonathan Fitzpatrick) 
          Game Siege Breaker Freshman Class of 2009
  @par Email maximilian.manndorff@digipen.edu
  @par Login maximilian.manndorff
  @par ID 50006909
  @par Course CS260
  @par Project 2
  @date 08.06.2010

  @brief 
    Allows manipulation of the windows console (mouse & keyboard input,
    colored output, cursor, font, text, title and color manipulation,
    output of colored buffer).

============================================================================= */
#include "conio.h"
#include <Windows.h>
#include <tchar.h>
#pragma comment(lib,"winmm.lib")
#define CON_WIN_WIDTH 100
#define CON_WIN_HEIGHT 35
#define TITLE "CS260SU10A3 - Maximilian Manndorff - Justin Dodson"

//////////////////////////////////////////////////////////////////////////
HWND consoleHandle;                    /*!< Windows handle to console window   */
HANDLE wHnd;                           /*!< Handle to write to the console.    */
HANDLE rHnd;                           /*!< Handle to read from the console.   */
INPUT_RECORD *eventBuffer;             /*!< User Input buffer                  */
DWORD numEvents;                       /*!< Amount of events occured           */
DWORD numEventsRead;                   /*!< Amount of events read              */
int consoleWidth = CON_WIN_WIDTH;      /*!< Width DIMENSION and BUFFER width   */
int consoleHeight = CON_WIN_HEIGHT;    /*!< Height DIMENSION and BUFFER height */

//////////////////////////////////////////////////////////////////////////
static enum {
  INPUT_KEY_TIME_TOLERANCE = 50,       /* Minimum number of miliseconds allowed between input update calls            */
  NUM_KEYS = 127                       /* Maximum number of keys allowed to register                                  */
};

static DWORD lastTime;                  /* The last time an input update was performed                                */
static DWORD thisTime;                  /* The current time during an input update attempt                            */

static bool MouseMoved;                 /* Wether or not the mouse has been moved this update                         */
static COORD MousePos;                  /* The position of the mouse this update                                      */

static bool triggered[NUM_KEYS];        /* Hidden "global" for triggered event. Public via isTriggered()              */
static bool down[NUM_KEYS];             /* Hidden "global" for down event. Public via isDown()                        */
static bool released[NUM_KEYS];         /* Hidden "global" for released event. Public via isReleased                  */

static bool thisTrigger[NUM_KEYS];      /* Wether or not a key (given by its array index) is triggered in this update */
static bool lastTrigger[NUM_KEYS];      /* Wether or not a key (given by its array index) is triggered in last update */

static bool thisDown[NUM_KEYS];         /* Wether or not a key (given by its array index) is held down in this update */
static bool lastDown[NUM_KEYS];         /* Wether or not a key (given by its array index) is held down in last update */

static bool thisRelease[NUM_KEYS];      /* Wether or not a key (given by its array index) is released in this update  */
static bool lastRelease[NUM_KEYS];      /* Wether or not a key (given by its array index) is released in last update  */

static bool anyKeyTriggered;            /*!< Has any key fired trigger event since last frame?                        */

/* static function prototype */
static void updateKeystate(void);       /* Input workhorse, does logical calculations to determine key states          */
static void inputInit(void);             /* Initialize the input variables. Called from conioInit()                    */


void conioInit()
{
  SMALL_RECT windowSize;
  COORD bufferSize;
  CONSOLE_CURSOR_INFO cursorInfo;

  cursorInfo.bVisible = TRUE;
  cursorInfo.dwSize = 1;

    /* Set up controlloing variables for input reading */
  numEvents = 0;
  numEventsRead = 0;

    /* Find Windows console handle */
  consoleHandle = GetConsoleWindow();


    /* Set up  and check the handles for reading/writing: */
  wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
  rHnd = GetStdHandle(STD_INPUT_HANDLE);


  SetConsoleTitle(TEXT(TITLE));
//   SetWindowPos(consoleHandle, HWND_TOP, 0, 0, consoleWidth, consoleHeight, SWP_NOSIZE);
  SetConsoleFont(wHnd, 10);
  SetConsoleCursorInfo(wHnd, &cursorInfo);


   /* 
    * Temporarily change buffer dimensions to make change of window dimensions safe.
    * Without this step increasing the window dimensions over the buffer dimensions
    * might fail the complete dimensions change
    */    
  bufferSize.X = 2000;
  bufferSize.Y = 2000;
  SetConsoleScreenBufferSize(wHnd, bufferSize);

    /* Now change the actual window size: */
  windowSize.Left = 0;
  windowSize.Top = 0;
  windowSize.Right = (short)(consoleWidth - 1);
  windowSize.Bottom = (short)(consoleHeight - 1);

  SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

    /* Now adjust buffer size: */
  bufferSize.X = (short)consoleWidth;
  bufferSize.Y = (short)consoleHeight;
  
  SetConsoleScreenBufferSize(wHnd, bufferSize);

  inputInit();
}





void conioSetWindowHeight(int height)
{
  consoleHeight = height;
}


void conioSetWindowWidth(int width)
{
    consoleWidth = width;
}


bool mouseClickedInArea(COORD top_Left, COORD bottom_Right)
{
  if(isClicked(LMB) && 
     MouseX() >= top_Left.X &&
     MouseY() >= top_Left.Y &&
     MouseX() <= bottom_Right.X &&
     MouseY() <= bottom_Right.Y)
  {
    return true;
  }
  return false;
}

bool mouseHighlightedInArea(COORD top_Left, COORD bottom_Right)
{
  if(MouseX() >= top_Left.X &&
     MouseY() >= top_Left.Y &&
     MouseX() <= bottom_Right.X &&
     MouseY() <= bottom_Right.Y)
  {
    return true;
  }
  return false;
}

short MouseX()
{
  return MousePos.X;
}


short MouseY()
{
  return MousePos.Y;
}


bool mouseMoved()
{
  return MouseMoved;
}


bool isClicked(int button)
{
  return thisTrigger[button];
}


bool isTriggered(int key)
{
  return thisTrigger[key];
}



bool isDown(int key)
{
  return thisDown[key];
}



bool isReleased(int key)
{
  return thisRelease[key];
}


void inputReset(void)
{
  int i;

  anyKeyTriggered = false;
  MouseMoved = false;
  for(i = 0; i < NUM_KEYS; i++)
  {
    triggered[i]    = false;
    released[i]     = false;

    thisTrigger[i]  = false;
    thisRelease[i]  = false;

    lastTrigger[i]  = false;
    lastRelease[i]  = false;
  }
}


static void inputInit(void)
{
  int i;

  lastTime = 0;

  MouseMoved = false;
  MousePos.X = 0;
  MousePos.Y = 0;
  for(i = 0; i < NUM_KEYS; i++)
  {
    triggered[i]    = false;
    down[i]         = false;
    released[i]     = false;

    thisTrigger[i]  = false;
    thisDown[i]     = false;
    thisRelease[i]  = false;

    lastTrigger[i]  = false;
    lastDown[i]     = false;
    lastRelease[i]  = false;
  }

  anyKeyTriggered = false;
}



void inputUpdate(void)
{
  thisTime = timeGetTime();
  if(thisTime - lastTime >= INPUT_KEY_TIME_TOLERANCE)
  {
    lastTime = timeGetTime();
    updateKeystate();
  }
  else
    inputReset();
}



static void updateKeystate(void)
{  
  unsigned int i;
  // Create a buffer of that size to store the events
  INPUT_RECORD * eventBuffer;
  // How many events have happened?
  DWORD numEvents = 0;
  // How many events have we read from the console?
  DWORD numEventsRead = 0;
  // Find out how many console events have happened:
  GetNumberOfConsoleInputEvents(rHnd, &numEvents); 

  /* move events to "last" event list, clear this event list */
  for(i = 0; i < NUM_KEYS; i++)
  {
    lastTrigger[i] = thisTrigger[i];
    lastDown[i]    = thisDown[i];
    lastRelease[i] = thisRelease[i];

    thisTrigger[i] = false;
    thisDown[i]    = false;
    thisRelease[i] = false;
  }
  MouseMoved = false;
  anyKeyTriggered = false;

  if (numEvents > 0)
  {
    eventBuffer = (INPUT_RECORD *) malloc(sizeof(INPUT_RECORD) * numEvents);

    /* Read the console events into that buffer, and save how
       many events have been read into numEventsRead.*/
    ReadConsoleInput(rHnd, eventBuffer, numEvents, &numEventsRead);

    /* Now, cycle through all the events that have happened:*/
    for (i = 0; i < numEventsRead; ++i)
    {
      /* KEY EVENTS */
        if (eventBuffer[i].EventType==KEY_EVENT)
        {
          anyKeyTriggered = true;

          /* CHARACTER KEYS */
          if(eventBuffer[i].Event.KeyEvent.uChar.AsciiChar)
          {
            /* CONVERT IF UPPER */
            if(eventBuffer[i].Event.KeyEvent.uChar.AsciiChar > 64 && eventBuffer[i].Event.KeyEvent.uChar.AsciiChar < 91)
            {
              eventBuffer[i].Event.KeyEvent.uChar.AsciiChar += 32;
            }

            /* KEY IS DOWN */
            if(eventBuffer[i].Event.KeyEvent.bKeyDown)
            {
              /* KEY WAS TRIGGERED, or KEY WAS DOWN */
              if(lastTrigger[eventBuffer[i].Event.KeyEvent.uChar.AsciiChar] || 
                 lastDown[eventBuffer[i].Event.KeyEvent.uChar.AsciiChar])
              {
                thisDown[eventBuffer[i].Event.KeyEvent.uChar.AsciiChar] = true; /* set key to down */
              }
              /* KEY WAS NOT TRIGGERED and WAS NOT DOWN */
              else
              {
                thisTrigger[eventBuffer[i].Event.KeyEvent.uChar.AsciiChar] = true; /* set key to triggered */
              }
            
            }
            /* KEY IS UP */
            else
            {
                thisRelease[eventBuffer[i].Event.KeyEvent.uChar.AsciiChar] = true; /* set key to released */
            }
          }
          /* NON-CHARACTER KEYS */
          else if(eventBuffer[i].Event.KeyEvent.wVirtualKeyCode)
          {
            /* KEY IS DOWN */
            if(eventBuffer[i].Event.KeyEvent.bKeyDown)
            {
              /* KEY WAS TRIGGERED, or KEY WAS DOWN */
              if(lastTrigger[eventBuffer[i].Event.KeyEvent.wVirtualKeyCode] || 
                 lastDown[eventBuffer[i].Event.KeyEvent.wVirtualKeyCode])
              {
                thisDown[eventBuffer[i].Event.KeyEvent.wVirtualKeyCode] = true; /* set key to down */
              }
              /* KEY WAS NOT TRIGGERED and WAS NOT DOWN */
              else
              {
                thisTrigger[eventBuffer[i].Event.KeyEvent.wVirtualKeyCode] = true; /* set key to triggered */
              }
            }
            /* KEY IS UP */
            else
            {
                thisRelease[eventBuffer[i].Event.KeyEvent.wVirtualKeyCode] = true; /* set key to released */
            }
          }
        }
        /* MOUSE EVENTS */
        else if (eventBuffer[i].EventType==MOUSE_EVENT)
        {
          MousePos = eventBuffer[i].Event.MouseEvent.dwMousePosition;/* update mouse position */
          
          /* LEFT MB PRESSED */
          if(eventBuffer[i].Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
          {
            thisTrigger[LMB] = true;
          }
          /* MIDDLE MB PRESSED */
          else if(eventBuffer[i].Event.MouseEvent.dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
          {
            thisTrigger[MMB] = true;
          }
          /* RIGHT MB PRESSED */
          else if(eventBuffer[i].Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
          {
            thisTrigger[RMB] = true;
          }
          /* MOUSE WAS MOVED */
          else if(eventBuffer[i].Event.MouseEvent.dwEventFlags & MOUSE_MOVED)
          {
            MouseMoved = true;
          }
          
        }        
    }

    free(eventBuffer);
  }
}


bool anyTriggered()
{
  return anyKeyTriggered;
}


BOOL WINAPI SetConsoleFont(HANDLE hOutput, DWORD fontIndex) 
{
	typedef BOOL (WINAPI *PSetConsoleFont)(HANDLE, DWORD);
	static PSetConsoleFont pSetConsoleFont = NULL;

	if(pSetConsoleFont == NULL)
  {
		pSetConsoleFont = (PSetConsoleFont)GetProcAddress(GetModuleHandle(_T("kernel32")), "SetConsoleFont");
  }

	if(pSetConsoleFont == NULL) 
  {
    return FALSE;
  }

	return pSetConsoleFont(hOutput, fontIndex);
}


BOOL WINAPI GetConsoleFontInfo(HANDLE hOutput, BOOL bMaximize, DWORD fontIndex, CONSOLE_FONT* info) 
{
	typedef BOOL (WINAPI *PGetConsoleFontInfo)(HANDLE, BOOL, DWORD, CONSOLE_FONT*);
	static PGetConsoleFontInfo pGetConsoleFontInfo = NULL;

	if(pGetConsoleFontInfo == NULL)
  {
		pGetConsoleFontInfo = (PGetConsoleFontInfo)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetConsoleFontInfo");
  }

  if(pGetConsoleFontInfo == NULL) 
  {
    return FALSE;
  }

	return pGetConsoleFontInfo(hOutput, bMaximize, fontIndex, info);
}


DWORD WINAPI GetNumberOfConsoleFonts() 
{
	typedef DWORD (WINAPI *PGetNumberOfConsoleFonts)();
	static PGetNumberOfConsoleFonts pGetNumberOfConsoleFonts = NULL;

	if(pGetNumberOfConsoleFonts == NULL)
  {
		pGetNumberOfConsoleFonts = (PGetNumberOfConsoleFonts)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetNumberOfConsoleFonts");
  }

	if(pGetNumberOfConsoleFonts == NULL) 
  {
    return 0;
  }
  
  return pGetNumberOfConsoleFonts();
}


BOOL WINAPI SetConsoleIcon(HICON hIcon) 
{
	typedef BOOL (WINAPI *PSetConsoleIcon)(HICON);
	static PSetConsoleIcon pSetConsoleIcon = NULL;

	if(pSetConsoleIcon == NULL)
  {
		pSetConsoleIcon = (PSetConsoleIcon)GetProcAddress(GetModuleHandle(_T("kernel32")), "SetConsoleIcon");
  }

	if(pSetConsoleIcon == NULL) 
  {
    return FALSE;
  }

	return pSetConsoleIcon(hIcon);
}
