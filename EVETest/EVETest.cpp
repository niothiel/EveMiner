#include "stdafx.h"

#include "IO.h"
#include "OpenCV.h"
#include "Timer.h"
#include "Util.h"

BOOL CALLBACK FindEveWindowProc(HWND hwnd, LPARAM lParam);
bool selectRightClickMenu(string firstAction);
bool selectRightClickMenu(string firstAction, string secondAction);

bool undock();
void depositOre();
bool approachAndFireMiningLasers();
bool selectAsteroid();
bool jumpToSystem(string systemName);

void openOverviewMine();
void openOverviewGates();
void openOverviewStations();
bool openOreHold();
bool openInv();

bool isAsteroidLocked();
bool isInWarp();
bool isDocked();
bool isInvOpen();
bool isInvFull();
bool isStopped();
bool isInSystem(string systemName);

bool waitToStop();
bool waitForSystem(string systemName);
bool waitForMinerDone();
bool waitForWarp();
bool waitUntilDocked();

void getNumber();
void setOverviewLocation();
void moveMouseAway();
void clickOnShip();

Timer t(1);			// The run timer.
Point overviewLoc;	// Location of the first entry in the overview, set when undocked.
bool overviewLocSet = false;// Flag specifying whether the overview location has been set or not.

HWND eveWindow;			// Our global handle to the eve window
int width;				// Client window resolution
int height;

#define NUM_MINING_SITES 3	// Set to 3 because the 4th one requires double warp
string miningSites[] = {
	"rmenu_mining1.bmp",
	"rmenu_mining2.bmp",
	"rmenu_mining3.bmp",
	"rmenu_mining4.bmp"
};
int curSiteIdx = 0;

#define VELD_PRICE 17.15
#define VELD_CAPACITY 231000

bool runOnce = true;// Flag for runOnce events that have to happen when undocked.

int main() {
	// TODO: Implement reconnecting and connection drop. The image name is "connlist.bmp"

	EnumWindows(FindEveWindowProc, NULL);
	if(eveWindow == NULL)
		fatalExit("Could not find EVE window, closing. Make sure EVE is open before running!");

	SetForegroundWindow(eveWindow);
	Sleep(500);

	// Set the client window's width and height.
	RECT rect;
	GetClientRect(eveWindow, &rect);
	width = rect.right;
	height = rect.bottom;

	// Figure out where we are, and hopefully where we need to go.
	if(isDocked()) {
		depositOre();
	}
	else if(isInSystem("rens")) {
		cout << "Looks like you're in rens, go home ship, you're drunk." << endl;
		goto in_rens;
	}
	else if(isInSystem("frarn")) {
		if(isInvFull()) {								// Check if we have a full inventory in frarn, if so, go home.
			cout << "You've got your hands full in frarn, going home." << endl;
			jumpToSystem("rens");
			goto in_rens;
		}
		else {											// I'm just going to assume your ass is in the minefield.
			openOverviewMine();							// Open the mining tab and check for rocks.
			if(isImageOnScreen("nav_veld.bmp", 0.95)) { //|| isImageOnScreen("nav_scord.bmp", 0.95)) {
				cout << "Found rocks, go mine!" << endl;
				goto at_minefield;
			}
			else {
				cout << "So you're somewhere in frarn, go get somewhere we can recognize." << endl;
				Sleep(10000);
				return 0;
			}
		}
	}
	else {
		fatalExit("We have no idea where you are, are you okay? Exiting.");
	}

	while(1) {
		t.start();

		undock();

		if(!isDocked() && runOnce) {					// Reset the overview by scrolling up on it so the closest objects appear on the screen.
			// Don't really need to scroll the overview very much.
			//MoveMouse(width - 80, height / 3, 0);		// Move mouse to the overview
			//for(int x = 0; x < 10; x++) {				// Scroll up on the overview to make sure the closest distance things are up first.
			//	scrollMouseUp();
			//	Sleep(200);
			//}

			setOverviewLocation();						// Find and set overviewLoc to the location of the first entry in the overview.
														// TODO: This is going to cause a problem, need to have a better way to initialize.
														// Or to stop using the GOTO's, because they will fail for miners.
			runOnce = false;
		}

		cout << "Heading to Destination" << endl;
		jumpToSystem("frarn");

		selectRightClickMenu(miningSites[curSiteIdx], "rmenu_warpto.bmp");// Warp to mining waypoint.
		waitForWarp();										// Warp through.

at_minefield:
		do {
			if(!selectAsteroid()) {							// Target and select an asteroid.
				curSiteIdx++;								// Update which mining site we go to.
				curSiteIdx %= NUM_MINING_SITES;
				cout << "Failed to select an asteroid for some reason, next mine site will be: " << miningSites[curSiteIdx] << endl;
				break;										// And break.
			}

			approachAndFireMiningLasers();					// Start firing teh lazerzzz.
		} while(!waitForMinerDone());						// Wait till we fill up the hangar or for the rock to disappear.

		cout << "Heading back Home" << endl;
		jumpToSystem("rens");
in_rens:
		selectRightClickMenu("rmenu_home.bmp", "rmenu_dock.bmp");
		waitForWarp();										// Warp through.
		waitUntilDocked();									// Dock.
		depositOre();										// Deposit.

		unsigned long runTime = t.elapsed();				// Grab the elapsed time.
		cout << "Time for current run: " << formatTime(runTime) << endl;
		cout << "Approx money for current run: " << VELD_PRICE * VELD_CAPACITY << " ISK" << endl;

		double mIskPerHour = (double)VELD_PRICE * VELD_CAPACITY / runTime * 1000*60*60 / 1000000;

		cout << "Hourly income based on current run: " << mIskPerHour << "Mil Isk" << endl;
															// R-r-r-r-r-r-repeat!
	}
	return 0;
}

bool isStopped() {
	return isImageOnScreen("stopped.bmp", 0.98);
}

bool isInWarp() {
	return isImageOnScreen("warpdrive.bmp", 0.91);
}

bool isInvOpen() {
	return isImageOnScreen("inv_cargoopen.bmp", 0.95);
}

bool isDocked() {
	return isImageOnScreen("undock.bmp", 0.85);
}

bool isInvFull() {
	if(!openInv()) {
		cout << "Failed to open inventory!" << endl;
		return false;
	}

	return isImageOnScreen("cargofull.bmp", 0.999);
}

// Returns whether or not the ship is currently located in the particular system.
bool isInSystem(string systemName) {
	cout << "Checking if you are in: " << systemName << endl;
	Sleep(1000);											// VM is really slow, maybe this will help?

	openOverviewStations();

	string imgStr = "nav_" + systemName + ".bmp";			// Build the search string for the overview.
	return isImageOnScreen(imgStr, 0.99);					// If the system name is shown in the stations overview, we are in this system.
}

bool isAsteroidLocked() {
	return isImageOnScreen("veld_lock.bmp", 0.98);
}

bool waitForWarp() {
	Timer t(15000);				// We'll set a timeout on how long it takes for a ship to get into warp to prevent blocking.
	while(!isInWarp()) {
		if(t.isDone()) {
			cout << "Couldn't detect warp." << endl;
			return false;
		}
	}

	cout << "Warp detected!" << endl;

	t.setInterval(60000);		// Another timeout for how long you're in warp. I think time dilation would screw this up, oh well, it's not perfect.
	t.start();
	while(isInWarp()) {
		if(t.isDone()) {
			cout << "Warping took to long. Exiting." << endl;
			return false;
		}
		Sleep(500);
	}

	cout << "Warp completed." << endl;
	return true;
}

bool waitToStop() {
	Timer t(40000);								// Seems like a reasonable time to stop.

	while(!isStopped()) {
		if(t.isDone()) {
			cout << "Something went wrong when trying to stop!" << endl;
			return false;
		}
	}

	cout << "Detected that you've stopped." << endl;
	return true;
}

bool selectAsteroid() {
	Point p;

	openOverviewMine();						// Open the mining navigation tab.

	moveMouseAway();						// Move away the mouse to make sure we don't mess with image recognition.

	if(!findAsteroidOnScreen(p)) {
		cout << "Couldn't find an asteroid to select!" << endl;
		return false;
	}

	double asteroidDistance = getDistance(p.x, p.y);
	cout << "Our asteroid distance is: " << asteroidDistance << " m" << endl;
											// Find how far away the asteroid is.
	if(asteroidDistance > 20 * 1000) {		// If it's over 20km then we say screw it.
		cout << "The selected asteroid is further than 20km away." << endl;
		return false;
	}

	moveMouse(p.x, p.y, 1);					// Click on it.
	pressKey(VK_LCONTROL);					// Target.
	Sleep(5000);							// Wait for target lock.
	// Check for lock symbol here!!!

	return true;
}

void fireMiningLasers() {
	pressKey(VK_F1);
	pressKey(VK_F2);
}

bool approachAndFireMiningLasers() {
	pressKey((unsigned short)0x51);			// Press Q to approach the thing

	Timer t(180000);						// Setup a timeout timer to avoid getting hung up 	
	do {
		if(t.isDone()) {
			cout << "Timeout during approach and firing of lasers!" << endl;
			return false;
		}

		Sleep(4000);						// Give it some time to approach.

		fireMiningLasers();
		cout << "Fired lasers." << endl;
		Sleep(500);							// Don't poll so agressively, plus give the messages time to display.
	} while(isImageOnScreen("rocktoofar.bmp", 0.9));

	cout << "Approached and successfully firing the lasers." << endl;
	return true;
}

void resetMiningLasers() {
	fireMiningLasers();						// Since we're not actually mining anything, and at least one laser is erroneously on,
											// turn them all off, without a target, this will cause the rest of them to blink.

	moveMouse(width - 20, height - 20, 2);	// Right click on the bottom corner of the screen to open the right click menu.
	Sleep(200);
	clickOnShip();	// Then click away to reset the mining lasers to off position.
}

bool waitForMinerDone() {
	Timer t(600000);					// 10 minutes to fill up hull, will almost certainly have to change this later.

	cout << "Waiting to get miner full message." << endl;

	while(!isInvFull()) {				// Repeat while the inventory is not full.
		if(t.isDone()) {
			cout << "Something went wrong, timeout for miner full exceeded." << endl;
			return false;
		}

		// Check if we still hold an active lock on an asteroid. If we don't, that means that the rock is gone.
		if(!isAsteroidLocked()) {
			cout << "Rock disappeared." << endl;
			resetMiningLasers();							// This is a bug, once a rock disappears, the mining lasers are supposed to stop. In this case, only one does.
			return false;
		}
	}

	cout << "Miner is full." << endl;
	return true;
}

// Right click menu integration
#define RMENU_WIDTH 210					// Width of the menu when right clicking in space
#define RMENU_HEIGHT 15					// Height of each item
#define RMENU_NUMITEMS 11				// Number of items in right click menu

bool selectRightClickMenu(string firstAction) {
	clickOnShip();										// Click in the center to ensure no menus are open
	moveMouse(width - RMENU_WIDTH / 5, height - 30, 2);	// Right click on the bottom right corner of the screen to bring up the menu.
	moveMouse(width - RMENU_WIDTH / 5, height - 5, 0);	// Move to the bottom of the menu to not obcure any options.


	if(!clickImageOnScreen(firstAction, 0.95)) {
		cout << "Failed to find/select option in first menu" << endl;
		return false;
	}
	return true;
}

bool selectRightClickMenu(string firstAction, string secondAction) {
	selectRightClickMenu(firstAction);		// Let's open the first menu

	POINT ptMouse;							// Get the mouse position
	GetCursorPos(&ptMouse);
	ScreenToClient(eveWindow, &ptMouse);

	int x = ptMouse.x - RMENU_WIDTH / 2 - 20;
	int y = ptMouse.y;						// Move the mouse to the left, just past the menu.

	moveMouse(x, y, 0);						// Clickety click!

	if(!clickImageOnScreen(secondAction, 0.95)) {
		cout << "Failed to find/select option in second menu" << endl;
		return false;
	}
	return true;
}

bool openInv() {
	if(!isInvOpen()) {
		keyDown(VK_MENU);				// Send alt.
		pressKey('C');					// Press C. The shortcut "ALT-C" should open up the inventory.
		keyUp(VK_MENU);					// Release alt.
	}
	//else
	//	cout << "Looks like the inventory is already open!" << endl;

	Sleep(300);							// Give it a little bit of time to actually open the inventory.

	return isInvOpen();
}

bool openOreHold() {
	if(!openInv())						// Need to open the inventory first.
		return false;
										// Click the ship select icon to reset
	clickImageOnScreen("inv_shipselect.bmp", 0.95);
										// Then click the orehold.
	bool result = clickImageOnScreen("inv_orehold.bmp", 0.95);

	if(!result) {
		cout << "Failed to open orehold!" << endl;
	}

	return result; 
}

void _openOverview(string name) {
	moveMouseAway();					// Move the mouse out of the way to not get in the way of image recognition.

	clickImageOnScreen(name, 0.95);
	Sleep(200);							// Give it some time to open the tab.
}

void openOverviewGates() {
	return _openOverview("overview_gates.bmp");
}

void openOverviewMine() {
	return _openOverview("overview_mining.bmp");
}

void openOverviewStations() {
	return _openOverview("overview_stations.bmp");
}

void _depositOreHelper(Point itemHangarPos, string oreName) {
	double corr;

	while(1) {
		Point orePos;
		findOnScreen(oreName, orePos, corr);
		if(corr > 0.95) {									// Looks like we've found the ore
			cout << "Ore found, moving to item hangar" << endl;
			dragMouse(orePos.x, orePos.y, itemHangarPos.x, itemHangarPos.y);
			Sleep(300);										// Wait for a minute for the UI to update.
		}
		else
			break;
	}
}

void depositOre() {
	openOreHold();						// Open the ore hold

	Point itemHangarPos;
	double corr;
										// Grab the position of the item hangar thing.
	findOnScreen("inv_itemhangar.bmp", itemHangarPos, corr);

	if(corr < 0.90) {
		cout << "Item hangar image was not found!" << endl;
		return;
	}

	_depositOreHelper(itemHangarPos, "inv_veld.bmp");
	//_depositOreHelper(itemHangarPos, "inv_scord.bmp");
}

bool undock() {
	Point loc;

	cout << "Attempting Undock." << endl;

	if(isDocked()) {
		cout << "It looks like you're docked." << endl;
		clickImageOnScreen("undock.bmp", 0.85);
		cout << "Clicked undock." << endl;

		Timer t(20000);						// Give the operation a 20 second timeout.
		while(isDocked()) {
			Sleep(2000);

			if(t.isDone()) {
				cout << "Error on undock!" << endl;
				return false;
			}
		}
		Sleep(3000);						// Let's give it three seconds to load it's stuff before continuing.

		cout << "Looks like you're out! Have fun!" << endl;
		return true;
	}
	else {
		cout << "Couldn't find undock symbol, are you undocked already?" << endl;
		return false;
	}
}

bool waitUntilDocked() {
	Timer t(60000);			// We'll give it 60 seconds of timeout to wait, to make sure we don't block forever.

	cout << "Waiting 60 seconds to dock..." << endl;

	while(!isDocked()) {
		if(t.isDone()) {
			cout << "Timer ran out, something's wrong." << endl;
			return false;
		}
		Sleep(2000);		// Calm down speedy gonzales, we gotta chilllll.
	}

	Sleep(1000);
	cout << "Docked!" << endl;
	return true;
}

bool waitForSystem(string systemName) {
	Timer t(60000);											// Timeout to wait until you are in a system is 60s.

	while(!isInSystem(systemName)) {
		if(t.isDone()) {
			cout << "Error waiting for current location: " << systemName << endl;
			return false;
		}
		Sleep(500);
	}

	Sleep(3000);
	cout << "Awesome, you are now in: " << systemName << endl;
	return true;
}

bool jumpToSystem(string systemName) {
	openOverviewGates();

	string navImg = "nav_" + systemName + ".bmp";

	bool result = false;

	if(isImageOnScreen(navImg, 0.98))					// Check if the system we want to go to exists.
		result = clickImageOnScreen(navImg, 0.98);
	else {												// Also check if it's highlighted.
		navImg = "nav_" + systemName + "_s.bmp";
		if(isImageOnScreen(navImg, 0.98))
			result = clickImageOnScreen(navImg, 0.98);
	}

	if(result) {										// Check if we've successfully found and clicked on the system we want to go to.
		pressKey((unsigned short)'D');					// Send the jump command
		waitForWarp();									// Need to do something about this, assuming we are far enough to warp to jumpgate
		waitForSystem(systemName);						// Wait until the call tells us we are in the system.
	}
	else {
		cout << "Unsuccessful in clicking on proper system for warp." << endl;
	}

	return result;
}

// Use caching and lazy evaluation for things like this.
Point getOverviewLocation() {
	if(overviewLocSet) {
		return overviewLoc;
	}

	if(isDocked()) {
		fatalExit("Error, tried to access overview location, but was reported as being docked!");
	}

	// TODO: Probably need more checks here..

	setOverviewLocation();
	return overviewLoc;
}

void setOverviewLocation() {
	openOverviewMine();					// Open the mining portion of the overview.
	clickOnShip();						// Click on the ship to deselect anything you hopefully have in the overview.

	Point p;
	double corr;
										// Find the position of the first element in the overview.
	findOnScreen("nav_overviewloc.bmp", p, corr);

	if(corr < 0.99)
		fatalExit("Failed to find overview location!");

	Mat templ = safeImageRead("nav_overviewloc.bmp");
	p.x += templ.cols / 2;				// Next we update the location to the bottom right of the template
	p.y += templ.rows / 2;				// since findOnScreen() gives the location of the center of the template.

	overviewLoc = p;
	overviewLocSet = true;				// Update the flag so it's just cached.
	cout << "Found overview start at: " << p.x << ", " << p.y << endl;
}

void clickOnShip() {
	moveMouse(width / 2, height / 2, 1);
}

void moveMouseAway() {
	moveMouse(width - 20, height - 20, 0);
}

BOOL CALLBACK FindEveWindowProc(HWND hwnd, LPARAM lParam) {
	char className[80];
	char title[80];
	GetClassNameA(hwnd, className, sizeof(className));
	GetWindowTextA(hwnd, title, sizeof(title));

	if(!strncmp(title, "EVE -", 5)) {
		cout << "Window title: " << title << endl;
		cout << "Class name: " << className << endl;
		eveWindow = hwnd;
	}

	return TRUE;
}