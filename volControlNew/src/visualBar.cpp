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
     With the exception of Y_OFFSET (that referes to the absolute position of
     the window on the SCREEN all X or Y constants that are postfixed with the
     word OFFSET are used with a modifying value and not as an absolute stand
     alone value. This naming convention was adopted to convey the differences
     in uses of these different constants */
  const int SLEEP_TIMES {9};  // Number of times to go to sleep.
  const int SLEEP_TIME {333}; // Time to sleep for in ms.
  // X offset (from the left) = displayWidth - WINDOW_LEN
  const int WINDOW_LEN {1280};	
  const int Y_OFFSET {2};	// Offset from the top of the screen.
  const int WINDOW_HEIGHT {15};	// Height of the window
  //  const int BARS_X {174};	 // Offset of bars from text on right.
  const int BARS_X {240};	 // Offset of bars from text on right.
  const int OUTER_BAR_Y {1};	 // Y Offset of outer bar
  const int OUTER_BAR_WIDTH {(WINDOW_LEN -BARS_X) / 10 + 2};
  // Size of bar (outer) and space
  const int BAR_SPACE_SIZE {OUTER_BAR_WIDTH -4};
  const int OUTER_BAR_HEIGHT {13};
  const int INNER_BAR_X_OFFSET {3};
  const int INNER_BAR_Y {4};
  const int INNER_BAR_WIDTH {(WINDOW_LEN -BARS_X) / 10 - 4};
  const int INNER_BAR_HEIGHT {7};
  const int STR_X_OFFSET {0};
  const int STR_Y {12};
  const int MINUS_X_OFFSET {117};
  const int MINUS_Y {12};
  const int PLUS_X {14};
  const int PLUS_Y {12};
  const int MINUS_ARC_X {112};
  const int MINUS_ARC_Y {0};
  const int PLUS_ARC_X {19};
  const int PLUS_ARC_Y {0};
  const int ARC_WIDTH {14};
  const int ARC_HEIGHT {ARC_WIDTH}; // We're only interested in circles here :).
  const int ARC_ANGLE_1 {0};
  const int ARC_ANGLE_2 {360*64};
};

enum
  {
   EXIT_SUCESS,	       // Program executed without errors.
   FATAL_ERROR_ONE = INT_MIN,	// Could not open display.
   FATAL_ERROR_TWO_CYAN	,	// Failed to allocate color.
   FATAL_ERROR_TWO_PURPLE,
   FATAL_ERROR_TWO_BLUE,
   FATAL_ERROR_TWO_GREEN,
   FATAL_ERROR_TWO_YELLOW,
   FATAL_ERROR_TWO_ORANGE,
   FATAL_ERROR_TWO_RED,
   FATAL_ERROR_TWO_DARK_RED,
   FATAL_ERROR_ARG1,
   FATAL_ERROR_ARG_NUM
  };


bool checkArgAndDisplay(const int level, const int LEVEL_RANGE_MIN,
			const int LEVEL_RANGE_MAX, const std::string unit);
/* Checks that a is inbetween or equal to rMax and rMin and that
   (a % iGran == 0) it is a multiple of iGran! */
bool checkRange(const int rMin, const int rMax, const int a);
// Save int a to file at path f.
void saveIntToFile(const std::string f, const int a);
void display(const int level, const int levelOrig, const std::string unit);
void init(context & con);
// Display bar and level on screen.
void draw(context & con, const int level, const int levelOrig,
	  const std::string unit);
void drawBars(context & con, const int level);
void drawBar(context & con, const int cu, const int nBar, const int nSpace);
void printUsage(const std::string name, const int range_min,
		const int range_max);


int main(const int argc, const char * argv[])
{
  constexpr int EXPECTED_ARGC {2}, ARG_INDEX_0 {1};
  constexpr int LEVEL_RANGE_MIN {0}, LEVEL_RANGE_MAX {100};
  int level {};
  const std::string unit {"Volume"};
  bool ret {EXIT_SUCESS};
	
  if(argc == EXPECTED_ARGC)
    {
      std::stringstream sSLevel {};
      sSLevel<<argv[ARG_INDEX_0];
      sSLevel>>level;
      ret = checkArgAndDisplay(level, LEVEL_RANGE_MIN, LEVEL_RANGE_MAX, unit);
    }
  else
    {
      if(argc > EXPECTED_ARGC)
	{
	  printUsage(argv[0], LEVEL_RANGE_MIN, LEVEL_RANGE_MAX);
	  ret = FATAL_ERROR_ARG_NUM;
	}
      else
	{
	  std::cin>>level;
	  ret = checkArgAndDisplay(level, LEVEL_RANGE_MIN, LEVEL_RANGE_MAX,
				   unit);
	}
    }

  return ret;
}


bool checkArgAndDisplay(const int level, const int LEVEL_RANGE_MIN,
			const int LEVEL_RANGE_MAX, const std::string unit)
{
  constexpr int LEVEL_INTERVAL_GRANULARITY {10}, ROUNDING_THRESHOLD {5};
  const int levelOrig {level};	// Unrounded level.
  bool ret {EXIT_SUCESS};

  if(!checkRange(LEVEL_RANGE_MIN, LEVEL_RANGE_MAX, level))
    {
      std::cerr<<"Argument \""<<level<<"\", not a number or not in range ["
	       <<LEVEL_RANGE_MIN<<","<<LEVEL_RANGE_MAX<<"]\n";
      ret = FATAL_ERROR_ARG1;
    }
  else
    {
      if(level % LEVEL_INTERVAL_GRANULARITY >= ROUNDING_THRESHOLD)
	{ // Show level.
	  display(level - (level % LEVEL_INTERVAL_GRANULARITY) +
		  LEVEL_INTERVAL_GRANULARITY, levelOrig, unit);
	}
      else
	{ // Show level.
	  display(level - (level % LEVEL_INTERVAL_GRANULARITY), levelOrig,
		  unit);
	}
    }
  return ret;
}


bool checkRange(const int rMin, const int rMax, const int a)
{
  if(a >= rMin && a <= rMax)
    {
      return true;
    }
  // If we have reached this point a does not contain a valid value!
  return false;
}


void display(const int level, const int levelOrig, const std::string unit)
{
  context con;
  init(con);
  // 333*9 ~ 3000 (Sleep for 3 seconds.) 
  for(int iter{}; iter < con.SLEEP_TIMES; ++iter)
    { 
      draw(con, level, levelOrig, unit);
      // Force x to flush it's buffers after we draw and before we sleep.
      XFlush(con.display);
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
  con.window = XCreateWindow(con.display, RootWindow(con.display, 0),
			     0, con.Y_OFFSET,
			     con.WINDOW_LEN, con.WINDOW_HEIGHT, 0,
			     CopyFromParent, CopyFromParent, CopyFromParent,
			     CWOverrideRedirect, &con.attribs);
  // 0x84ffdc cool colour.
  XSetWindowBackground(con.display, con.window, 0x1900ff);
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
      rc = XAllocNamedColor(con.display, con.cmap, "Purple", &con.purple,
			    &con.purple);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'purple' color.\n";
      exit(FATAL_ERROR_TWO_PURPLE);
    }
      rc = XAllocNamedColor(con.display, con.cmap, "Blue", &con.blue,
			    &con.blue);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'blue' color.\n";
      exit(FATAL_ERROR_TWO_BLUE);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Green", &con.green,
			  &con.green);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'green' color.\n";
      exit(FATAL_ERROR_TWO_GREEN);
    }
  rc = XAllocNamedColor(con.display, con.cmap, "Yellow", &con.yellow,
			&con.yellow);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'yellow' color.\n";
      exit(FATAL_ERROR_TWO_YELLOW);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Orange", &con.orange,
			  &con.orange);
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
    rc = XAllocNamedColor(con.display, con.cmap, "Dark Red", &con.darkRed,
			  &con.darkRed);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'dark red' color.\n";
      exit(FATAL_ERROR_TWO_DARK_RED);
    }
}


void draw(context & con, const int level, const int levelOrig,
	  const std::string unit)
{ // Black magic numbers in this function :'( (Can't be bothered to fix rn.)
  std::stringstream textInfo {};

  if(level < 100)
    textInfo<<' ';
  // Everything to the left of the level bar's
  textInfo<<unit.c_str()<<":       "<<levelOrig<<"%";
  XSetForeground(con.display, con.gc, con.cyan.pixel);//set forground colour
  XDrawString(con.display, con.window, con.gc, con.STR_X_OFFSET,
	      con.STR_Y, textInfo.str().c_str(),
	      textInfo.str().size());
  XDrawString(con.display, con.window, con.gc, con.MINUS_X_OFFSET, con.MINUS_Y,
	      "-", 1);
  XDrawString(con.display, con.window, con.gc, con.WINDOW_LEN - con.PLUS_X,
	      con.PLUS_Y, "+", 1);
  /* FROM: "https://tronche.com/gui/x/xlib/graphics/drawing/XDrawArc.html"
     For an arc specified as [ x, y, width, height, angle1, angle2 ], the angles
     must be specified in the effectively skewed coordinate system of the
     ellipse (for a circle, the angles and coordinate systems are identical).
     The relationship between these angles and angles expressed in the normal
     coordinate system of the screen (as measured with a protractor) is as
     follows:
     skewed - angle = atan ( tan ( normal-angle ) * width / height ) + adjust */
  // Right circle around "-"
  XDrawArc(con.display, con.window, con.gc, con.MINUS_ARC_X, con.MINUS_ARC_Y,
	   con.ARC_WIDTH, con.ARC_HEIGHT, con.ARC_ANGLE_1, con.ARC_ANGLE_2);
  // Left circle around "+".
  XDrawArc(con.display, con.window, con.gc, con.WINDOW_LEN - con.PLUS_ARC_X,
	   con.PLUS_ARC_Y, con.ARC_WIDTH, con.ARC_HEIGHT, con.ARC_ANGLE_1,
	   con.ARC_ANGLE_2); 
  drawBars(con, level);  //draw the level bars
}


void drawBars(context & con, const int level)
{
  constexpr int levels = 10;//levels should be a devisor of 100
  for(int nBar {}, nSpace{}; nBar < level; nBar +=levels, nSpace++)
    {
      std::stringstream barNumber {};
      barNumber<<(nSpace+1);
      if(nBar < 50)
        drawBar(con, con.green.pixel, nBar, nSpace);
      else
        if(nBar < 70)
          drawBar(con, con.yellow.pixel, nBar, nSpace);
        else
          if(nBar < 80)
            drawBar(con, con.orange.pixel, nBar, nSpace);
          else
            if(nBar < 90)
              drawBar(con, con.red.pixel, nBar, nSpace);
            else
              drawBar(con, con.darkRed.pixel, nBar, nSpace);
      barNumber.clear();
      }
}


void drawBar(context & con, const int cu, const int nBar, const int stride)
{				// Draw outer bar.
  XSetForeground(con.display, con.gc, cu);  
  XFillRectangle(con.display, con.window, con.gc,
		 (138 + nBar +
				  (stride*con.BAR_SPACE_SIZE)), con.OUTER_BAR_Y,
		 con.OUTER_BAR_WIDTH, con.OUTER_BAR_HEIGHT);
  // Draw inner bar.
  XSetForeground(con.display, con.gc, con.blue.pixel);
  XFillRectangle(con.display, con.window, con.gc,
		 (138 + nBar +
				  (stride*con.BAR_SPACE_SIZE) +
				  con.INNER_BAR_X_OFFSET),
		 con.INNER_BAR_Y, con.INNER_BAR_WIDTH, con.INNER_BAR_HEIGHT);
  XSetForeground(con.display, con.gc, con.cyan.pixel);
}


void printUsage(const std::string name, const int range_min,
		const int range_max)
{
  std::cerr<<"error!\nusage: "<<name<<" n\n"<<"Where n is in the range ["
	   <<range_min<<", "<<range_max<<"].\nIf no argument is given the "
    "program will try to\nread a number in the previously specified range "
    "from the standard input.\n";
}
