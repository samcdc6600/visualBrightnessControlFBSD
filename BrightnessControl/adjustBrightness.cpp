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
  int windowLen;
};
constexpr int FATAL_ERROR_ONE		 	{-1}; // AdjustBR_Val returned -1.
constexpr int FATAL_ERROR_TWO	 		{-2}; // Could not open display.
constexpr int FATAL_ERROR_THREE_CYAN 		{-3}; // Failed to allocate color cyan.
constexpr int FATAL_ERROR_THREE_PURPLE	 	{-4}; // Failed to allocate color purple.
constexpr int FATAL_ERROR_THREE_BLUE 		{-5}; // Failed to allocate color blue.
constexpr int FATAL_ERROR_THREE_GREEN		{-6}; // Failed to allocate color green.
constexpr int FATAL_ERROR_THREE_YELLOW		{-7}; // Failed to allocate color yellow.
constexpr int FATAL_ERROR_THREE_ORANGE 		{-8}; // Failed to allocate color orange.
constexpr int FATAL_ERROR_THREE_RED		{-9}; // Failed to allocate color red.
constexpr int FATAL_ERROR_THREE_DARK_RED	{-10}; // Failed to allocate color dark red.


void printUsage(const std::string name);
// Read an int for the file f.
int getIntFromFile(const int rDefalut, const char f []);
// Checks that a is inbetween or equal to rMax and rMin and that (a % iGran == 0) it is a multiple of iGran!
bool checkBR_Val(const int rMin, const int rMax, const int iGran, const int a);
/* Returns (a + iGran) or (a - iGran) depending on whether arg is a '+' or a '-'.
   Additionally a will not be altered if altering it would cause it to fall outside of the range
   [rMin, rMax], in effect a saturates at these ranges. */
int adjustBR_Val(const int rMax, const int rMin, const int iGran, const char arg, const int a);
// Display brightness level on screen
void display(const int brLevel);
void init(context & con);
void draw(context & con, const int brLevel);
void drawBars(context & con, const int brLeve);
void drawBar(context & con, const int cu, const int nBar, const int nSpace, const std::string barNumber);
// Save int a to file at path f.
void saveIntToFile(const std::string f, const int a);
// Cange the brightness level, save the brightness level, show the brightness level.
void doWork(const int level, const char file []);
bool handle2ndArg(const char * argv [], const int BR_RANGE_MIN, const int BR_RANGE_MAX,
		  const int BR_INTERVAL_GRANULARITY, const int ARG_2_INDEX, int & brLevel);


/* Main requires 1 or 2 command line argument's (this includes the program name)
   the 2nd argument must be either '+' or '-', if there are 2 arguments. */
int main(const int argc, const char * argv[])
{
  constexpr int BR_RANGE_MIN {10}, BR_RANGE_MAX {100}, BR_INTERVAL_GRANULARITY {10}, BR_DEFAULT {80};
  constexpr int MAX_ARGC {2}, MIN_ARGC {1}, ARG_2_INDEX {1}; // ARG_1_INDEX {0}
  constexpr char brLevelFileName [] = "/usr/tmp/brLevel";
  int brLevel {getIntFromFile(BR_DEFAULT, brLevelFileName)}; // Attempt to get current brightness level.

  if(!checkBR_Val(BR_RANGE_MIN, BR_RANGE_MAX, BR_INTERVAL_GRANULARITY, brLevel))
    {
      std::cout<<"Brightness level stored in \""<<brLevelFileName<<"\", not evenly divisible "
	"by BR_INTERVAL_GRANULARITY ("<<BR_INTERVAL_GRANULARITY<<"). Setting brightness level to dealfult ("
	       <<BR_DEFAULT<<")\n";
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
	    if(!checkBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, brLevel))
	      brLevel = BR_DEFAULT;	// If we did not get a good brightness value from getIntFromFile.
	    doWork(brLevel, brLevelFileName);
	    return 0;
	  }
	else
	  printUsage(argv[0]);
    }





  /*
  if(argc == MAX_ARGC)
    {
      const char arg {argv[ARG_2_INDEX][0]}; // If there is only one argument argv[ARG_2_INDEX] should equal '\0' 
      
      if(argv[ARG_2_INDEX][1] == '\0' && (arg == '+' || arg == '-'))
	{			// We have a well formed second argument.
	  brLevel = adjustBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, arg, brLevel);
	  std::cout<<"level = "<<brLevel<<std::endl;
	  if(brLevel == -1)
	    {			// Arg did not match any case
	      puts("FATAL ERROR: adjustBR_Val returned -1");
	      return brLevel;
	    }
	  if(!checkBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, brLevel))
	    brLevel = BR_DEFAULT; // If we did not get a good brightness value from getIntFromFile.
	  doWork(brLevel, brLevelFileName);
	  return 0;
	}
	else
	  printUsage(argv[0]);	// Malformed input.
    }
  else
    if(argc == MIN_ARGC)
      {				/* Set brightness to level specified in brLevelFileName or if that value is out of
				   range set level to BR_DEFAULT 
	if(!checkBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, brLevel))
	  brLevel = BR_DEFAULT;	// If we did not get a good brightness value from getIntFromFile.
	doWork(brLevel, brLevelFileName);
	return 0;
      }
    else
      printUsage(argv[0]);
  
  return -1;*/
}


void printUsage(const std::string name)
{
  std::cerr<<"error!\nusage: "<<name<<" [options]\nOPTIONS\n\t+\tIncrease screen brightness\n\t-\tDecrease screen"
    "brightness\n";
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
      ret = rDefault;
    return ret;
}


bool checkBR_Val(const int rMin, const int rMax, const int iGran, const int a)
{
  if(a >= rMin && a <= rMax)
    if((a % iGran) == 0)
      return true;		// A has passed all the tests. :)
  return false;	// If we have reached this point a does not contain a valid value.
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
  return -1;
}


void display(const int brLevel)
{
  context con;
  init(con);
  for(int iter{}; iter < 9; ++iter) // 333*9 ~ 3000 (Sleep for 3 seconds.) 
    {
      draw(con, brLevel);
      XFlush(con.display); // Force x to flush it's buffers after we and draw before we sleep.
      std::this_thread::sleep_for(std::chrono::milliseconds(333));
    }
  XCloseDisplay(con.display);
}


void init(context & con)
{
  con.display = XOpenDisplay(nullptr);
  if( !con.display )
    {
      std::cerr<< "Cannot to open con.display.";
      exit(FATAL_ERROR_TWO);
    }
				// Get screen geometry.
  con.screenNum = DefaultScreen(con.display);
  con.displayWidth = DisplayWidth(con.display, con.screenNum);
  con.displayHeight = DisplayHeight(con.display, con.screenNum);
  
  con.attribs.override_redirect = 1; // Non bordered / decorated window.

  con.windowLen = 683;
  con.window = XCreateWindow(con.display, RootWindow(con.display, 0), con.displayWidth -con.windowLen, 2,
			     con.windowLen, 15, 0, CopyFromParent, CopyFromParent, CopyFromParent,
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
      exit(FATAL_ERROR_THREE_CYAN);
    }
      rc = XAllocNamedColor(con.display, con.cmap, "Purple", &con.purple, &con.purple);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'purple' color.\n";
      exit(FATAL_ERROR_THREE_PURPLE);
    }
      rc = XAllocNamedColor(con.display, con.cmap, "Blue", &con.blue, &con.blue);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'blue' color.\n";
      exit(FATAL_ERROR_THREE_BLUE);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Green", &con.green, &con.green);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'green' color.\n";
      exit(FATAL_ERROR_THREE_GREEN);
    }
  rc = XAllocNamedColor(con.display, con.cmap, "Yellow", &con.yellow, &con.yellow);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'yellow' color.\n";
      exit(FATAL_ERROR_THREE_YELLOW);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Orange", &con.orange, &con.orange);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'orange' color.\n";
      exit(FATAL_ERROR_THREE_ORANGE);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Red", &con.red, &con.red);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'red' color.\n";
      exit(FATAL_ERROR_THREE_RED);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Dark Red", &con.darkRed, &con.darkRed);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'dark red' color.\n";
      exit(FATAL_ERROR_THREE_DARK_RED);
    }
}


void draw(context & con, const int brLevel)
{
  std::stringstream textInfo {};
  /*  if(brLevel < 10)//it never goes below 20!
      textInfo<<"  ";*/
  if(brLevel < 100)
    textInfo<<' ';
  textInfo<<"%"<<brLevel<<" :ssenthgirB";//everything to the left of the brLevel bar's    
  XSetForeground(con.display, con.gc, con.cyan.pixel);//set forground colour
  XDrawString(con.display, con.window, con.gc, con.windowLen -100, 12, textInfo.str().c_str(), textInfo.str().size());
  XDrawString(con.display, con.window, con.gc, con.windowLen -117, 12, "-", 1);
  XDrawString(con.display, con.window, con.gc, 10, 12, "+", 1);
  XDrawArc(con.display, con.window, con.gc, 5, 0, 14, 14, 0, 360*64);//right circle around "-"
  XDrawArc(con.display, con.window, con.gc, con.windowLen -122, 0, 14, 14, 0, 360*64);//left circle around "+"
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


void drawBar(context & con, const int cu, const int nBar, const int nSpace, const std::string barNumber)
{
          XSetForeground(con.display, con.gc, cu);
          XFillRectangle(con.display, con.window, con.gc, con.windowLen -(174 + nBar + (nSpace*44)) , 1, 48, 13);
//        XFillRectangle(con.display, con.window, con.gc, 108 + nBar + (nSpace*18), 1, 16, 13);	  
          XSetForeground(con.display, con.gc, con.blue.pixel);
          XFillRectangle(con.display, con.window, con.gc, con.windowLen -(174 + nBar + (nSpace*44) - 3), 4, 42, 7);
//        XFillRectangle(con.display, con.window, con.gc, 108 + nBar + (nSpace*18) +3, 4, 10, 7);
          XSetForeground(con.display, con.gc, con.cyan.pixel);
}


void saveIntToFile(const std::string f, const int a)
{
  std::ofstream out(f.c_str());      
  out<<a<<'\0';
  out.close();
}


// Cange the brightness level, save the brightness level, show the brightness level.
void doWork(const int level, const char file [])
{
  std::stringstream command {};
  command<<"~/.config/brightness/brctl.sh "<<std::to_string(level);
  system(command.str().c_str()); // Change brightness level.
  std::cout<<"level = "<<level<<'\n';
  saveIntToFile(file, level); // Save brightness level.
  display(level);	// Show brightness level.
}


bool handle2ndArg(const char * argv [], const int BR_RANGE_MIN, const int BR_RANGE_MAX,
		  const int BR_INTERVAL_GRANULARITY, const int ARG_2_INDEX, int & brLevel)
{
  const char arg {argv[ARG_2_INDEX][0]}; // If there is only one argument argv[ARG_2_INDEX] should equal '\0' 
      
  if(argv[ARG_2_INDEX][1] == '\0' && (arg == '+' || arg == '-'))
    {			// We have a well formed second argument.
      brLevel = adjustBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, arg, brLevel);
      std::cout<<"level = "<<brLevel<<'\n';
      if(brLevel == -1)
	{			// Arg did not match any case
	  std::cout<<"FATAL ERROR: adjustBR_Val returned -1, brLevel = "<<brLevel<<'\n';
	  exit(FATAL_ERROR_ONE);
	}
      return true;
    }
  else
    {
      return false;
    }
}
