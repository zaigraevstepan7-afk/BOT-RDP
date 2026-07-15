#include "monoString.h"

extern ImFont* verdanaFont;

using namespace ImGui;

uintptr_t getPhoton(uintptr_t player) {
	if (!player) return 0;
	return rpm<uintptr_t>(player + 0x160);
}

monoString getPlayerName(uintptr_t player) {
	if (!player) return {};
	auto photon = getPhoton(player);
	if (!photon) return {};
	uintptr_t nickPtr = rpm<uintptr_t>(photon + 0x20);
	if (!nickPtr) return {};
	return rpm<monoString>(nickPtr);
}

template<typename T>
T getProp(uintptr_t player, const char* tag) {
	if (!player || !tag) return NULL;
	T property = NULL;
	uintptr_t props = rpm<uintptr_t>(getPhoton(player) + 0x38);
	if (!props) return property;
	int size = rpm<int>(props + 0x20);
	if (size <= 0) return property;
	for (int i = 0; i < size; i++) {
		uintptr_t propKey = rpm<uintptr_t>(rpm<uintptr_t>(props + 0x18) + 0x28 + 0x18 * i);
		uintptr_t propValue = rpm<uintptr_t>(rpm<uintptr_t>(props + 0x18) + 0x30 + 0x18 * i);
		if (!propKey) continue;
		string keyValue = rpm<monoString>(propKey).asUtf8();
		if (keyValue.empty()) continue;
		if (strstr(keyValue.c_str(), tag)) {
			property = rpm<T>(propValue + 0x10);
			break;
		}
	}
	return property;
}

uintptr_t getInstance2(uintptr_t typeInfoAddress, bool parent, uintptr_t field) {
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
		
// Third person: pull the local camera back along its own forward axis.
// Writes the camera transform's local position (TMatrix array entry), so the
// game recomputes the view matrix from it and ESP stays aligned automatically.
void thirdPerson() {
    if (!thirdperson) return;
    uintptr_t pm = (uintptr_t)resolvePlayerManager();
    if (!pm) return;
    uintptr_t localPlayer = rpm<uintptr_t>(pm + 0x70);
    if (!localPlayer) return;
    uintptr_t mainCam = rpm<uintptr_t>(localPlayer + 0xE8); // PlayerMainCamera
    if (!mainCam) return;
    uintptr_t camTransform = rpm<uintptr_t>(mainCam + 0x38); // _transform
    if (!camTransform) return;

    uintptr_t unk1 = rpm<uintptr_t>(camTransform + 0x10);
    if (!unk1) return;
    uintptr_t matrix = rpm<uintptr_t>(unk1 + 0x38);
    if (!matrix) return;
    int index = rpm<int>(unk1 + 0x40);
    uintptr_t matrix_list = rpm<uintptr_t>(matrix + 0x18);
    if (!matrix_list) return;

    uintptr_t entry = matrix_list + sizeof(TMatrix) * (uintptr_t)index;
    TMatrix tm = rpm<TMatrix>(entry);
    tm.position.z = -thirdperson_distance; // pull camera back along local forward
    wpm<TMatrix>(entry, tm);
}

// Third person model/arms handling:
//  - show the local player's own body (restore bone scale that FP hides)
//  - hide the first-person arms (push them out of view); arms return in 1st person
void thirdPersonModel() {
    uintptr_t pm = (uintptr_t)resolvePlayerManager();
    if (!pm) return;
    uintptr_t lp = rpm<uintptr_t>(pm + 0x70);
    if (!lp) return;

    bool tp = thirdperson;

    // (3) Arms: visible only in first person.
    uintptr_t armsController = rpm<uintptr_t>(lp + 0xA0);
    if (armsController) {
        if (tp)
            wpm<Vector3>(armsController + 0xE8, Vector3(0.0f, -1000.0f, 0.0f)); // push away
        else if (!arms_position)
            wpm<Vector3>(armsController + 0xE8, Vector3(0.0f, 0.0f, 0.0f));     // restore
    }

    // (2) Body: show my own model in third person.
    if (!tp) return;

    // 2a) IsCharacterVisible flag — the exact byte the game's own
    //     SetCharacterVisible(bool) writes (PlayerController + 0xD8).
    wpm<bool>(lp + 0xD8, true);

    // 2b) Force the body LOD group's renderer-enabled flag (SkinnedMeshLodGroup
    //     base bool @ 0x20) so the game keeps the mesh drawn, not shadows-only.
    uintptr_t charLod = rpm<uintptr_t>(lp + 0x128); // CharacterLodGroup
    if (charLod) wpm<bool>(charLod + 0x20, true);
    uintptr_t skinLod = rpm<uintptr_t>(lp + 0x120); // SkinnedMeshLodGroup
    if (skinLod) wpm<bool>(skinLod + 0x20, true);

    // 2c) Force PlayerCharacterView visibility flags (bool @ 0x30).
    for (int cvOff : { 0x48, 0x50, 0xD0 }) {
        uintptr_t cv = rpm<uintptr_t>(lp + cvOff);
        if (cv) wpm<bool>(cv + 0x30, true);
    }

    // 2c) Restore body bone scale (covers the case where FP hides via scale=0).
    uintptr_t characterView = rpm<uintptr_t>(lp + 0x48);
    if (!characterView) return;
    uintptr_t bipedMap = rpm<uintptr_t>(characterView + 0x48);
    if (!bipedMap) return;

    for (int off = 0x20; off <= 0x120; off += 0x8) {
        uintptr_t bone = rpm<uintptr_t>(bipedMap + off);
        if (!bone) continue;
        uintptr_t ti = rpm<uintptr_t>(bone + 0x10);
        if (!ti) continue;
        uintptr_t matrix = rpm<uintptr_t>(ti + 0x38);
        uint64_t index = rpm<uint64_t>(ti + 0x40);
        if (!matrix) continue;
        uintptr_t matrixList = rpm<uintptr_t>(matrix + 0x18);
        if (!matrixList) continue;
        uintptr_t tmAddr = matrixList + sizeof(TMatrix) * index;
        if (tmAddr <= 0x10000) continue;
        wpm<float>(tmAddr + 0x20, 1.0f);
        wpm<float>(tmAddr + 0x24, 1.0f);
        wpm<float>(tmAddr + 0x28, 1.0f);
    }
}

// ---------------------------------------------------------------------------
// "Show own body" via code-cave + external inline hook (NO ptrace).
// We patch ONE instruction of PlayerController.LateUpdate to branch into a code
// cave (unused zero padding inside libunity .text). The cave runs a small stub
// that resolves the local player and calls the game's own SetCharacterVisible
// (true) on the game thread, then runs the original instruction and branches
// back. All PC-relative fields are fixed by RVA; only two literals depend on
// the runtime libunity base. Everything is written via /proc/pid/mem (no ptrace
// => the anti-cheat's tracer check is not tripped).
// ---------------------------------------------------------------------------
namespace tpbody {
    static const uintptr_t RVA_LATEUPDATE = 0x76A839C;
    static const uintptr_t RVA_CAVE1      = 0x8C95044; // 60-byte zero cave (code)
    static const uintptr_t RVA_CAVE2      = 0x8C95560; // 32-byte zero cave (literals)
    static const uintptr_t RVA_SETCHARVIS = 0x7697854; // PlayerController.SetCharacterVisible(bool)
    static const uintptr_t RVA_SETTPSVIEW = 0x7697560; // PlayerController.SetTPSView()
    static const uintptr_t RVA_CALLEE     = RVA_SETTPSVIEW; // function the cave calls on the local player
    static const uint64_t  OFF_PM_STATIC  = 178356728; // PlayerManager il2cpp static slot
    static const uint32_t  ORIG_LU_INSN   = 0xA9BE57FE; // stp x30,x21,[sp,#-0x20]!

    static inline uint32_t bImm(uintptr_t target, uintptr_t pc) {
        int64_t off = (int64_t)target - (int64_t)pc;
        return 0x14000000u | (((uint32_t)(off >> 2)) & 0x03FFFFFFu);
    }
    static inline uint32_t cbz(int rt, uintptr_t target, uintptr_t pc) {
        int64_t off = (int64_t)target - (int64_t)pc;
        return 0xB4000000u | ((((uint32_t)(off >> 2)) & 0x7FFFFu) << 5) | (rt & 0x1F);
    }
    static inline uint32_t ldrlit(int rt, uintptr_t litRVA, uintptr_t pcRVA) {
        int64_t off = (int64_t)litRVA - (int64_t)pcRVA;
        return 0x58000000u | ((((uint32_t)(off >> 2)) & 0x7FFFFu) << 5) | (rt & 0x1F);
    }
}

void installBodyHook(bool enable) {
    using namespace tpbody;
    static bool installed = false;
    uintptr_t base = proc.base;
    if (!base) return;

    if (enable && !installed) {
        // cave2: two 64-bit literals (depend on runtime base).
        wpm<uint64_t>(base + RVA_CAVE2 + 0, (uint64_t)(base + OFF_PM_STATIC));
        wpm<uint64_t>(base + RVA_CAVE2 + 8, (uint64_t)(base + RVA_CALLEE));

        // cave1: 14-instruction stub.
        uint32_t code[14];
        code[0]  = 0xA9BF7BE0;                                   // stp x0,x30,[sp,#-16]!
        code[1]  = ldrlit(9,  RVA_CAVE2 + 0, RVA_CAVE1 + 0x04);  // ldr x9, =pm_static_addr
        code[2]  = 0xF9400129;                                   // ldr x9,[x9]           (cls)
        code[3]  = 0xF9403129;                                   // ldr x9,[x9,#0x60]     (static_fields)
        code[4]  = 0xF9400929;                                   // ldr x9,[x9,#0x10]     (PlayerManager)
        code[5]  = cbz(9,  RVA_CAVE1 + 0x2C, RVA_CAVE1 + 0x14);  // cbz x9, skip
        code[6]  = 0xF9403920;                                   // ldr x0,[x9,#0x70]     (localPlayer)
        code[7]  = cbz(0,  RVA_CAVE1 + 0x2C, RVA_CAVE1 + 0x1C);  // cbz x0, skip
        code[8]  = 0x52800021;                                   // mov w1,#1
        code[9]  = ldrlit(16, RVA_CAVE2 + 8, RVA_CAVE1 + 0x24);  // ldr x16, =SetCharacterVisible
        code[10] = 0xD63F0200;                                   // blr x16
        code[11] = 0xA8C17BE0;                                   // skip: ldp x0,x30,[sp],#16
        code[12] = 0xA9BE57FE;                                   // stp x30,x21,[sp,#-0x20]! (original)
        code[13] = bImm(RVA_LATEUPDATE + 4, RVA_CAVE1 + 0x34);   // b LateUpdate+4
        for (int i = 0; i < 14; i++) wpm<uint32_t>(base + RVA_CAVE1 + i * 4, code[i]);

        // Activate: redirect LateUpdate's first instruction into the cave.
        uint32_t bword = bImm(RVA_CAVE1, RVA_LATEUPDATE);
        wpm<uint32_t>(base + RVA_LATEUPDATE, bword);
        installed = true;

        // Diagnostic: read everything back so we can tell if writing to the
        // read-only .text actually worked on this device/kernel.
        uint32_t lu_back   = rpm<uint32_t>(base + RVA_LATEUPDATE);
        uint32_t cave_back = rpm<uint32_t>(base + RVA_CAVE1);
        uint64_t lit_back  = rpm<uint64_t>(base + RVA_CAVE2 + 8);
        FILE* f = fopen("/data/local/tmp/tpbody.txt", "w");
        if (f) {
            fprintf(f, "base=0x%lx\n", (unsigned long)base);
            fprintf(f, "LateUpdate: wrote=0x%08x  readback=0x%08x  %s\n",
                    bword, lu_back, (lu_back == bword ? "WRITE-OK" : "WRITE-FAILED(.text RO)"));
            fprintf(f, "cave1[0]: wrote=0x%08x readback=0x%08x  %s\n",
                    0xA9BF7BE0u, cave_back, (cave_back == 0xA9BF7BE0u ? "OK" : "FAILED"));
            fprintf(f, "cave2 callee lit readback=0x%lx expect=0x%lx (SetTPSView)\n",
                    (unsigned long)lit_back, (unsigned long)(base + RVA_CALLEE));
            fclose(f);
        }
    } else if (!enable && installed) {
        wpm<uint32_t>(base + RVA_LATEUPDATE, ORIG_LU_INSN); // restore original
        installed = false;
    }
}

// Anti-aim, run from the high-frequency thread so it out-writes the game and
// holds a stable direction (the body no longer flips toward the camera).
void antiAimTick() {
    if (!antiaim) return;
    uintptr_t pm = (uintptr_t)resolvePlayerManager();
    if (!pm) return;
    uintptr_t lp = rpm<uintptr_t>(pm + 0x70);
    if (!lp) return;

    uintptr_t aim_controller = rpm<uintptr_t>(lp + 0x80);
    if (!aim_controller) return;
    uintptr_t movement_controller = rpm<uintptr_t>(aim_controller + 0xB8);
    uintptr_t transform = rpm<uintptr_t>(movement_controller + 0xC0);
    uintptr_t character = rpm<uintptr_t>(aim_controller + 0xD0);
    if (!transform && !character) return;

    static float aa_spin = 0.0f;
    aa_spin += aa_spin_speed * 0.01f;          // smooth spin regardless of tick rate
    if (aa_spin >= 360.0f) aa_spin -= 360.0f;

    float yaw = 0.0f;
    switch (aa_yaw_mode) {
        case 1: yaw = aa_yaw_value; break;                                   // static (one side)
        case 2: yaw = aa_spin; break;                                        // spin
        case 3: yaw = ((rand() & 1) ? aa_yaw_value : -aa_yaw_value); break;  // jitter
        default: yaw = 0.0f; break;
    }
    float pitch = 0.0f;
    switch (aa_pitch_mode) {
        case 1: pitch = aa_pitch_value; break;                 // down
        case 2: pitch = -aa_pitch_value; break;                // up
        case 3: pitch = 0.0f; break;                           // zero
        case 4: pitch = (float)((rand() % 180) - 90); break;   // jitter
        default: pitch = 0.0f; break;
    }

    Vector3 rot(pitch, yaw, 0.0f);
    if (transform) SetEulerAngels(transform, rot);
    if (aa_body && character) SetEulerAngels(character, rot);
}

void drawEsp() {
    if (!esp.enable) return;
    uintptr_t playerManager = (uintptr_t)resolvePlayerManager();
    if (!playerManager) return;
    uintptr_t playerList = rpm<uintptr_t>(playerManager + 0x28);
    if (!playerList) return;
    uintptr_t localPlayer = rpm<uintptr_t>(playerManager + 0x70);
    if (!localPlayer) return;
    int playerCount = rpm<int>(playerList + 0x20);
    if (playerCount <= 0) return;
    worldMatrix viewMatrix = rpm<worldMatrix>(rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(localPlayer + 0xE8) + 0x20) + 0x10) + 0x100);
    for (int i = 0; i < playerCount; i++) {
        uintptr_t currentPlayer = rpm<uintptr_t>(rpm<uintptr_t>(playerList + 0x18) + 0x30 + 0x18 * i);
        Vector3 playerPosition = rpm<Vector3>(rpm<uintptr_t>(rpm<uintptr_t>(currentPlayer + 0x98) + 0xB0) + 0x44);
		int health = getProp<int>(currentPlayer, "health");
		int armor = getProp<int>(currentPlayer, "armor");
		if ((!currentPlayer || currentPlayer == localPlayer) || (rpm<uint8_t>(currentPlayer + 0x79) == rpm<uint8_t>(localPlayer + 0x79) || (playerPosition == Vector3::Zero()) || (health < 0 || health > 100) || (armor < 0 || armor > 100))) continue;
        bool isHeadVisible, isFootVisible;
		auto playerHeadPos = worldToScreen(playerPosition + Vector3(0, 1.67f, 0), viewMatrix, &isHeadVisible);
		auto playerFeetPos = worldToScreen(playerPosition, viewMatrix, &isFootVisible);
		if (!isHeadVisible || !isFootVisible) continue;
		float boxHeight = playerFeetPos.y - playerHeadPos.y;
		float boxWidth = boxHeight * 0.35f;
		ImVec2 boxTopLeft = ImVec2(playerHeadPos.x - boxWidth, playerHeadPos.y);
		ImVec2 boxBottomRight = ImVec2(playerHeadPos.x + boxWidth, playerFeetPos.y);
        float boxBoxHeight = boxBottomRight.y - boxTopLeft.y;
		if (esp.box) {
			if (esp.boxFilled) {
				float espColor[3] = {255, 255, 255};
				GetBackgroundDrawList()->AddRect(boxTopLeft, boxBottomRight, ImColor(espColor[0], espColor[1], espColor[2], float(esp.boxFilledOpacity) / 100.0f), 0);
			}
			GetBackgroundDrawList()->AddRect(boxTopLeft, boxBottomRight, ImColor(0, 0, 0), 0, 0, 5);
			GetBackgroundDrawList()->AddRect(boxTopLeft, boxBottomRight, ImColor(255, 255, 255), 0, 0, 2);
		}
		if (esp.name) {
			auto playerName = getPlayerName(currentPlayer);
			string name = playerName.asUtf8();
			float centerX = (boxTopLeft.x + boxBottomRight.x) * 0.5f;
			ImVec2 textSize = verdanaFont->CalcTextSizeA(17.0f, FLT_MAX, 0.0f, name.c_str());
			float textX = centerX - textSize.x * 0.5f;
			float textY = boxTopLeft.y - 20.0f;
			ImVec2 textPos = ImVec2(textX, textY);
			GetBackgroundDrawList()->AddText(verdanaFont, 18.0f, ImVec2(textPos.x, textPos.y + 2), ImColor(0, 0, 0), name.c_str());	
			GetBackgroundDrawList()->AddText(verdanaFont, 18.0f, textPos, ImColor(255, 255, 255), name.c_str());	
		}
		if (esp.health) {
			ImVec2 healthBarBgStart = ImVec2(boxTopLeft.x - 8, boxBottomRight.y + 0.7);
			ImVec2 healthBarBgEnd = ImVec2(boxTopLeft.x - 4, boxBottomRight.y - boxBoxHeight - 0.7);
			GetBackgroundDrawList()->AddRectFilled(healthBarBgStart, healthBarBgEnd, ImColor(0, 0, 0));
			float healthHeight = (health / 100.0f) * boxBoxHeight;
			ImVec2 healthBarStart = ImVec2(boxTopLeft.x - 8, boxBottomRight.y + 0.7);
			ImVec2 healthBarEnd = ImVec2(boxTopLeft.x - 4, boxBottomRight.y - healthHeight - 0.7);
			if (esp.healthBarGradient) {
				GetBackgroundDrawList()->AddRectFilledMultiColor(healthBarStart, healthBarEnd, ImColor(esp.healthBarGradientBottomColor[0], esp.healthBarGradientBottomColor[1], esp.healthBarGradientBottomColor[2]), ImColor(esp.healthBarGradientBottomColor[0], esp.healthBarGradientBottomColor[1], esp.healthBarGradientBottomColor[2]), ImColor(esp.healthBarGradientTopColor[0], esp.healthBarGradientTopColor[1], esp.healthBarGradientTopColor[2]), ImColor(esp.healthBarGradientTopColor[0], esp.healthBarGradientTopColor[1], esp.healthBarGradientTopColor[2]));
			} else if (esp.healthBarAdaptive) {
				if (health > 66) {
					GetBackgroundDrawList()->AddRectFilled(healthBarStart, healthBarEnd, ImColor(0, 255, 0));
				} else if (health > 33) {
					GetBackgroundDrawList()->AddRectFilled(healthBarStart, healthBarEnd, ImColor(255, 165, 0));
				} else if (health > 0) {
					GetBackgroundDrawList()->AddRectFilled(healthBarStart, healthBarEnd, ImColor(255, 0, 0));
				}
			} else {
				GetBackgroundDrawList()->AddRectFilled(healthBarStart, healthBarEnd, ImColor(esp.healthBarColor[0], esp.healthBarColor[1], esp.healthBarColor[2]));
			}
			GetBackgroundDrawList()->AddRect(healthBarBgStart, healthBarBgEnd, ImColor(0, 0, 0), 0, 0, 1.5);
		}
		if (esp.armor) {
			float fullWidth = (boxBottomRight.x - boxTopLeft.x);
			ImVec2 armorBarBgStart = ImVec2(boxTopLeft.x - 1.0f, boxBottomRight.y + 4);
			ImVec2 armorBarBgEnd = ImVec2(boxTopLeft.x + fullWidth + 1.0f, boxBottomRight.y + 8);
			GetBackgroundDrawList()->AddRectFilled(armorBarBgStart, armorBarBgEnd, ImColor(0, 0, 0));
			float armorWidth = (armor / 100.0f) * (boxBottomRight.x - boxTopLeft.x);
			ImVec2 armorBarStart = ImVec2(boxTopLeft.x - 1.0f, boxBottomRight.y + 4);
			ImVec2 armorBarEnd = ImVec2(boxTopLeft.x + armorWidth + 1.0f, boxBottomRight.y + 8);
			if (esp.armorBarGradient) {
				GetBackgroundDrawList()->AddRectFilledMultiColor(armorBarStart, armorBarEnd, ImColor(esp.armorBarGradientRightColor[0], esp.armorBarGradientRightColor[1], esp.armorBarGradientRightColor[2]), ImColor(esp.armorBarGradientLeftColor[0], esp.armorBarGradientLeftColor[1], esp.armorBarGradientLeftColor[2]), ImColor(esp.armorBarGradientLeftColor[0], esp.armorBarGradientLeftColor[1], esp.armorBarGradientLeftColor[2]), ImColor(esp.armorBarGradientRightColor[0], esp.armorBarGradientRightColor[1], esp.armorBarGradientRightColor[2]));
			} else if (esp.armorBarAdaptive) {
				if (armor > 66) {
					GetBackgroundDrawList()->AddRectFilled(armorBarStart, armorBarEnd, ImColor(0, 255, 0));
				} else if (armor > 33) {
					GetBackgroundDrawList()->AddRectFilled(armorBarStart, armorBarEnd, ImColor(255, 165, 0));
				} else if (armor > 0) {
					GetBackgroundDrawList()->AddRectFilled(armorBarStart, armorBarEnd, ImColor(255, 0, 0));
				}
			} else {
				GetBackgroundDrawList()->AddRectFilled(armorBarStart, armorBarEnd, ImColor(esp.armorBarColor[0], esp.armorBarColor[1], esp.armorBarColor[2]));
			}
			GetBackgroundDrawList()->AddRect(armorBarBgStart, armorBarBgEnd, ImColor(0, 0, 0), 0, 0, 1.5);
		}
    }
}

template<typename T>
void setProp(uintptr_t player, const char* tag, T value) {
	if (!player || !tag) return;
	T property = NULL;
	uintptr_t props = rpm<uintptr_t>(getPhoton(player) + 0x38);
	if (!props) return;
	int size = rpm<int>(props + 0x20);
	if (size <= 0) return;
	for (int i = 0; i < size; i++) {
		uintptr_t propKey = rpm<uintptr_t>(rpm<uintptr_t>(props + 0x18) + 0x28 + 0x18 * i);
		uintptr_t propValue = rpm<uintptr_t>(rpm<uintptr_t>(props + 0x18) + 0x30 + 0x18 * i);
		if (!propKey) continue;
		string keyValue = rpm<monoString>(propKey).asUtf8();
		if (keyValue.empty()) continue;
		if (strstr(keyValue.c_str(), tag)) {
			wpm<int>(propValue + 0x10, value);
			break;
		}
	}
	return;
}

#define LOBYTE(x)   (*((uint8_t*)&(x)))
#define LOWORD(x)   (*((uint16_t*)&(x)))
#define LODWORD(x)  (*((uint32_t*)&(x)))
#define HIBYTE(x)   (*((uint8_t*)&(x)+1))
#define HIWORD(x)   (*((uint16_t*)&(x)+1))
#define HIDWORD(x)  (*((uint32_t*)&(x)+1))
#define BYTEn(x, n)   (*((uint8_t*)&(x)+n))
#define WORDn(x, n)   (*((uint16_t*)&(x)+n))

struct SafeFloat
{
    int key;
    int value;
    union fi { float f; int i; };
    float get()
    {
        fi vV;
        if ((key & 1) == 0)
        {
            vV.i = value ^ key;
            return vV.f;
        }
        vV.i = BYTEn(value,2) | value & 0xFF00FF00 | (LOBYTE(value) << 16);
        return vV.f;
    }
    
    static void Write(uintptr_t address, float value)
    {
        int cryptoKey = rpm<int>(address);
        cryptoKey |= 1;
        fi v;
        v.f = value;
        fi v2;
        v2.i = (v.i ^ cryptoKey);
        wpm(address, cryptoKey);
        wpm(address + 4, v2.f);
    }
};

void setSafe(uintptr_t inst, uintptr_t offset, bool nullable, int32_t actual_value)
{
    if (nullable)
        offset += 0x4;
    wpm<int32_t>(inst + offset + 0x4, actual_value ^ rpm<int32_t>(inst + offset));
}

struct DamageVal {
    int id;
    std::array<int, 4> vals;
    DamageVal(int _id, std::array<int, 4> _vals) : id(_id), vals(_vals) {}
};

bool enableDamage = false;

void gun(uintptr_t controller, uintptr_t parameters) {
    static std::vector<DamageVal> DamageVals;
    static bool initialized = false;
    auto Damage = rpm<uintptr_t>(parameters + 0x140);  

    if (!initialized && Damage != 0) {
        std::array<int, 4> tempD{};  
        for (int i = 0; i < 4; i++) {  
            tempD[i] = rpm<int>(Damage + 0x14 + i * 0x4);  
        }  
        DamageVals.emplace_back(44, tempD);  
        initialized = true;  
    }  

    if (Damage != 0 && enableDamage) {
    for (int i = 0; i < 4; i++) {  
        setSafe(Damage, 0x28 + i * 0xC, true, static_cast<int32_t>(200));  
        setSafe(Damage, 0x34 + i * 0xC, true, static_cast<int32_t>(200));  
        setSafe(Damage, 0x40 + i * 0xC, true, static_cast<int32_t>(200));  
        setSafe(Damage, 0x4C + i * 0xC, true, static_cast<int32_t>(200));  
    }  
}

else if (Damage != 0 && !enableDamage && initialized) {
    for (int i = 0; i < 4; i++) {  
        setSafe(Damage, 0x28 + i * 0xC, false, DamageVals[0].vals[i]);  
        setSafe(Damage, 0x34 + i * 0xC, false, DamageVals[0].vals[i]);  
        setSafe(Damage, 0x40 + i * 0xC, false, DamageVals[0].vals[i]);  
        setSafe(Damage, 0x4C + i * 0xC, false, DamageVals[0].vals[i]);  
    }  
}
}

void offer()
{
	if (!invismode) {
	}
	uintptr_t playerManager = (uintptr_t)resolvePlayerManager();
	
    if (!playerManager) {
	}
	
    uintptr_t playerList = rpm<uintptr_t>(playerManager + oxorany(0x28));
    
    if (!playerList) {
	}
	
    auto localPlayer = rpm<uintptr_t>(playerManager + oxorany(0x70));
    
    if (!localPlayer) {
	}
	
    auto weaponry = rpm<uintptr_t>(localPlayer + oxorany(0x88));
    
    if (!weaponry) {
	}
	
    auto weaponcontroller = rpm<uintptr_t>(weaponry + oxorany(0xA0));
    
    if (!weaponcontroller) {
	}
	
	auto weaponparam= rpm<uintptr_t>(weaponcontroller + oxorany(0xA0));
	
    if (!weaponparam) {
	}
	
	uintptr_t gameManager = rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(proc.base + oxorany(135050840)) + 0x198) + 0xC0) + 0x0);
	
    if (!gameManager) {
	}
	
	uintptr_t localGame = rpm<uintptr_t>(gameManager + oxorany(0x58));
	
    if (!localGame) {
	}
	
   
	
	if (skiptable) {
		wpm<float>(localGame + oxorany(0x98), 0.0f);
	}
	
	uintptr_t bombManager = rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(proc.base + oxorany(0xAA09DB8)) + 0x190) + 0x198) + 0x0);
	
    if (!bombManager) {
	}
	
	uintptr_t localBomb = rpm<uintptr_t>(bombManager + oxorany(0xB0));
	
    if (!localBomb) {
	}
	
	if (bexplos) {
		SafeFloat::Write(localBomb + oxorany(0x158) + 4, 0.0f);
	}
	
	if (bplant) {
		SafeFloat::Write(localBomb + oxorany(0x14C) + 4, 0.0f);
		SafeFloat::Write(localBomb + oxorany(0x17C) + 4, 0.0f);
	}
	
	if (bimpulse) {
		SafeFloat::Write(localBomb + oxorany(0x188) + 4, 999999999999.0f);
	}
	
	if (bradius) {
		SafeFloat::Write(localBomb + oxorany(0x164) + 4, 999999999.0f);
	}
	
	if (bdamage) {
		SafeFloat::Write(localBomb + oxorany(0x170) + 4, 99999999999.0f);
	}
   
    auto playercontrols = rpm<uintptr_t>(proc.base + oxorany(151547040));
    auto static_fields = rpm<uintptr_t>(playercontrols + oxorany(0x30));

    if (high_jump) {
        Change<safe>(static_fields + oxorany(0x40),
               [](safe *obj) {
                  obj->set<float>(high_value);
               });

        Change<safe>(static_fields + oxorany(0x48),
               [](safe *obj) {
                  obj->set<bool>(true);
               });

        Change<safe>(static_fields + oxorany(0x50),
               [](safe *obj) {
                  obj->set<bool>(true);
               });
    }

	if (antiaim) {
    // moved to antiAimTick() in the high-frequency thread so it out-writes the
    // game each frame and holds a single direction (no flicker toward camera).
    }

	if (aafk) {
		uintptr_t pp = rpm<uintptr_t>(localPlayer + oxorany(0x138));
		wpm<bool>(pp + oxorany(0x31), false);
	}
	
}
