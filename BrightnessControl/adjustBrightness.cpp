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
// Brightness level constraints.
constexpr int BR_RANGE_MAX {100}, BR_RANGE_MIN {10}, BR_INTERVAL_GRANULARITY {10}, BR_DEFAULT {80};

void printUsage(const std::string name);
// Read an int for the file f.
int getIntFromFile(const int rDefalut, const std::string f);
// Checks that a is inbetween or equal to rMax and rMin and that (a % iGran == 0) it is a multiple of iGran!
bool checkBR_Val(const int rMax, const int rMin, const int iGran, const int a);
int adjustBR_Val(const int rMax, const int rMin, const int iGran, const char arg, const int a);
// Display brightness level on screen
void display(const int brLevel);
void init(context & con);
void draw(context & con, const int brLevel);
void drawBars(context & con, const int brLeve);
void drawBar(context & con, const int cu, const int nBar, const int nSpace, const std::string barNumber);
// Save new brightness value to the file f (no error checking done).
void saveIntToFile(const std::string f, const int a);

/* Main requires 1 or 2 command line argument's (this includes the program name)
   the 2nd argument must be either '+' or '-', if there are 2 arguments. */
int main(int argc, char * argv[])
{
  constexpr int MAX_ARGC {2}, MIN_ARGC {1}, ARG_1_INDEX {0}, ARG_2_INDEX {1};
  const std::string brLevelFileName {"/usr/tmp/brLevel"};
  int brLevel {getIntFromFile(BR_DEFAULT, brLevelFileName)}; // Attempt to get current brightness level.


  if(argc == MAX_ARGC)
    {
      const char arg {argv[ARG_2_INDEX][0]}; // If there is only one argument argv[ARG_2_INDEX] should equal '\0' 
      
      if(argv[ARG_2_INDEX][1] == '\0' && (arg == '+' || arg == '-'))
	{			// We have a well formed second argument.
	  brLevel = adjustBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, arg, brLevel);
	  std::cout<<"level = "<<brLevel<<std::endl;
	  if(brLevel == -1)
	    return brLevel;	// Arg did not match any case
	  if(!checkBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, brLevel))
	    brLevel = BR_DEFAULT; // If we did not get a good brightness value from getIntFromFile.	  
	  std::stringstream command {};
	  command<<"/usr/home/cyan/.config/brightness/brctl.sh "<<std::to_string(brLevel);
	  system(command.str().c_str());
	  saveIntToFile(brLevelFileName, brLevel);
	  display(brLevel);
	  return 0;
	}
	else
	  printUsage(argv[0]);
    }
  else
    if(argc == MIN_ARGC)
      {
	if(!checkBR_Val(BR_RANGE_MAX, BR_RANGE_MIN, BR_INTERVAL_GRANULARITY, brLevel))
	  brLevel = BR_DEFAULT;	// If we did not get a good brightness value from getIntFromFile.
	    
	std::stringstream command {};
	command<<"/usr/home/cyan/.config/brightness/brctl.sh "<<std::to_string(brLevel);
	system(command.str().c_str());
	std::cout<<"level = "<<brLevel<<std::endl;
	saveIntToFile(brLevelFileName, brLevel);
	display(brLevel);
	return 0;
      }
    else
      printUsage(argv[0]);
  
  return -1;
}


void printUsage(const std::string name)
{
  std::cerr<<"error!\nusage: "<<name<<" [options]\nOPTIONS\n\t+\tIncrease screen brightness\n\t-\tDecrease screen"
    "brightness\n";
}


int getIntFromFile(const int rDefault, const std::string f)
{
    int ret {};
    std::ifstream in(f.c_str());
    if(in.is_open())
      {
	in>>ret;
	in.close();
      }
    else
      ret = rDefault;
    return ret;
}


bool checkBR_Val(const int rMax, const int rMin, const int iGran, const int a)
{
  if(a >= rMin && a <= rMax)
    if(a % iGran == 0)
      return true;		// A has passed all the tests. :)
				// If we have reached this point 'a' should not hold a valid value.
  return false;
}


int adjustBR_Val(const int rMax, const int rMin, const int iGran, const char arg, const int a)
{
  switch(arg)
    {
    case '+':
      if(a == rMax)
	break;
      return a + iGran;
      
    case '-':
      if(a == rMin)
	break;
      return a - iGran;
    default:			// No matching case for arg!
      return -1;
    }
  return a;
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
      exit(1);
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
      exit(1);
    }
      rc = XAllocNamedColor(con.display, con.cmap, "Purple", &con.purple, &con.purple);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'purple' color.\n";
      exit(1);
    }
      rc = XAllocNamedColor(con.display, con.cmap, "Blue", &con.blue, &con.blue);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'blue' color.\n";
      exit(1);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Green", &con.green, &con.green);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'green' color.\n";
      exit(1);
    }
  rc = XAllocNamedColor(con.display, con.cmap, "Yellow", &con.yellow, &con.yellow);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'yellow' color.\n";
      exit(1);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Orange", &con.orange, &con.orange);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'orange' color.\n";
      exit(1);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Red", &con.red, &con.red);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'red' color.\n";
      exit(1);
    }
    rc = XAllocNamedColor(con.display, con.cmap, "Dark Red", &con.darkRed, &con.darkRed);
  if(rc == 0)
    {
      std::cerr<<"XAllocNamedColor - failed to allocated 'dark red' color.\n";
      exit(1);
    }
}


void draw(context & con, const int brLevel)
{
  std::stringstream textInfo {};
  /*  if(brLevel < 10)//it never goes below 20!
      textInfo<<"  ";*/
  if(brLevel < 100)
    textInfo<<" ";
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
  int level = brLeve / levels;
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
  out<<a<<"\0";
  out.close();
}
