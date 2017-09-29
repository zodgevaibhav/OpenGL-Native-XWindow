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

float angle = 0;

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
			GLX_RED_SIZE,8,    //for double buffer it is set to 8 lease it is 1 for single buffer
			GLX_GREEN_SIZE,8,
			GLX_BLUE_SIZE,8,
			GLX_ALPHA_SIZE,8,
			GLX_DEPTH_SIZE,24,   //for double buffer it is set to 24
			GLX_DOUBLEBUFFER,True,
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

	XStoreName(gpDisplay,gWindow, "Assignment 6 - 3D Rotating Shape");

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
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	
	glLoadIdentity();
	
	angle=angle+0.1;
	
	if(angle>360)
		angle=0;
	
	glTranslatef(-2.0f, 0.0f, -6.0f);
	glRotatef(angle, 0.0f, 1.0f, 0.0f);

	glBegin(GL_TRIANGLES);
	//FRONT FACE
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //apex

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-corner of front face

	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, -1.0f, 1.0f); //right-corner of front face

								   //RIGHT FACE
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //apex

	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, -1.0f, 1.0f); //left-corner of right face

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, -1.0f, -1.0f); //right-corner of right face

									//BACK FACE
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //apex

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, -1.0f, -1.0f); //left-corner of back face

	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(-1.0f, -1.0f, -1.0f); //right-corner of back face

									 //LEFT FACE
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //apex

	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-corner of left face

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(-1.0f, -1.0f, 1.0f); //right-corner of left face
	glEnd();
	//*********************************************************************************************

	glLoadIdentity();
	glTranslatef(2.0f, 0.0f, -6.0f);
	glScalef(0.75f, 0.75f, 0.75f);
	glRotatef(angle, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	//TOP FACE
	glColor3f(1.0f, 0.0f, 0.0f); //RED
	glVertex3f(1.0f, 1.0f, -1.0f);  //right-top corner of top face
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top corner of top face
	glVertex3f(-1.0f, 1.0f, 1.0f); //left-bottom corner of top face
	glVertex3f(1.0f, 1.0f, 1.0f); //right-bottom corner of top face

								  //BOTTOM FACE
	glColor3f(0.0f, 1.0f, 0.0f); //GREEN
	glVertex3f(1.0f, -1.0f, -1.0f); //right-top corner of bottom face
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-top corner of bottom face
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-bottom corner of bottom face
	glVertex3f(1.0f, -1.0f, 1.0f); //right-bottom corner of bottom face

								   //FRONT FACE
	glColor3f(0.0f, 0.0f, 1.0f); //BLUE
	glVertex3f(1.0f, 1.0f, 1.0f); //right-top corner of front face
	glVertex3f(-1.0f, 1.0f, 1.0f); //left-top corner of front face
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-bottom corner of front face
	glVertex3f(1.0f, -1.0f, 1.0f); //right-bottom corner of front face

								   //BACK FACE
	glColor3f(0.0f, 1.0f, 1.0f); //CYAN
	glVertex3f(1.0f, 1.0f, -1.0f); //right-top of back face
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top of back face
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom of back face
	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom of back face

									//RIGHT FACE
	glColor3f(1.0f, 0.0f, 1.0f); //MAGENTA
	glVertex3f(1.0f, 1.0f, -1.0f); //right-top of right face
	glVertex3f(1.0f, 1.0f, 1.0f); //left-top of right face
	glVertex3f(1.0f, -1.0f, 1.0f); //left-bottom of right face
	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom of right face

									//LEFT FACE
	glColor3f(1.0f, 1.0f, 0.0f); //YELLOW
	glVertex3f(-1.0f, 1.0f, 1.0f); //right-top of left face
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top of left face
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom of left face
	glVertex3f(-1.0f, -1.0f, 1.0f); //right-bottom of left face
	glEnd();


	glXSwapBuffers(gpDisplay,gWindow);
}

void resize(int width, int height)
{
	
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
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
















