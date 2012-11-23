// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream>

// TODO: reference additional headers your program requires here
using namespace std;

#define DIRECT_IO				// Flag that specifies whether we use SendMessage() or SendInput()
								// If defined, we use SendInput, and therefore need focus of the window at all times.
								// TODO: It's 6am, what the hell am I doing? More elegant solution maybe..
								// Did this because Util.cpp wasn't picking it up as defined in ensureFocus().