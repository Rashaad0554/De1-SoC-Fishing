#include <stdlib.h>
#include <stdbool.h>

void draw_line(int x0, int y0, int x1, int y1, short int colour);
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void swap(int *a, int *b);
void wait_for_vsync();

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	volatile int* slider_ptr = (int *)0xFF200040;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)
    int boxX,boxY,fishX,fishY,fishDeltaY,boxWidth,boxHeight,fishWidth,fishHeight,netX,netY,netDeltaY,netWidth,netHeight;
	short int fishCol = 0x0000FF;
	short int boxCol = 0xffffff;
	short int netCol = 0xFFCCCB;
	
	//set starting locations for box and fish and net
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

    while (1)
    {
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
		
		//update timer stuff
		if(timerCount == timer){
			fishDeltaY = rand()%(2*stepDiff[diffI]+1)-stepDiff[diffI];
			timerCount = 0;
			timer = (rand()%7) + timeAdjust;
		}
        // code for updating the locations of boxes (not shown)
		if(fishY+fishDeltaY <= boxY || fishY+fishDeltaY+fishHeight >= boxY + boxHeight){
			fishDeltaY *= -1;
		}
		//update  net location
		if(*(slider_ptr) == 1){
			netDeltaY = 10;
		}
		else{
			netDeltaY = -10;
		}
		if(netY + netDeltaY > boxY && netY+netDeltaY+netHeight < boxY + boxHeight){	
			netY += netDeltaY;
		}
		fishY += fishDeltaY;
		timerCount++;
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

// code for subroutines (not shown)

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
    volatile int * pixel_ctrl_ptr = (int *) 0xff203020; // base address
    int status;
    *pixel_ctrl_ptr = 1; // start the synchronization process
    // write 1 into front buffer address register
    status = *(pixel_ctrl_ptr + 3); // read the status register
    while ((status & 0x01) != 0) // polling loop waiting for S bit to go to 0
    {
        status = *(pixel_ctrl_ptr + 3);
    } // polling loop/function exits when status bit goes to 0
}