#include "console.h"
#include "font.h"
#include "utils.c"
#include "stdarg.h"
#include "disk.c"

#define WIDTH 800
#define HEIGHT 600

//volatile unsigned char* framebuffer;
unsigned char* framebuffer;
struct MultibootInfo* mbi;
int cc = 0, cr = 0; //current column(x), current row(y)
// WIDTH / CHAR_WIDTH = number of columns
// HEIGHT / CHAR_HEIGHT = number of rows

void console_init(struct MultibootInfo* m)
{
	//framebuffer = (volatile unsigned char*) (unsigned) m -> mbiFramebufferAddress;
	framebuffer = (unsigned char*) (unsigned) m -> mbiFramebufferAddress;
	mbi = m;
}

void set_pixel(int x, int y, int r, int g, int b)
{
    unsigned frameBufferPosition;

	//Color conversion (bit-shift right assignment of 8-bit integers)
	r >>= (8-mbi->mbiFramebufferRedMask);
	g >>= (8-mbi->mbiFramebufferGreenMask);
	b >>= (8-mbi->mbiFramebufferBlueMask);

    //bitwise shift left each RGB value by respective color position
	unsigned short colorValue = (b<<mbi->mbiFramebufferBluePos) | (g<<mbi->mbiFramebufferGreenPos) | (r<<mbi->mbiFramebufferRedPos); //blue comes first in RAM   //2 bytes
	//2 declarations of colorValue, one for each byte

    //assign x, y values to framebuffer array, using mbi variables
	frameBufferPosition = (y * mbi->mbiFramebufferPitch) + (x * 2);

	framebuffer[frameBufferPosition] = colorValue;   //colorValue cast to char
	framebuffer[frameBufferPosition+1] = (colorValue >> 8);   //colorValue shifted right, casted to char

}

void console_draw_char(int x, int y, char ch)
{
    int cy, cx;	//in C, can't define variables inside for loops!
	const int * C = font_data[(int) ch];
	for(cy = 0; cy < CHAR_HEIGHT; ++cy)
	{
		for(cx = 0; cx < CHAR_WIDTH; ++cx)
		{

			if((MASK_VALUE >> cx) & C[cy])
				set_pixel(cx+x, cy+y, 255, 255, 255);
			else
			  	set_pixel(cx+x, cy+y, 0, 0, 0);	//set pixel to default OS color
		}
	}
}

void console_draw_string(int x, int y, const char* s)
{
    while(s[0] != 0)
    {
        console_draw_char(x, y, s[0]);
        x += CHAR_WIDTH;
        s++;
    }
}

void scroll()
{
    if(cr >= (HEIGHT / CHAR_HEIGHT))
    {
        //find new first line
        kmemcopy(framebuffer, framebuffer + (CHAR_HEIGHT * mbi->mbiFramebufferPitch), (HEIGHT * mbi->mbiFramebufferPitch) - (CHAR_HEIGHT * mbi->mbiFramebufferPitch));

        //clear the last row (start at the last row, not the beginning of the file)
        kmemset(framebuffer + (HEIGHT - CHAR_HEIGHT) * mbi->mbiFramebufferPitch, '\0', CHAR_HEIGHT * mbi->mbiFramebufferPitch);
        cr = (HEIGHT / CHAR_HEIGHT) - 1;
    }
}

void console_putc(char c)
{
    outb( 0x3f8, c );   //debugger outputs c to terminal, NOT OS window!

    switch(c)
    {
        case '\x7f':
            // move back one character (rubout previous character)
            if(cc == 0)
            {
                if(cr == 0)
                {
                    cc = 0;
                    cr = 0;
                }
                else
                {
                    cr--;
                    cc = (WIDTH / CHAR_WIDTH) - 1;
                }
            }
            else
            {
                cc--;
            }
            console_draw_char(cc * CHAR_WIDTH, cr * CHAR_HEIGHT, ' ');
            break;

        case '\b':
            // move back one character (not rubout)
            if(cc == 0)
            {
                if(cr == 0)
                {
                    cc = 0;
                    cr = 0;
                }
                else
                {
                    cr--;
                    cc = (WIDTH / CHAR_WIDTH) - 1;
                }
            }
            else
            {
                cc--;
            }
            console_draw_char(cc * CHAR_WIDTH, cr * CHAR_HEIGHT, c);
            break;

        case '\t':
            // move to next column divisible by 8
            if(cc % 8 == 0)
            {
                cc += 8;
                if (cc + 8 >= (WIDTH / CHAR_WIDTH))
                {
                    cr++;
                    cc = 0;
                    scroll();
                }
            }

            while(cc % 8)
            {
                cc++;
                if (cc + 8 > (WIDTH / CHAR_WIDTH))
                {
                    cr++;
                    cc = 0;
                    scroll();
                }
            }
            break;

        case '\f':
            // clear screen
            cr = 0;
            cc = 0;
            break;

        case '\r':
            // back to the start of the current line
            cc = 0;
            break;

        case '\n':
            // move down to next line, back to left side
            cr++;
            cc = 0;
            scroll();
            break;

        default:
            console_draw_char(cc * CHAR_WIDTH, cr * CHAR_HEIGHT, c);
            cc++;
            if (cc >= WIDTH / CHAR_WIDTH)
            {
                cr++;
                cc = 0;
                scroll();
            }
            break;
    }
}
