//******** Standerd Library
#include<stdio.h>
#include<iostream>
#include<stdlib.h> //for exit(0) function
#include<memory.h> //for memset() function


//*******  X11 Library
#include<X11/Xlib.h> //this is main file of Xlib, majority of Xlib symbols are declared in this file.
#include<X11/Xutil.h>  //this library contain symbols required for inter-client communication (?)
#include<X11/XKBlib.h> //for keyboard functions
#include<X11/keysym.h> //for key code to symbol conversion functions

//*******  OpenGL libraries
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glx.h>  //bridge library between opengl and X Window

//name space
using namespace std; //directives for all identifier in current name space. Directive means, it help to define the scope to identify the "functions, variables.." within current name space. It also organize the code logically"

bool bFullscreen=false;
Display *gpDisplay=NULL;  //This is used as Handle to XServer which can be used to request the server. All the member under this struct indicase the specifics of XServer that the client (program) connected to.

XVisualInfo *gpXVisualInfo=NULL;  //Single display can support multiple screens, each screen can have multiple visuals types supported at different depth.
Colormap gColormap;  //each xwindow is associated with color map which provides level of indirection between pixel value and color display on the screen.
Window gWindow;  // This struct holds all the information about the Window

int giWindowWidth=800;
int giWindowHeight=600;

int intX = 0, intY = 0;


GLXContext gGLXContext; //Created rendering context

int main(void)
{
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void display(void);
	void resize(int, int);
	void uninitialize();

	int winWidth=giWindowWidth;
	int winHeight = giWindowHeight;
	
	bool bDone=false;

	CreateWindow();

	initialize();

	XEvent event; //Every event in XWindow has its own struct (actually struct XAnyEvent) with type {GraphicExpose etc..}, XEvent is union of all the events.
	KeySym keysym;  //KeySym is the key symbol mapped with key code, when any key press event occur XWindow send KeyPress event alomg with KeyCode of the key pressed. KeyCode and KeySym are mapped together. KeyCode is Physycal Key lies between range of 8 to 255, in geometry fashion, so that it can be interpreted server-dependent fashion and desierd KeySym can be mapped. Mapping between Key and KeyCode can not be changed

	while(bDone==false)
	{
		while(XPending(gpDisplay))  //XPending return the event count (int) that are received from XServer for the display but not yet been removed from queue. If no event in list then it returns 0.
		{
			char ascii[26];
			XNextEvent(gpDisplay,&event);  //get next event from the event queue and then flush it from the queue for the display. If no event received then it flush the output buffer and block until an event is reveived.
			switch(event.type)
				{
					case MapNotify:
						break;
					case KeyPress:
						keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
						switch(keysym)
							{
								case XK_Escape:
									bDone=true;
									break;

								case XK_F:
								case XK_f:
									if(bFullscreen==false)
										{
											ToggleFullscreen();
											bFullscreen=true;
										}
									else
										{
											ToggleFullscreen();
											bFullscreen=false;	
										}
									break;
								default:
									break;
							}
						XLookupString(&event.xkey, ascii, sizeof(ascii),NULL, NULL);
						switch(ascii[0])
						{
							case '1': //R
								intX = 0;
								intY = winHeight / 2;	
								resize(winWidth/2, winHeight/2);
								//glViewport(0, (GLsizei)winHeight/ 2, (GLsizei)winWidth/4, (GLsizei)winHeight/4);
								break;
							case '2':
								intX = winWidth / 2;
								intY = winHeight / 2;
								resize(winWidth/2, winHeight/2);
								break;
							case '3':
								intX = 0;
								intY =0;
								resize(winWidth/2, winHeight/2);
								break;
							case '4': //R
								intX = winWidth/2;
								intY = 0;
								resize(winWidth/2, winHeight/2);
								break;
							case '5': //R
								intX = 0;
								intY = 0;
								resize(winWidth/2, winHeight);
								break;
							case '6': //R
								intX = winWidth / 2;
								intY = 0;
								resize(winWidth / 2, winHeight);
								break;

							case '7': //R
								intX = 0;
								intY = winHeight/2;
								resize(winWidth, winHeight/2);
								break;

							case '8': //R
								intX = 0;
								intY = 0;
								resize(winWidth, winHeight/2);
								break;

							case '9': //R
								intX = winWidth/4;
								intY = winHeight/4;
								resize(winWidth/2, winHeight/2 );
								break;
						}
						break;
					
					case ButtonPress:
						switch(event.xbutton.button)
						{
							case 1:
								break;
							case 2:
								break;
							case 3:
								break;
							default:
								break;
						}
						break;
					case MotionNotify:
						break;
					case ConfigureNotify:
						winWidth=event.xconfigure.width;
						winHeight=event.xconfigure.height;
						resize(winWidth,winHeight);
						break;
					case Expose:
						break;
					case DestroyNotify:
						break;
					case 33:
						bDone=true;
						break;
					default:
						break;
				}
		}
		display();
	}
	return(0);
}

void CreateWindow(void)
{
	void uninitialize(void);
	
	XSetWindowAttributes winAttribs;

	int defaultScreen;
	int defaultDepth;
	int styleMask;
	
	static int frameBufferAttributes[]=
		{
			GLX_RGBA, //Rendering type
			GLX_RED_SIZE,1,
			GLX_GREEN_SIZE,1,
			GLX_BLUE_SIZE,1,
			GLX_ALPHA_SIZE,1,
			None  //macro is used to mention array is getting stopped
		};
	gpDisplay=XOpenDisplay(NULL); //before program start use of display, we must create connection to XServer. To create connection to XServer we use XOpenDisplay function/macro. 
				// parameter to this function is char* displayName. if we have multiple display available then we can provide which display to be used. If we want to use default screen connected, then we can send NULL, it is POSICS standerd.

	if(gpDisplay==NULL)
	{
		printf("ERROR: Unable tp create connection to XServer. \n Exitting now... \n");
		uninitialize();
		exit(1);	
	}

	defaultScreen = XDefaultScreen(gpDisplay);

	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,
						RootWindow(gpDisplay,gpXVisualInfo->screen),
						gpXVisualInfo->visual,
						AllocNone);
	gColormap=winAttribs.colormap;
	
	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);
	
	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask;

	styleMask=CWBorderPixel|CWBackPixel|CWEventMask|CWColormap;

	gWindow=XCreateWindow(gpDisplay,
				RootWindow(gpDisplay,gpXVisualInfo->screen),
				0,
				0,
				giWindowWidth,
				giWindowHeight,
				0,
				gpXVisualInfo->depth,
				InputOutput,
				gpXVisualInfo->visual,
				styleMask,
				&winAttribs);

	if(!gWindow)
	{
		printf("ERROR: Unable to create window.\n Exiting now... \n");
		uninitialize();
		exit(1);
	}

	XStoreName(gpDisplay,gWindow, "Assignment 7 - ViewPortDivision	");

	Atom windowManagerDelete=XInternAtom(gpDisplay, "WM_DELETE_WINDOW",True);

	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);
	
}

void uninitialize(void)
{
	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();

	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}

	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay, gGLXContext);
	}
	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}
	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);
	}
	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;	
	}
}


void initialize(void)
{
	void resize(int,int);
	
	gGLXContext=glXCreateContext(gpDisplay, gpXVisualInfo,NULL,GL_TRUE);

	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	glClearColor(0.0f,0.0f,0.0f,0.0f);

	resize(giWindowWidth,giWindowHeight);

}

void display(void)
{
	
glLineWidth(2);

	glBegin(GL_TRIANGLES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.50f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.50f, -0.50f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.50f, -0.50f, 0.0f);

	glEnd();
	
	glXSwapBuffers(gpDisplay,gWindow);
}

void resize(int width, int height)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(0.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	if(height==0)
		height=1;

	glViewport(intX, intY, (GLsizei)width, (GLsizei)height);
	
	//gluPerspective(45, (GLfloat)width / (GLfloat)height, 100.0f, 100.0f);
}

void ToggleFullscreen(void)
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	
	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	memset(&xev,0,sizeof(xev));

	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1;

	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);

	xev.xclient.data.l[1]=fullscreen;

	XSendEvent(gpDisplay,
		RootWindow(gpDisplay,gpXVisualInfo->screen),
		False,
		StructureNotifyMask,
		&xev);
}