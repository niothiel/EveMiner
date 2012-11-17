#ifndef IO_H
#define IO_H

void ClickMouse();
void RightClickMouse();
void MoveMouse(int x, int y, int rmouse);
void _mouseEventHelper(bool left, bool down);

void keyDown(unsigned short key);
void keyUp(unsigned short key);
void pressKey(unsigned short key);

//void scrollMouseUp();
//void scrollMouseDown();

#endif