#include "stdafx.h"

#include <Windows.h>

extern HWND eveWindow;			// Our global handle to the eve window

//#define WM_IO					// Flag that specifies whether we use SendMessage() or SendInput()

int windowX = 0;				// Global values for keeping track of the mouse location.
int windowY = 0;

unsigned int lButton = 0;				// Flag denoting whether the left and right buttons are on.
unsigned int rButton = 0;

void _checkFG() {				// Checks whether eve is the foreground window, if not, make it so.
	if(GetForegroundWindow() != eveWindow) {
		//SetForegroundWindow(eveWindow);
		Sleep(400 + rand() % 250);
	}
}

/*
void _scrollWheelHelper(bool down) {		// We scroll down (towards the user, when true).
	INPUT mouseScroll;
	mouseScroll.type = INPUT_MOUSE;
	mouseScroll.mi.dwExtraInfo = NULL;
	mouseScroll.mi.time = 0;
	mouseScroll.mi.dwFlags = MOUSEEVENTF_WHEEL;

	if(down)
		mouseScroll.mi.mouseData = -WHEEL_DELTA;
	else
		mouseScroll.mi.mouseData = WHEEL_DELTA;

	SendInput(1, &mouseScroll, sizeof(mouseScroll));
}

void scrollMouseDown() {
	_scrollWheelHelper(true);
}

void scrollMouseUp() {
	_scrollWheelHelper(false);
}
*/

#ifdef WM_IO
void _mouseEventHelper(bool left, bool down) {
	DWORD flag;

	if(left) {
		if(down) {
			flag = WM_LBUTTONDOWN;
			lButton = MK_LBUTTON;
		}
		else {
			flag = WM_LBUTTONUP;
			lButton = 0;
		}
	}
	else {
		if(down) {
			flag = WM_RBUTTONDOWN;
			rButton = MK_RBUTTON;
		}
		else {
			flag = WM_RBUTTONUP;
			rButton = 0;
		}
	}

	SendMessage(eveWindow, flag, lButton | rButton, MAKELPARAM(windowX, windowY));
}
#else
void _mouseEventHelper(bool left, bool down) {
	INPUT mouse;
	mouse.type = INPUT_MOUSE;
	mouse.mi.mouseData = 0;
	mouse.mi.dwExtraInfo = NULL;
	mouse.mi.time = 0;

	DWORD flag;

	if(left) {								// Left mouse button
		if(down)							// Down
			flag = MOUSEEVENTF_LEFTDOWN;
		else								// Up
			flag = MOUSEEVENTF_LEFTUP;
	}
	else {									// Right mouse button
		if(down)							// Down
			flag = MOUSEEVENTF_RIGHTDOWN;
		else								// Up
			flag = MOUSEEVENTF_RIGHTUP;
	}

	mouse.mi.dwFlags = flag;

	SendInput(1, &mouse, sizeof(mouse));
}
#endif

void ClickMouse() {
	//SetForegroundWindow(eveWindow);
	Sleep(400);

	_mouseEventHelper(true, true);		// Left mouse down
	Sleep(200);
	_mouseEventHelper(true, false);		// Left mouse up
	Sleep(200);
}

void RightClickMouse() {
	//SetForegroundWindow(eveWindow);
	Sleep(400);

	_mouseEventHelper(false, true);
	Sleep(200);
	_mouseEventHelper(false, false);
	Sleep(200);
}

void MoveMouse(int x, int y, int rmouse) {
#ifdef WM_IO
	SendMessage(eveWindow, WM_MOUSEMOVE, lButton | rButton, MAKELPARAM(x, y));
//#else
	//SetForegroundWindow(eveWindow);
	Sleep(200);

	POINT p;
	p.x = x;
	p.y = y;

	ClientToScreen(eveWindow, &p);

	int finalX = p.x;
	int finalY = p.y;

	SetCursorPos(finalX, finalY);
#endif
	Sleep(200);

	if(rmouse == 1)
		ClickMouse();
	else if(rmouse == 2)
        RightClickMouse();
}

void _keyHelper(unsigned short key, bool down) {
#ifdef WM_IO
	DWORD flag;
	unsigned int lParam = 0;

	if(down) {
		flag = WM_KEYDOWN;
	}
	else {
		flag = WM_KEYUP;
		lParam = lParam | (0x1 << 31);				// Set the transition state for the keyup event.
	}

	SendMessage(eveWindow, WM_KEYDOWN, key, 0);
#else
	INPUT i;
	i.type = INPUT_KEYBOARD;
	i.ki.wVk = key;
	i.ki.time = 0;
	i.ki.wScan = 0;
	i.ki.dwExtraInfo = 0;

	if(down)
		i.ki.dwFlags = 0;
	else
		i.ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(1, &i, sizeof(i));
#endif
}

void keyDown(unsigned short key) {
	_keyHelper(key, true);
}

void keyUp(unsigned short key) {
	_keyHelper(key, false);
}

void pressKey(unsigned short key) {
	//SetForegroundWindow(eveWindow);
	Sleep(400 + rand() % 250);

	keyDown(key);
	Sleep(200);
	keyUp(key);
}