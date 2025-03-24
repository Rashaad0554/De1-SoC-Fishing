#include <stdlib.h>
#include <stdbool.h>

#define PS2_BASE 0xFF200100
#define PIXEL_BUF_CTRL_BASE 0xFF203020
#define HEX3_HEX0_BASE			0xFF200020
#define HEX5_HEX4_BASE			0xFF200030

void draw_line(int x0, int y0, int x1, int y1, short int colour);
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void swap(int *a, int *b);
void wait_for_vsync();

void keyboard();
void HEX_PS2(char, char,char);

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];


int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
	// volatile int* slider_ptr = (int *)0xFF200040;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)
    int boxX,boxY,fishX,fishY,fishDeltaY,boxWidth,boxHeight,fishWidth,fishHeight,netX,netY,netDeltaY,netWidth,netHeight,pbarX,pbarY,pbarWidth,pbarHeight;
	short int pbarCol = 0x00FF00;
	short int fishCol = 0x0000FF;
	short int boxCol = 0xffffff;
	short int netCol = 0xFFCCCB;
	
	//score
	int score = 1;
	
	//set starting locations for box and fish and net and progress bar (pbar)
	pbarX = 110;
	pbarY = 200;
	pbarWidth = 10;
	pbarHeight = 10;
	boxX = 50;
	boxY = 10;
	boxWidth = 50;
	boxHeight = 200;
	fishX = 65;
	fishY = 100;
	fishWidth = 20;
	fishHeight = 30;
	netWidth = 30;
	netHeight = 60;
	netX = 60;
	netY = 70;
	netDeltaY = 10;
	
	
	srand(rand()%5 + 1);
	int timerDiff[7] = {1, 3, 5, 5, 5, 7, 7};
	int stepDiff[7] = {9, 9, 6, 6, 6, 4, 4};
	int diffI = rand()%7;
	int timeAdjust = timerDiff[diffI];
	//set starting direction for fish
	int timer = (rand()%7) + timeAdjust;
	int timerCount = 0;
    
    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	
	volatile int * PS2_ptr = (int *)PS2_BASE;
    
    int PS2_data, RVALID;
    char byte1 = 0, byte2 = 0, byte3 = 0;
    
    // PS/2 mouse needs to be reset (must be already plugged in)
    *(PS2_ptr) = 0xFF; // reset
	
	
    while (1)
    {
        PS2_data = *(PS2_ptr); // read the Data register in the PS/2 port
        RVALID = PS2_data & 0x8000; // extract the RVALID field
		int RAVAIL = PS2_data & 0xFFFF0000;
        if (RVALID) {
            /* shift the next data byte into the display */
            byte1 = byte2;
            byte2 = byte3;
            byte3 = PS2_data & 0xFF;
			if (byte3 == byte2)
            {
                PS2_data =(PS2_ptr);
            }

            HEX_PS2(byte1, byte2, byte3);

        }
        /* Erase any boxes and lines that were drawn in the last iteration */
        clear_screen();
		
		//draw box
		for (int w = boxY; w < boxY + boxHeight; w++)
            {
                for (int h = boxX; h < boxX + boxWidth; h++)
                {
                    plot_pixel(h, w, boxCol);
                }
            }
		
		//draw net
		for(int h = netY ; h < netY + netHeight ; h++){
			for(int w = netX ; w < netX + netWidth ; w++){
				plot_pixel(w,h,netCol);
			}
		}
        
        
        //draw fish
        for (int w = fishY; w < fishY + fishHeight; w++)
            {
                for (int h = fishX; h < fishX + fishWidth; h++)
                {
                    plot_pixel(h, w, fishCol);
                }
            }
		
		//draw pbar
		for(int w = pbarX ; w < pbarX + pbarWidth;  w++){
			for(int h = pbarY ; h < pbarY + pbarHeight ; h++){
				plot_pixel(w,h,pbarCol);
			}
		}
		
		//drawing score
		for(int i = 0 ; i < score ; i++){
			for(int w = 300 - 30*i ; w < 300 - 30*i + 10 ; w++){
				for(int h = 10 ; h < 20 ; h++){
					plot_pixel(w,h,pbarCol);
				}
			}
		}
		
		//update timer stuff
		if(timerCount == timer){
			fishDeltaY = rand()%(2*stepDiff[diffI]+1)-stepDiff[diffI];
			timerCount = 0;
			timer = (rand()%7) + timeAdjust;
		}
		//checking if fish is within net
		if(fishY + fishHeight > netY && fishY < netY + netHeight){
			if(pbarY > 10){	
				pbarY -= 1;
				pbarHeight += 1;
			}
		}
		else{
			if(pbarY < 200){
				pbarY +=2;
				pbarHeight -=2;
			}
		}
		//updating score
		if(pbarHeight == 190){
			score++;
			pbarY = 200;
			pbarHeight = 10;
			diffI = rand()%7;
		}

        // code for updating the locations of boxes (not shown)
		if(fishY+fishDeltaY <= boxY || fishY+fishDeltaY+fishHeight >= boxY + boxHeight){
			fishDeltaY *= -1;
		}

		//update net location
		if (byte2 == 0x29 && byte3 == 0x29) {
            netDeltaY= -13;
		}
        else {
            netDeltaY = 10;
        }
		if(netY + netDeltaY > boxY && netY+netDeltaY+netHeight < boxY + boxHeight){	
			netY += netDeltaY;
		}

        // update fish location
		fishY += fishDeltaY;
		timerCount++;
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

// VGA subroutines

void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;
        
        one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
        
        *one_pixel_address = line_color;
}

void clear_screen()
{
   for (int x = 0; x < 320; x++)
    {
        for (int y = 0; y < 240; y++)
        {
            plot_pixel(x, y, 0x0000);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int colour)
{
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    if (is_steep)
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    int y_step = y0 < y1 ? 1 : -1;

    for (int x = x0; x <= x1; x++)
    {
        if (is_steep)
        {
            plot_pixel(y, x, colour);
        }
        else
        {
            plot_pixel(x, y, colour);
        }
        error = error + deltay;
        if (error > 0)
        {
            y += y_step;
            error -= deltax;
        }
    }
}

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void wait_for_vsync()
{
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE; // base address
    int status;
    *pixel_ctrl_ptr = 1; // start the synchronization process
    // write 1 into front buffer address register
    status = *(pixel_ctrl_ptr + 3); // read the status register
    while ((status & 0x01) != 0) // polling loop waiting for S bit to go to 0
    {
        status = *(pixel_ctrl_ptr + 3);
    } // polling loop/function exits when status bit goes to 0
}

// PS/2 subroutines
void keyboard() {
    
}

// just to test keyboard()
void HEX_PS2(char b1, char b2, char b3) {
    volatile int * HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;
    volatile int * HEX5_HEX4_ptr = (int *)HEX5_HEX4_BASE;
    /* SEVEN_SEGMENT_DECODE_TABLE gives the on/off settings for all segments in
    * a single 7-seg display in the DE1-SoC Computer, for the hex digits 0 - F
    */
    unsigned char seven_seg_decode_table[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
        0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    unsigned char hex_segs[] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int shift_buffer, nibble;
    unsigned char code;
    int i;
    shift_buffer = (b1 << 16) | (b2 << 8) | b3;
    for (i = 0; i < 6; ++i) {
        nibble = shift_buffer & 0x0000000F; // character is in rightmost nibble
        code = seven_seg_decode_table[nibble];
        hex_segs[i] = code;
        shift_buffer = shift_buffer >> 4;
    }
    /* drive the hex displays */
    *(HEX3_HEX0_ptr) = *(int *)(hex_segs);
    *(HEX5_HEX4_ptr) = *(int *)(hex_segs + 4);
}
