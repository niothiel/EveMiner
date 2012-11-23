#include "stdafx.h"

#include "IO.h"
#include "Util.h"

extern HWND eveWindow;			// Our global handle to the eve window

#ifdef DIRECT_IO

void _mouseButton(bool left, bool down) {
	ensureFocus();

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

void _mouseMove(int x, int y) {
	POINT p;
	p.x = x;
	p.y = y;

	ClientToScreen(eveWindow, &p);

	SetCursorPos(p.x, p.y);
}

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

void _keyHelper(unsigned short key, bool down) {
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
}

#else

int windowX = 0;				// Global values for keeping track of the mouse location.
int windowY = 0;

unsigned int lButton = 0;		// Flag denoting whether the left and right buttons are on.
unsigned int rButton = 0;

void mouseEvent(bool left, bool down) {
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

void _moveHelper(int x, int y) {
	SendMessage(eveWindow, WM_MOUSEMOVE, lButton | rButton, MAKELPARAM(x, y));
}

void _scrollWheelHelper(bool down) {		// We scroll down (towards the user, when true).

}

void _keyHelper(unsigned short key, bool down) {
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
}

#endif

void leftClick() {
	Sleep(400);

	_mouseButton(true, true);		// Left mouse down
	Sleep(200);
	_mouseButton(true, false);		// Left mouse up
	Sleep(200);
}

void rightClick() {
	Sleep(400);

	_mouseButton(false, true);
	Sleep(200);
	_mouseButton(false, false);
	Sleep(200);
}

void moveMouse(int x, int y, int rMouse) {
	ensureFocus();

	_mouseMove(x, y);
	Sleep(200);

	if(rMouse == 1)
		leftClick();
	else if(rMouse == 2)
        rightClick();
}

void dragMouse(int fromX, int fromY, int toX, int toY) {
	ensureFocus();

	_mouseMove(fromX, fromY);				// Go to the from position
	_mouseButton(true, true);				// Mouse down
	Sleep(200);

	_mouseMove(toX, toY);					// Go to the to position
	_mouseButton(true, false);				// Mouse down
	Sleep(200);
}

void scrollUp() {
	ensureFocus();
	_scrollWheelHelper(false);
}

void scrollDown() {
	ensureFocus();
	_scrollWheelHelper(true);
}

void keyDown(unsigned short key) {
	ensureFocus();
	_keyHelper(key, true);
}

void keyUp(unsigned short key) {
	ensureFocus();
	_keyHelper(key, false);
}

void pressKey(unsigned short key) {
	Sleep(400);

	keyDown(key);
	Sleep(200);
	keyUp(key);
	Sleep(200);
}