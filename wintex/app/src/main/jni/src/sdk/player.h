#include <atomic>
#include <unordered_set>
#include <string>
#include <ctime>

int CalcValue(int a1, int a2) { 
      if ((a1 & 1) != 0) { 
        return a1 ^ a2; 
      } else { 
        return (a2 & 0xFF00FF00) | ((uint8_t)a2 << 16) | ((a2 >> 16) & 0xFF); 
      } 
}

void setSafeB(uintptr_t inst, uintptr_t offset, bool nullable, int32_t actual_value)
{
      if (nullable)
        offset += 0x4;
      wpm<int32_t>(inst + offset + 0x4, actual_value ^ rpm<int32_t>(inst + offset));
}

void WriteSafe(uintptr_t address, int value) {
      int a1 = rpm<int>(address);
      int encodedValue = CalcValue(a1, value);
      wpm<int>(address + 0x4, encodedValue); 
}

std::string utf16le_to_utf8(const std::u16string &u16str) {      
      if (u16str.empty()) {
        return std::string();
      }

      const char16_t *p = u16str.data();   
      std::u16string::size_type len = u16str.length();  
      if (p[0] == 0xFEFF) {      
        p += 1;      
        len -= 1;   
      } 

      std::string u8str;   
      u8str.reserve(len * 3);      
      char16_t u16char;
      
      for (std::u16string::size_type i = 0; i < len; ++i) {       
        u16char = p[i];        
        if (u16char < 0x0080) {          
            u8str.push_back((char) (u16char & 0x00FF));          
            continue;       
        }
        
        if (u16char >= 0x0080 && u16char <= 0x07FF) {          
            u8str.push_back((char) (((u16char >> 6) & 0x1F) | 0xC0));        
            u8str.push_back((char) ((u16char & 0x3F) | 0x80));          
            continue;       
        }
        
        if (u16char >= 0xD800 && u16char <= 0xDBFF) {           
            uint32_t highSur = u16char;          
            uint32_t lowSur = p[++i];          
            uint32_t codePoint = highSur - 0xD800;      
            codePoint <<= 10;        
            codePoint |= lowSur - 0xDC00;      
            codePoint += 0x10000;           
            u8str.push_back((char) ((codePoint >> 18) | 0xF0));     
            u8str.push_back((char) (((codePoint >> 12) & 0x3F) | 0x80));         
            u8str.push_back((char) (((codePoint >> 06) & 0x3F) | 0x80));            
            u8str.push_back((char) ((codePoint & 0x3F) | 0x80));        
            continue;       
        } { 
        u8str.push_back((char) (((u16char >> 12) & 0x0F) | 0xE0));          
        u8str.push_back((char) (((u16char >> 6) & 0x3F) | 0x80));      
        u8str.push_back((char) ((u16char & 0x3F) | 0x80));         
        continue;      
        }      
      }   
      return u8str;
}

uint64_t get_photon(uint64_t player) {
	return rpm<uint64_t>(player + 0x160);
}

struct String {
      uint64_t klass;
      uint64_t monitor_data;
      int length;
      char first_char[ 256 ];

      std::string Get() {
        u16string utf16( ( char16_t * ) first_char, 0, length);
        std::string converted = utf16le_to_utf8( utf16 );
        return converted;
      }
      
};

template<typename T>
T set_prop(uint64_t player, const char* tag, T value) {
	T property = NULL;
	uint64_t props = rpm<uint64_t>(get_photon(player) + 0x38);
	if(props) {
		int size = rpm<int>(props + 0x20);
		for(int i = 0; i < size; i++) {
			uint64_t propkey = rpm<uint64_t>(rpm<uint64_t>(props+0x18) + 0x28 + 0x18*i);
			uint64_t propval = rpm<uint64_t>(rpm<uint64_t>(props+0x18) + 0x30 + 0x18*i);
			if(propkey != 0) {
				std::string keyVal = rpm<String>(propkey).Get();
				if(strstr(keyVal.c_str(), tag)) {
					wpm<T>(propval + 0x10, value);
				}
			}
		}
	}
	return property;
}

#define oxorany
		
   void clearHashSet(uintptr_t HashSet) {
      int clear[] = {0, 0, -1};
      wpm(HashSet + 0x20, clear);
   }

   uintptr_t getInstance(uintptr_t typeInfoAddress, bool parent, uintptr_t field) {
        auto pm1 = rpm<uintptr_t>(typeInfoAddress);
        if (pm1 == 0)
            return 0;
        auto pm2 = parent ? rpm<uintptr_t>(pm1 + 0x190) : pm1;
        if (pm2 == 0)
            return 0;
        auto pm3 = rpm<uintptr_t>(pm2 + 0x198);
        if (pm3 == 0)
            return 0;
        return rpm<uintptr_t>(pm3 + field);
      }

#include <atomic>

uintptr_t getMainController() {
uintptr_t baseMainController = rpm<uintptr_t>(proc.base + oxorany(135065448));
if (!baseMainController) return 0;
      return rpm<uintptr_t>(baseMainController + 0x8);
}

uintptr_t getProfileController() {
uintptr_t MainController = getMainController();
      if (!MainController) return 0;
      return rpm<uintptr_t>(MainController + 0xA8);
}

uintptr_t getPlayerProfileController() {
uintptr_t ProfileController = getProfileController();
      if (!ProfileController) return 0;
      return rpm<uintptr_t>(ProfileController + 0x70);
}

uintptr_t siskipiski() {
uintptr_t MainController = getMainController();
uintptr_t PlayerProfileController = getPlayerProfileController();
uintptr_t ProfileController = getProfileController();
}

uint64_t get_photonA(uint64_t ProfileController) {
	return rpm<uint64_t>(ProfileController + 0x158);
}

template<typename T>
T set_propA(uint64_t ProfileController, const char* tag, T value) {
	T property = NULL;
	uint64_t props = rpm<uint64_t>(get_photonA(ProfileController) + 0x38);
	if(props) {
		int size = rpm<int>(props + 0x20);
		for(int i = 0; i < size; i++) {
			uint64_t propkey = rpm<uint64_t>(rpm<uint64_t>(props+0x18) + 0x28 + 0x18*i);
			uint64_t propval = rpm<uint64_t>(rpm<uint64_t>(props+0x18) + 0x30 + 0x18*i);
			if(propkey != 0) {
				std::string keyVal = rpm<String>(propkey).Get();
				if(strstr(keyVal.c_str(), tag)) {
					wpm<T>(propval + 0x10, value);
				}
			}
		}
	}
	return property;
}


uintptr_t getPlayerManager() {
      return (uintptr_t)resolvePlayerManager();
}

static uintptr_t getLocalPlayer() {
      uintptr_t manager = getPlayerManager();
      if (!manager) return 0;
      return rpm<uintptr_t>(manager + 0x70);
}

Vector3 GetPlayerLocation(uintptr_t player) {
      if (!player) return Vector3{0,0,0};
      uintptr_t transform = rpm<uintptr_t>(player + 0x98);
      if (!transform) return Vector3{0,0,0};
      uintptr_t transformData = rpm<uintptr_t>(transform + 0xB0);
      if (!transformData) return Vector3{0,0,0};
      return rpm<Vector3>(transformData + 0x44);
}

void setpos(uint64_t player, Vector3 newPos) {
      if (!player) return;
      uint64_t transform = rpm<uint64_t>(player + 0x98);
      if (!transform) return;
      uint64_t transformData = rpm<uint64_t>(transform + 0xB0);
      if (!transformData) return;
      wpm<Vector3>(transformData + 0x44, newPos);
}

void zeroRigidbody(uintptr_t player) {
      uintptr_t rb = rpm<uintptr_t>(player + 0xA8);
      if (!rb) return;
      Vector3 zero{0,0,0};
      wpm<Vector3>(rb + 0x20, zero);
      wpm<Vector3>(rb + 0x2C, zero);
}

void teleport_tick() {
      uintptr_t localPlayer = getLocalPlayer();
      if (!localPlayer) return;
      zeroRigidbody(localPlayer);
      Vector3 pos = GetPlayerLocation(localPlayer);
      if (tpForward)  { pos.z += teleportStep; tpForward = false; }
      if (tpBackward) { pos.z -= teleportStep; tpBackward = false; }
      if (tpRight)      { pos.x += teleportStep; tpRight = false; }
      if (tpLeft)     { pos.x -= teleportStep; tpLeft = false; }
      if (tpUp)       { pos.y += teleportStep; tpUp = false; }
      if (tpDown)     { pos.y -= teleportStep; tpDown = false; }
      setpos(localPlayer, pos);
}

static uint8_t getLocalTeam() {
  uintptr_t player = getLocalPlayer();
  if (!player) return 0;
  return rpm<uint8_t>(player + 0x79);
}

void player() {
      uint64_t playermanager = resolvePlayerManager();
      
  /*    uintptr_t _anticheat = oxorany(151491024);
      uint64_t anticheat = rpm<uint64_t>(
      rpm<uint64_t>(
      rpm<uint64_t>(
      rpm<uint64_t>(proc.base + _anticheat) + oxorany(0x20)
      ) + oxorany(0x30)
      ) + oxorany(0x0)
      );

      if (anticheat) {
      wpm<bool>(anticheat + oxorany(0x28), false);
      wpm<int32_t>(anticheat + oxorany(0x24), 0);
      }*/
      
      uint64_t localPlayer = rpm<uint64_t>(playermanager + oxorany(0x70));
      uint64_t local_player = rpm<uint64_t>(playermanager + oxorany(0x70));
      uint8_t localTeam = PlayerManager::Utils::getLocalTeam();
      
      uint64_t movementcontroller = rpm<uint64_t>(local_player + oxorany(0x98));
      uint64_t translationparameters = rpm<uint64_t>(movementcontroller + oxorany(0xA8));
      uint64_t jumpparameters = rpm<uint64_t>(translationparameters + oxorany(0x50));
      uintptr_t crouchParameters = rpm<uintptr_t>(translationparameters + oxorany(0x48));
      uintptr_t translation_data = rpm<uintptr_t>(movementcontroller + oxorany(0xB0));
      
      uint64_t WeaponryController = rpm<uint64_t>(local_player + oxorany(0x88));
      uint64_t WeaponController = rpm<uint64_t>(WeaponryController + oxorany(0xA0));
      uint64_t Gunparameters = rpm<uint64_t>(WeaponController + oxorany(0x160));
      uint64_t weaponparameters = rpm<uint64_t>(WeaponController + oxorany(0xA8));
      uint64_t GunController = rpm<uint64_t>(WeaponController + oxorany(0x148));
      uint64_t movementController = rpm<uint64_t>(localPlayer + oxorany(0x98));
      
      uint64_t playercamera = rpm<uint64_t>(localPlayer + oxorany(0xE8));
      uint64_t scopezoomed = rpm<uint64_t>(playercamera + oxorany(0x28));
      uint64_t camera = rpm<uint64_t>(playercamera + oxorany(0x20));
      uint64_t player_main_camera = rpm<uint64_t>(localPlayer + oxorany(0xE8));
      uint64_t camera_scope_zoomer = rpm<uint64_t>(player_main_camera + oxorany(0x28));
      uint64_t native_camera = rpm<uint64_t>(camera + oxorany(0x10));
      uint64_t obj = rpm<uint64_t>(camera + oxorany(0x10));
      
      uint64_t arms_controller = rpm<uint64_t>(localPlayer + oxorany(0xA0));
      
      uint64_t PhotonView = rpm<uintptr_t>(localPlayer + oxorany(0x150));
      
      uint64_t gameController = getInstance(proc.base + oxorany(0xAA12E60), false, oxorany(0x8));
      
      uint64_t PlayerControls = rpm<uint64_t>(gameController + oxorany(0x2A0));
		
      uint64_t settings = rpm<uintptr_t>(gameController + oxorany(0xC0));
      uint64_t settingsCT = rpm<uintptr_t>(settings + oxorany(0x38));
      uint64_t settingsTR = rpm<uintptr_t>(settings + oxorany(0x40));
      uint64_t settingsG = rpm<uintptr_t>(gameController + oxorany(0xC0));
      
      auto PhotonNetwork = getInstance(proc.base + oxorany(0xAA17EA0), false, oxorany(0x18));
      auto room = rpm<uintptr_t>(PhotonNetwork + oxorany(0x170));
      
      if (airstafe) {
      wpm<Vector3>(translation_data + oxorany(0x68), Vector3(0, 0, 0));
      wpm<float>(jumpparameters + oxorany(0x60), 6.950);
      }
 
      if (buy_anywhere_a) {
        wpm<bool>(settingsCT + oxorany(0x1A), true);
        wpm<bool>(settingsTR + oxorany(0x1A), true);
      }
      
      if (infinity_buy_a) {
        wpm<bool>(settingsCT + oxorany(0x19), true);
        wpm<bool>(settingsTR + oxorany(0x19), true);
      }
      
      if (moneyhack_a) {
        wpm<int>(settingsCT + oxorany(0x2C), 10000);
        wpm<int>(settingsTR + oxorany(0x2C), 10000);
        wpm<int>(settingsCT + oxorany(0x28), 10000);
        wpm<int>(settingsTR + oxorany(0x28), 10000);
      }
      
      
      if (hahahalol228) {
  //  wpm<int>(PlayerNameView + oxorany(0x28), 999);
      uintptr_t ProfileController = getProfileController();
  
      set_propA<int>(ProfileController, "gold", 100);
      set_propA<int>(ProfileController, "time_in_game", 100);
      set_propA<int>(ProfileController, "level", 100);
      }
	
      if (arms_position) {
      wpm<Vector3>(arms_controller + oxorany(0xE8), Vector3(ARMSX, ARMSY, ARMSZ) / -10.0f);
      }
	
	if (force_team_ct) {
        set_prop<int>(localPlayer, "team", 2);
        set_prop<int>(localPlayer, "match_team", 2);
      }
      
      if (force_team_tr) {
        set_prop<int>(localPlayer, "team", 1);
        set_prop<int>(localPlayer, "match_team", 1);
      }
      
      if (set_statrack) {
      set_prop<int>(localPlayer, "stattrack_value", srarrackval);
      }
      
      if (set_pattern) {
      set_prop<int>(localPlayer, "pattern", patternval);
      }
      
      if (set_avatar) {
      set_prop<int>(localPlayer, "avatarFrameId", 7031);
      }
      
      if (set_medalsc) {
      set_prop<int>(localPlayer, "badgeId", 8017);
      }
      
      if (skip_warmup) {
      set_prop<int>(localPlayer, "badgeId", 8017);
      }
      
      if (skip_warmup) {
      set_prop<double>(room, "Time", 0.0);
      }
      
      if (autowin_ct) {
      set_prop<int>(room, "CtScore", 99);
      set_prop<int>(room, "Round", 99);
      set_prop<double>(room, "Time", 0.0);
      }
      
      if (autowin_tr) {
      set_prop<int>(room, "TrScore", 99);
      set_prop<int>(room, "Round", 99);
      set_prop<double>(room, "Time", 0.0);
      }
      
       if (airjump) {
        uint64_t CharacterController = rpm<uint64_t>(localPlayer + oxorany(0x118));
         if (CharacterController > 0) {
            uint64_t object = rpm<uint64_t>(CharacterController + oxorany(0x10));
            uint64_t pmttest = rpm<uint64_t>(object + oxorany(0xCC));
                pmttest |= (1 << 2);
                wpm<uint64_t>(object + oxorany(0xCC), pmttest);
            }
      }
      
      if (invismode) {
        wpm<int>(WeaponryController + oxorany(0x88), -1);
        wpm<bool>(WeaponryController + oxorany(0x89), true);
   }
   
   	   if (bunnyhope) {
       wpm<float>(jumpparameters + oxorany(0x10), hope_value);
       wpm<float>(jumpparameters + oxorany(0x60), hope_value);
        }
        
       if (fastreload) {
       wpm<float>(Gunparameters + 0x1AC, 0.2f);
       wpm<float>(Gunparameters + 0x1B4, 0.2f);
       wpm<float>(Gunparameters + 0x1C0, 0.2f);
       wpm<float>(Gunparameters + 0x1C8, 0.2f);     
        }

      if (sky) {
      float rgba[4] = { sky_color[0], sky_color[1], sky_color[2], 1.0f };
      wpm<int>(obj + oxorany(0x418), 2);
      wpm<float>(obj + oxorany(0x41C), rgba[0]);
      wpm<float>(obj + oxorany(0x420), rgba[1]);
      wpm<float>(obj + oxorany(0x424), rgba[2]);
      wpm<float>(obj + oxorany(0x428), rgba[3]);
      } else {
      wpm<int>(obj + oxorany(0x418), 1);
      }

      if (camerafov) {
      wpm<float>(scopezoomed + oxorany(0x38), fov_value);  
      }
      
      if (aspect) {
      wpm<float>(native_camera + oxorany(0x4F0), aspect_value);
      wpm<float>(native_camera + oxorany(0x180), 59.9f);
      wpm<float>(camera_scope_zoomer + oxorany(0x38), aspect_value);
      }
      
      if (clumsy) {
      wpm<int>(PhotonView + 0x50, islag ? 0 : 2);
      }
	
	if (speedhack){
      wpm<float>(translationparameters + oxorany(0x10), 7.0f);
      wpm<float>(translationparameters + oxorany(0x14), 7.0f);
      wpm<float>(translationparameters + oxorany(0x20), 7.0f);
      }
	
	if (crouch) {
		wpm<float>(crouchParameters + oxorany(0x10), 990.0f);
		wpm<float>(crouchParameters + oxorany(0x14), 990.0f);
		wpm<float>(crouchParameters + oxorany(0x20), 990.0f);
	}
	
	if (set_score){
       set_prop<int>(local_player, "score", score_value);
      }
      
      if (armorhack){
      set_prop<int>(local_player, "armor", 9999);
      }
      
      if (sethp){
      set_prop<int>(local_player, "health", healthvalls);
      }
      
      if (sethl9998){
      set_prop<int>(local_player, "health", 9999);
      }
      
      if (damagehacks){
      set_prop<int>(local_player, "damage", 9999);
      }
      
      if (setdamage){
      set_prop<int>(local_player, "damage", 9999);
      }
      
      if (set_round) {
      set_prop<int>(local_player, "Round", roundval);
      }
      
      if (setkills){
      set_prop<int>(local_player, "kills", killsval);
      }
      if (set_time) {
      set_prop<float>(local_player, "Time", timeval);
      }
      
      if (cencmatchmm){
      set_prop<int>(local_player, "kills", 99);
      set_prop<int>(room, "CtScore", 99);
      set_prop<int>(room, "TrScore", 99);
      set_prop<double>(room, "Time", 0.0);
      }
      
      if (setassists){
      set_prop<int>(local_player, "assists", assistsval);
      }
      
      if (setdeath){  
      set_prop<int>(local_player, "death", deathval);
      }
      
      if (set_mvp) {
      set_prop<int>(local_player, "mvp", 1);
      }
      
      if (set_money) {
      set_prop<int>(local_player, "money", 16000);
      }
      
      if (set_ping) {
      set_prop<int>(local_player, "ping", pingval);
      }
        
      if (fastwin) {
      set_prop<int>(local_player, "Time", 0);
      set_prop<int>(local_player, "Round", 0);
      set_prop<int>(local_player, "round_count", 0);
      }
      
      if (moneyhacked){
      set_prop<int>(local_player, "money", 5000);
      }
  
      if (spoofers.hideid){
      set_prop<int>(local_player, "uid", 0);
      }
  
      if (spoofers.hideclantag){
      set_prop<int>(local_player, "clan_tag", 0);
      }
  
      if (autoRefill) {
      int ammo = rpm<int>(WeaponController + 0x110);
      if (ammo < 2) {
        wpm<int>(WeaponController + 0x110, 7);
      }
}
	
	/*if (knif2) {
	      int weaponid = rpm<int>(weaponparameters + oxorany(0x18));
        static const std::unordered_set<int> allowedIDs = {
        70, 71, 72, 73,
        75, 77, 78, 79,
        80, 81, 82, 83,
        85, 86, 88
        };
       if (allowedIDs.count(weaponid) == 0)
       return;
      	wpm<float>(WeaponController + oxorany(0x110), 0.0f); 
      	}*/
      	
	if (mbf) {
		clearHashSet(rpm<uintptr_t>(PlayerControls + oxorany(0x68)));
        clearHashSet(rpm<uintptr_t>(PlayerControls + oxorany(0x70)));
        clearHashSet(rpm<uintptr_t>(PlayerControls + oxorany(0x78)));
	}

if (head_scale) {
      uint64_t playersList = rpm<uint64_t>(playermanager + oxorany(0x28));
      if (localPlayer && playersList) {
        int playersListSize = rpm<int>(playersList + oxorany(0x20));
        uint64_t playersArray = rpm<uint64_t>(playersList + oxorany(0x18));

        for (int i = 0; i < playersListSize; i++) {
            uint64_t player = rpm<uint64_t>(playersArray + oxorany(0x30) + oxorany(0x18) * i);
            if (!player || player == localPlayer) continue;

            uint64_t characterView = rpm<uint64_t>(player + oxorany(0x48));
            if (!characterView) continue;

            uint64_t bipedMap = rpm<uint64_t>(characterView + oxorany(0x48));
            if (!bipedMap) continue;

            std::vector<int> bodyOffsets = {
                oxorany(0x20),
                oxorany(0x28)
            };

            for (int bodyPartOffset : bodyOffsets) {
                uint64_t bodyTransform = rpm<uint64_t>(bipedMap + bodyPartOffset);
                if (!bodyTransform) continue;

                uint64_t transformInternal = rpm<uint64_t>(bodyTransform + oxorany(0x10));
                if (!transformInternal) continue;

                uint64_t matrix = rpm<uint64_t>(transformInternal + oxorany(0x38));
                uint64_t index  = rpm<uint64_t>(transformInternal + oxorany(0x40));
                if (!matrix) continue;

                uint64_t matrixList = rpm<uint64_t>(matrix + oxorany(0x18));
                if (!matrixList) continue;

                uint64_t tmatrixAddr = matrixList + oxorany(0x30) * index;

                if (tmatrixAddr > 0x10000) {
                    float absValue = fabs(headScaleValue);
                    float finalScale = (headScaleValue >= 0.0f) ? absValue : (1.0f / absValue);

                    wpm<float>(tmatrixAddr + oxorany(0x20), finalScale);
                    wpm<float>(tmatrixAddr + oxorany(0x24), finalScale);
                    wpm<float>(tmatrixAddr + oxorany(0x28), finalScale);
                }
            }
        }
      }
}

if (player_scale) {
      uint64_t playersList = rpm<uint64_t>(playermanager + oxorany(0x28));
      if (localPlayer && playersList) {
        int playersListSize = rpm<int>(playersList + oxorany(0x20));
        uint64_t playersArray = rpm<uint64_t>(playersList + oxorany(0x18));

        for (int i = 0; i < playersListSize; i++) {
            uint64_t player = rpm<uint64_t>(playersArray + oxorany(0x30) + oxorany(0x18) * i);
            if (!player || player == localPlayer) continue;

            uint64_t characterView = rpm<uint64_t>(player + oxorany(0x48));
            if (!characterView) continue;

            uint64_t bipedMap = rpm<uint64_t>(characterView + oxorany(0x48));
            if (!bipedMap) continue;

            std::vector<int> bodyOffsets = {
                oxorany(0x20),  oxorany(0x28),  oxorany(0x30),  oxorany(0x38),  oxorany(0x40),
                oxorany(0x48),  oxorany(0x50),  oxorany(0x58),  oxorany(0x60),
                oxorany(0x68),  oxorany(0x70),  oxorany(0x78),  oxorany(0x80),
                oxorany(0x88),
                oxorany(0x90),  oxorany(0x98),  oxorany(0xA0),  oxorany(0xA8),
                oxorany(0xB0),  oxorany(0xB8),  oxorany(0xC0),  oxorany(0xC8),
                oxorany(0xD0),  oxorany(0xD8),  oxorany(0xE0),  oxorany(0xE8),  oxorany(0xF0),
                oxorany(0xF8),  oxorany(0x100), oxorany(0x108), oxorany(0x110), oxorany(0x118),
                oxorany(0x120), oxorany(0x128), oxorany(0x130), oxorany(0x138), oxorany(0x140),
                oxorany(0x148), oxorany(0x150), oxorany(0x158), oxorany(0x160), oxorany(0x168),
                oxorany(0x170), oxorany(0x178), oxorany(0x180), oxorany(0x188),
                oxorany(0x190), oxorany(0x198), oxorany(0x1A0), oxorany(0x1A8), oxorany(0x1B0),
                oxorany(0x1B8), oxorany(0x1C0), oxorany(0x1C8), oxorany(0x1D0), oxorany(0x1D8),
                oxorany(0x1E0), oxorany(0x1E8), oxorany(0x1F0), oxorany(0x1F8), oxorany(0x200),
                oxorany(0x208), oxorany(0x210), oxorany(0x218), oxorany(0x220), oxorany(0x228),
                oxorany(0x230), oxorany(0x238), oxorany(0x240), oxorany(0x248),
                oxorany(0x250), oxorany(0x258)
            };

            for (int bodyPartOffset : bodyOffsets) {
                uint64_t bodyTransform = rpm<uint64_t>(bipedMap + bodyPartOffset);
                if (!bodyTransform) continue;

                uint64_t transformInternal = rpm<uint64_t>(bodyTransform + oxorany(0x10));
                if (!transformInternal) continue;

                uint64_t matrix = rpm<uint64_t>(transformInternal + oxorany(0x38));
                uint64_t index  = rpm<uint64_t>(transformInternal + oxorany(0x40));
                if (!matrix) continue;

                uint64_t matrixList = rpm<uint64_t>(matrix + oxorany(0x18));
                if (!matrixList) continue;

                uint64_t tmatrixAddr = matrixList + oxorany(0x30) * index;

                if (tmatrixAddr > 0x10000) {
                    float absValue = fabs(playerscaleval);
                    float finalScale = (playerscaleval >= 0.0f) ? absValue : (1.0f / absValue);

                    wpm<float>(tmatrixAddr + oxorany(0x20), finalScale);
                    wpm<float>(tmatrixAddr + oxorany(0x24), finalScale);
                    wpm<float>(tmatrixAddr + oxorany(0x28), finalScale);
                }
            }
        }
      }
}

if (strafe) {
      if (localPlayer) {
        uint64_t movementController = rpm<uint64_t>(localPlayer + oxorany(0x98));
        if (movementController) {
            uint64_t translationParams = rpm<uint64_t>(movementController + oxorany(0xA8));
            if (translationParams) {
                wpm<float>(translationParams + oxorany(0x2C), strafe_speed);
                wpm<float>(translationParams + oxorany(0x30), strafe_speed);
            }
        }
      }
}
	
	auto w = resolvePlayerManager();
      auto e = rpm<uint64_t>(w + oxorany(0x70));
      auto r = rpm<uint64_t>(w + oxorany(0x28));
      int t = rpm<int>(r + oxorany(0x20));
      
      for (int i = 0; i < t; i++) {
       auto player = rpm<uint64_t>(rpm<uint64_t>(r + oxorany(0x18)) + oxorany(0x30) + oxorany(0x18) * i);
       
       if (autowin) {
       set_prop<int>(player, "health", -99);
       }
       
       if (kickplayers){
       set_prop<int>(player, "team", 0);
       set_prop<int>(player, "match_team", 0);
       }
       
       if (set_score_all){
       set_prop<int>(player, "score", score_all_v);
       set_prop<int>(localPlayer, "score", score_all_v);
       }
       
       if (hide_id_all){
       set_prop<int>(player, "uid", 0);
       set_prop<int>(localPlayer, "uid", 0);
       }
  
      if (hide_clan_tag_all){
      set_prop<int>(player, "clan_tag", 0);
      set_prop<int>(localPlayer, "clan_tag", 0);
      }
      
      if (set_death_all){  
      set_prop<int>(player, "death", death_all_v);
      set_prop<int>(localPlayer, "death", death_all_v);
      }
      
      if (set_money_all) {
      set_prop<int>(player, "money", money_all_v);
      set_prop<int>(localPlayer, "money", money_all_v);
      }
      
      if (set_ping_all) {
      set_prop<int>(player, "ping", ping_all_vl);
      set_prop<int>(localPlayer, "ping", ping_all_vl);
      }
      
      if (set_kills_all){
      set_prop<int>(player, "kills", kills_all_v);
      set_prop<int>(localPlayer, "kills", kills_all_v);
      }
      
      if (fake_avatar_all) {
      set_prop<int>(player, "avatarFrameId", 7031);
      set_prop<int>(localPlayer, "avatarFrameId", 7031);
      }
      
      if (fake_medal_all) {
      set_prop<int>(player, "badgeId", 8017);
      set_prop<int>(localPlayer, "badgeId", 8017);
      }
       
    /*  if (zalupa) {
      set_prop<int>(player, "stattrack_value", srarrackval);
      set_prop<int>(player, "pattern", patternval);
      set_prop<int>(player, "avatarFrameId", 7031);
      set_prop<int>(player, "badgeId", 8017);
      }*/
       
   }
}

uintptr_t Room()
{
      auto PhotonNetwork = getInstance(proc.base + oxorany(0xAA17EA0), false, oxorany(0x18));
      auto room = rpm<uintptr_t>(PhotonNetwork + oxorany(0x170)); //160
      return room;
}

bool is_host() 
{
      uintptr_t playerManager = (uintptr_t)resolvePlayerManager();
      if (!playerManager) return false;
      
      uintptr_t localPlayer = rpm<uintptr_t>(playerManager + oxorany(0x70));
      if (!localPlayer) return false;
      
      uintptr_t photon = rpm<uintptr_t>(localPlayer + oxorany(0x160));
      if (!photon) return false;
      
      uintptr_t room = Room();
      if (!room) return false;
      
      return rpm<int32_t>(room + oxorany(0x48)) == rpm<int32_t>(photon + oxorany(0x18));
}
