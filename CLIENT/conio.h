/*! ============================================================================
  @file conio.h

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
#ifndef CONIO_H
#define CONIO_H 

#include <windows.h>     

void conioInit();                       
bool conioVirtualKeyPressed(WORD KEY); 
bool conioAsciiKeyPressed(char KEY);     
void conioSetWindowHeight(int height); 
void conioSetWindowWidth(int width);       
void conioCmdLineArg(char * arg);         

void inputUpdate(void);                 
bool anyTriggered();                    
void inputReset(void);
short MouseX();                          
short MouseY();                         
bool mouseMoved();                 
bool isClicked(int button);          
bool isTriggered(int key);              
bool isDown(int key);              
bool isReleased(int key);            

bool mouseClickedInArea(COORD top_Left, 
                        COORD bottom_Right); 
bool mouseHighlightedInArea(COORD top_Left,
                        COORD bottom_Right);


#pragma once

typedef struct _CONSOLE_FONT 
{
	 DWORD index;
	 COORD dim;
} CONSOLE_FONT;

BOOL WINAPI SetConsoleFont(HANDLE hOutput, DWORD fontIndex);
BOOL WINAPI GetConsoleFontInfo(HANDLE hOutput, BOOL bMaximize, DWORD numFonts, CONSOLE_FONT* info);
DWORD WINAPI GetNumberOfConsoleFonts();
BOOL WINAPI SetConsoleIcon(HICON hIcon);

/* 
 * Short-cut KEYWORDS for non-ascii keys on the keyboard, which 
 * can be passed as arguments to the key event getter functions
 */
enum _INPUT_KEY
{
  LMB = 0x01,
  RMB = 0x02,
  MMB = 0x04,
  BACKSPACE = 0x08,
  TAB = 0x09,
  ENTER = 0x0D,
  SHIFT = 0x10,
  CTRL = 0x11,
  ALT = 0x12,
  PAUSE = 0x13,
  CAPSLOCK = 0x14,
  ESC = 0x1B,
  SPACE = 0x20,
  PAGEUP,
  PAGEDOWN,
  END,
  HOME,
  LEFT,
  UP,
  RIGHT,
  DOWN,
  SELECT,
  PRINT,
  EXECUTE,
  PRINTSCREEN,
  INSERT,
  DEL,
  HELP
};

#endif