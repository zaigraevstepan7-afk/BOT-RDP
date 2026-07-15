#include <vector>
#include <string>
#include <fstream>
#include <dirent.h>

void functionsweapon() {
	uint64_t playermanager = resolvePlayerManager();
    uint64_t local_player = rpm<uint64_t>(playermanager + 0x70);
    uint64_t WeaponryController = rpm<uint64_t>(local_player + 0x88);  
    uint64_t WeaponController = rpm<uint64_t>(WeaponryController + 0xA0);      
	uint64_t Gunparameters = rpm<uint64_t>(WeaponController + 0x168);
    uint64_t weaponparameters = rpm<uint64_t>(WeaponController + 0xA8);
    uint64_t GunController = rpm<uint64_t>(WeaponController + oxorany(0x148));

    if (weapon.reversammo) {
        int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
        static const std::unordered_set<int> allowedIDs = {
                    11, 12, 13, 15, 16, 17, 18,
                    32, 33, 34, 35, 36, 37,
                    42, 43, 44, 45, 46, 47, 48, 49,
                    51, 52, 53,
                    62, 63, 64, 65
        };
        if (allowedIDs.count(weaponid) == 0)
        return;
        uint64_t ammunition = rpm<uint64_t>(weaponparameters + oxorany(0x130));
        if (ammunition) {
        wpm<short>(ammunition + oxorany(0x12), 10000);
        setSafeB(ammunition, oxorany(0x14), true, 10000);
        wpm<short>(ammunition + oxorany(0x12), 10000);
    }
} 

        if (weapon.infinityammo) {
        int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
        static const std::unordered_set<int> allowedIDs = {
                    11, 12, 13, 15, 16, 17, 18,
                    32, 33, 34, 35, 36, 37,
                    42, 43, 44, 45, 46, 47, 48, 49,
                    51, 52, 53,
                    62, 63, 64, 65
            };
            if (allowedIDs.count(weaponid) == 0)
            return;
            wpm<int>(WeaponController + oxorany(0x128), 999);
      } 

       if (weapon.wallshoot) {
       int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
       static const std::unordered_set<int> allowedIDs = {
                    11, 12, 13, 15, 16, 17, 18,
                    32, 33, 34, 35, 36, 37,
                    42, 43, 44, 45, 46, 47, 48, 49,
                    51, 52, 53,
                    62, 63, 64, 65
            };
          if (allowedIDs.count(weaponid) == 0)
          return;
          WriteSafe(Gunparameters + oxorany(0x264), 5);
       }
       
       if (weapon.onehitkill) {
       int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
       static const std::unordered_set<int> allowedIDs = {
                    11, 12, 13, 15, 16, 17, 18,
                    32, 33, 34, 35, 36, 37,
                    42, 43, 44, 45, 46, 47, 48, 49,
                    51, 52, 53,
                    62, 63, 64, 65
            };
          if (allowedIDs.count(weaponid) == 0)
          return;
			uintptr_t damage = rpm<uintptr_t>(weaponparameters + oxorany(0x140));
			wpm<int>(damage + oxorany(0x28) + 4, 130);
			wpm<int>(damage + oxorany(0x34) + 4, 130);
			wpm<int>(damage + oxorany(0x40) + 4, 130);
			wpm<int>(damage + oxorany(0x4C) + 4, 130);
		}
       
	   if (weapon.firerate) {
	   int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
       static const std::unordered_set<int> allowedIDs = {
                    11, 12, 13, 15, 16, 17, 18,
                    32, 33, 34, 35, 36, 37,
                    42, 43, 44, 45, 46, 47, 48, 49,
                    51, 52, 53,
                    62, 63, 64, 65
            };
       if (allowedIDs.count(weaponid) == 0)
       return;
		WriteSafe(WeaponController + oxorany(0x110), 0);
		}
		
		if (weapon.norecoil) {
        int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
        static const std::unordered_set<int> allowedIDs = {
        11, 12, 13, 15, 16, 17, 18,
        32, 33, 34, 35, 36, 37,
        42, 43, 44, 45, 46, 47, 48, 49,
        51, 52, 53, 62, 63, 64, 65
        };
        if (allowedIDs.count(weaponid) == 0)
        return;
		uintptr_t recoildata = rpm<uint64_t>(WeaponController + (oxorany(0x158)));
		wpm<float>(WeaponController + oxorany(0x240), 0.1f);
        wpm<float>(WeaponController + oxorany(0x244), 0.1f);
        wpm<float>(WeaponController + oxorany(0x248), 0.1f);
		wpm<float>(recoildata + oxorany(0x10), 0.1f);
        wpm<float>(recoildata + oxorany(0x14), 0.1f);
        wpm<float>(recoildata + oxorany(0x18), 0.1f);
	}
    	
    }
    
    void functionsknife() {
	uint64_t playermanager = resolvePlayerManager();
    uint64_t local_player = rpm<uint64_t>(playermanager + 0x70);
    uint64_t WeaponryController = rpm<uint64_t>(local_player + 0x88);  
    uint64_t WeaponController = rpm<uint64_t>(WeaponryController + 0xA0);      
	uint64_t Gunparameters = rpm<uint64_t>(WeaponController + 0x168);
    uint64_t weaponparameters = rpm<uint64_t>(WeaponController + 0xA8);
    uint64_t GunController = rpm<uint64_t>(WeaponController + oxorany(0x148));
    
    if (weapon.fastknife){
	int weaponid2 = rpm<int>(weaponparameters + oxorany(0x18));
        static const std::unordered_set<int> allowedIDs2 = {
        70
        };
       if (allowedIDs2.count(weaponid2) == 0)
       return;
    	wpm<float>(WeaponController + oxorany(0x120), 0.0f);
    	}
    	    	
  	}
