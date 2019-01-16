/* brctl.sh contains the following:
	   #!/bin/sh
	   #the valid value's for $1 are from 10 to 100!
	   acpi_call -p '\_SB.PCI0.VID.LCD0._BCM' -i $1
*/
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <X11/Xlib.h>
#include <thread>
#include <chrono>


struct context
{				// State info for our xlib graphics >:^)
  Display *display = NULL;
  Window window;
  XSetWindowAttributes attribs;
  Colormap cmap;
  XColor cyan, purple, blue, green, yellow, orange, red, darkRed;
  GC gc;
  int screenNum;
  unsigned int displayWidth;
  unsigned int displayHeight;
  /* I've put the following constants here for convenience.
     With the exception of Y_OFFSET (that referes to the absolute position of the window on the SCREEN all X or Y
     constants that are postfixed with the word OFFSET are used with a modifying value and not as an absolute stand
     alone value. This naming convention was adopted to convey the differences in uses of these different constants */
  const int SLEEP_TIMES {9};  // Number of times to go to sleep.
  const int SLEEP_TIME {333}; // Time to sleep for in ms.
  const int WINDOW_LEN {683};	// X offset (from the left) = displayWidth - WINDOW_LEN
  const int Y_OFFSET {2};	// Offset from the top of the screen.
  const int WINDOW_HEIGHT {15};	// Height of the window
  const int BAR_SPACE_SIZE {44}; // Size of bar (outer) and space
  const int BARS_X {174};	 // Offset of bars from text on right.
  const int OUTER_BAR_Y {1};	 // Y Offset of outer bar
  const int OUTER_BAR_WIDTH {48};
  const int OUTER_BAR_HEIGHT {13};
  const int INNER_BAR_X_OFFSET {3};
  const int INNER_BAR_Y {4};
  const int INNER_BAR_WIDTH {42};
  const int INNER_BAR_HEIGHT {7};
  const int STR_X_OFFSET {100};
  const int STR_Y {12};
  const int MINUS_X_OFFSET {117};
  const int MINUS_Y {12};
  const int PLUS_X {10};
  const int PLUS_Y {12};
  const int MINUS_ARC_X_OFFSET {122};
  const int MINUS_ARC_Y {0};
  const int PLUS_ARC_X {5};
  const int PLUS_ARC_Y {0};
  const int ARC_WIDTH {14};
  const int ARC_HEIGHT {ARC_WIDTH}; // We're only interested in circles here :).
  const int ARC_ANGLE_1 {0};
  const int ARC_ANGLE_2 {360*64};
};
constexpr int EXIT_SUCESS			{1}; // Program executed without errors (hopefully.)
constexpr int FATAL_ERROR_ONE		 	{-1}; // Could not open display.
constexpr int FATAL_ERROR_TWO_CYAN 		{-2}; // Failed to allocate color cyan.
constexpr int FATAL_ERROR_TWO_PURPLE	 	{-3}; // Failed to allocate color purple.
constexpr int FATAL_ERROR_TWO_BLUE 		{-4}; // Failed to allocate color blue.
constexpr int FATAL_ERROR_TWO_GREEN		{-5}; // Failed to allocate color green.
constexpr int FATAL_ERROR_TWO_YELLOW		{-6}; // Failed to allocate color yellow.
constexpr int FATAL_ERROR_TWO_ORANGE 		{-7}; // Failed to allocate color orange.
constexpr int FATAL_ERROR_TWO_RED		{-8}; // Failed to allocate color red.
constexpr int FATAL_ERROR_TWO_DARK_RED	{-9}; // Failed to allocate color dark red.


// Read an int from the file f.
int getIntFromFile(const int rDefalut, const char f []);
// Checks that a is inbetween or equal to rMax and rMin and that (a % iGran == 0) it is a multiple of iGran!
bool checkBR_Val(const int rMin, const int rMax, const int iGran, const int a);
// Cange the brightness level, save the brightness level, show the brightness level.
void doWork(const int level, const char file []);
// Save int a to file at path f.
void saveIntToFile(const std::string f, const int a);
void display(const int brLevel);
void init(context & con);
// Display brightness level on screen.
void draw(context & con, const int brLevel);
void drawBars(context & con, const int brLeve);
void drawBar(context & con, const int cu, const int nBar, const int nSpace, const std::string barNumber);
bool handle2ndArg(const char * argv [], const int BR_RANGE_MIN, const int BR_RANGE_MAX,
		  const int BR_INTERVAL_GRANULARITY, const int ARG_2_INDEX, int & brLevel);
/* Returns (a + iGran) or (a - iGran) depending on whether arg is a '+' or a '-'.
   Additionally a will not be altered if altering it would cause it to fall outside of the range
   [rMin, rMax], in effect a saturates at these ranges. */
int adjustBR_Val(const int rMax, const int rMin, const int iGran, const char arg, const int a);
void printUsage(const std::string name);


/* Main requires 1 or 2 command line argument's (this includes the program name)
   the 2nd argument must be either '+' or '-', if there are 2 arguments. */
int main(const int argc, const char * argv[])
{				// BR_DEFAULT should be divisible by BR_INTERVAL_GRANULARITY!
  constexpr int BR_RANGE_MIN {10}, BR_RANGE_MAX {100}, BR_INTERVAL_GRANULARITY {10}, BR_DEFAULT {80};
  constexpr int MAX_ARGC {2}, MIN_ARGC {1}, ARG_2_INDEX {1}; // ARG_1_INDEX {0}.
  constexpr char brLevelFileName [] = "/usr/tmp/brLevel";
  int brLevel {getIntFromFile(BR_DEFAULT, brLevelFileName)}; // Attempt to get current brightness level.

  if(!checkBR_Val(BR_RANGE_MIN, BR_RANGE_MAX, BR_INTERVAL_GRANULARITY, brLevel))
    {
      std::cerr<<"Brightness level stored in \""<<brLevelFileName<<"\", not evenly divisible by "
	"BR_INTERVAL_GRANULARITY ("<<BR_INTERVAL_GRANULARITY<<") or was not able to open file containing"
	" brightness level. Setting brightness level to dealfult ("<<BR_DEFAULT<<")\n";
      brLevel = BR_DEFAULT;
      doWork(brLevel, brLevelFileName);      
    }
  else
    {
      if(argc == MAX_ARGC)
	{
	  if(handle2ndArg(argv, BR_RANGE_MIN, BR_RANGE_MAX, BR_INTERVAL_GRANULARITY, ARG_2_INDEX, brLevel))
	    {
	      doWork(brLevel, brLevelFileName);
	    }
	  else
	    {			   // Malformed input.
	      printUsage(argv[0]);
	    }
	}
      else
	if(argc == MIN_ARGC)
	  { /* Set brightness to level specified in brLevelFileName or if that value is out of
	       range set level to BR_DEFAULT */
	    doWork(brLevel, brLevelFileName);
	    return 0;
	  }
	else
	  printUsage(argv[0]);
    }

  return (EXIT_SUCESS);
}


int getIntFromFile(const int rDefault, const char f [])
{
    int ret {};
    std::ifstream in(f);
    if(in.is_open())
      {
	in>>ret;
	in.close();
      }
    else
      {
	std::cerr<<"Error can't open file at \""<<f<<"\".\n";
	ret = rDefault;
      }
    return ret;
}


bool checkBR_Val(const int rMin, const int rMax, const int iGran, const int a)
{
  if(a >= rMin && a <= rMax)
    if((a % iGran) == 0)
      return true;		// A has passed all the tests. :)
  return false;	// If we have reached this point a does not contain a valid value.
}


void doWork(const int level, const char file [])
{
  std::stringstream command {};
  command<<"~/.config/brightness/brctl.sh "<<std::to_string(level);
  system(command.str().c_str()); // Change brightness level.
  std::cout<<"level = "<<level<<'\n';
  saveIntToFile(file, level); // Save brightness level.
  display(level);	// Show brightness level.
}


void saveIntToFile(const std::string f, const int a)
{
  std::ofstream out(f.c_str());
  if(out.is_open())
    {
      out<<a<<'\0';    
      out.close();
    }
  else
    {
      std::cerr<<"Error cant open file \""<<f<<"\" for writing.\n";
    }
}


void display(const int brLevel)
{
  context con;
  init(con);
  for(int iter{}; iter < con.SLEEP_TIMES; ++iter) // 333*9 ~ 3000 (Sleep for 3 seconds.) 
    {
      draw(con, brLevel);
      XFlush(con.display); // Force x to flush it's buffers after we and draw before we sleep.
      std::this_thread::sleep_for(std::chrono::milliseconds(con.SLEEP_TIME));
    }
  XCloseDisplay(con.display);
}


void init(context & con)
{
  con.display = XOpenDisplay(nullptr);
  if( !con.display )
    {
      std::cerr<< "Error can't open con.display.";
      exit(FATAL_ERROR_ONE);
    }
				// Get screen geometry.
  con.screenNum = DefaultScreen(con.display);
  con.displayWidth = DisplayWidth(con.display, con.screenNum);
  con.displayHeight = DisplayHeight(con.display, con.screenNum);
  
  con.attribs.override_redirect = 1; // Non bordered / decorated window.

  //con.windowLen = 683;
  con.window = XCreateWindow(con.display, RootWindow(con.display, 0), con.displayWidth -con.WINDOW_LEN, con.Y_OFFSET,
			     con.WINDOW_LEN, con.WINDOW_HEIGHT, 0, CopyFromParent, CopyFromParent, CopyFromParent,
			     CWOverrideRedirect, &con.attribs);
  XSetWindowBackground(con.display, con.window, 0x1900ff); // 0x84ffdc cool colour.
  XClearWindow(con.display, con.window );
  XMapWindow(con.display, con.window );	// Make window appear.

  XGCValues values;
  con.cmap = DefaultColormap(con.display, con.screenNum);
  con.gc = XCreateGC(con.display, con.window, 0, &values);

  Status rc;
    rc = XAllocNamedColor(con.display, con.cmap, "Cyan", &con.cyan, &con.cyan);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'cyan' color.\n";
      exit(FATAL_ERROR_TWO_CYAN);
    }
      rc = XAllocNamedColor(con.display, con.cmap, "Purple", &con.purple, &con.purple);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'purple' color.\n";
      exit(FATAL_ERROR_TWO_PURPLE);
    }
      rc = XAllocNamedColor(con.display, con.cmap, "Blue", &con.blue, &con.blue);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'blue' color.\n";
      exit(FATAL_ERROR_TWO_BLUE);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Green", &con.green, &con.green);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'green' color.\n";
      exit(FATAL_ERROR_TWO_GREEN);
    }
  rc = XAllocNamedColor(con.display, con.cmap, "Yellow", &con.yellow, &con.yellow);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'yellow' color.\n";
      exit(FATAL_ERROR_TWO_YELLOW);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Orange", &con.orange, &con.orange);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'orange' color.\n";
      exit(FATAL_ERROR_TWO_ORANGE);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Red", &con.red, &con.red);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'red' color.\n";
      exit(FATAL_ERROR_TWO_RED);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Dark Red", &con.darkRed, &con.darkRed);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'dark red' color.\n";
      exit(FATAL_ERROR_TWO_DARK_RED);
    }
}


void draw(context & con, const int brLevel)
{				// Black magic numbers in this function :'( (Can't be bothered to fix rn.)
  std::stringstream textInfo {};
  /*  if(brLevel < 10)//it never goes below 20!
      textInfo<<"  ";*/
  if(brLevel < 100)
    textInfo<<' ';
  textInfo<<"%"<<brLevel<<" :ssenthgirB";//everything to the left of the brLevel bar's    
  XSetForeground(con.display, con.gc, con.cyan.pixel);//set forground colour
  XDrawString(con.display, con.window, con.gc, con.WINDOW_LEN -con.STR_X_OFFSET, con.STR_Y, textInfo.str().c_str(),
	      textInfo.str().size());
  XDrawString(con.display, con.window, con.gc, con.WINDOW_LEN -con.MINUS_X_OFFSET, con.MINUS_Y, "-", 1);
  XDrawString(con.display, con.window, con.gc, con.PLUS_X, con.PLUS_Y, "+", 1);
  /* FROM: "https://tronche.com/gui/x/xlib/graphics/drawing/XDrawArc.html"
    For an arc specified as [ x, y, width, height, angle1, angle2 ], the angles must be specified in the
    effectively skewed coordinate system of the ellipse (for a circle, the angles and coordinate systems are
    identical). The relationship between these angles and angles expressed in the normal coordinate system of the
    screen (as measured with a protractor) is as follows:
    skewed-angle = atan ( tan ( normal-angle ) * width / height ) + adjust */
  XDrawArc(con.display, con.window, con.gc, con.WINDOW_LEN -con.MINUS_ARC_X_OFFSET,
	   con.MINUS_ARC_Y, con.ARC_WIDTH, con.ARC_HEIGHT, con.ARC_ANGLE_1,
	   con.ARC_ANGLE_2); // Right circle around "-"
  XDrawArc(con.display, con.window, con.gc, con.PLUS_ARC_X, con.PLUS_ARC_Y, con.ARC_WIDTH, con.ARC_HEIGHT,
	   con.ARC_ANGLE_1, con.ARC_ANGLE_2); // Left circle around "+".
  drawBars(con, brLevel);  //draw the brLevel bars                                    
}


void drawBars(context & con, const int brLeve)
{
  constexpr int levels = 10;//levels should be a devisor of 100                       
  for(int nBar {}, nSpace{}; nBar < brLeve; nBar +=levels, nSpace++)
    {
      std::stringstream barNumber {};
      barNumber<<(nSpace+1);
      if(nBar < 50)
        drawBar(con, con.green.pixel, nBar, nSpace, barNumber.str());
      else
        if(nBar < 70)
          drawBar(con, con.yellow.pixel, nBar, nSpace, barNumber.str());
        else
          if(nBar < 80)
            drawBar(con, con.orange.pixel, nBar, nSpace, barNumber.str());
          else
            if(nBar < 90)
              drawBar(con, con.red.pixel, nBar, nSpace, barNumber.str());
            else
              drawBar(con, con.darkRed.pixel, nBar, nSpace, barNumber.str());
      barNumber.clear();
      }
}


void drawBar(context & con, const int cu, const int nBar, const int stride, const std::string barNumber)
{				// Draw outer bar.
  XSetForeground(con.display, con.gc, cu);  
  XFillRectangle(con.display, con.window, con.gc,
		 con.WINDOW_LEN -(con.BARS_X + nBar + (stride*con.BAR_SPACE_SIZE)),
		 con.OUTER_BAR_Y, con.OUTER_BAR_WIDTH, con.OUTER_BAR_HEIGHT);
  // Draw inner bar.
  XSetForeground(con.display, con.gc, con.blue.pixel);
  XFillRectangle(con.display, con.window, con.gc,
		 con.WINDOW_LEN -(con.BARS_X + nBar + (stride*con.BAR_SPACE_SIZE) - con.INNER_BAR_X_OFFSET),
		 con.INNER_BAR_Y, con.INNER_BAR_WIDTH, con.INNER_BAR_HEIGHT);
  XSetForeground(con.display, con.gc, con.cyan.pixel);
}


bool handle2ndArg(const char * argv [], const int BR_RANGE_MIN, const int BR_RANGE_MAX,
		  const int BR_INTERVAL_GRANULARITY, const int ARG_2_INDEX, int & brLevel)
{
  const char arg {argv[ARG_2_INDEX][0]}; // If there is only one argument argv[ARG_2_INDEX] should equal '\0' 
      
  if(argv[ARG_2_INDEX][1] == '\0' && (arg == '+' || arg == '-'))
    {			// We have a well formed second argument.
      brLevel = adjustBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, arg, brLevel);
      return true;
    }
  return false;
}


int adjustBR_Val(const int rMax, const int rMin, const int iGran, const char sign, const int a)
{
  switch(sign)
    {
    case '+':
      if(a == rMax)
	break;
      return a + iGran;      
    case '-':
      if(a == rMin)
	break;
      return a - iGran;
    }
  return a;
}


void printUsage(const std::string name)
{
  std::cout<<"error!\nusage: "<<name<<" [options]\nOPTIONS\n\t+\tIncrease screen brightness\n\t-\tDecrease screen"
    "brightness\n";
}
