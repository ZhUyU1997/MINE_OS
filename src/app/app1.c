/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : Reversi.c
Purpose     : Simple 'reversi' game.
----------------------------------------------------------------------
*/

#include <string.h>
#include <stdio.h>

#include "GUI.h"
#include "DIALOG.h"
#include "MENU.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define NUM_CELL_X        8
#define NUM_CELL_Y        8

#define AA_USE_HIRES      0
#define AA_FACTOR         1

#if AA_USE_HIRES
#define AA_CALCFACTOR   AA_FACTOR
#else
#define AA_CALCFACTOR   1
#endif

#define DESKTOP_COLOR     GUI_DARKGREEN
#define CLIENT_COLOR      GUI_DARKGRAY
#define GRID_COLOR        GUI_BROWN

#define ID_MENU_NEW       (GUI_ID_USER +  0)
#define ID_MENU_PASS      (GUI_ID_USER +  1)
#define ID_MENU_EXIT      (GUI_ID_USER +  2)
#define ID_MENU_SETTINGS  (GUI_ID_USER +  3)
#define ID_MENU_ABOUT     (GUI_ID_USER +  4)

#define ID_MENU_TEST      (GUI_ID_USER +  5)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
	U8  aCells[NUM_CELL_X][NUM_CELL_Y];
	U8  aMoves[NUM_CELL_X][NUM_CELL_Y];
	int ActPlayer;
} BOARD;

typedef char REVERSI_AI_Func(const BOARD* pBoard, int* px, int* py);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/* Global data */
static WM_HWIN  _hContextMenu;
static WM_HWIN  _hFrame;

static REVERSI_AI_Func* _pPlayerAI[2];

static BOARD  _Board;
static int    _GameOver;
static int    _BoardX0;
static int    _BoardY0;
static int    _CellSize;

/* Game settings */
static int      _ShowPossibleMoves = 1;

/*********************************************************************
*
*       Static data, Micrium logo (65% size)
*
**********************************************************************
*/

/*   Palette
The following are the entries of the palette table.
Every entry is a 32-bit value (of which 24 bits are actually used)
the lower   8 bits represent the Red component,
the middle  8 bits represent the Green component,
the highest 8 bits (of the 24 bits used) represent the Blue component
as follows:   0xBBGGRR
*/

static GUI_CONST_STORAGE GUI_COLOR ColorsMicriumLogo[] = {
	0x000000, 0x000080, 0x008000, 0x008080
	, 0x800000, 0x800080, 0x808000, 0x808080
	, 0xC0C0C0, 0x0000FF, 0x00FF00, 0x00FFFF
	, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalMicriumLogo = {
	16,	/* number of entries */
	0, 	/* No transparency */
	&ColorsMicriumLogo[0]
};

static GUI_CONST_STORAGE unsigned char acMicriumLogo[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xF0, 0x00, 0x00, 0x7F, 0xFF, 0xF8, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x70, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xF0, 0x00, 0x00, 0x08, 0xFF, 0xF7, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xF0, 0x00, 0x00, 0x07, 0xFF, 0xF0, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x70, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xF0, 0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0x07, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xF0, 0x00, 0x00, 0x00, 0xFF, 0x70, 0x07, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x08, 0x00, 0x07, 0x70, 0x00, 0xFF, 0xF0, 0x00, 0x7F, 0xF8, 0x00, 0x08, 0xFF, 0x00, 0x07, 0x87, 0x00, 0x07, 0xF8, 0x00,
	0x07, 0xFF, 0xF0,
	0x0F, 0xF0, 0x00, 0x07, 0x00, 0xFF, 0x00, 0x08, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF8, 0x00, 0x07, 0xFF, 0x00, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00,
	0x00, 0x7F, 0xF0,
	0x0F, 0xF0, 0x00, 0x08, 0x00, 0x8F, 0x00, 0x0F, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xF0, 0x00, 0x07, 0x70, 0x00, 0x08, 0xFF, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x7F, 0xF8, 0x00, 0x07, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF0, 0x00, 0x0F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0x70, 0x00, 0x0F, 0xF7, 0x00, 0x07, 0xFF, 0x00, 0x00, 0x78, 0x8F, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x7F, 0xF8, 0x00, 0x07, 0xFF, 0x00, 0x00, 0x78, 0x00, 0x00, 0x07, 0x80,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF0, 0x00, 0x0F, 0x00, 0x07, 0x00, 0x8F, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x7F, 0xF8, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x7F, 0xF0,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x7F, 0xF8, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x7F, 0xF0,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF0, 0x00, 0x0F, 0x70, 0x00, 0x00, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x88, 0xFF, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF7, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x7F, 0xF0,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF0, 0x00, 0x0F, 0x70, 0x00, 0x00, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0x70, 0x00, 0x7F, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x7F, 0xF0,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF0, 0x00, 0x0F, 0xF7, 0x00, 0x08, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x7F, 0xF0,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF0, 0x00, 0x0F, 0xF7, 0x00, 0x08, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0x00, 0x07, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x7F, 0xF0,
	0x00, 0x0F, 0xF0,
	0x0F, 0xF7, 0x77, 0x7F, 0xFF, 0x77, 0x7F, 0xFF, 0x77, 0x77, 0xFF, 0x77, 0x77, 0xFF, 0xFF, 0xF7, 0x70, 0x07, 0x7F, 0xFF, 0xFF, 0x77, 0x77, 0xFF, 0xFF, 0x77, 0x77, 0xFF, 0xF0, 0x00, 0x77, 0x08, 0x77, 0x78, 0xFF, 0x77, 0x77, 0xFF, 0x87, 0x77, 0x8F, 0xF7,
	0x77, 0x7F, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x77, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xF0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00
};

GUI_CONST_STORAGE GUI_BITMAP _Logo = {
	90, /* XSize */
	42, /* YSize */
	45, /* BytesPerLine */
	4, /* BitsPerPixel */
	acMicriumLogo,  /* Pointer to picture data (indices) */
	&PalMicriumLogo  /* Pointer to palette */
};


/*********************************************************************
*
*       Function declarations
*
**********************************************************************
*/

static void _StartNewGame(void);

/*********************************************************************
*
*       Static code, player AIs
*
**********************************************************************
*/
/*********************************************************************
*
*       _PlayerAI_FirstValid
*/
static char _PlayerAI_FirstValid(const BOARD* pBoard, int* px, int* py) {
	int x, y;
	for (y = 0; y < NUM_CELL_Y; y++) {
		for (x = 0; x < NUM_CELL_X; x++) {
			if (pBoard->aMoves[x][y]) {
				goto End;
			}
		}
	}
End:
	*px = x;
	*py = y;
	return 1;
}

/*********************************************************************
*
*       Static code, helper functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _Delay
*/
static void _Delay(int Period) {
	int EndTime = GUI_GetTime() + Period;
	int tRem; /* remaining Time */
	while (tRem = EndTime - GUI_GetTime(), tRem > 0) {
		GUI_Exec();
		GUI_X_Delay((tRem > 5) ? 5 : tRem);
	}
}

/*******************************************************************
*
*       _AddMenuItem
*/
static void _AddMenuItem(MENU_Handle hMenu, MENU_Handle hSubmenu, const char* pText, U16 Id, U16 Flags) {
	MENU_ITEM_DATA Item;
	Item.pText    = pText;
	Item.hSubmenu = hSubmenu;
	Item.Flags    = Flags;
	Item.Id       = Id;
	MENU_AddItem(hMenu, &Item);
}

/*******************************************************************
*
*       _CreateMenu
*/
static void _CreateMenu(WM_HWIN hWin) {
	MENU_Handle hMenu, hMenuGame, hMenuOptions, hMenuHelp;
	MENU_SetDefaultFont(&GUI_Font10_1);
	/* Create menu 'Game' */
	hMenuGame = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	_AddMenuItem(hMenuGame, 0, "New game", ID_MENU_NEW,  0);
	_AddMenuItem(hMenuGame, 0, "Pass",     ID_MENU_PASS, 0);
	_AddMenuItem(hMenuGame, 0, 0,          0,            MENU_IF_SEPARATOR);
	_AddMenuItem(hMenuGame, 0, "Exit",     ID_MENU_EXIT, 0);
	/* Create menu 'Options' */
	hMenuOptions = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	_AddMenuItem(hMenuOptions, 0, "Game settings...", ID_MENU_SETTINGS, 0);
	/* Create menu 'Help' */
	hMenuHelp = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	_AddMenuItem(hMenuHelp, 0, "About Reversi...", ID_MENU_ABOUT, 0);
	/* Create main menu */
	hMenu = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_HORIZONTAL, 0);
	_AddMenuItem(hMenu, hMenuGame,    "Game",    0, 0);
	_AddMenuItem(hMenu, hMenuOptions, "Options", 0, 0);
	_AddMenuItem(hMenu, hMenuHelp,    "Help",    0, 0);
	/* Attach menu to framewin */
	FRAMEWIN_AddMenu(hWin, hMenu);
}

/*******************************************************************
*
*       _CalcBoardDimensions
*/
static void _CalcBoardDimensions(void) {
	GUI_RECT r;
	WM_GetClientRectEx(WM_GetClientWindow(_hFrame), &r);
	_CellSize  = ((r.x1 > r.y1) ? r.y1 : r.x1) >> 3;
	_BoardX0   = (r.x1 - (_CellSize << 3)) >> 1;
	_BoardY0   = (r.y1 - (_CellSize << 3)) >> 1;
}

/*******************************************************************
*
*       _InvalidateBoard
*/
static void _InvalidateBoard(void) {
	WM_InvalidateWindow(WM_GetClientWindow(_hFrame));
}

/*******************************************************************
*
*       _InvalidateCell
*/
static void _InvalidateCell(int x, int y) {
	GUI_RECT r;
	r.x0 = _BoardX0 + (x * _CellSize);
	r.y0 = _BoardY0 + (y * _CellSize);
	r.x1 = r.x0 + _CellSize - 1;
	r.y1 = r.y0 + _CellSize - 1;
	WM_InvalidateRect(WM_GetClientWindow(_hFrame), &r);
}

/*********************************************************************
*
*       _SetCapture
*/
static void _SetCapture(void) {
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
	WM_HWIN hWin;
	hWin = WM_GetClientWindow(_hFrame);
	if (WM_HasCaptured(hWin) == 0) {
		WM_SetCapture(hWin, 0);
		GUI_CURSOR_Select(&GUI_CursorCrossS);
	}
#endif
}

/*********************************************************************
*
*       _ReleaseCapture
*/
static void _ReleaseCapture(void) {
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
	WM_HWIN hWin;
	hWin = WM_GetClientWindow(_hFrame);
	if (WM_HasCaptured(hWin)) {
		WM_ReleaseCapture();
		GUI_CURSOR_Select(&GUI_CursorArrowM);
	}
#endif
}

/*********************************************************************
*
*       Static code, game API routines
*
**********************************************************************
*/
/*******************************************************************
*
*       _GetStone
*/
static char _GetStone(const BOARD* pBoard, int x, int y) {
	char r = 0;
	if ((x >= 0) && (y >= 0) && (x < NUM_CELL_X) && (y < NUM_CELL_Y)) {
		r = pBoard->aCells[x][y];
	}
	return r;
}

/*******************************************************************
*
*       _SetStone
*/
static void _SetStone(BOARD* pBoard, int x, int y) {
	if ((x >= 0) && (y >= 0) && (x < NUM_CELL_X) && (y < NUM_CELL_Y)) {
		pBoard->aCells[x][y] = pBoard->ActPlayer;
		_InvalidateCell(x, y);
	}
}

/*******************************************************************
*
*       _IsValidMove
*/
static char _IsValidMove(BOARD* pBoard, int x, int y) {
	char r = 0;
	if ((x >= 0) && (y >= 0) && (x < NUM_CELL_X) && (y < NUM_CELL_Y)) {
		r = ((pBoard->aMoves[x][y]) ? 1 : 0);
	}
	return r;
}

/*******************************************************************
*
*       _GetNextPlayer
*/
static int _GetNextPlayer(int ActPlayer) {
	return (3 - ActPlayer);
}

/*******************************************************************
*
*       _CheckDirection
*/
static char _CheckDirection(const BOARD* pBoard, int x, int y, int dx, int dy) {
	char Cell;
	x += dx;
	y += dy;
	Cell = _GetStone(pBoard, x, y);
	if ((Cell != pBoard->ActPlayer) && (Cell != 0)) {
		do {
			x += dx;
			y += dy;
			Cell = _GetStone(pBoard, x, y);
		} while ((Cell != pBoard->ActPlayer) && (Cell != 0));
		return ((Cell == pBoard->ActPlayer) ? 1 : 0);
	}
	return 0;
}

/*******************************************************************
*
*       _CalcValidMoves
*/
static int _CalcValidMoves(BOARD* pBoard) {
	int x, y, r = 0;
	U8 Valid;
	for (y = 0; y < NUM_CELL_Y; y++) {
		for (x = 0; x < NUM_CELL_X; x++) {
			Valid = 0;
			if (pBoard->aCells[x][y] == 0) {
				Valid |= _CheckDirection(pBoard, x, y, -1, -1) << 0;
				Valid |= _CheckDirection(pBoard, x, y,  0, -1) << 1;
				Valid |= _CheckDirection(pBoard, x, y,  1, -1) << 2;
				Valid |= _CheckDirection(pBoard, x, y,  1,  0) << 3;
				Valid |= _CheckDirection(pBoard, x, y,  1,  1) << 4;
				Valid |= _CheckDirection(pBoard, x, y,  0,  1) << 5;
				Valid |= _CheckDirection(pBoard, x, y, -1,  1) << 6;
				Valid |= _CheckDirection(pBoard, x, y, -1,  0) << 7;
				if (Valid) {
					r++;
				}
			}
			if (Valid != pBoard->aMoves[x][y]) {
				pBoard->aMoves[x][y] = Valid;
				_InvalidateCell(x, y);
			}
		}
	}
	return r;
}

/*******************************************************************
*
*       _DoDirection
*/
static void _DoDirection(BOARD* pBoard, int x, int y, int dx, int dy) {
	do {
		_SetStone(pBoard, x, y);
		x += dx;
		y += dy;
	} while (_GetStone(pBoard, x, y) != pBoard->ActPlayer);
}

/*******************************************************************
*
*       _MakeMove
*/
static void _MakeMove(BOARD* pBoard, int x, int y) {
	U8 Valid;
	_SetStone(pBoard, x, y);
	_Delay(100);
	Valid = pBoard->aMoves[x][y];
	if (Valid & (U8)(1 << 0)) {
		_DoDirection(pBoard, x, y, -1, -1);
	}
	if (Valid & (U8)(1 << 1)) {
		_DoDirection(pBoard, x, y,  0, -1);
	}
	if (Valid & (U8)(1 << 2)) {
		_DoDirection(pBoard, x, y,  1, -1);
	}
	if (Valid & (U8)(1 << 3)) {
		_DoDirection(pBoard, x, y,  1,  0);
	}
	if (Valid & (U8)(1 << 4)) {
		_DoDirection(pBoard, x, y,  1,  1);
	}
	if (Valid & (U8)(1 << 5)) {
		_DoDirection(pBoard, x, y,  0,  1);
	}
	if (Valid & (U8)(1 << 6)) {
		_DoDirection(pBoard, x, y, -1,  1);
	}
	if (Valid & (U8)(1 << 7)) {
		_DoDirection(pBoard, x, y, -1,  0);
	}
}

/*******************************************************************
*
*       _CalcScore
*/
static int _CalcScore(const BOARD* pBoard) {
	int x, y, r = 0;
	char Cell;
	for (y = 0; y < NUM_CELL_Y; y++) {
		for (x = 0; x < NUM_CELL_X; x++) {
			Cell = pBoard->aCells[x][y];
			if (Cell) {
				r += (Cell == pBoard->ActPlayer) ? (1) : (-1);
			}
		}
	}
	return r;
}





/*********************************************************************
*
*       Static code, about box
*
**********************************************************************
*/
/*******************************************************************
*
*       _cbAboutBox
*/
static void _cbAboutBox(WM_MESSAGE* pMsg) {
	WM_HWIN hWin = pMsg->hWin;
	switch (pMsg->MsgId) {
		case WM_PAINT: {
			char acText[16] = "V";
			strcat(acText, GUI_GetVersionString());
			GUI_SetColor(GUI_BLACK);
			GUI_SetFont(&GUI_Font10_1);
			GUI_SetTextMode(GUI_TM_TRANS);
			GUI_DrawBitmap(&_Logo, 4, 4);
			GUI_DispStringHCenterAt("Reversi V1.0", 49, 48);
			GUI_DispStringHCenterAt("µC/GUI", 138, 38);
			GUI_DispStringHCenterAt(acText,  138, 48);
			GUI_DispStringHCenterAt("Compiled " __DATE__ " "__TIME__, 88, 68);
			GUI_DispStringHCenterAt("(c) 1998-2005", 88, 87);
			GUI_DispStringHCenterAt("Micrium", 88, 97);
			GUI_DispStringHCenterAt("www.micrium.com", 88, 107);
			GUI_DispStringHCenterAt("Programmed by Tobias Quecke", 88, 126);
		}
		break;
		case WM_NOTIFY_PARENT:
			if (pMsg->Data.v == WM_NOTIFICATION_RELEASED) {
				GUI_EndDialog(hWin, 1);
			}
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}

/*********************************************************************
*
*       _ShowAboutBox
*/
static void _ShowAboutBox(void) {
	WM_HWIN hFrame, hItem;
	/* Create framewin */
	hFrame = FRAMEWIN_CreateEx(70, 40, 180, 160, WM_HBKWIN, WM_CF_SHOW, FRAMEWIN_CF_MOVEABLE, 0, "About Reversi", &_cbAboutBox);
	FRAMEWIN_SetClientColor   (hFrame, GUI_WHITE);
	FRAMEWIN_SetFont          (hFrame, &GUI_Font10_1);
	FRAMEWIN_SetTextAlign     (hFrame, GUI_TA_HCENTER);
	/* Create dialog items */
	hItem = BUTTON_CreateEx(111, 7, 55, 18, WM_GetClientWindow(hFrame), WM_CF_SHOW, 0, GUI_ID_OK);
	BUTTON_SetText         (hItem, "Ok");
	/* Exec modal dialog */
	WM_SetFocus(hFrame);
	WM_MakeModal(hFrame);
	GUI_ExecCreatedDialog(hFrame);
	WM_SetFocus(_hFrame);
}

/*********************************************************************
*
*       Static code, message box
*
**********************************************************************
*/
/*******************************************************************
*
*       _cbMessageBox
*/
static void _cbMessageBox(WM_MESSAGE* pMsg) {
	WM_HWIN hWin = pMsg->hWin;
	switch (pMsg->MsgId) {
		case WM_NOTIFY_PARENT:
			if (pMsg->Data.v == WM_NOTIFICATION_RELEASED) {
				int Id = WM_GetId(pMsg->hWinSrc);
				GUI_EndDialog(hWin, (Id == GUI_ID_OK) ? 1 : 0);
			}
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}

/*********************************************************************
*
*       _ShowMessageBox
*/
static int _ShowMessageBox(const char* pTitle, const char* pText, int YesNo) {
	WM_HWIN hFrame, hClient, hBut;
	int r;
	/* Create framewin */
	hFrame = FRAMEWIN_CreateEx(65, 75, 190, 90, WM_HBKWIN, WM_CF_SHOW, FRAMEWIN_CF_MOVEABLE, 0, pTitle, &_cbMessageBox);
	FRAMEWIN_SetClientColor   (hFrame, GUI_WHITE);
	FRAMEWIN_SetFont          (hFrame, &GUI_Font10_1);
	FRAMEWIN_SetTextAlign     (hFrame, GUI_TA_HCENTER);
	/* Create dialog items */
	hClient = WM_GetClientWindow(hFrame);
	TEXT_CreateEx(10, 7, 170, 30, hClient, WM_CF_SHOW, GUI_TA_HCENTER, 0, pText);
	if (YesNo) {
		hBut = BUTTON_CreateEx(97, 45, 55, 18, hClient, WM_CF_SHOW, 0, GUI_ID_CANCEL);
		BUTTON_SetText        (hBut, "No");
		hBut = BUTTON_CreateEx(32, 45, 55, 18, hClient, WM_CF_SHOW, 0, GUI_ID_OK);
		BUTTON_SetText        (hBut, "Yes");
	} else {
		hBut = BUTTON_CreateEx(64, 45, 55, 18, hClient, WM_CF_SHOW, 0, GUI_ID_OK);
		BUTTON_SetText        (hBut, "Ok");
	}
	/* Exec modal dialog */
	WM_SetFocus(hFrame);
	WM_MakeModal(hFrame);
	r = GUI_ExecCreatedDialog(hFrame);
	WM_SetFocus(_hFrame);
	return r;
}

/*********************************************************************
*
*       Static code, dialog settings
*
**********************************************************************
*/
/*******************************************************************
*
*       _cbDialogSettings
*/
static void _cbDialogSettings(WM_MESSAGE* pMsg) {
	WM_HWIN hWin = pMsg->hWin;
	switch (pMsg->MsgId) {
		case WM_NOTIFY_PARENT:
			if (pMsg->Data.v == WM_NOTIFICATION_RELEASED) {
				WM_HWIN hItem = pMsg->hWinSrc;
				switch (WM_GetId(hItem)) {
					case GUI_ID_OK:
						GUI_EndDialog(hWin, 1);
						break;
					case GUI_ID_CHECK0:
						_ShowPossibleMoves = CHECKBOX_GetState(hItem);
						_InvalidateBoard();
						break;
				}
			}
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}

/*********************************************************************
*
*       _ShowDialogSettings
*/
static void _ShowDialogSettings(void) {
	WM_HWIN hFrame, hClient, hItem;
	/* Create framewin */
	hFrame = FRAMEWIN_CreateEx(70, 75, 180, 90, WM_HBKWIN, WM_CF_SHOW, FRAMEWIN_CF_MOVEABLE, 0, "Game settings", &_cbDialogSettings);
	FRAMEWIN_SetClientColor   (hFrame, GUI_WHITE);
	FRAMEWIN_SetFont          (hFrame, &GUI_Font10_1);
	FRAMEWIN_SetTextAlign     (hFrame, GUI_TA_HCENTER);
	/* Create dialog items */
	hClient = WM_GetClientWindow(hFrame);
	/* Create button */
	hItem = BUTTON_CreateEx(59, 46, 55, 18, hClient, WM_CF_SHOW, 0, GUI_ID_OK);
	BUTTON_SetText         (hItem, "Ok");
	/* Create checkbox */
	hItem = CHECKBOX_CreateEx(10, 10, 140, 0, hClient, WM_CF_SHOW, 0, GUI_ID_CHECK0);
	CHECKBOX_SetText         (hItem, "Show possible moves");
	CHECKBOX_SetBkColor      (hItem, GUI_INVALID_COLOR);
	CHECKBOX_SetState        (hItem, _ShowPossibleMoves);
	/* Exec modal dialog */
	WM_SetFocus(hFrame);
	WM_MakeModal(hFrame);
	GUI_ExecCreatedDialog(hFrame);
	WM_SetFocus(_hFrame);
}

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*******************************************************************
*
*       _SetPlayer
*/
static void _SetPlayer(int Player) {
	int PossibleMoves;
	_Board.ActPlayer = Player;
	if (Player == 1) {
		FRAMEWIN_SetText(_hFrame, "Reversi - Player 1");
	} else {
		FRAMEWIN_SetText(_hFrame, "Reversi - Player 2");
	}
	FRAMEWIN_SetBarColor(_hFrame, 1, (Player == 1) ? GUI_RED : GUI_BLUE);
	PossibleMoves = _CalcValidMoves(&_Board);
	GUI_Exec();

	if (!PossibleMoves) {
		int ValidMoves;
		GUI_Exec();
		_Board.ActPlayer = 3 - Player;
		ValidMoves = _CalcValidMoves(&_Board);
		_Board.ActPlayer = Player;
		_CalcValidMoves(&_Board);
		if (ValidMoves) {   /* No valid moves, player must pass */
			if (_pPlayerAI[_Board.ActPlayer - 1] == NULL) {
				_ShowMessageBox("Reversi", "No possible moves.\nYou have to pass!", 0);
			} else {
				/* Pass ai player */
			}
			_SetPlayer(3 - _Board.ActPlayer);
		} else {            /* No valid moves for all players, game is over */
			char ac[256];
			int Score;
			_GameOver = 1;
			_Board.ActPlayer = 1;
			Score = _CalcScore(&_Board);
			if (Score > 0) {
				sprintf(ac, "Red wins by %d stones!\nDo you want to start a new game?", Score);
			} else if (Score) {
				sprintf(ac, "Blue wins by %d stones!\nDo you want to start a new game?", -Score);
			} else {
				strcpy(ac, "The game ends in a draw!\nDo you want to start a new game?");
			}
			if (_ShowMessageBox("Reversi", ac, 1)) {
				_StartNewGame();
			}
		}
	}
}

/*******************************************************************
*
*       _NextPlayer
*/
static void _NextPlayer(void) {
	do {
		_SetPlayer(3 - _Board.ActPlayer);
		if (_pPlayerAI[_Board.ActPlayer - 1] && !_GameOver) {
			int x, y;
			char DoMove;
			DoMove = (*_pPlayerAI[_Board.ActPlayer - 1])(&_Board, &x, &y);
			_Delay(100);
			if (DoMove) {
				_MakeMove(&_Board, x, y);
			}
		}
	} while (_pPlayerAI[_Board.ActPlayer - 1] && !_GameOver);
}

/*******************************************************************
*
*       _StartNewGame
*/
static void _StartNewGame(void) {
	memset(&_Board, 0, sizeof(BOARD));
	_Board.aCells[3][3] = 1;
	_Board.aCells[4][4] = 1;
	_Board.aCells[3][4] = 2;
	_Board.aCells[4][3] = 2;
	_GameOver    = 0;
	_SetPlayer(1);
	_InvalidateBoard();
}

/*******************************************************************
*
*       _HandlePID
*/
static void _HandlePID(int x, int y, int Pressed) {
	static int _IsInHandlePID = 0;
	if (_IsInHandlePID++ == 0) {
		_CalcBoardDimensions();
		x -= _BoardX0;
		y -= _BoardY0;
		if ((x >= 0) && (y >= 0)) {
			x /= _CellSize;
			y /= _CellSize;
			if ((x < 8) && (y < 8)) {
				if (_IsValidMove(&_Board, x, y)) {
					if (Pressed == 0) {
						_ReleaseCapture();
						_MakeMove(&_Board, x, y);
						_NextPlayer();
					} else {
						_SetCapture();
					}
					goto EndHandlePID;
				}
			}
		}
		_ReleaseCapture();
	}
EndHandlePID:
	_IsInHandlePID--;
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(WM_MESSAGE* pMsg) {
	const GUI_PID_STATE* pState = (const GUI_PID_STATE*)pMsg->Data.p;
	if (pState) {  /* Something happened in our area (pressed or released) */
		_HandlePID(pState->x, pState->y, pState->Pressed);
	}
}

/*********************************************************************
*
*       _OnMouseOver
*/
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
static void _OnMouseOver(WM_MESSAGE* pMsg) {
	const GUI_PID_STATE* pState = (const GUI_PID_STATE *)pMsg->Data.p;
	if (pState) {
		_HandlePID(pState->x, pState->y, -1);
	}
}
#endif

/*******************************************************************
*
*       _OnMenu
*/
static void _OnMenu(WM_MESSAGE* pMsg) {
	MENU_MSG_DATA* pData = (MENU_MSG_DATA*)pMsg->Data.p;
	MENU_Handle    hMenu = pMsg->hWinSrc;
	switch (pData->MsgType) {
		case MENU_ON_INITMENU:
			if (_GameOver) {
				MENU_DisableItem(hMenu, ID_MENU_PASS);
			} else {
				MENU_EnableItem(hMenu, ID_MENU_PASS);
			}
			break;
		case MENU_ON_ITEMSELECT:
			switch (pData->ItemId) {
				case ID_MENU_NEW:
					_StartNewGame();
					break;
				case ID_MENU_PASS:
					_NextPlayer();
					break;
				case ID_MENU_EXIT:
					WM_DeleteWindow(_hFrame);
					break;
				case ID_MENU_SETTINGS:
					_ShowDialogSettings();
					break;
				case ID_MENU_ABOUT:
					_ShowAboutBox();
					break;
				case ID_MENU_TEST:
					WM_ReleaseCapture();
					WM_DetachWindow(_hContextMenu);
					break;
			}
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}

/*******************************************************************
*
*       _OnPaint
*/
static void _OnPaint(WM_HWIN hWin) {
	GUI_COLOR Color;
	GUI_RECT  r;
	int x, y, xPos, yPos;
	int CellSize, rStone, rMove;
	char Cell, IsValidMove;
	_CalcBoardDimensions();
	GUI_AA_SetFactor(AA_FACTOR);
#if AA_USE_HIRES
	GUI_AA_EnableHiRes();
#endif
	LCD_SetBkColor(CLIENT_COLOR);
	WM_GetClientRectEx(hWin, &r);
	GUI_ClearRect(r.x0, r.y0, r.x1, _BoardY0 - 1);
	GUI_ClearRect(r.x0, _BoardY0, _BoardX0 - 1, _BoardY0 + (8 * _CellSize));
	GUI_ClearRect(_BoardX0 + (8 * _CellSize) + 1, _BoardY0, r.x1, _BoardY0 + (8 * _CellSize));
	GUI_ClearRect(r.x0, _BoardY0 + (8 * _CellSize) + 1, r.x1, r.y1);
	CellSize = _CellSize - 1;
	rStone = ((CellSize - 3) * AA_CALCFACTOR) >> 1;
	rMove  = ((CellSize - 2) * AA_CALCFACTOR) >> 3;
	if (rStone < AA_CALCFACTOR) {
		rStone = AA_CALCFACTOR;
	}
	if (rMove < (AA_CALCFACTOR >> AA_USE_HIRES)) {
		rMove = (AA_CALCFACTOR >> AA_USE_HIRES);
	}
	for (yPos = _BoardY0, y = 0; y < 8; y++) {
		for (xPos = _BoardX0, x = 0; x < 8; x++) {
			Color = ((x + (y & 1)) & 1) ? (GUI_LIGHTGRAY) : (GUI_WHITE);
			LCD_SetColor(Color);
			GUI_FillRect(xPos + 1, yPos + 1, xPos + CellSize, yPos + CellSize);
			Cell        = _GetStone(&_Board, x, y);
			IsValidMove = (_ShowPossibleMoves) ? _IsValidMove(&_Board, x, y) : 0;
			if (_pPlayerAI[_Board.ActPlayer - 1]) {
				IsValidMove = 0;
			}
			if (Cell || IsValidMove) {
				int xCircle, yCircle;
				xCircle = (xPos + 1) * AA_CALCFACTOR + ((CellSize * AA_CALCFACTOR) >> 1);
				yCircle = (yPos + 1) * AA_CALCFACTOR + ((CellSize * AA_CALCFACTOR) >> 1);
				if (Cell) {
					Color = (Cell == 1) ? (GUI_RED) : (GUI_BLUE);
					LCD_SetColor(Color);
#if (AA_FACTOR > 1)
					GUI_AA_FillCircle(xCircle, yCircle, rStone);
#else
					GUI_FillCircle(xCircle, yCircle, rStone);
#endif
				} else {
					LCD_SetColor(GUI_BLACK);
#if (AA_FACTOR > 1)
					GUI_AA_FillCircle(xCircle, yCircle, rMove);
#else
					GUI_FillCircle(xCircle, yCircle, rMove);
#endif
				}
			}
			LCD_SetColor(GRID_COLOR);
			GUI_DrawVLine(xPos, yPos + 1, yPos + CellSize);
			xPos += _CellSize;
		}
		GUI_DrawVLine(xPos, yPos + 1, yPos + CellSize);
		GUI_DrawHLine(yPos, _BoardX0, _BoardX0 + (_CellSize << 3));
		yPos += _CellSize;
	}
	GUI_DrawHLine(yPos, _BoardX0, _BoardX0 + (_CellSize << 3));
}

/*******************************************************************
*
*       _cbReversiWin
*/
static void _cbReversiWin(WM_MESSAGE* pMsg) {
	WM_HWIN hWin = pMsg->hWin;
	switch (pMsg->MsgId) {
		case WM_PAINT:
			_OnPaint(hWin);
			break;
		case WM_TOUCH:
			_OnTouch(pMsg);
			break;
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
		case WM_MOUSEOVER:
			_OnMouseOver(pMsg);
			break;
#endif
		case WM_MENU:
			_OnMenu(pMsg);
			break;
		case WM_DELETE:
			_hFrame = 0;
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}

/*********************************************************************
*
*       _StartReversi
*/
static void _StartReversi(void) {
	_hFrame = FRAMEWIN_CreateEx(64, 11, 191, 218, WM_HBKWIN, WM_CF_SHOW, FRAMEWIN_CF_MOVEABLE, 0, NULL, &_cbReversiWin);
	FRAMEWIN_SetClientColor(_hFrame, GUI_INVALID_COLOR);
	FRAMEWIN_SetFont       (_hFrame, &GUI_Font10_1);
	FRAMEWIN_SetTextAlign  (_hFrame, GUI_TA_HCENTER);
	FRAMEWIN_AddCloseButton(_hFrame, FRAMEWIN_BUTTON_LEFT,  0);
	FRAMEWIN_AddMaxButton  (_hFrame, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMinButton  (_hFrame, FRAMEWIN_BUTTON_RIGHT, 1);
	FRAMEWIN_SetResizeable (_hFrame, 1);
	_CreateMenu(_hFrame);
	_StartNewGame();
	WM_SetFocus(_hFrame);
	while (_hFrame) {
		GUI_Delay(500);
	}
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#if 1

/*********************************************************************
*
*       _GetNearValue
*/
static int _GetNearValue(int x, int y) {
	int r = 0xFF;
	if ((x >= 0) && (y >= 0)) {
		int x1, y1;
		x1 = 7 - x;
		y1 = 7 - y;
		x  = (x < x1) ? x : x1;
		y  = (y < y1) ? y : y1;
		r  = (x < y)  ? x : y;
	}
	return r;
}

/*********************************************************************
*
*       _PlayerAI_NearToBorder
*/
static char _PlayerAI_NearToBorder(U16 Board[8][8], U16 Flags, int Player, int* px, int* py) {
	int x, y, xNear, yNear;
	xNear = -1;
	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			if (((y == 1) || (y == 6)) && ((x != 0) && (x != 7))) {
				continue;
			}
			if (((x == 1) || (x == 6)) && ((y != 0) && (y != 7))) {
				continue;
			}
			if (Board[x][y] & 0xFF00) {
				if (_GetNearValue(x, y) < _GetNearValue(xNear, yNear)) {
					xNear = x;
					yNear = y;
				}
			}
		}
	}
	if (xNear >= 0) {
		*px = xNear;
		*py = yNear;
	} else {
		return _PlayerAI_FirstValid(&_Board, px, py);
	}
	return 1;
}
#endif

/*********************************************************************
*
*       MainTask
*/
void MainTask1(void);
void MainTask1(void) {
	GUI_Init();
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
	GUI_CURSOR_Select(&GUI_CursorArrowM);
	GUI_CURSOR_Show();
#endif

	WM_SetDesktopColor(DESKTOP_COLOR);
	WM_SetCreateFlags(WM_CF_MEMDEV);  /* Use memory devices on all windows to avoid flicker */
	//_pPlayerAI[1] = &_PlayerAI_NearToBorder;
	_pPlayerAI[1] = &_PlayerAI_FirstValid;
	while (1) {
		_StartReversi();
		GUI_Delay(1000);
	}
}

/*************************** End of file ****************************/

