#include "Main.h"

HDC			hDC=NULL;
HGLRC		hRC=NULL;
HWND		hWnd=NULL;
HINSTANCE	hInstance;

bool	keys[256];
bool	active = true;

float colors[4][3] = {0};

const int	wallWidth = 20,
			wallHeight = 8;
bool bricks[wallHeight * wallWidth];

float paddlePosition = 0.0f;

float ballDirection = -135.0f;
float ballX = 45.0f, ballY = 0.0f;

bool rebuild = false;

bool play = false;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height == 0)
		height = 1;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glOrtho(-200.0f, 200.0f, -150.0f, 150.0f, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return true;
}

int DrawGLScene(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// BRICKS RENDER
	glLoadIdentity();
	glTranslatef(-110.0f, 30.0f, 1.0f);
	for (int i = 0; i < wallHeight * wallWidth; i++)
	{
		glColor3f(colors[(i / wallWidth) / 2][0], 
				  colors[(i / wallWidth) / 2][1], 
				  colors[(i / wallWidth) / 2][2]);
		glTranslatef(10.0f, 0.0f, 0.0f);
		if (i % wallWidth == 0 && i > 0)
			glTranslatef(-10.0f * wallWidth, 3.0f, 0.0f);
		if (bricks[i] == false) continue;
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 2.0f, 0.0f);
		glVertex3f(9.0f, 2.0f, 0.0f);
		glVertex3f(9.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glEnd();
	}

	// PADDLE RENDER
	glLoadIdentity();
	glTranslatef(0.0f, -50.0f, 1.0f);
	glColor3f(1.0f, 0.75f, 0.5f);
	glBegin(GL_QUADS);
	glVertex3f(-7.0f + paddlePosition, 2.0f, 0.0f);
	glVertex3f(7.0f + paddlePosition, 2.0f, 0.0f);
	glVertex3f(7.0f + paddlePosition, 0.0f, 0.0f);
	glVertex3f(-7.0f + paddlePosition, 0.0f, 0.0f);
	glEnd();
	
	// BALL RENDER
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	glVertex3f( 0.0f + ballX, -1.0f + ballY, 1.0f);
	glVertex3f( 1.5f + ballX,  0.0f + ballY, 1.0f);
	glVertex3f( 0.0f + ballX,  1.0f + ballY, 1.0f);
	glVertex3f( 0.0f + ballX, -1.0f + ballY, 1.0f);
	glVertex3f(-1.5f + ballX,  0.0f + ballY, 1.0f);
	glVertex3f( 0.0f + ballX,  1.0f + ballY, 1.0f);
	glEnd();
	return true;
}

GLvoid KillGLWindow(GLvoid)
{
	if (hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		if (!wglDeleteContext(hRC))
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);

		hRC=NULL;
	}

	if (hDC && !ReleaseDC(hWnd, hDC))
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}

	if (hWnd && !DestroyWindow(hWnd))
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}

	if (!UnregisterClass("OpenGL", hInstance))
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

bool CreateGLWindow(char* title, int width, int height, int bits, bool fullscreen)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen = false;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return true;									// Success
}

LRESULT CALLBACK WndProc(HWND hWnd,
						 UINT uMsg,
						 WPARAM wParam,
						 LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_ACTIVATE:
		{
			if (!HIWORD(wParam))
				active = true;
			else
				active = false;
			return 0;
		}

		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				return 0;
			}
			break;
		}

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_KEYDOWN:
		{
			keys[wParam] = true;
			return 0;
		}

		case WM_KEYUP:
		{
			keys[wParam] = false;
			return 0;
		}

		case WM_SIZE:
		{
			ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	MSG msg;
	bool done = false;
	for (int i = 0; i < wallHeight * wallWidth; i++)
		bricks[i] = true;
	
	colors[0][0] = colors[0][1] = colors[1][0] = colors[1][2] =
		colors[2][1] = colors[2][2] = 1.0f;
	colors[0][2] = colors[1][1] = colors[2][0] = 0.5f;
	colors[3][0] = 0.75f;
	colors[3][1] = 0.50f;
	colors[3][2] = 1.00f;

	if (!CreateGLWindow("BreakGL", 1024, 768, 24, false))
		return 0;
	

	int oldTime = 0;
	while (!done)
	{
		int time = timeGetTime();
		int deltaTime = time - oldTime;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)
				done = true;
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else
		{
			if (active)
			{
				if (play)
				{
					ballX += (float) cos(ballDirection * PI / 180) * deltaTime * 0.3f;
					ballY += (float) sin(ballDirection * PI / 180) * deltaTime * 0.3f;
					if (ballX > 100 || ballX < -100)
						ballDirection = 180 - ballDirection;
					if (ballY > 60)
						ballDirection = - ballDirection;
					if (ballY < -65)
					{
						paddlePosition = 0.0f;
						ballDirection = -135.0f;
						ballX = 45.0f;
						ballY = 0.0f;
						play = false;
					}

					if (ballY <= -48 && ballY > -50 && ballX >= -7 + paddlePosition && ballX <= 7 + paddlePosition)
					{
						if (rebuild)
						{
							for (int i = 0; i < wallWidth * wallHeight; i++)
								bricks[i] = true;
							rebuild = false;
						}

						float tmp = (paddlePosition - ballX) / 7;
						ballDirection = float(acos(tmp) * 180.0 / PI) +((paddlePosition - ballX < 7) ? 90 : 0);
					}

					for (int i = 0; i < wallWidth * wallHeight; i++)
					{
						if (bricks[i] == false)
							continue;
						float brickX1, brickX2, brickY1, brickY2;
						brickX1 = -100.0f + (10.0f * (i % wallWidth));
						brickX2 = brickX1 + 9.0f;
						brickY1 = 30.0f + (3.0f * (i / wallWidth));
						brickY2 = brickY1 + 2.0f;
						if (ballX + 1.5f > brickX1 && ballX - 1.5f < brickX2 && 
							ballY + 1 > brickY1 && ballY - 1 < brickY2)
						{
							if (ballX < brickX1 + 0.1f || ballX > brickX2 - 0.1f && ballY < brickY2)
								ballDirection = 180 - ballDirection;
							else
								ballDirection = -ballDirection;
							bricks[i] = false;
							rebuild = true;
							for (int j = 0; j < wallWidth * wallHeight; j++)
								if (bricks[j])
									rebuild = false;
							break;
						}
					}
					if (keys[VK_LEFT])
						paddlePosition -= 0.2f * deltaTime;
					if (keys[VK_RIGHT])
						paddlePosition += 0.2f * deltaTime;
				}

				if (keys[VK_ESCAPE])
					done = true;
				if (!play && keys[VK_SPACE])
					play = true;

				paddlePosition = CLAMP(paddlePosition, -93, 90);
				DrawGLScene();
				SwapBuffers(hDC);
			}
		}
		oldTime = timeGetTime();
		Sleep(1000 / 200);
	}
	return 0;
}