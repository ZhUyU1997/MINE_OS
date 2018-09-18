#include <stdio.h>
#include <stdlib.h>
/**
	This is a demo for the vt100 functionality. 

	To do the tests, you may have to comment out some of them because all
	the strings take up a lot of ram. :)
	
	Copyright: Martin K. Schröder (info@fortmax.se) 2014/10/27
*/

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <timer.h>
#include "ili9340.h"
#include "vt100.h"

/**
  Tests following commands:

  Cursor movement (<n>=how many chars or lines), cursor stop at margin.
  Up
    Esc  [ <n> A
    033 133   101
  Down
    Esc  [ <n> B
    033 133   102
  Right
    Esc  [ <n> C
    033 133   103
  Left
    Esc  [  n  D
    033 133   104
  Cursor position  (<n1>=y,<n2>=x, from top of screen or scroll region)
       Esc  [ <n1> ; <n2> H
       033 133    073    110
    Or Esc  [ <n1> ; <n2> f
       033 133    073    146
  Index (cursor down with scroll up when at margin)
    Esc  D
    033 104
  Reverse index (cursor up with scroll down when at margin)
    Esc  M
    033 115
  Next line (CR+Index)
    Esc  E
    033 105
  Save cursor and attribute
    Esc  7
    033 067
  Restore cursor and attribute
    Esc  8
    033 070
*/

void test_cursor(){
	char buf[16]; 
	// clear screen
	vt100_puts("\e[c\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// draw a line of "*"
	for(int c = 0; c < VT100_WIDTH; c++){
		vt100_putc('*'); 
	}
	// draw left and right border
	for(int c = 0; c < VT100_HEIGHT; c++){
		sprintf(buf, "\e[%d;1H*\e[%d;%dH*", c + 1, c + 1, VT100_WIDTH);
		vt100_puts(buf);
	}
	// draw bottom line
	sprintf(buf, "\e[%d;1H", VT100_HEIGHT);
	vt100_puts(buf); 
	for(int c = 0; c < VT100_WIDTH; c++){
		vt100_putc('*');
	}
	// draw inner border of +
	vt100_puts("\e[2;2H");
	// draw a line of "*"
	for(int c = 0; c < VT100_WIDTH - 2; c++){
		vt100_putc('+'); 
	}
	// draw left and right border
	for(int c = 1; c < VT100_HEIGHT - 1; c++){
		sprintf(buf, "\e[%d;2H+\e[%d;%dH+", c + 1, c + 1, VT100_WIDTH - 1);
		vt100_puts(buf);
	}
	// draw bottom line
	sprintf(buf, "\e[%d;2H", VT100_HEIGHT - 1);
	vt100_puts(buf); 
	for(int c = 0; c < VT100_WIDTH - 2; c++){
		vt100_putc('+');
	}

	// draw middle window
	// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
	// E                            E
	// E This must be an unbroken a E
	// E rea of text with 1 free bo E
	// E rder around the text.      E
	// E                            E
	// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
	vt100_puts("\e[10;6H");
	for(int c = 0; c < 30; c++){
		vt100_putc('E');
	}
	// test normal movement
	vt100_puts("\e[11;6H");
	// test cursor store and restore...
	vt100_puts("\e7\e[35;10H\e8");
	vt100_puts("E\e[11;35HE");
	// goto 12;6, print E, move cursor 29 (already moved +1) to right and print E
	vt100_puts("\e[12;6HE\e[28CE");
	// move cursor 31 to left, 1 down, print E, move 30 right, print E
	vt100_puts("\e[30D\e[BE\e[28CE");
	vt100_puts("\e[15;6H\e[AE\e[28CE");
	vt100_puts("\e[15;6HE\e[15;35HE"); 
	
	vt100_puts("\e[16;6H");
	for(int c = 0; c < 30; c++){
		vt100_putc('E');
	}

	const char *text[] = {"This must be an unbroken a", "rea of text with 1 free bo", "rder around the text.     "};
	for(int c = 0; c < 3; c++){
		sprintf(buf, "\e[%d;8H", c + 12);
		vt100_puts(buf);
		vt100_puts(text[c]);
	}

	// now lets draw two parallel columns of Es
	vt100_puts("\e[20;19H"); 
	for(int c = 0; c < 10; c++){
		// draw E (cursor moves right), step one right, draw F, step 3 left and 1 down
		vt100_puts("E\e[1CF\e[3D\e[B");
	}
	
	// Test index (escD - down with scroll)
	// Test reverse index (escM)
	// next line (escE) - cr + index
	// save and restore cursor

	// move to last line and scroll down 8 lines
	vt100_puts("\e[40;1H");
	for(int c = 0; c < 7; c++){
		vt100_puts("\eD");
	}
	mdelay(100); 
	// now scroll same number of lines back and then back again (to test up scroll)
	vt100_puts("\e[1;1H");
	for(int c = 0; c < 7; c++){
		vt100_puts("\eM");
	}
	mdelay(100); 
	vt100_puts("\e[40;1H");
	for(int c = 0; c < 7; c++){
		vt100_puts("\eD");
	}
	
	// we now have the Es at the third line (or we SHOULD have)
	// refill the top border and clear bottom borders
	for(int c = 1; c < VT100_WIDTH - 1; c++){
		// we print * then move down and left, print + and go back right and top
		// (good way to test cursor navigation keys)
		sprintf(buf, "\e[1;%dH*\e[B\e[D+\e[A", c + 1); 
		vt100_puts(buf);
	}
	// clear the border that scrolled up
	for(int c = 2; c < VT100_WIDTH - 2; c++){
		// space, down, left, space, up
		sprintf(buf, "\e[32;%dH \e[B\e[D \e[A", c + 1); 
		vt100_puts(buf);
	}
	
	// redraw left and right border
	for(int c = 1; c < VT100_HEIGHT; c++){
		sprintf(buf, "\e[%d;1H*+\e[%d;%dH+*", c + 1, c + 1, VT100_WIDTH - 1);
		vt100_puts(buf);
	}
	
	// fill border at the bottom
	for(int c = 1; c < VT100_WIDTH - 1; c++){
		sprintf(buf, "\e[39;%dH+\e[B\e[D*\e[A", c + 1); 
		vt100_puts(buf);
	}
	// draw the explanation string
	vt100_puts("\e[30;6HShould see two columns of E F"); 
	vt100_puts("\e[31;6HText box must start at line 3"); 
}

/**
	Tests setting scroll region and moving the cursor inside the scroll region.

*/

void test_scroll(){
	char buf[16]; 
	// reset terminal and clear screen. Cursor at 1;1. 
	vt100_puts("\e[c\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// set top margin 3 lines, bottom margin 5 lines
	vt100_puts("\e[4;29r");

	// draw top and bottom windows
	vt100_puts("\e[1;1H#\e[2;1H#\e[3;1H#\e[1;80H#\e[2;80H#\e[3;80H#");
	vt100_puts("\e[30;1H#\e[31;1H#\e[32;1H#\e[33;1H#\e[34;1H#");
	vt100_puts("\e[30;80H#\e[31;80H#\e[32;80H#\e[33;80H#\e[34;80H#");
	vt100_puts("\e[1;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1); 
	vt100_puts("\e[3;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1); 
	vt100_puts("\e[30;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1); 
	vt100_puts("\e[34;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1);

	// print some text that should not move
	vt100_puts("\e[2;4HThis is top text (should not move)"); 
	vt100_puts("\e[32;3HThis is bottom text (should not move)");

	// set origin mode and print border around the scroll region
	vt100_puts("\e[?6h");
	vt100_puts("\e[1;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('!'),mdelay(1);
	// origin mode should snap 99 to last line in scroll region
	vt100_puts("\e[99;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('!'),mdelay(1);
	for(int y = 0; y < VT100_HEIGHT; y++){
		//sprintf(buf, "\e[%d;1H!\e[%d;%dH!", y+1, y+1, VT100_WIDTH);
		sprintf(buf, "\e[%d;1H", y+1);
		vt100_puts(buf);
		for(int c = 0; c < VT100_WIDTH; c++){
			vt100_putc('!');mdelay(1);
		}
	}

	// scroll the scroll region
	vt100_puts("\e[99;1H\eD\eD");
	vt100_puts("\e[1;1H\eM\eM");
	vt100_puts("\e[99;1H\eD");

	// clear out an area in the middle and draw text
	for(int y = 0; y < 5; y++){
		sprintf(buf, "\e[%d;6H", y+10);
		vt100_puts(buf);
	
		for(int c = 0; c < 30; c++){
			vt100_putc(' ');mdelay(1);
		}
	}
	vt100_puts("\e[11;10HMust be ! filled with 2");
	vt100_puts("\e[12;10H    empty lines at");
	vt100_puts("\e[13;10H    top and bottom! ");
	
}

/**

Erasing
  Erase in line
    End of line (including cursor position)
         Esc  [   K
         033 133 113
      Or Esc  [   0   K
         033 133 060 113
    Beginning of line (including cursor position)
      Esc  [   1   K
      033 133 061 113
    Complete line
      Esc  [   2   K
      033 133 062 113
  Erase in display
    End of screen (including cursor position)
         Esc  [   J
         033 133 112
      Or Esc  [   0   J
         033 133 060 112
    Beginning of screen (including cursor position)
      Esc  [   1   J
      033 133 061 112
    Complete display
      Esc  [   2   J
      033 133 062 112
 
 
Computer editing
  Delete characters (<n> characters right from cursor
    Esc  [ <n> P
    033 133   120
  Insert line (<n> lines)
    Esc  [ <n> L
    033 133   114
  Delete line (<n> lines)
    Esc  [ <n> M
    033 133   115
*/

void test_edit(){
	char buf[32];

	// clear screen
	vt100_puts("\e[c\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// enable auto wrap mode
	vt100_puts("\e[?7h");
	
	// fill entire screen with 'x'
	int size = (VT100_WIDTH * VT100_HEIGHT); 
	for(int c = 0; c < size; c++){
		vt100_putc('x');mdelay(1);
	}

	// clear bottom and top halves (will remove all 'x' s)
	vt100_puts("\e[20;1H"); 
	vt100_puts("\e[J");
	mdelay(1000);
	vt100_puts("\e[1J");
	vt100_puts("\e[?7l");
	mdelay(1000); 
	
	// draw left and right borders using erase function
	for(int c = 23; c < VT100_HEIGHT; c+=2){
		sprintf(buf, "\e[%d;1H", c + 1);
		vt100_puts(buf);
		// draw two lines of *
		for(int j = 0; j < VT100_WIDTH; j++){
			vt100_puts("*\e[B\e[D*\e[A");mdelay(1);
		}
		// erase end of first line and beginning of second line
		// goto c;3, erase end, goto c;(w-1), write **
		sprintf(buf, "\e[%d;3H\e[0K\e[%d;%dH**",
			c + 1, c + 1, VT100_WIDTH - 1);
		vt100_puts(buf);
		// goto (c+1);(width-2), erase beginning of line, goto (c+2);1, write **
		sprintf(buf, "\e[%d;%dH\e[1K\e[%d;1H**",
			c + 2, VT100_WIDTH - 2, c + 2);
		vt100_puts(buf); 
	}
	
	// fill border at the bottom
	for(int c = 2; c < VT100_WIDTH - 2; c++){
		sprintf(buf, "\e[24;%dH*\e[B\e[D*\e[A", c + 1);mdelay(1);
		vt100_puts(buf);
	}
	// fill border at the bottom
	for(int c = 2; c < VT100_WIDTH - 2; c++){
		sprintf(buf, "\e[33;%dH*\e[B\e[D*\e[A", c + 1);mdelay(1);
		vt100_puts(buf);
	}
	// draw text
	vt100_puts("\e[30;4HYou should see border and NO x:s"); 
}
/**
	Tests terminal colors

	(i just copy the scroll example here and add some colors)
	
	Esc[Value;...;Valuem 	Set Graphics Mode:
		Calls the graphics functions specified by the following values. These specified functions remain active until the next occurrence of this escape sequence. Graphics mode changes the colors and attributes of text (such as bold and underline) displayed on the screen.
		 
		Text attributes
		0	All attributes off 
		1	Bold on (we currently don't support bold on this limited display)
		4	Underscore (not supported)
		5	Blink on (not supported)
		7	Reverse video on (not supported)
		8	Concealed on (not supported)
		 
		Foreground colors
		30	Black
		31	Red
		32	Green
		33	Yellow
		34	Blue
		35	Magenta
		36	Cyan
		37	White
		 
		Background colors
		40	Black
		41	Red
		42	Green
		43	Yellow
		44	Blue
		45	Magenta
		46	Cyan
		47	White
		 
		Parameters 30 through 47 meet the ISO 6429 standard.
*/

void test_colors(){
	char buf[32];
	
	// reset terminal and clear screen. Cursor at 1;1.
	// reset all modes
	vt100_puts("\e[c\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// set top margin 3 lines, bottom margin 5 lines
	vt100_puts("\e[4;29r");

	// set bg color to red and text to white
	vt100_puts("\e[41;37m");
	
	// draw top and bottom windows
	vt100_puts("\e[1;1H#\e[2;1H#\e[3;1H#\e[1;80H#\e[2;80H#\e[3;80H#");
	vt100_puts("\e[1;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1);
	vt100_puts("\e[3;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1);

	// background blue
	vt100_puts("\e[44;37m");

	vt100_puts("\e[30;1H#\e[31;1H#\e[32;1H#\e[33;1H#\e[34;1H#");
	vt100_puts("\e[30;80H#\e[31;80H#\e[32;80H#\e[33;80H#\e[34;80H#");
	vt100_puts("\e[30;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1);
	vt100_puts("\e[34;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('#'),mdelay(1);

	// foreground white
	vt100_puts("\e[37;40m");
	
	// print some text that should not move
	vt100_puts("\e[2;4HThis is top text (should not move)"); 
	vt100_puts("\e[32;3HThis is bottom text (should not move)");

	// green background, black text
	vt100_puts("\e[42;30m");

	// set origin mode and print border around the scroll region
	vt100_puts("\e[?6h");
	vt100_puts("\e[1;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('!'),mdelay(1);
	// origin mode should snap 99 to last line in scroll region
	vt100_puts("\e[99;1H"); for(int x = 0; x < VT100_WIDTH; x++) vt100_putc('!'),mdelay(1);
	for(int y = 0; y < VT100_HEIGHT; y++){
		//sprintf(buf, "\e[%d;1H!\e[%d;%dH!", y+1, y+1, VT100_WIDTH);
		sprintf(buf, "\e[%d;1H", y+1);mdelay(1);
		vt100_puts(buf);
		for(int c = 0; c < VT100_WIDTH; c++){
			vt100_putc('!');mdelay(1);
		}
	}

	// scroll the scroll region
	vt100_puts("\e[99;1H\eD\eD");
	vt100_puts("\e[1;1H\eM\eM");
	vt100_puts("\e[99;1H\eD");

	// black background, yellow text
	vt100_puts("\e[33;40m");
	
	// clear out an area in the middle and draw text
	for(int y = 0; y < 5; y++){
		sprintf(buf, "\e[%d;6H", y+10);
		vt100_puts(buf);
	
		for(int c = 0; c < 30; c++){
			vt100_putc(' ');mdelay(1);
		}
	}
	vt100_puts("\e[11;10HMust be ! filled with 2");
	vt100_puts("\e[12;10H    empty lines at");
	vt100_puts("\e[13;10H    top and bottom! ");

}
