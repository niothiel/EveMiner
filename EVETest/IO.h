#pragma once

void leftClick();
void rightClick();
//void mouseEvent(bool left, bool down);
void moveMouse(int x, int y, int rMouse = 0);
void dragMouse(int fromX, int fromY, int toX, int toY);
void scrollUp();
void scrollDown();

void keyDown(unsigned short key);
void keyUp(unsigned short key);
void pressKey(unsigned short key);

/*
void ClickMouse();
void RightClickMouse();
void MoveMouse(int x, int y, int rmouse);
void _mouseEventHelper(bool left, bool down);

//void scrollMouseUp();
//void scrollMouseDown();
*/

void setFG();