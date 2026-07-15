void functionsgranade() {
	uint64_t playermanager = resolvePlayerManager();
    uint64_t local_player = rpm<uint64_t>(playermanager + 0x70);
    uint64_t WeaponryController = rpm<uint64_t>(local_player + 0x88);  
    uint64_t WeaponController = rpm<uint64_t>(WeaponryController + 0xA0);      
	uint64_t Gunparameters = rpm<uint64_t>(WeaponController + 0x168);
    uint64_t weaponparameters = rpm<uint64_t>(WeaponController + 0xA8);
    uint64_t GunController = rpm<uint64_t>(WeaponController + oxorany(0x148));
    uintptr_t a1 = rpm<uintptr_t>(local_player + 0x88);
	uintptr_t a2 = rpm<uintptr_t>(a1 + 0xA0);
	uintptr_t a3 = rpm<uintptr_t>(a2 + 0xA8);

    if (gUnlimitedGrenades) {
    int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
    static const std::unordered_set<int> allowedIDs = {
    91, 92, 93, 94, 95
    };
    if (allowedIDs.count(weaponid) == 0)
    return;
Change<safe>(WeaponController + oxorany(0x120),
    [](safe *obj) {
    obj->set<bool>(true);
         });
    }
    	
	if (gdetonate) {
	int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
    static const std::unordered_set<int> allowedIDs = {
    91, 92, 93, 94, 95
    };
    if (allowedIDs.count(weaponid) == 0)
    return;
    setSafe(a3, oxorany(0x1D4), true, 0.0f);
	}
	
	if (gnodamage) {
	int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
    static const std::unordered_set<int> allowedIDs = {
    91, 92, 93, 94, 95
    };
    if (allowedIDs.count(weaponid) == 0)
    return;
	setSafe(a3, oxorany(0x1EC), true, 0.0f);
	}
	
	if (dmgh) {
	int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
    static const std::unordered_set<int> allowedIDs = {
    91, 92, 93, 94, 95
    };
    if (allowedIDs.count(weaponid) == 0)
    return;
	setSafe(a3, oxorany(0x1EC), true, 999.0f);
	}
	
	}