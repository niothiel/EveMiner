#pragma once

void ClickMouse();
void RightClickMouse();
void MoveMouse(int x, int y, int rmouse);
void _mouseEventHelper(bool left, bool down);

void keyDown(unsigned short key);
void keyUp(unsigned short key);
void pressKey(unsigned short key);

//void scrollMouseUp();
//void scrollMouseDown();

void setFG();