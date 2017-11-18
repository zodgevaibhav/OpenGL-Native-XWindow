//******** Standerd Library
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<memory.h>	

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

FILE * gpFile=NULL;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

bool gbFullscreen=false;

Display *gpDisplay=NULL;  //This is used as Handle to XServer which can be used to request the server. All the member under this struct indicase the specifics of XServer that the client (program) connected to.

XVisualInfo *gpXVisualInfo=NULL;  //Single display can support multiple screens, each screen can have multiple visuals types supported at different depth.
Colormap gColormap;  //each xwindow is associated with color map which provides level of indirection between pixel value and color display on the screen.
Window gWindow;  // This struct holds all the information about the Window


typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB=NULL;
GLXFBConfig gGLXFBConfig;
GLXContext gGLXContext; //parallel to HGLRC


int main(void)
{
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void display(void);
	void resize(int, int);
	void uninitialize();

	// create log file
	gpFile=fopen("Log.txt", "w");
	if (gpFile==NULL)
	{
		printf("ERROR: Log File Can Not Be Created. Exitting Now ...\n");
		exit(0);
	}
	else
	{
		fprintf(gpFile, "INFO: Log File Is Successfully Opened.\n");
	}
	
	CreateWindow();
	
	initialize();
	
	XEvent event; //parallel to 'MSG' structure
	KeySym keySym;
	int winWidth;
	int winHeight;
	bool bDone=false;
	
	while(bDone==false) //Game loop
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event); //parallel to GetMessage()
			switch(event.type) //parallel to 'iMsg'
			{
				case MapNotify: //parallel to WM_CREATE
					break;
				case KeyPress: //parallel to WM_KEYDOWN
					keySym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
					switch(keySym)
					{
						case XK_Escape:
							bDone=true; //Close on Escape key press
							break;
						case XK_F:
						case XK_f:
							if(gbFullscreen==false)
							{
								ToggleFullscreen();
								gbFullscreen=true;
							}
							else
							{
								ToggleFullscreen();
								gbFullscreen=false;
							}
							break;
						default:
							break;
					}
					break;
				case ButtonPress:
					switch(event.xbutton.button)
					{
						case 1:	//Button pressed events
							break;
						case 2:
							break;
						case 3:
							break;
						default: 
							break;
					}
					break;
				case MotionNotify: //parallel to WM_MOUSEMOVE
					break;
					
				case ConfigureNotify: //parallel to WM_SIZE
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
					
				case Expose: //parallel to WM_PAINT
					break;
					
				case DestroyNotify:
					break;
					
				case 33: //close button, system menu -> close
					bDone=true;
					break;
				default:
					break;
			}
		}
		
		display();
	}
	
	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	//function declaration
	void uninitialize(void);
	
	//variable declarations
	XSetWindowAttributes winAttribs;
	
	GLXFBConfig *pGLXFBConfigs=NULL;
	GLXFBConfig bestGLXFBConfig;
	
	XVisualInfo *pTempXVisualInfo=NULL;
	
	int iNumFBConfigs=0;
	int styleMask;
	int i;
	
	static int frameBufferAttributes[]={
		GLX_X_RENDERABLE,True,
		GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
		GLX_RED_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DEPTH_SIZE,24,
		GLX_STENCIL_SIZE,8,
		GLX_DOUBLEBUFFER,True,
		//GLX_SAMPLE_BUFFERS,1,
		//GLX_SAMPLES,4,
		None}; // array must be terminated by 0
	
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("ERROR0: Unable To Obtain X Display.\n");
		uninitialize();
		exit(1);
	}
	
	//****************************** get a new framebuffer config that meets our attrib requirements
	pGLXFBConfigs=glXChooseFBConfig(gpDisplay,DefaultScreen(gpDisplay),frameBufferAttributes,&iNumFBConfigs);
	if(pGLXFBConfigs==NULL)
	{
		printf( "ERROR: Failed To Get Valid Framebuffer Config. Exitting Now ...\n");
		uninitialize();
		exit(1);
	}
	
	printf("%d Matching FB Configs Found.\n",iNumFBConfigs);
	
	//****************************** pick that FB config/visual with the most samples per pixel
	int bestFramebufferconfig=-1,worstFramebufferConfig=-1,bestNumberOfSamples=-1,worstNumberOfSamples=999;
	for(i=0;i<iNumFBConfigs;i++)
	{
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfigs[i]);
		if(pTempXVisualInfo)
		{
			int sampleBuffer,samples;
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLE_BUFFERS,&sampleBuffer);
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLES,&samples);
			printf("Matching Framebuffer Config=%d : Visual ID=0x%lu : SAMPLE_BUFFERS=%d : SAMPLES=%d\n",i,pTempXVisualInfo->visualid,sampleBuffer,samples);
			if(bestFramebufferconfig < 0 || sampleBuffer && samples > bestNumberOfSamples)
			{
				bestFramebufferconfig=i;
				bestNumberOfSamples=samples;
			}
			if( worstFramebufferConfig < 0 || !sampleBuffer || samples < worstNumberOfSamples)
			{
				worstFramebufferConfig=i;
			    worstNumberOfSamples=samples;
			}
		}
		XFree(pTempXVisualInfo);
	}
	bestGLXFBConfig = pGLXFBConfigs[bestFramebufferconfig];
	
	// set global GLXFBConfig
	gGLXFBConfig=bestGLXFBConfig;
	
	// be sure to free FBConfig list allocated by glXChooseFBConfig()
	XFree(pGLXFBConfigs);
	
	fprintf(gpFile, "INFO: Get visual infor From best GLXFBConfig.\n");
	gpXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);
	printf("Chosen Visual ID=0x%lu\n",gpXVisualInfo->visualid );
	
	//setting window attributes
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,
										RootWindow(gpDisplay,gpXVisualInfo->screen), //we can give default Screen as well
										gpXVisualInfo->visual,
										AllocNone); 	//for 'movable' memory allocation
	
	fprintf(gpFile, "INFO: CreateColorMap done.\n");
	winAttribs.event_mask=StructureNotifyMask | KeyPressMask | ButtonPressMask |
						  ExposureMask | VisibilityChangeMask | PointerMotionMask;
	
	styleMask=CWBorderPixel | CWEventMask | CWColormap;
	gColormap=winAttribs.colormap;										           
	
	gWindow=XCreateWindow(gpDisplay,
						  RootWindow(gpDisplay,gpXVisualInfo->screen),
						  0,
						  0,
						  WIN_WIDTH,
						  WIN_HEIGHT,
						  0, 		//border width
						  gpXVisualInfo->depth, //depth of visual (depth for Colormap)          
						  InputOutput, //class type of window
						  gpXVisualInfo->visual,
						  styleMask,
						  &winAttribs);
	fprintf(gpFile, "INFO: CreateWindow done.\n");	
	
	if(!gWindow)
	{
		printf("ERROR: Unable to create window.\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay,gWindow,"OpenGL Window");
	
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_WINDOW_DELETE",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);
	fprintf(gpFile, "OUT: Create window.\n");
}

void ToggleFullscreen(void)
{

	Atom wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False); //Normal window state
	
	XEvent event;
	memset(&event,0,sizeof(XEvent));  // Memory allocation of size structure XEvent
	
	event.type=ClientMessage;
	event.xclient.window=gWindow;
	event.xclient.message_type=wm_state;
	event.xclient.format=32; //32-bit
	event.xclient.data.l[0]=gbFullscreen ? 0 : 1;

	Atom fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);	
	event.xclient.data.l[1]=fullscreen;
	
	//parallel to SendMessage()
	XSendEvent(gpDisplay,
			   RootWindow(gpDisplay,gpXVisualInfo->screen),
			   False, 				//do not send this message to Sibling windows
			   StructureNotifyMask, //resizing mask (event_mask)
			   &event);	
}

void initialize(void)
{
	fprintf(gpFile, "IN: initialize() .\n");
	
	void uninitialize(void);
	void resize(int,int);
	
//****************************** create a new GL context 4.5 for rendering (My PC Graphics Card OpenGL 4.4
	
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
	fprintf(gpFile, "INFO: glXCreateContextAttribsARB. Done\n");
	
	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,4,
		GLX_CONTEXT_MINOR_VERSION_ARB,4,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0 }; // array must be terminated by 0, to say it is end of array
	
	fprintf(gpFile, "INFO: glXCreateContextAttribsARB before .\n");
	gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
	fprintf(gpFile, "INFO: CreateContextAttribs done .\n");
	
	if(!gGLXContext) // fallback to safe old style 2.x context
	{
		// When a context version below 3.0 is requested, implementations will return 
		// the newest context version compatible with OpenGL versions less than version 3.0.
		GLint attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			0 }; // array must be terminated by 0
		printf("ERROR: Failed To Create GLX 4.4 context. Hence Using Old-Style GLX Context\n");
		gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
	}
	else // successfully created 4.4 context
	{
		printf("INFO: OpenGL Context 4.4 Is Created.\n");
	}
	
	// verifying that context is a direct context
	if(!glXIsDirect(gpDisplay,gGLXContext))
	{
		printf("INFO: Indirect GLX Rendering Context Obtained\n");
	}
	else
	{
		printf("INFO: Direct GLX Rendering Context Obtained\n" );
	}
	
	fprintf(gpFile, "INFO: make current context as OpenGL context .\n");
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	//code
	glShadeModel(GL_SMOOTH);
	// set-up depth buffer
	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	// set really nice percpective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	// We will always cull back faces for better performance
	glEnable(GL_CULL_FACE);

	// set background clearing color
	glClearColor(0.0f,0.0f,1.0f,0.0f); // blue
	
	// resize
	resize(WIN_WIDTH, WIN_HEIGHT);
	fprintf(gpFile, "OUT: initialize() .\n");
}

void resize(int width,int height)
{
	if(height==0)
		height=1;		
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glXSwapBuffers(gpDisplay,gWindow);
}

void uninitialize(void)
{
	//************************************ Releasing OpenGL related and XWindow related objects 	
	GLXContext currentContext=glXGetCurrentContext();
	if(currentContext!=NULL && currentContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}
	
	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay,gGLXContext);
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

	if (gpFile)
	{
		fprintf(gpFile, "INFO: Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
	
	
	