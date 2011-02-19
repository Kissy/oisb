#include <OISBGlobal.h>
#include <OISBSystem.h>
#include <OISBAction.h>
#include <OISBActionSchema.h>
#include <OISBAnalogAxisAction.h>
#include <OISBSequenceAction.h>
#include <OISBBinding.h>

#include <OISInputManager.h>

#include <iostream>
#include <sstream>

////////////////////////////////////Needed Windows Headers////////////
#if defined OIS_WIN32_PLATFORM
#  define WIN32_LEAN_AND_MEAN
#  include "windows.h"
#  ifdef min
#    undef min
#  endif
#include "resource.h"
   LRESULT DlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
//////////////////////////////////////////////////////////////////////
////////////////////////////////////Needed Linux Headers//////////////
#elif defined OIS_LINUX_PLATFORM
#  include <X11/Xlib.h>
   void checkX11Events();
//////////////////////////////////////////////////////////////////////
////////////////////////////////////Needed Mac Headers//////////////
#elif defined OIS_APPLE_PLATFORM
#  include <Carbon/Carbon.h>
   void checkMacEvents();
#endif
//////////////////////////////////////////////////////////////////////
using namespace OIS;

//-- Some local prototypes --//
void doStartup();

//-- Easy access globals --//
bool appRunning = true;				//Global Exit Flag

InputManager *g_InputManager = 0;	//Our Input System

//-- OS Specific Globals --//
#if defined OIS_WIN32_PLATFORM
  HWND hWnd = 0;
#elif defined OIS_LINUX_PLATFORM
  Display *xDisp = 0;
  Window xWin = 0;
#elif defined OIS_APPLE_PLATFORM
  WindowRef mWin = 0;
#endif

int main()
{
	std::cout << "\n\n*** OISB Console Demo App is starting up... *** \n";
	try
	{
		doStartup();
		std::cout << "\nStartup done... Hit 'q' or ESC to exit.\n\n";

		// load the schema from xml
		OISB::System::getSingleton().loadActionSchemaFromXML("example_schema.xml");


		OISB::ActionSchema *schema = OISB::System::getSingleton().getActionSchema("Default");
		OISB::Action *mExitAction = schema->getAction("Quit");

		while(appRunning)
		{
			//Throttle down CPU usage
			#if defined OIS_WIN32_PLATFORM
			  Sleep(90);
			  MSG  msg;
			  while( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			  {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			  }
			#elif defined OIS_LINUX_PLATFORM
			  checkX11Events();
			  usleep( 500 );
            #elif defined OIS_APPLE_PLATFORM
			  checkMacEvents();
			  usleep( 500 );
			#endif

			  //check if we should exit
			  if (mExitAction->isActive())
			  {
				  break;
			  }

			  // TODO: add proper timer in here
			  OISB::System::getSingleton().process(0);
		}
	}
	catch(std::exception &ex)
	{
		std::cout << "Caught std::exception: what = " << ex.what() << std::endl;
	}

	//Destroying the manager will cleanup unfreed devices
	if( g_InputManager )
		InputManager::destroyInputSystem(g_InputManager);

#if defined OIS_LINUX_PLATFORM
	// Be nice to X and clean up the x window
	XDestroyWindow(xDisp, xWin);
	XCloseDisplay(xDisp);
#endif

	std::cout << "\n\nGoodbye\n\n";
	return 0;
}

void doStartup()
{
	ParamList pl;

#if defined OIS_WIN32_PLATFORM
	//Create a capture window for Input Grabbing
	hWnd = CreateDialog( 0, MAKEINTRESOURCE(IDD_DIALOG1), 0,(DLGPROC)DlgProc);
	if( hWnd == NULL )
	{
		std::cout << "Failed to create Win32 Window Dialog!" << std::endl;
		exit(1);
	}

	ShowWindow(hWnd, SW_SHOW);

	std::ostringstream wnd;
	wnd << (size_t)hWnd;

	pl.insert(std::make_pair( std::string("WINDOW"), wnd.str() ));

	//Default mode is foreground exclusive..but, we want to show mouse - so nonexclusive
//	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
//	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
	//Connects to default X window
	if( !(xDisp = XOpenDisplay(0)) )
		OIS_EXCEPT(E_General, "Error opening X!");
	//Create a window
	xWin = XCreateSimpleWindow(xDisp,DefaultRootWindow(xDisp), 0,0, 100,100, 0, 0, 0);
	//bind our connection to that window
	XMapWindow(xDisp, xWin);
	//Select what events we want to listen to locally
	XSelectInput(xDisp, xWin, StructureNotifyMask);
	XEvent evtent;
	do
	{
		XNextEvent(xDisp, &evtent);
	} while(evtent.type != MapNotify);

	std::ostringstream wnd;
	wnd << xWin;

	pl.insert(std::make_pair(std::string("WINDOW"), wnd.str()));

	//For this demo, show mouse and do not grab (confine to window)
//	pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
//	pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
#elif defined OIS_APPLE_PLATFORM
    // create the window rect in global coords
    ::Rect windowRect;
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = 300;
    windowRect.bottom = 300;
    
    // set the default attributes for the window
    WindowAttributes windowAttrs = kWindowStandardDocumentAttributes
        | kWindowStandardHandlerAttribute 
        | kWindowInWindowMenuAttribute
        | kWindowHideOnFullScreenAttribute;
    
    // Create the window
    CreateNewWindow(kDocumentWindowClass, windowAttrs, &windowRect, &mWin);
    
    // Color the window background black
    SetThemeWindowBackground (mWin, kThemeBrushBlack, true);
    
    // Set the title of our window
    CFStringRef titleRef = CFStringCreateWithCString( kCFAllocatorDefault, "OIS Input", kCFStringEncodingASCII );
    SetWindowTitleWithCFString( mWin, titleRef );
    
    // Center our window on the screen
    RepositionWindow( mWin, NULL, kWindowCenterOnMainScreen );
    
    // Install the event handler for the window
    InstallStandardEventHandler(GetWindowEventTarget(mWin));
    
    // This will give our window focus, and not lock it to the terminal
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType( &psn, kProcessTransformToForegroundApplication );
	SetFrontProcess(&psn);
    
    // Display and select our window
    ShowWindow(mWin);
    SelectWindow(mWin);

    std::ostringstream wnd;
	wnd << (unsigned int)mWin; //cast to int so it gets encoded correctly (else it gets stored as a hex string)
    std::cout << "WindowRef: " << mWin << " WindowRef as int: " << wnd.str() << "\n";
	pl.insert(std::make_pair(std::string("WINDOW"), wnd.str()));
#endif

	//This never returns null.. it will raise an exception on errors
	g_InputManager = InputManager::createInputSystem(pl);

    new OISB::System();
    OISB::System::getSingleton().initialize(g_InputManager);
}

#if defined OIS_WIN32_PLATFORM
LRESULT DlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return FALSE;
}
#endif

#if defined OIS_LINUX_PLATFORM
//This is just here to show that you still recieve x11 events, as the lib only needs mouse/key events
void checkX11Events()
{
	XEvent event;

	//Poll x11 for events (keyboard and mouse events are caught here)
	while( XPending(xDisp) > 0 )
	{
		XNextEvent(xDisp, &event);
		//Handle Resize events
		if( event.type == ConfigureNotify )
		{
			if( g_m )
			{
				const MouseState &ms = g_m->getMouseState();
				ms.width = event.xconfigure.width;
				ms.height = event.xconfigure.height;
			}
		}
		else if( event.type == DestroyNotify )
		{
			std::cout << "Exiting...\n";
			appRunning = false;
		}
		else
			std::cout << "\nUnknown X Event: " << event.type << std::endl;
	}
}
#endif

#if defined OIS_APPLE_PLATFORM
void checkMacEvents()
{	
	//TODO - Check for window resize events, and then adjust the members of mousestate
	EventRef event = NULL;
	EventTargetRef targetWindow = GetEventDispatcherTarget();
	
	if( ReceiveNextEvent( 0, NULL, kEventDurationNoWait, true, &event ) == noErr )
	{
		SendEventToEventTarget(event, targetWindow);
		std::cout << "Event : " << GetEventKind(event) << "\n";
		ReleaseEvent(event);
	}
}
#endif
