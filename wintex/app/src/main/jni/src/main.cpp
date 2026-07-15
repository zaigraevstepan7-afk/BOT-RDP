#include "main.h"

int displayX, displayY;

#include "include.h"

std::mutex _mutex;
bool _enable = true;

std::atomic<bool> _thread_flag{true};
std::atomic<bool> _need_update{false};
	PlayerManager::Aim aim;
void _thread() {
    while (_thread_flag) {
        if (_enable) {
			
            _need_update = false;
        }
        usleep(20000);
    }
}

int main() {  
    screen_config();
    ::displayX = (displayInfo.height > displayInfo.width ? displayInfo.height : displayInfo.width);
    ::displayY = (displayInfo.height < displayInfo.width ? displayInfo.height : displayInfo.width);
    ::native_window_screen_x = (displayInfo.height > displayInfo.width ? displayInfo.height : displayInfo.width);
    ::native_window_screen_y = (displayInfo.height > displayInfo.width ? displayInfo.height : displayInfo.width);
    initGUI_draw(native_window_screen_x, native_window_screen_y, true);
    touch::init(displayInfo.width, displayInfo.height, (uint8_t)displayInfo.orientation);
	proc.pid = getPid("com.axlebolt.standoff2");
	proc.base = getBase("libunity.so");
	if (proc.pid <= 0 || proc.base <= 0) exit(-1);

	// Dedicated high-frequency thread to keep third person stable: the game
	// resets the camera every LateUpdate, so we must out-write it each frame.
	std::thread([]() {
		while (true) {
			thirdPerson();
			antiAimTick();
			usleep(400);
		}
	}).detach();

	PlayerManager::Esp esp;
	PlayerManager::Aim aim;


	
	while (true) {
		drawBegin();
	    drawMenu();
		esp.draw();
        offer();
		static bool _last_update = false;		
		huinarnad();
		aim.aimBot();
		player();
		thirdPersonModel();
		installBodyHook(thirdperson);
		functionsweapon();
		functionsknife();
		functionsgranade();
		drawEsp();
		drawEnd();
	}
	
	_thread_flag = false;

	return 0;
}
