#include "world/vector3.h"
#include "world/angles.h"
#include <random>
//#include "bools.h"
//#include "json.h"
//#include "monoString.h"
using namespace std;
using namespace json;

//#include "configs.h"

using namespace ImGui;
#define oxorany

using std::string;

extern ImFont* verdanaFont;
extern ImFont* esp_fonts;
extern ImFont* weapons_fonts;
extern ImFont* knife_fonts;
extern ImFont* granades_fonts;
extern ImFont* pistols_fonts;

using namespace ImGui;

// Canonical PlayerManager static-field resolver for Standoff 2 v0.38.2.
// IL2CPP layout: (proc.base + typeOffset) -> +0x60 (static_fields) -> +0x10 (instance).
static inline bool wtx_likely_ptr(uint64_t p) {
    return p > 0x10000ull && p < 0x0000FFFFFFFFFFFFull;
}
static inline uint64_t resolvePlayerManager() {
    const uint64_t candidates[] = { 153868920ull, 132435632ull };
    for (uint64_t off : candidates) {
        uint64_t cls = rpm<uint64_t>(proc.base + off);
        if (!cls) continue;
        uint64_t obj = rpm<uint64_t>(cls + 0x60);
        if (!obj) continue;
        uint64_t fields = rpm<uint64_t>(obj + 0x10);
        if (wtx_likely_ptr(fields)) return fields;
        uint64_t indirect = rpm<uint64_t>(fields);
        if (wtx_likely_ptr(indirect)) return indirect;
    }
    return 0;
}

std::string utf16le_to_utf8s(const std::u16string &u16str) {
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

struct Strings
{
    uint64_t klass;
    uint64_t monitor_data;
    int length;
    char first_char[ 256 ];

    std::string Get()
    {
        u16string utf16( ( char16_t * ) first_char, 0, length);

        std::string converted = utf16le_to_utf8s( utf16 );

        return converted;
    }
};

//ImFont*weapon;
std::string;


struct HitInfo {
    std::string name;
    int damage;
    float timeLeft;
    float offsetY;
};

static std::vector<HitInfo> hits;
static std::unordered_map<uintptr_t, int> lastHealth;

void DrawOffscreenArrow(
    const ImVec2& screenCenter,
    const ImVec2& targetPos,
    float radius,
    ImU32 color,
    float size,
    bool glow,
    float glowStrength
)
{
    ImVec2 dir = ImVec2(targetPos.x - screenCenter.x, targetPos.y - screenCenter.y);
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f) return;
    
    dir.x /= len;
    dir.y /= len;
    
    ImVec2 arrowCenter = ImVec2(screenCenter.x + dir.x * radius, screenCenter.y + dir.y * radius);
    ImVec2 perp(-dir.y, dir.x);
    
    ImVec2 p1 = ImVec2(arrowCenter.x + dir.x * size, arrowCenter.y + dir.y * size);
    ImVec2 p2 = ImVec2(arrowCenter.x - dir.x * size * 0.5f + perp.x * size * 0.5f, arrowCenter.y - dir.y * size * 0.5f + perp.y * size * 0.5f);
    ImVec2 p3 = ImVec2(arrowCenter.x - dir.x * size * 0.5f - perp.x * size * 0.5f, arrowCenter.y - dir.y * size * 0.5f - perp.y * size * 0.5f);
    
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    
    if (glow && glowStrength > 0)
    {
        for (int i = 1; i <= (int)glowStrength; i++)
        {
            float a = (1.0f - (float)i / glowStrength) * 0.5f;
            ImU32 gcol = (color & 0x00FFFFFF) | ((ImU32)(a * 255) << 24);
            dl->AddTriangleFilled(p1, p2, p3, gcol);
        }
    }
    
    dl->AddTriangleFilled(p1, p2, p3, color);
}


struct Vector4 {
        float x,y,z,w;
};

struct TMatrix {
    Vector4 position;
    Quaternion rotation;
    Vector4 scale;
};

static Quaternion EulerToQuaternion(Vector3 angles){
        float halfRoll  = angles.x * 0.5f;
        float halfPitch = angles.y * 0.5f;
        float halfYaw   = angles.z * 0.5f;
        float cr = std::cos(halfRoll);
        float sr = std::sin(halfRoll);
        float cp = std::cos(halfPitch);
        float sp = std::sin(halfPitch);
        float cy = std::cos(halfYaw);
        float sy = std::sin(halfYaw);
        Quaternion q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;
        return q;
    }

    static void SetEulerAngels(uintptr_t transform, Vector3 value){
        auto unk1 = rpm<uintptr_t>(transform + 0x10);
        auto matrix = rpm<uintptr_t>(unk1 + 0x38);
        auto index = rpm<int>(unk1 + 0x40);
        auto matrix_list = rpm<uintptr_t>(matrix + 0x18);
        auto matr = rpm<TMatrix>(matrix_list + sizeof(TMatrix) * index);
        Quaternion newRotation = EulerToQuaternion(value);
        matr.rotation = newRotation;
        wpm<TMatrix>(matrix_list + sizeof(TMatrix) * index, matr);
    }

 Vector3 getPosition(uint64_t transObj2) {
        uint64_t transObj = rpm<uint64_t>(transObj2 + 0x10);
    uint64_t matrix = rpm<uint64_t>(transObj + 0x38);
    uint64_t index = rpm<uint64_t>(transObj + 0x40);
    uint64_t matrix_list = rpm<uint64_t>(matrix + 0x18);
    uint64_t matrix_indices = rpm<uint64_t>(matrix + 0x20);
    Vector3 result = rpm<Vector3>(matrix_list + sizeof(TMatrix) * index);
    int transformIndex = rpm<int>(matrix_indices + sizeof(int) * index);
    while(transformIndex >= 0) {
        TMatrix tMatrix = rpm<TMatrix>(matrix_list + sizeof(TMatrix) * transformIndex);
        float rotX = tMatrix.rotation.x;
        float rotY = tMatrix.rotation.y;
        float rotZ = tMatrix.rotation.z;
        float rotW = tMatrix.rotation.w;
        float scaleX = result.x * tMatrix.scale.x;
        float scaleY = result.y * tMatrix.scale.y;
        float scaleZ = result.z * tMatrix.scale.z;
        result.x = tMatrix.position.x + scaleX + (scaleX * ((rotY * rotY * -2.0) - (rotZ * rotZ * 2.0))) + (scaleY * ((rotW * rotZ * -2.0) - (rotY * rotX * -2.0))) + (scaleZ * ((rotZ * rotX * 2.0) - (rotW * rotY * -2.0)));
        result.y = tMatrix.position.y + scaleY + (scaleX * ((rotX * rotY * 2.0) - (rotW * rotZ * -2.0))) + (scaleY * ((rotZ * rotZ * -2.0) - (rotX * rotX * 2.0))) + (scaleZ * ((rotW * rotX * -2.0) - (rotZ * rotY * -2.0)));
        result.z = tMatrix.position.z + scaleZ + (scaleX * ((rotW * rotY * -2.0) - (rotX * rotZ * -2.0))) + (scaleY * ((rotY * rotZ * 2.0) - (rotW * rotX * -2.0))) + (scaleZ * ((rotX * rotX * -2.0) - (rotY * rotY * 2.0)));
        transformIndex = rpm<int>(matrix_indices + sizeof(int) * transformIndex);
    }
 
    return result;
}


namespace MonoString {
    string utf16ToUtf8(const u16string& utf16Str) {
        if (utf16Str.empty()) return {};
        constexpr char16_t marker = 0xFEFF;
        const char16_t* source = utf16Str.data();
        size_t remainingLength = utf16Str.length();
        if (source[0] == marker) {
            source++;
            remainingLength--;
        }
        string utf8Str;
        utf8Str.reserve(remainingLength * 3);
        for (size_t i = 0; i < remainingLength; ++i) {
            const char16_t codeUnit = source[i];
            if (codeUnit < 0x80) {
                utf8Str.push_back(static_cast<char>(codeUnit));
                continue;
            }
            if (codeUnit < 0x800) {
                utf8Str.push_back(static_cast<char>((codeUnit >> 6) | 0xC0));
                utf8Str.push_back(static_cast<char>((codeUnit & 0x3F) | 0x80));
                continue;
            }
            if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF && i + 1 < remainingLength) {
                const char16_t highSurrogate = codeUnit;
                const char16_t lowSurrogate = source[++i];
                const uint32_t codePoint = 0x10000 + ((highSurrogate - 0xD800) << 10) + (lowSurrogate - 0xDC00);           
                utf8Str.push_back(static_cast<char>((codePoint >> 18) | 0xF0));
                utf8Str.push_back(static_cast<char>(((codePoint >> 12) & 0x3F) | 0x80));
                utf8Str.push_back(static_cast<char>(((codePoint >> 6) & 0x3F) | 0x80));
                utf8Str.push_back(static_cast<char>((codePoint & 0x3F) | 0x80));
                continue;
            }
            utf8Str.push_back(static_cast<char>((codeUnit >> 12) | 0xE0));
            utf8Str.push_back(static_cast<char>(((codeUnit >> 6) & 0x3F) | 0x80));
            utf8Str.push_back(static_cast<char>((codeUnit & 0x3F) | 0x80));
        }
        return utf8Str;
    }
    
    struct monoString {
        uintptr_t classPointer;
        uintptr_t synchronizationData;
        int stringLength;
        char16_t buffer[128];
        string asUtf8() {
            if (stringLength <= 0 || stringLength > 128) return {};
            return utf16ToUtf8(u16string(buffer, stringLength));
        }
    };
}

namespace PlayerManager {
    using namespace MonoString; 
    struct Matrix {             
        float row1Column1, row1Column2, row1Column3, row1Column4;
        float row2Column1, row2Column2, row2Column3, row2Column4;
        float row3Column1, row3Column2, row3Column3, row3Column4;
        float row4Column1, row4Column2, row4Column3, row4Column4;
    };
    
    struct Stringer
{
    uint64_t klass;
    uint64_t monitor_data;
    int length;
    char first_char[ 256 ];

    std::string Get()
    {
        u16string utf16( ( char16_t * ) first_char, 0, length);

       std::string converted = utf16le_to_utf8s( utf16 );

        return converted;
    }
};
    
    Stringer get_weapon_name(uint64_t player) {
        uint64_t weapry = rpm<uint64_t>(player + 0x88);
        if(weapry) {
                uint64_t weap = rpm<uint64_t>(weapry + 0xA0);
                if(weap) {
                        uint64_t weapp = rpm<uint64_t>(weap + 0xA8);
                        if(weapp) {
                                Stringer name = rpm<Stringer>(rpm<uint64_t>(weapp + 0x20));
                                return name;
                        }
                }
        }
        return {};
}
struct csharparray{
 void*info;void*info2;void*info3;
  int size;
  void *ptr[0];
};

int get_weaponid(uint64_t player) {
        uint64_t weapry = rpm<uint64_t>(player + 0x88);
        if(weapry) {
                uint64_t weap = rpm<uint64_t>(weapry + 0xA0);
                if(weap) {
                        uint64_t weapp = rpm<uint64_t>(weap + 0xA8);
                        if(weapp) {
                                auto name = rpm<int>(rpm<uint64_t>(weapp + 0x18));
                                return name;
                        }
                }
        }
        return {};
}
ImFont*weapon;
std::string;
void TextOutline1(ImColor color, const char *text, ImVec2 position, float size, float width) {
  float stroke = 0.95;
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x - stroke), (int)(position.y + stroke)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x - stroke), (int)(position.y - stroke)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x + stroke), (int)(position.y + stroke)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x + stroke), (int)(position.y - stroke)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x), (int)(position.y + stroke)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x), (int)(position.y - stroke)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x + stroke), (int)(position.y)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, ImVec2((int)(position.x + stroke), (int)(position.y)), ImColor(0, 0, 0), text);
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), size, position, color, text);
    }
        void AddTextt(ImFont* f, float s, bool shadow, bool outline, const ImVec2& pos, ImColor color, const char* value, ImDrawList* drawlist = ImGui::GetBackgroundDrawList()) {
    if (outline) {
        drawlist->AddText(f, s, {pos.x, pos.y}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x + 0.75f, pos.y + 0.75f}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x, pos.y + 0.75f}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x + 0.75f, pos.y}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x - 0.75f, pos.y - 0.75f}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x - 0.75f, pos.y}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x, pos.y - 0.75f}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x, pos.y}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x + 1, pos.y + 1}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x, pos.y + 1}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x + 1, pos.y}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x - 1, pos.y - 1}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x - 1, pos.y}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
        drawlist->AddText(f, s, {pos.x, pos.y - 1}, ImColor(0, 0, 0, (int)float(color.Value.w * 255)), value);
    }
    if (shadow) drawlist->AddText(f, s, { pos.x + 1, pos.y + 1 }, ImColor(5, 5, 5, (int)float(color.Value.w * 255)), value);
    drawlist->AddText(f, s, pos, color, value);
}
std::string getWeaponString123(uint64_t player) {
    if (player) {
        std::string gay = "";
                
        std::string weaponn = get_weapon_name(player).Get();
                
        if (!weaponn.empty()) {
            std::string weaponName = weaponn.c_str();
            if (weaponName == "G22") {
                gay = "D";
            } else if (weaponName == "USP") {
                gay = "G";
            } else if (weaponName == "P350") {
                gay = "G";
            } else if (weaponName == "Berretas") {
                gay = "B";
            } else if (weaponName == "TEC-9") {
                gay = "H";
            } else if (weaponName == "F/S") {
                gay = "C";
            } else if (weaponName == "Desert Eagle") {
                gay = "A";
            } else if (weaponName == "FabM") {
                gay = "c";
            } else if (weaponName == "SM1014") {
                gay = "b";
            } else if (weaponName == "SPAS") {
                gay = "d";
            } else if (weaponName == "M60") {
                gay = "f";
            } else if (weaponName == "MAC10") {
                gay = "O";
            } else if (weaponName == "UMP45") {
                gay = "L";
            } else if (weaponName == "MP5") {
                gay = "L";
            } else if (weaponName == "MP7") {
                gay = "N";
            } else if (weaponName == "P90") {
                gay = "P";
                        } else if   (weaponName == "Akimbo Uzi") {
                gay = "I";
            } else if (weaponName == "FN FAL") {
                gay = "Q";
            } else if (weaponName == "FAMAS") {
                gay = "R";
            } else if (weaponName == "VAL") {
                gay = "M";
                    } else if (weaponName == "AKR") {
                gay = "W";
            } else if (weaponName == "M4") {
                gay = "S";
            } else if (weaponName == "M4A1") {
                gay = "T";
            } else if (weaponName == "AKR12") {
                gay = "U";
            } else if (weaponName == "M16") {
                gay = "V";
            } else if (weaponName == "M40") {
                gay = "a";
            } else if (weaponName == "AWM") {
                gay = "Z";
            } else if (weaponName == "M110") {
                gay = "Y";

            } else if (weaponName == "Bomb") {
                gay = "o";
            } else if (weaponName == "Knife") {
                gay = "1";
            } else if (weaponName == "KnifeBayonet") {
                gay = "5";
            } else if (weaponName == "KnifeKarambit") {
                gay = "4";
            } else if (weaponName == "jKommando") {
                gay = "6";
            } else if (weaponName == "KnifeButterfly") {
                gay = "8";
            } else if (weaponName == "FlipKnife") {
                gay = "10";
            } else if (weaponName == "ScorpionKnife") {
                gay = "3";
            } else if (weaponName == "KunaiKnife") {
                gay = "0";
            } else if (weaponName == "DaggerKnife") {
                gay = "9";
            } else if (weaponName == "KnifeStilet") {
                gay = "0";
                        } else if (weaponName == "KnifeMantis") {
                gay = "4";
            } else if (weaponName == "GrenadeSmoke") {
                gay = "k";
            } else if (weaponName == "GrenadeFlash") {
                gay = "i";
            } else if (weaponName == "GrenadeHE") {
                gay = "j";
            } else if (weaponName == "GrenadeMolotov") {
                gay = "k";
            } else if (weaponName == "GrenadeIncendiary") {
                gay = "l";

            }
        }
        return gay;
    }
     return "";
}

struct WeaponIcon {
    std::string text;
    ImFont* font = nullptr;
};

WeaponIcon getWeaponIcon(uint64_t player) {
    std::string name = get_weapon_name(player).Get();

    if (name == "M4A1")          return {"g", weapons_fonts};
    if (name == "AKR")           return {"h", weapons_fonts};
    if (name == "AWM")           return {"c", weapons_fonts};
    if (name == "G22")           return {"q", pistols_fonts};
    if (name == "USP")           return {"w", pistols_fonts};
    if (name == "P350")          return {"e", pistols_fonts};
    if (name == "Desert Eagle")        return {"u", pistols_fonts};
    if (name == "TEC-9")          return {"t", pistols_fonts};
    if (name == "F/S")     return {"y", pistols_fonts};
    if (name == "Berettas")      return {"r", pistols_fonts};
    if (name == "UMP45")         return {"y", weapons_fonts};
    if (name == "Akimbo Uzi")     return {"o", weapons_fonts};
    if (name == "MP7")           return {"i", weapons_fonts};
    if (name == "P90")           return {"p", weapons_fonts};
    if (name == "MP5")           return {"u", weapons_fonts};
    if (name == "MAC10")         return {"t", weapons_fonts};
    if (name == "VAL")           return {"d", weapons_fonts};
    if (name == "AKR12")         return {"j", weapons_fonts};
    if (name == "M4")            return {"f", weapons_fonts};
    if (name == "M16")           return {"k", weapons_fonts};
    if (name == "FAMAS")         return {"s", weapons_fonts};
    if (name == "FN FAL")         return {"a", weapons_fonts};
    if (name == "M40")           return {"l", weapons_fonts};
    if (name == "M110")          return {"x", weapons_fonts};
    if (name == "Mallard")       return {"z", weapons_fonts};
    if (name == "SM1014")        return {"w", weapons_fonts};
    if (name == "FabM")          return {"q", weapons_fonts};
    if (name == "M60")           return {"r", weapons_fonts};
    if (name == "SPAS")          return {"e", weapons_fonts};
    if (name == "Knife")         return {"b", knife_fonts};
    if (name == "M9 Bayonet")  return {"q", knife_fonts};
    if (name == "Karambit") return {"w", knife_fonts};
    if (name == "jKommando")     return {"e", knife_fonts};
    if (name == "Butterfly")return {"r", knife_fonts};
    if (name == "Flip")     return {"t", knife_fonts};
    if (name == "Kunai")    return {"y", knife_fonts};
    if (name == "Scorpion") return {"u", knife_fonts};
    if (name == "Tanto")    return {"i", knife_fonts};
    if (name == "Dual Daggers")   return {"m", knife_fonts};
    if (name == "Kukri")    return {"p", knife_fonts};
    if (name == "Stiletto")   return {"a", knife_fonts};
    if (name == "Mantis")   return {"s", knife_fonts};
    if (name == "Fang")     return {"d", knife_fonts};
    if (name == "Sting")    return {"v", knife_fonts};
    if (name == "Flash")     return {"q", granades_fonts};
    if (name == "HE")  return {"t", granades_fonts};
    if (name == "Smoke")  return {"e", granades_fonts};
    if (name == "Molotov") return {"r", granades_fonts};
    if (name == "Thermite") return {"w", granades_fonts};
    if (name == "Bomb") return {"a", granades_fonts};
    if (name == "None")          return {"", nullptr};

    return {"?", esp_fonts};
}

    class Utils {
    public:
        struct Player {
            uintptr_t ptr;
            Vector3 Position;
            Player(uintptr_t ptr) : ptr(ptr) {}
            bool update() {
                if (!ptr) return false;
                Position = getPlayerLocation(ptr);
                return Position != Vector3{0, 0, 0};
            }
        };

        struct PlayerList {
            std::vector<Player> players;
            void update(uintptr_t playerArray, int playersCount) {
                if (!playerArray || playersCount <= 0) return;
                players.clear();
                for (int i = 0; i < playersCount; i++) {
                    uintptr_t ptr = rpm<uintptr_t>(playerArray + 0x30 + 0x18 * i);
                    if (!ptr) continue;
                    Player initPlayer(ptr);
                    if (initPlayer.update()) {
                        players.push_back(initPlayer);
                    }
                }
            }
            size_t size() const { return players.size(); }
            Player& operator[](size_t idx) { return players[idx]; }
        };

        struct PlayerData {
            Vector3 Position;
            ImVec2 headScreenPosition, feetScreenPosition;
            float boxHead, boxFeet, height, width;
            bool isHeadVisible, isFeetVisible;
            bool isValid;
        };

        static ImVec2 worldToDisplay(Vector3 position, Matrix view, bool* clip) {
            if (!clip) return ImVec2{0, 0};
            float projX = (view.row1Column1 * position.x) + (view.row2Column1 * position.y) + (view.row3Column1 * position.z) + view.row4Column1;
            float projY = (view.row1Column2 * position.x) + (view.row2Column2 * position.y) + (view.row3Column2 * position.z) + view.row4Column2;
            float projW = (view.row1Column4 * position.x) + (view.row2Column4 * position.y) + (view.row3Column4 * position.z) + view.row4Column4;
            float screenCenterX = displayX / 2.0f;
            float screenCenterY = displayY / 2.0f;
            float x = screenCenterX + (screenCenterX * projX / projW);
            float y = screenCenterY - (screenCenterY * projY / projW);
            *clip = projW > 0.0001f;
            return ImVec2{x, y};
        }

        static uintptr_t getPlayerManager() {
            return (uintptr_t)resolvePlayerManager();
        }

        static uintptr_t getLocalPlayer() {
            uintptr_t manager = getPlayerManager();
            if (!manager) return 0;
            return rpm<uintptr_t>(manager + 0x70);
        }

        static uintptr_t getPlayersList() {
            uintptr_t manager = getPlayerManager();
            if (!manager) return 0;
            return rpm<uintptr_t>(manager + 0x28);
        }

        static int getPlayersCount() {
            uintptr_t list = getPlayersList();
            if (!list) return 0;
            return rpm<int>(list + 0x20);
        }

        static uint8_t getLocalTeam() {
            uintptr_t player = getLocalPlayer();
            if (!player) return 0;
  //          return rpm<uint8_t>(player + 0x59);
            return rpm<uint8_t>(player + 0x79);
        }

        static Vector3 getPlayerLocation(uintptr_t player) {
            if (!player) return Vector3{0, 0, 0};
      //      uintptr_t transform = rpm<uintptr_t>(player + 0x78);
      uintptr_t transform = rpm<uintptr_t>(player + 0x98);
            if (!transform) return Vector3{0, 0, 0};
   //         uintptr_t transformData = rpm<uintptr_t>(transform + 0xB0);
   uintptr_t transformData = rpm<uintptr_t>(transform + 0xB0);
            if (!transformData) return Vector3{0, 0, 0};
            return rpm<Vector3>(transformData + 0x44);
        }

        static uintptr_t getPhoton(uintptr_t ptr) {
            if (!ptr) return 0;
            return rpm<uintptr_t>(ptr + 0x160);
        }

        template<typename T>
        static T getProp(uintptr_t ptr, const char* tag) {
            if (!ptr || !tag) return NULL;
            T property = NULL;
            uintptr_t props = rpm<uintptr_t>(getPhoton(ptr) + 0x38);
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

        static Matrix getViewMatrix(uintptr_t localPlayer) {
            Matrix empty = {};
            if (!localPlayer) return empty;
        uintptr_t transform = rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(localPlayer + 0xE8) + 0x20) + 0x10);
            if (!transform) return empty;
            return rpm<Matrix>(transform + 0x100);
        }
        
        static Vector3 getPosition(uint64_t transObj2) {
        uint64_t transObj = rpm<uint64_t>(transObj2 + 0x10);
        if (!transObj) return Vector3{0, 0, 0};
        
        uint64_t matrix = rpm<uint64_t>(transObj + 0x38);
        uint64_t index = rpm<uint64_t>(transObj + 0x40);
        uint64_t matrix_list = rpm<uint64_t>(matrix + 0x18);
        uint64_t matrix_indices = rpm<uint64_t>(matrix + 0x20);
        
        Vector3 result = rpm<Vector3>(matrix_list + sizeof(TMatrix) * index);
        int transformIndex = rpm<int>(matrix_indices + sizeof(int) * index);
        
        while (transformIndex >= 0) {
            TMatrix tMatrix = rpm<TMatrix>(matrix_list + sizeof(TMatrix) * transformIndex);
            float rotX = tMatrix.rotation.x;
            float rotY = tMatrix.rotation.y;
            float rotZ = tMatrix.rotation.z;
            float rotW = tMatrix.rotation.w;
            
            float scaleX = result.x * tMatrix.scale.x;
            float scaleY = result.y * tMatrix.scale.y;
            float scaleZ = result.z * tMatrix.scale.z;
            
            result.x = tMatrix.position.x + scaleX + (scaleX * ((rotY * rotY * -2.0) - (rotZ * rotZ * 2.0))) + (scaleY * ((rotW * rotZ * -2.0) - (rotY * rotX * -2.0))) + (scaleZ * ((rotZ * rotX * 2.0) - (rotW * rotY * -2.0)));
            result.y = tMatrix.position.y + scaleY + (scaleX * ((rotX * rotY * 2.0) - (rotW * rotZ * -2.0))) + (scaleY * ((rotZ * rotZ * -2.0) - (rotX * rotX * 2.0))) + (scaleZ * ((rotW * rotX * -2.0) - (rotZ * rotY * -2.0)));
            result.z = tMatrix.position.z + scaleZ + (scaleX * ((rotW * rotY * -2.0) - (rotX * rotZ * -2.0))) + (scaleY * ((rotY * rotZ * 2.0) - (rotW * rotX * -2.0))) + (scaleZ * ((rotX * rotX * -2.0) - (rotY * rotY * 2.0)));
            
            transformIndex = rpm<int>(matrix_indices + sizeof(int) * transformIndex);
        }
        
        return result;
    }
        
                Vector3 pasta;
Matrix mat;
bool pat;

        static PlayerData getPlayerData(uintptr_t player, uintptr_t localPlayer, Matrix view) {
            PlayerData data = {};
                        
                        bool ahaj;
                                Matrix hui = getViewMatrix(localPlayer);

                        
            if (!player || !localPlayer) return data;
            data.Position = getPlayerLocation(player);
            data.isValid = data.Position != Vector3{0, 0, 0};
            if (!data.isValid) return data;
            data.headScreenPosition = worldToDisplay(data.Position + Vector3{0, 1.8f, 0}, view, &data.isHeadVisible);
            data.feetScreenPosition = worldToDisplay(data.Position - Vector3{0, 0.14f, 0}, view, &data.isFeetVisible);
            data.height = fabs(data.feetScreenPosition.y - data.headScreenPosition.y);
            data.width = data.height * 0.25f;
            data.boxHead = data.headScreenPosition.x + 10;
            data.boxFeet = data.feetScreenPosition.x;
            if (data.boxHead > data.boxFeet) std::swap(data.boxHead, data.boxFeet);
            data.isValid = data.isHeadVisible && data.isFeetVisible;
            return data;
        }

        static int getDistance(Vector3 myPosition, Vector3 enemyPosition) {
            if (myPosition == Vector3{0,0,0} || enemyPosition == Vector3{0,0,0}) return 0;
            float distanceX = myPosition.x - enemyPosition.x;
            float distanceY = myPosition.y - enemyPosition.y;
            float distanceZ = myPosition.z - enemyPosition.z;
            int distance = sqrt(static_cast<int>(distanceX * distanceX) + static_cast<double>(distanceY * distanceY) + static_cast<double>(distanceZ * distanceZ));
            return distance;
        }

        static uintptr_t getWeaponAddress(uintptr_t ptr, const char* name) {
            if (!ptr || !name) return 0;
            uintptr_t weaponryController = rpm<uintptr_t>(ptr + 0x68);
            if (!weaponryController) return 0;
            uintptr_t weaponController = rpm<uintptr_t>(weaponryController + 0x98);
            if (!weaponController) return 0;
            uintptr_t gunParameters = rpm<uintptr_t>(weaponController + 0xA0);
            if (!gunParameters) return 0;
            if (name == "controller") return weaponController;
            if (name == "parameters") return gunParameters;
            return 0;
        }

        static monoString getPlayerNickName(uintptr_t ptr) {
            monoString empty = {};
            if (!ptr) return empty;
            auto photon = getPhoton(ptr);
            if (!photon) return empty;
            uintptr_t nickPtr = rpm<uintptr_t>(photon + 0x20);
            if (!nickPtr) return empty;
            return rpm<monoString>(nickPtr);
        }

        static monoString getWeaponName(uintptr_t ptr) {
            monoString defaultName = {};
            defaultName.stringLength = 4;
            memcpy(defaultName.buffer, u"None", 8);
            if (!ptr) return defaultName;
            uintptr_t gunParameters = getWeaponAddress(ptr, "parameters");
            if (!gunParameters) return defaultName;
            uintptr_t stringPtr = rpm<uintptr_t>(gunParameters + 0x18);
            if (!stringPtr) return defaultName;
            monoString weaponName = rpm<monoString>(stringPtr);
            if (weaponName.stringLength <= 0 || weaponName.stringLength > 128) return defaultName;
            return weaponName;
        }

        static void addOutlineText(const ImVec2& textPosition, ImColor color, const char* text) {
            if (!text) return;
            for (float x = -1; x <= 1; x += 1) for (float y = -1; y <= 1; y += 1) if (x != 0 || y != 0) GetBackgroundDrawList()->AddText(ImVec2{textPosition.x + x, textPosition.y + y}, ImColor(0, 0, 0), text);
            GetBackgroundDrawList()->AddText(textPosition, color, text);
        }

        static int getCurrentWeaponId() {
            uintptr_t localPlayer = getLocalPlayer();
            if (!localPlayer) return 0;
            uintptr_t gunParameters = getWeaponAddress(localPlayer, "parameters");
            if (!gunParameters) return 0;
            return rpm<int>(gunParameters + 0x18);
        }

        static bool checkWeaponClass(int currentId, const char* type) {
            if (!type || currentId <= 0) return true;
            vector<int> weaponIds;
            if (type == "knife") {
                weaponIds = {70, 71, 72, 73, 75, 77, 78, 79, 80, 81, 82, 83, 85, 86, 88};
                for (int id : weaponIds) if (currentId == id) return false;
            }
                        if (type == "grenade") {
                weaponIds = {91, 92, 93, 94};
                for (int id : weaponIds) if (currentId == id) return false;
            }
            return true;
        }
    };

    
    monoString getPlayerName(uintptr_t player) {
        if (!player) return {};
        auto photon = Utils::getPhoton(player);
        if (!photon) return {};
        uintptr_t nickPtr = rpm<uintptr_t>(photon + 0x20);
        if (!nickPtr) return {};
        return rpm<monoString>(nickPtr);
}

class Esp {
public:

void draw() {        
        if (enablespi) {
        uintptr_t localPlayer = Utils::getLocalPlayer();
        if (!localPlayer) return;

        uintptr_t playersList = Utils::getPlayersList();
        if (!playersList) return;

        int playersCount = Utils::getPlayersCount();
        if (playersCount <= 0) return;
        
        uint8_t localTeam = Utils::getLocalTeam();
        Matrix view = Utils::getViewMatrix(localPlayer);
        
        float espColor[3];
        Utils::PlayerList players;
        players.update(rpm<uintptr_t>(playersList + 0x18), playersCount);
        
        auto w = resolvePlayerManager();
        auto e = rpm<uint64_t>(w + oxorany(0x70));
        auto r = rpm<uint64_t>(w + oxorany(0x28));
        int t = rpm<int>(r + oxorany(0x20));
        int i = 0;
        auto player = rpm<uint64_t>(rpm<uint64_t>(r + oxorany(0x18)) + oxorany(0x30) + oxorany(0x18) * i);
        auto data = Utils::getPlayerData(player, localPlayer, view);
        extern float viewMatrix[16];

        for (size_t i = 0; i < players.size(); i++) {
        uintptr_t player = players[i].ptr;
        if (!player || player == localPlayer || rpm<uint8_t>(player + 0x79) == localTeam) continue;

            auto data = Utils::getPlayerData(player, localPlayer, view);
            if (!data.isValid) continue;
            
            uintptr_t playerVisibility = rpm<uintptr_t>(player + 0x48);
            if (!playerVisibility) break;
            
            bool isVisible = false;
            uintptr_t occludee = rpm<uintptr_t>(player + 0xB0);
            
        if (occludee) {
            int visState = rpm<int>(occludee + 0x34);
            int occluded = rpm<int>(occludee + 0x38);
            isVisible = (visState == 2 && occluded != 1);
        }

        float espColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        if (esp.isvis) {
            if (isVisible) {
                espColor[0] = esp.espVColor[0];
                espColor[1] = esp.espVColor[1];
                espColor[2] = esp.espVColor[2];
                espColor[3] = esp.espVColor[3];
            } else {
                espColor[0] = esp.espIColor[0];
                espColor[1] = esp.espIColor[1];
                espColor[2] = esp.espIColor[2];
                espColor[3] = esp.espIColor[3];
            }
        } else {
            espColor[0] = esp.espColor[0];
            espColor[1] = esp.espColor[1];
            espColor[2] = esp.espColor[2];
            espColor[3] = esp.espColor[3];
        }

        ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(espColor[0], espColor[1], espColor[2], espColor[3]));
        /*
            if (esp.box3d) {
                float width = 0.5f;
                float height = 1.94f;

                Vector3 corners[8] = {
                    {data.Position.x - width, data.Position.y - 0.14f, data.Position.z - width},
                    {data.Position.x + width, data.Position.y - 0.14f, data.Position.z - width},
                    {data.Position.x + width, data.Position.y - 0.14f, data.Position.z + width},
                    {data.Position.x - width, data.Position.y - 0.14f, data.Position.z + width},

                    {data.Position.x - width, data.Position.y + 1.8f, data.Position.z - width},
                    {data.Position.x + width, data.Position.y + 1.8f, data.Position.z - width},
                    {data.Position.x + width, data.Position.y + 1.8f, data.Position.z + width},
                    {data.Position.x - width, data.Position.y + 1.8f, data.Position.z + width}
                };

                ImVec2 screenCorners[8];
                bool visible[8];
                for (int j = 0; j < 8; j++) {
                    screenCorners[j] = Utils::worldToDisplay(corners[j], view, &visible[j]);
                }

                const int edges[12][2] = {
                    {0, 1}, {1, 2}, {2, 3}, {3, 0},
                    {4, 5}, {5, 6}, {6, 7}, {7, 4},
                    {0, 4}, {1, 5}, {2, 6}, {3, 7}
                };

                for (const auto& edge : edges) {
                    if (visible[edge[0]] && visible[edge[1]]) {
                        GetBackgroundDrawList()->AddLine(
                            screenCorners[edge[0]],
                            screenCorners[edge[1]],
                            ImColor(espColor[0], espColor[1], espColor[2]),
                            1.5f
                        );
                    }
                }
            } 
            */
            
    if (esp.box3d) {
    float width = 0.5f;
    float height = 1.94f;

    Vector3 corners[8] = {
        {data.Position.x - width, data.Position.y - 0.14f, data.Position.z - width},
        {data.Position.x + width, data.Position.y - 0.14f, data.Position.z - width},
        {data.Position.x + width, data.Position.y - 0.14f, data.Position.z + width},
        {data.Position.x - width, data.Position.y - 0.14f, data.Position.z + width},
        {data.Position.x - width, data.Position.y + 1.8f, data.Position.z - width},
        {data.Position.x + width, data.Position.y + 1.8f, data.Position.z - width},
        {data.Position.x + width, data.Position.y + 1.8f, data.Position.z + width},
        {data.Position.x - width, data.Position.y + 1.8f, data.Position.z + width}
    };

    ImVec2 screenCorners[8];
    bool visible[8];
    for (int j = 0; j < 8; j++) {
        screenCorners[j] = Utils::worldToDisplay(corners[j], view, &visible[j]);
    }

    const int faces[6][4] = {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {0, 1, 5, 4},
        {2, 3, 7, 6},
        {1, 2, 6, 5},
        {0, 3, 7, 4}
    };

    if (boxfv > 0) {
        ImColor fillColor = ImColor(espColor[0], espColor[1], espColor[2], boxfv / 100.0f);
        
        for (const auto& face : faces) {
            if (visible[face[0]] && visible[face[1]] && visible[face[2]] && visible[face[3]]) {
                ImVec2 points[4] = {
                    screenCorners[face[0]],
                    screenCorners[face[1]],
                    screenCorners[face[2]],
                    screenCorners[face[3]]
                };
                GetBackgroundDrawList()->AddConvexPolyFilled(points, 4, fillColor);
            }
        }
    }

    float glow = ImClamp(esp.boxGlowStrength, 0.0f, 50.0f);
    ImColor boxColor = ImColor(espColor[0], espColor[1], espColor[2]);

    const int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    if (glow > 0.0f) {
        for (int i = 1; i <= (int)glow; ++i) {
            float t = 1.0f - (float)i / glow;
            float alpha = t * t * 0.9f;

            ImU32 neonCol = ImGui::GetColorU32(
                ImVec4(boxColor.Value.x, boxColor.Value.y, boxColor.Value.z, alpha)
            );

            for (const auto& edge : edges) {
                if (visible[edge[0]] && visible[edge[1]]) {
                    GetBackgroundDrawList()->AddLine(
                        screenCorners[edge[0]],
                        screenCorners[edge[1]],
                        neonCol,
                        2.0f + i * 0.3f
                    );
                }
            }
        }
    }

    for (const auto& edge : edges) {
        if (visible[edge[0]] && visible[edge[1]]) {
            GetBackgroundDrawList()->AddLine(
                screenCorners[edge[0]],
                screenCorners[edge[1]],
                ImColor(0, 0, 0),
                2.5f
            );
        }
    }

    for (const auto& edge : edges) {
        if (visible[edge[0]] && visible[edge[1]]) {
            GetBackgroundDrawList()->AddLine(
                screenCorners[edge[0]],
                screenCorners[edge[1]],
                ImColor(espColor[0], espColor[1], espColor[2]),
                1.5f
            );
        }
    }
}
            

if (esp.box) {
    if (!esp.box3d) {
        ImVec2 topLeft(data.boxHead - data.width, data.headScreenPosition.y);
        ImVec2 bottomRight(data.boxFeet + data.width, data.feetScreenPosition.y);

        ImColor boxColor = ImColor(espColor[0], espColor[1], espColor[2]);
        float glow = ImClamp(esp.boxGlowStrength, 0.0f, 50.0f);

        if (boxfv > 0)
        {
            GetBackgroundDrawList()->AddRectFilled(
                topLeft,
                bottomRight,
                ImColor(espColor[0], espColor[1], espColor[2], boxfv / 100.0f),
                esp.rounding
            );
        }

        if (glow > 0.0f)
        {
            for (int i = 1; i <= (int)glow; ++i)
            {
                float t = 1.0f - (float)i / glow;
                float alpha = t * t * 0.9f;

                ImU32 neonCol = ImGui::GetColorU32(
                    ImVec4(boxColor.Value.x, boxColor.Value.y, boxColor.Value.z, alpha)
                );

                GetBackgroundDrawList()->AddRect(
                    ImVec2(topLeft.x - i, topLeft.y - i),
                    ImVec2(bottomRight.x + i, bottomRight.y + i),
                    neonCol,
                    esp.rounding,
                    0,
                    2.0f
                );

                GetBackgroundDrawList()->AddRect(
                    ImVec2(topLeft.x + i, topLeft.y + i),
                    ImVec2(bottomRight.x - i, bottomRight.y - i),
                    neonCol,
                    esp.rounding,
                    0,
                    2.0f
                );
            }
        }

        float ot = ImClamp(esp.boxOutlineThickness + 2.0f, 2.0f, 8.0f);
        float bt = ImClamp(esp.boxOutlineThickness, 1.0f, 5.0f);

        if (esp.boxStyle == 1)
        {
            float cw = (bottomRight.x - topLeft.x) * ImClamp(esp.cornerLength, 0.05f, 0.45f);
            float ch = (bottomRight.y - topLeft.y) * ImClamp(esp.cornerLength, 0.05f, 0.45f);
            ImU32 outBox = IM_COL32(0, 0, 0, 220);
            auto drawCorner = [&](ImVec2 o, float sx, float sy, ImU32 c, float th)
            {
                GetBackgroundDrawList()->AddLine(o, ImVec2(o.x + sx * cw, o.y), c, th);
                GetBackgroundDrawList()->AddLine(o, ImVec2(o.x, o.y + sy * ch), c, th);
            };
            drawCorner(topLeft,                                1,  1, outBox, ot);
            drawCorner(ImVec2(bottomRight.x, topLeft.y),     -1,  1, outBox, ot);
            drawCorner(ImVec2(topLeft.x, bottomRight.y),      1, -1, outBox, ot);
            drawCorner(bottomRight,                          -1, -1, outBox, ot);
            drawCorner(topLeft,                                1,  1, (ImU32)boxColor, bt);
            drawCorner(ImVec2(bottomRight.x, topLeft.y),     -1,  1, (ImU32)boxColor, bt);
            drawCorner(ImVec2(topLeft.x, bottomRight.y),      1, -1, (ImU32)boxColor, bt);
            drawCorner(bottomRight,                          -1, -1, (ImU32)boxColor, bt);
        }
        else
        {
            GetBackgroundDrawList()->AddRect(topLeft, bottomRight, IM_COL32(0, 0, 0, 220), esp.rounding, 0, ot);
            GetBackgroundDrawList()->AddRect(topLeft, bottomRight, boxColor, esp.rounding, 0, bt);
        }

if (esp.foto) {
    static TextureInfo k_texture;
    static bool textureLoaded = false;
    if (!textureLoaded) {
        k_texture = CreateTextureFromFile("/storage/emulated/0/#wintyx_ext/foto.png");
        textureLoaded = true;
    }
    if (k_texture.textureId != nullptr) {
        GetBackgroundDrawList()->AddImage(k_texture.textureId, topLeft, bottomRight, ImVec2(0,0), ImVec2(1,1), IM_COL32_WHITE);
    }
}
}
}

static std::vector<struct BulletHit> bulletHits;
static uintptr_t lastShotPtr = 0;

struct BulletHit {
    Vector3 dst;
    float time;
};

struct MonoList {
    uintptr_t _1;
    uintptr_t _2;
    uintptr_t items;
    int size;
    int ver;
};

if (esp.bullettrace)
{
    float dt = ImGui::GetIO().DeltaTime;
    if (dt <= 0.0f || dt > 0.1f) dt = 0.016f;

    bulletHits.erase(
        std::remove_if(
            bulletHits.begin(),
            bulletHits.end(),
            [dt](BulletHit& h) {
                h.time -= dt;
                return h.time <= 0.0f;
            }),
        bulletHits.end()
    );

    uintptr_t local = Utils::getLocalPlayer();
    if (local)
    {
        uintptr_t weaponry = rpm<uintptr_t>(local + 0x88);
        if (weaponry)
        {
            uintptr_t gc = rpm<uintptr_t>(weaponry + 0xA0);
            if (gc)
            {
                uintptr_t weapon = rpm<uintptr_t>(gc + 0xA8);
                if (weapon && rpm<int>(weapon + 0x18) <= 70)
                {
                    uintptr_t listAddr = rpm<uintptr_t>(gc + 0x190);
                    if (listAddr)
                    {
                        MonoList list = rpm<MonoList>(listAddr);
                        if (list.items && list.size > 0 && list.size <= 128)
                        {
                            uintptr_t base = list.items + 0x20;
                            for (int i = 0; i < list.size; ++i)
                            {
                                uintptr_t entry = rpm<uintptr_t>(base + i * 8);
                                if (!entry) continue;

                                uintptr_t buffer = rpm<uintptr_t>(entry + 0x18);
                                if (!buffer) continue;

                                int cap = rpm<int>(buffer + 0x18);
                                if (cap <= 0 || cap > 256) continue;

                                uintptr_t bufBase = buffer + 0x20;
                                for (int j = 0; j < cap; ++j)
                                {
                                    uintptr_t shot = rpm<uintptr_t>(bufBase + j * 8);
                                    if (!shot || shot == lastShotPtr) continue;

                                    Vector3 dst = rpm<Vector3>(shot + 0x1C);
                                    if (!std::isfinite(dst.x) || !std::isfinite(dst.y) || !std::isfinite(dst.z)) continue;
                                    if (dst.x == 0.0f && dst.y == 0.0f && dst.z == 0.0f) continue;

                                    bulletHits.push_back({ dst, esp.bullettraceTime });
                                    lastShotPtr = shot;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (bulletHits.size() > 30)
        bulletHits.erase(bulletHits.begin(), bulletHits.end() - 30);
}

if (esp.bullettrace && !bulletHits.empty())
{
    ImDrawList* dl = GetBackgroundDrawList();

    float displayX = ImGui::GetIO().DisplaySize.x;
    float displayY = ImGui::GetIO().DisplaySize.y;
    ImVec2 screenCenter(displayX / 2.f, displayY / 2.f);

    uintptr_t local = Utils::getLocalPlayer();
    if (!local) return;

    Matrix view = Utils::getViewMatrix(local);

    for (const auto& hit : bulletHits)
    {
        if (hit.time <= 0.0f) continue;

        float t = hit.time / esp.bullettraceTime;
        if (t <= 0.0f || t > 1.0f) continue;

        float alpha = t * t;
        if (alpha < 0.08f) alpha = 0.08f;

        float thickness = esp.bullettraceThickness * (0.6f + t * 0.4f);

        bool dVis = false;
        ImVec2 dst2D = Utils::worldToDisplay(hit.dst, view, &dVis);
        if (!dVis) continue;

        float dx = dst2D.x - screenCenter.x;
        float dy = dst2D.y - screenCenter.y;
        float distance = sqrtf(dx * dx + dy * dy);
        if (distance < 5.0f) continue;

        ImVec2 traceStart = esp.bullettraceFromCrosshair ? screenCenter : ImVec2(displayX * 0.5f, displayY * 0.5f);

        int r = (int)(esp.bullettraceColor[0] * 255);
        int g = (int)(esp.bullettraceColor[1] * 255);
        int b = (int)(esp.bullettraceColor[2] * 255);
        int gr = (int)(esp.bullettraceGlowColor[0] * 255);
        int gg = (int)(esp.bullettraceGlowColor[1] * 255);
        int gb = (int)(esp.bullettraceGlowColor[2] * 255);
        ImU32 beamCol = IM_COL32(r, g, b, (int)(alpha * 255));

        int glowSteps = ImClamp((int)esp.bullettraceGlow, 0, 12);

        if (esp.bullettraceStyle == 0)
        {
            for (int i = 1; i <= glowSteps; ++i)
            {
                float gA = alpha * (1.0f - (float)i / (glowSteps + 1)) * 0.35f;
                dl->AddLine(traceStart, dst2D, IM_COL32(gr, gg, gb, (int)(gA * 255)), thickness + i * 0.4f);
            }
            dl->AddLine(traceStart, dst2D, IM_COL32(0, 0, 0, (int)(alpha * 130)), thickness + 0.8f);
            dl->AddLine(traceStart, dst2D, beamCol, thickness);
        }
        else if (esp.bullettraceStyle == 1)
        {
            for (int i = glowSteps; i >= 1; --i)
            {
                float gA = alpha * (1.0f - (float)i / (glowSteps + 1)) * 0.5f;
                dl->AddLine(traceStart, dst2D, IM_COL32(gr, gg, gb, (int)(gA * 255)), thickness + i * 0.7f);
            }
            dl->AddLine(traceStart, dst2D, IM_COL32(0, 0, 0, (int)(alpha * 160)), thickness + 1.2f);
            dl->AddLine(traceStart, dst2D, beamCol, thickness * 1.3f);
            if (esp.bullettraceNeonCore)
            {
                dl->AddLine(traceStart, dst2D, IM_COL32(255, 255, 255, (int)(alpha * 180)), thickness * 0.35f);
            }
        }
        else if (esp.bullettraceStyle == 2)
        {
            int segments = (int)(distance / 14.0f);
            if (segments < 2) segments = 2;
            if (segments > 40) segments = 40;
            for (int i = 0; i <= segments; ++i)
            {
                float s = (float)i / segments;
                float px = traceStart.x + (dst2D.x - traceStart.x) * s;
                float py = traceStart.y + (dst2D.y - traceStart.y) * s;
                float pulseAlpha = alpha * sinf(s * 3.14159f) * (0.5f + 0.5f * t);
                float pulseR = thickness * (0.4f + 0.6f * sinf(s * 3.14159f));
                dl->AddCircleFilled(ImVec2(px, py), pulseR + 1.0f, IM_COL32(0, 0, 0, (int)(pulseAlpha * 120)));
                dl->AddCircleFilled(ImVec2(px, py), pulseR, IM_COL32(r, g, b, (int)(pulseAlpha * 255)));
            }
        }

        float hitSize = esp.bullettraceHitSize * (0.8f + t * 0.4f);

        if (esp.bullettraceHitStyle == 0)
        {
            dl->AddCircleFilled(dst2D, hitSize + 1.2f, IM_COL32(0, 0, 0, (int)(alpha * 140)));
            dl->AddCircleFilled(dst2D, hitSize, beamCol);
            if (hitSize > 2.5f)
                dl->AddCircle(dst2D, hitSize + 2.0f, IM_COL32(255, 255, 255, (int)(alpha * 70)), 0, 0.6f);
        }
        else if (esp.bullettraceHitStyle == 1)
        {
            float cs = hitSize * 1.4f;
            dl->AddLine(ImVec2(dst2D.x - cs, dst2D.y), ImVec2(dst2D.x + cs, dst2D.y), IM_COL32(0, 0, 0, (int)(alpha * 140)), 2.5f);
            dl->AddLine(ImVec2(dst2D.x, dst2D.y - cs), ImVec2(dst2D.x, dst2D.y + cs), IM_COL32(0, 0, 0, (int)(alpha * 140)), 2.5f);
            dl->AddLine(ImVec2(dst2D.x - cs, dst2D.y), ImVec2(dst2D.x + cs, dst2D.y), beamCol, 1.8f);
            dl->AddLine(ImVec2(dst2D.x, dst2D.y - cs), ImVec2(dst2D.x, dst2D.y + cs), beamCol, 1.8f);
        }
        else if (esp.bullettraceHitStyle == 2)
        {
            int pts = 5;
            float outerR = hitSize * 1.5f;
            float innerR = hitSize * 0.6f;
            for (int i = 0; i < pts * 2; ++i)
            {
                float a0 = (float)i / (pts * 2) * 6.28318f - 1.5708f;
                float a1 = (float)(i + 1) / (pts * 2) * 6.28318f - 1.5708f;
                float r0 = (i % 2 == 0) ? outerR : innerR;
                float r1 = (i % 2 == 0) ? innerR : outerR;
                ImVec2 p0(dst2D.x + cosf(a0) * r0, dst2D.y + sinf(a0) * r0);
                ImVec2 p1(dst2D.x + cosf(a1) * r1, dst2D.y + sinf(a1) * r1);
                dl->AddLine(p0, p1, IM_COL32(0, 0, 0, (int)(alpha * 140)), 2.2f);
                dl->AddLine(p0, p1, beamCol, 1.5f);
            }
        }
    }
}

if (esp.skeleton)
{
    static bool localSeen = false;
    static double localSeenTime = 0.0;

    uintptr_t local = Utils::getLocalPlayer();
    if (!local)
    {
        localSeen = false;
        localSeenTime = 0.0;
        continue;
    }

    double nowTime = ImGui::GetTime();
    if (!localSeen)
    {
        localSeen = true;
        localSeenTime = nowTime;
        continue;
    }

    if (nowTime - localSeenTime < 1.0)
        continue;

    struct FrozenSkeletonData {
        std::vector<ImVec2> bones;
        float minY;
        float maxY;
        float minX;
        float maxX;
        float centerX;
    };

    static std::unordered_map<uintptr_t, FrozenSkeletonData> frozenSkeleton;
    static std::unordered_map<uintptr_t, bool> hasFrozen;

    auto drawBoneLine = [&](ImVec2 a, ImVec2 b, ImU32 col, float thickness)
    {
        if (a.x > 1 && a.y > 1 && b.x > 1 && b.y > 1 &&
            a.x < displayX && a.y < displayY &&
            b.x < displayX && b.y < displayY)
            GetBackgroundDrawList()->AddLine(a, b, col, thickness);
    };

    auto AlignSkeletonUniform = [&](const FrozenSkeletonData& src, float dstHeadY, float dstFeetY, float dstCenterX) -> std::vector<ImVec2>
    {
        std::vector<ImVec2> bones = src.bones;

        float srcH = src.maxY - src.minY;
        float boxH = dstFeetY - dstHeadY;
        float dstH = boxH * 0.75f;

        if (srcH < 1.0f || boxH < 1.0f)
            return bones;

        float scale = dstH / srcH;

        float dstTop = dstHeadY + (boxH - dstH) * 0.5f;

        for (auto& b : bones)
        {
            b.x = dstCenterX + (b.x - src.centerX) * scale;
            b.y = dstTop + (b.y - src.minY) * scale;
        }

        return bones;
    };

    ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(
        esp.skeletonColor[0],
        esp.skeletonColor[1],
        esp.skeletonColor[2],
        1.0f));

    ImU32 outlineCol = IM_COL32(0, 0, 0, 255);

    uintptr_t playerVisibility = rpm<uintptr_t>(player + 0x48);
    if (!playerVisibility) { frozenSkeleton.erase(player); hasFrozen.erase(player); continue; }

    uintptr_t bipedMap = rpm<uintptr_t>(playerVisibility + 0x48);
    if (!bipedMap) { frozenSkeleton.erase(player); hasFrozen.erase(player); continue; }

    auto getBonePos = [&](uint64_t offset) -> Vector3
    {
        uint64_t transform = rpm<uint64_t>(bipedMap + offset);
        if (!transform) return Vector3::Zero();
        return Utils::getPosition(transform);
    };

    Vector3 head = getBonePos(oxorany(0x20));
    Vector3 hip  = getBonePos(oxorany(0x88));
    if (head == Vector3::Zero() || hip == Vector3::Zero())
    {
        frozenSkeleton.erase(player);
        hasFrozen.erase(player);
        continue;
    }

    bool dummy;
    std::vector<ImVec2> currentBones =
    {
        Utils::worldToDisplay(head, view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x28)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x40)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x38)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x30)), view, &dummy),
        Utils::worldToDisplay(hip, view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x48)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x50)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x58)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x68)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x70)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x78)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x90)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0x98)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0xA0)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0xB0)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0xB8)), view, &dummy),
        Utils::worldToDisplay(getBonePos(oxorany(0xC0)), view, &dummy)
    };

    float minY = FLT_MAX, maxY = -FLT_MAX;
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float centerX = 0.0f;
    bool allValid = true;

    for (auto& b : currentBones)
    {
        if (b.x <= 1.0f || b.y <= 1.0f || b.x >= displayX || b.y >= displayY)
        {
            allValid = false;
            break;
        }
        minY = std::min(minY, b.y);
        maxY = std::max(maxY, b.y);
        minX = std::min(minX, b.x);
        maxX = std::max(maxX, b.x);
        centerX += b.x;
    }

    if (allValid && !currentBones.empty())
    {
        centerX /= currentBones.size();
        float h = maxY - minY;
        float w = maxX - minX;

        if (h > 15.0f && w > 5.0f && h > w * 1.2f)
        {
            FrozenSkeletonData fd;
            fd.bones = currentBones;
            fd.minY = minY;
            fd.maxY = maxY;
            fd.minX = minX;
            fd.maxX = maxX;
            fd.centerX = centerX;
            frozenSkeleton[player] = fd;
            hasFrozen[player] = true;
        }
    }

    std::vector<ImVec2> bonesToDraw;

    if (isVisible && allValid)
    {
        bonesToDraw = currentBones;
    }
    else if (hasFrozen[player])
    {
        float dstHeadY = data.headScreenPosition.y;
        float dstFeetY = data.feetScreenPosition.y;
        float dstCenterX = data.headScreenPosition.x;

        bonesToDraw = AlignSkeletonUniform(frozenSkeleton[player], dstHeadY, dstFeetY, dstCenterX);
    }
    else
    {
        continue;
    }

    if (bonesToDraw.size() < 18)
        continue;

    auto drawBones = [&](ImU32 c, float t)
    {
        drawBoneLine(bonesToDraw[0], bonesToDraw[1], c, t);
        drawBoneLine(bonesToDraw[1], bonesToDraw[2], c, t);
        drawBoneLine(bonesToDraw[2], bonesToDraw[3], c, t);
        drawBoneLine(bonesToDraw[3], bonesToDraw[4], c, t);
        drawBoneLine(bonesToDraw[4], bonesToDraw[5], c, t);
        drawBoneLine(bonesToDraw[1], bonesToDraw[6], c, t);
        drawBoneLine(bonesToDraw[6], bonesToDraw[7], c, t);
        drawBoneLine(bonesToDraw[7], bonesToDraw[8], c, t);
        drawBoneLine(bonesToDraw[1], bonesToDraw[9], c, t);
        drawBoneLine(bonesToDraw[9], bonesToDraw[10], c, t);
        drawBoneLine(bonesToDraw[10], bonesToDraw[11], c, t);
        drawBoneLine(bonesToDraw[5], bonesToDraw[12], c, t);
        drawBoneLine(bonesToDraw[12], bonesToDraw[13], c, t);
        drawBoneLine(bonesToDraw[13], bonesToDraw[14], c, t);
        drawBoneLine(bonesToDraw[5], bonesToDraw[15], c, t);
        drawBoneLine(bonesToDraw[15], bonesToDraw[16], c, t);
        drawBoneLine(bonesToDraw[16], bonesToDraw[17], c, t);
    };

    if (esp.skeletonGlow > 0.0f)
    {
        int glowPasses = ImClamp((int)esp.skeletonGlow, 1, 8);
        for (int gi = glowPasses; gi >= 1; --gi)
        {
            float gFrac = 1.0f - (float)gi / (glowPasses + 1);
            float gAlpha = gFrac * gFrac * 0.55f;
            ImU32 gCol = ImGui::GetColorU32(ImVec4(
                esp.skeletonColor[0],
                esp.skeletonColor[1],
                esp.skeletonColor[2],
                gAlpha));
            drawBones(gCol, esp.skeletonThickness + gi * 0.8f);
        }
    }

    if (esp.skeletonOutline)
        drawBones(outlineCol, esp.skeletonOutlineThickness);

    drawBones(col, esp.skeletonThickness);

    ImU32 jointCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
        esp.skeletonJointColor[0],
        esp.skeletonJointColor[1],
        esp.skeletonJointColor[2],
        1.0f));

    if (esp.skeletonJointDots)
    {
        const int jointIndices[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        float jr = esp.skeletonJointSize;
        for (int ji : jointIndices)
        {
            ImVec2 jp = bonesToDraw[ji];
            if (jp.x > 1 && jp.y > 1 && jp.x < displayX && jp.y < displayY)
            {
                if (esp.skeletonJointGlow > 0.0f)
                {
                    int jg = ImClamp((int)esp.skeletonJointGlow, 1, 6);
                    for (int gi = jg; gi >= 1; --gi)
                    {
                        float gA = 0.45f * (1.0f - (float)gi / (jg + 1));
                        GetBackgroundDrawList()->AddCircleFilled(jp, jr + gi * 0.8f,
                            ImGui::GetColorU32(ImVec4(esp.skeletonJointColor[0], esp.skeletonJointColor[1], esp.skeletonJointColor[2], gA)));
                    }
                }
                GetBackgroundDrawList()->AddCircleFilled(jp, jr + 0.8f, outlineCol);
                GetBackgroundDrawList()->AddCircleFilled(jp, jr, jointCol);
            }
        }
    }

    ImVec2 headPos = bonesToDraw[0];
    if (headPos.x > 1 && headPos.y > 1 && headPos.x < displayX && headPos.y < displayY)
    {
        float headRadius = ImClamp(esp.headCircleSize, 2.0f, 18.0f);
        if (esp.headCircleStyle == 0)
        {
            if (esp.skeletonOutline)
                GetBackgroundDrawList()->AddCircle(headPos, headRadius + 1.2f, outlineCol, 0, esp.skeletonOutlineThickness);
            GetBackgroundDrawList()->AddCircle(headPos, headRadius, col, 0, 1.6f);
        }
        else if (esp.headCircleStyle == 1)
        {
            if (esp.skeletonOutline)
                GetBackgroundDrawList()->AddCircleFilled(headPos, headRadius + 1.2f, outlineCol);
            GetBackgroundDrawList()->AddCircleFilled(headPos, headRadius, col);
        }
        else if (esp.headCircleStyle == 2)
        {
            if (esp.skeletonOutline)
            {
                GetBackgroundDrawList()->AddCircle(headPos, headRadius + 1.2f, outlineCol, 0, esp.skeletonOutlineThickness);
                GetBackgroundDrawList()->AddCircle(headPos, headRadius * 0.55f + 1.2f, outlineCol, 0, esp.skeletonOutlineThickness);
            }
            GetBackgroundDrawList()->AddCircle(headPos, headRadius, col, 0, 1.6f);
            GetBackgroundDrawList()->AddCircle(headPos, headRadius * 0.55f, col, 0, 1.2f);
        }
    }
}

if (esp.weapon) {
    auto weaponStr = get_weapon_name(player);
    std::string wpn = weaponStr.Get();
    if (!wpn.empty()) {
        SetWindowFontScale(0.7f);
        SetNextWindowPos(ImVec2{10000, 10000});

        ImVec2 textSize = CalcTextSize(wpn.c_str());

        float startX = data.boxHead - data.width;
        float endX   = data.boxFeet + data.width;

        float armorY = data.feetScreenPosition.y + esp.armorOffset;
        float weaponY = armorY + esp.armorThickness + 4.0f;

        ImVec2 textPosition(
            (startX + endX) * 0.5f - textSize.x * 0.5f,
            weaponY
        );

        ImColor weaponColor(
            esp.weaponColor[0],
            esp.weaponColor[1],
            esp.weaponColor[2]
        );

        Utils::addOutlineText(
            textPosition,
            weaponColor,
            wpn.c_str()
        );
    }
}

if (esp.weaponicon) {
    WeaponIcon wi = getWeaponIcon(player);
    if (!wi.text.empty() && wi.font != nullptr) {
        PushFont(wi.font);

        SetWindowFontScale(1.8f);
        SetNextWindowPos(ImVec2{10000, 10000});

        ImVec2 textSize = ImGui::CalcTextSize(wi.text.c_str());

        float leftX  = data.boxHead - data.width;
        float rightX = data.boxFeet + data.width;
        
        float iconY;
        if (esp.weapon) {
            iconY = data.feetScreenPosition.y + esp.armorOffset + esp.armorThickness + 4.0f + 20.0f;
        } else {
            iconY = data.feetScreenPosition.y + esp.armorOffset + esp.armorThickness + 0.0f;
        }

        ImVec2 iconPos(
            leftX + (rightX - leftX) * 0.5f - textSize.x * 0.5f,
            iconY
        );

        ImColor col(
            esp.weaponColor[0],
            esp.weaponColor[1],
            esp.weaponColor[2],
            1.0f
        );

        Utils::addOutlineText(iconPos, col, wi.text.c_str());

        PopFont();
    }
}

    if (esp.offscreenArrow)
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImVec2 screenCenter(screen.x * 0.5f, screen.y * 0.5f);
    
    bool onScreen = data.headScreenPosition.x > 0 && data.headScreenPosition.y > 0 &&
                    data.headScreenPosition.x < screen.x && data.headScreenPosition.y < screen.y;
    
    if (!onScreen)
    {
        float radius = fmin(screen.x, screen.y) * esp.offscreenArrowRadius;
        
        ImU32 arrowColor = ImGui::ColorConvertFloat4ToU32(
            ImVec4(esp.offscreenArrowColor[0], esp.offscreenArrowColor[1], 
                  esp.offscreenArrowColor[2], esp.offscreenArrowColor[3]));
        
        ImU32 outlineColor = IM_COL32(0, 0, 0, 255);

        DrawOffscreenArrow(
            screenCenter,
            data.headScreenPosition,
            radius,
            outlineColor,
            esp.offscreenArrowSize + 2.0f,
            esp.offscreenArrowGlow,
            esp.offscreenArrowGlowStrength * 0.7f
        );
        
        DrawOffscreenArrow(
            screenCenter,
            data.headScreenPosition,
            radius,
            arrowColor,
            esp.offscreenArrowSize,
            esp.offscreenArrowGlow,
            esp.offscreenArrowGlowStrength
        );
    }
}

if (esp.line) {
    ImVec2 displaySize = GetMainViewport()->Size;
    ImVec2 playerCenter((data.boxHead + data.boxFeet) * 0.5f, (data.headScreenPosition.y + data.feetScreenPosition.y) * 0.5f);

    ImVec2 lineStartPosition;
    ImVec2 lineEndPosition;

    if (esp.snaplineStyle == 0)
    {
        lineStartPosition = ImVec2(displaySize.x * 0.5f, displaySize.y);
        lineEndPosition   = ImVec2((data.boxHead + data.boxFeet) * 0.5f, data.feetScreenPosition.y);
    }
    else if (esp.snaplineStyle == 1)
    {
        lineStartPosition = ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f);
        lineEndPosition   = playerCenter;
    }
    else
    {
        lineStartPosition = ImVec2(displaySize.x * 0.5f, 0.0f);
        lineEndPosition   = ImVec2((data.boxHead + data.boxFeet) * 0.5f, data.headScreenPosition.y);
    }

    ImColor baseColor(espColor[0], espColor[1], espColor[2]);
    float glow = ImClamp(esp.lineGlowStrength, 0.0f, 40.0f);

    for (int i = 1; i <= (int)glow; ++i) {
        float t = 1.0f - (float)i / glow;
        float alpha = t * t * 0.6f;
        ImU32 glowCol = ImGui::GetColorU32(ImVec4(baseColor.Value.x, baseColor.Value.y, baseColor.Value.z, alpha));
        GetBackgroundDrawList()->AddLine(lineStartPosition, lineEndPosition, glowCol, esp.espLineThickness + i * 1.2f);
    }

    GetBackgroundDrawList()->AddLine(lineStartPosition, lineEndPosition, IM_COL32(0, 0, 0, 200), esp.espLineThickness + 2.0f);
    GetBackgroundDrawList()->AddLine(lineStartPosition, lineEndPosition, baseColor, esp.espLineThickness);

    if (esp.lineEndStyle == 1)
    {
        GetBackgroundDrawList()->AddCircleFilled(lineEndPosition, esp.espLineThickness + 1.5f, IM_COL32(0, 0, 0, 200));
        GetBackgroundDrawList()->AddCircleFilled(lineEndPosition, esp.espLineThickness * 0.85f, (ImU32)baseColor);
    }
    else if (esp.lineEndStyle == 2)
    {
        float cs = esp.espLineThickness + 2.0f;
        GetBackgroundDrawList()->AddLine(ImVec2(lineEndPosition.x - cs, lineEndPosition.y), ImVec2(lineEndPosition.x + cs, lineEndPosition.y), IM_COL32(0,0,0,200), 2.0f);
        GetBackgroundDrawList()->AddLine(ImVec2(lineEndPosition.x, lineEndPosition.y - cs), ImVec2(lineEndPosition.x, lineEndPosition.y + cs), IM_COL32(0,0,0,200), 2.0f);
        GetBackgroundDrawList()->AddLine(ImVec2(lineEndPosition.x - cs + 1, lineEndPosition.y), ImVec2(lineEndPosition.x + cs - 1, lineEndPosition.y), (ImU32)baseColor, 1.5f);
        GetBackgroundDrawList()->AddLine(ImVec2(lineEndPosition.x, lineEndPosition.y - cs + 1), ImVec2(lineEndPosition.x, lineEndPosition.y + cs - 1), (ImU32)baseColor, 1.5f);
    }
}

if (esp.hpBar)
{
    int playerHealth = Utils::getProp<int>(player, "health");
    if (playerHealth < 1 || playerHealth > 133) continue;

    float barWidth     = esp.hpbarscaless;
    float borderSize   = 1.0f;
    float barHeight    = data.height;
    float healthHeight = barHeight * (playerHealth / 100.0f);

    float barX         = data.boxHead - data.width - barWidth - borderSize - 5.0f;
    float barYTop      = data.headScreenPosition.y;
    float barYBottom   = data.feetScreenPosition.y;

    ImVec2 hpMin(barX - borderSize, barYTop - borderSize);
    ImVec2 hpMax(barX + barWidth + borderSize, barYBottom + borderSize);

    int glowRadius     = (int)ImClamp(esp.hpGlowStrength, 0.0f, 50.0f);
    float glowMaxAlpha = ImClamp(esp.hpGlowStrength / 30.0f, 0.05f, 0.9f);

    if (glowRadius > 0)
    {
        for (int i = 1; i <= glowRadius; ++i)
        {
            float t = 1.0f - (float)i / (float)glowRadius;
            float alpha = glowMaxAlpha * t * t;

            ImVec4 from = esp.hpGradient ? esp.hpColor1 : esp.hpColor;
            ImVec4 to   = esp.hpGradient ? esp.hpColor2 : esp.hpColor;

            ImVec4 glowColor = ImVec4(
                from.x + (to.x - from.x) * (1.0f - t),
                from.y + (to.y - from.y) * (1.0f - t),
                from.z + (to.z - from.z) * (1.0f - t),
                alpha
            );

            ImU32 glowCol = ImGui::ColorConvertFloat4ToU32(glowColor);

            ImVec2 gMin(hpMin.x - i, hpMin.y - i);
            ImVec2 gMax(hpMax.x + i, hpMax.y + i);

            GetBackgroundDrawList()->AddRect(gMin, gMax, glowCol, 0.0f, 0, 2.0f);
        }
    }

    GetBackgroundDrawList()->AddRect(hpMin, hpMax, IM_COL32(0, 0, 0, 220), esp.hpBarRounding, 0, 2.0f);
    GetBackgroundDrawList()->AddRectFilled(ImVec2(barX, barYTop), ImVec2(barX + barWidth, barYBottom), IM_COL32(0, 0, 0, 200), esp.hpBarRounding);

    float hp01 = ImClamp(playerHealth / 100.0f, 0.0f, 1.0f);

    ImU32 hpC1, hpC2;

    if (esp.hpAdaptiveColor)
    {
        ImVec4 green(0.1f, 1.0f, 0.3f, 1.0f);
        ImVec4 yellow(1.0f, 0.85f, 0.0f, 1.0f);
        ImVec4 red(1.0f, 0.08f, 0.05f, 1.0f);
        
        ImVec4 adaptiveColor;
        if (hp01 >= 0.5f)
        {
            float f = (hp01 - 0.5f) * 2.0f;
            adaptiveColor = ImLerp(yellow, green, f);
        }
        else
        {
            float f = hp01 * 2.0f;
            adaptiveColor = ImLerp(red, yellow, f);
        }
        
        hpC1 = ImGui::ColorConvertFloat4ToU32(adaptiveColor);
        hpC2 = hpC1;
    }
    else
    {
        ImVec4 topColor = esp.hpGradient ? esp.hpColor1 : esp.hpColor;
        ImVec4 bottomColor = esp.hpGradient ? esp.hpColor2 : esp.hpColor;
        hpC1 = ImGui::ColorConvertFloat4ToU32(topColor);
        hpC2 = ImGui::ColorConvertFloat4ToU32(bottomColor);
    }

    if (hpC1 == hpC2)
        GetBackgroundDrawList()->AddRectFilled(ImVec2(barX, barYBottom - healthHeight), ImVec2(barX + barWidth, barYBottom), hpC1, esp.hpBarRounding);
    else
        GetBackgroundDrawList()->AddRectFilledMultiColor(
            ImVec2(barX, barYBottom - healthHeight), ImVec2(barX + barWidth, barYBottom),
            hpC1, hpC1, hpC2, hpC2
        );
}

if (esp.armor)
{
    int playerArmor = Utils::getProp<int>(player, "armor");
    if (playerArmor < 0 || playerArmor > 100) continue;

    float armorPercent = playerArmor / 100.0f;

    float barHeightOffset = esp.armorOffset;
    float barThickness    = esp.armorThickness;
    float glowStrength    = esp.armorGlowStrength;

    float startX = data.boxHead - data.width;
    float endX   = data.boxFeet + data.width;
    float yPos   = data.feetScreenPosition.y + barHeightOffset;

    float filledX = startX + (endX - startX) * armorPercent;

    int glow = (int)ImClamp(glowStrength, 0.0f, 40.0f);

    ImU32 finalColor1, finalColor2;
    bool useGradient = false;

    if (esp.armorAdaptiveColor)
    {
        ImVec4 lowColor(0.95f, 0.25f, 0.15f, 1.0f);
        ImVec4 midColor(1.0f, 0.65f, 0.1f, 1.0f);
        ImVec4 highColor(0.2f, 0.75f, 0.95f, 1.0f);
        
        ImVec4 adaptiveColor;
        if (armorPercent >= 0.7f)
        {
            float f = (armorPercent - 0.7f) / 0.3f;
            adaptiveColor = ImLerp(midColor, highColor, f);
        }
        else if (armorPercent >= 0.3f)
        {
            float f = (armorPercent - 0.3f) / 0.4f;
            adaptiveColor = ImLerp(lowColor, midColor, f);
        }
        else
        {
            float f = armorPercent / 0.3f;
            adaptiveColor = ImLerp(ImVec4(0.5f, 0.1f, 0.05f, 1.0f), lowColor, f);
        }
        
        finalColor1 = ImGui::ColorConvertFloat4ToU32(adaptiveColor);
        finalColor2 = finalColor1;
        useGradient = false;
    }
    else
    {
        finalColor1 = ImGui::ColorConvertFloat4ToU32(esp.armorColor1);
        finalColor2 = ImGui::ColorConvertFloat4ToU32(esp.armorColor2);
        useGradient = esp.armorGradient;
    }

    if (glow > 0)
    {
        for (int i = 1; i <= glow; ++i)
        {
            float t = 1.0f - (float)i / (float)glow;
            float alpha = t * t * 0.6f;

            ImVec4 glowColor;
            if (esp.armorAdaptiveColor)
            {
                ImVec4 base = ImGui::ColorConvertU32ToFloat4(finalColor1);
                glowColor = ImVec4(base.x, base.y, base.z, alpha);
            }
            else
            {
                ImVec4 from = useGradient ? esp.armorColor1 : esp.armorColor;
                ImVec4 to   = useGradient ? esp.armorColor2 : esp.armorColor;
                glowColor = ImVec4(
                    from.x + (to.x - from.x) * (1.0f - t),
                    from.y + (to.y - from.y) * (1.0f - t),
                    from.z + (to.z - from.z) * (1.0f - t),
                    alpha
                );
            }

            ImU32 glowCol = ImGui::ColorConvertFloat4ToU32(glowColor);
            float thick   = barThickness + i * 0.9f;

            GetBackgroundDrawList()->AddLine(ImVec2(startX - i, yPos),       ImVec2(filledX + i, yPos),       glowCol, thick);
            GetBackgroundDrawList()->AddLine(ImVec2(startX,     yPos - i),   ImVec2(filledX,     yPos - i),   glowCol, thick);
            GetBackgroundDrawList()->AddLine(ImVec2(startX,     yPos + i),   ImVec2(filledX,     yPos + i),   glowCol, thick);
        }
    }

    GetBackgroundDrawList()->AddLine(ImVec2(startX, yPos), ImVec2(endX, yPos), ImColor(0, 0, 0, 220), barThickness + 2.0f);

    if (!useGradient || finalColor1 == finalColor2)
    {
        GetBackgroundDrawList()->AddLine(ImVec2(startX, yPos), ImVec2(filledX, yPos), finalColor1, barThickness);
    }
    else
    {
        const int steps = 64;
        ImDrawList* dl = GetBackgroundDrawList();
        for (int s = 0; s < steps; ++s)
        {
            float t   = (float)s / steps;
            float x   = startX + (filledX - startX) * t;
            float seg = (endX - startX) / steps;

            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImLerp(
                ImGui::ColorConvertU32ToFloat4(finalColor1),
                ImGui::ColorConvertU32ToFloat4(finalColor2),
                t
            ));

            dl->AddLine(ImVec2(x, yPos), ImVec2(x + seg, yPos), col, barThickness);
        }
    }
}

if (esp.pulseWave)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float time = (float)ImGui::GetTime() * esp.pulseSpeed;

    Vector3 player_feet = data.Position;
    player_feet.y -= 0.14f;

    float maxRadius = 3.5f;
    int waveCount = esp.pulseCount;
    if (waveCount > 3) waveCount = 3;
    if (waveCount < 1) waveCount = 1;

    ImVec2 screen_top;
    bool clip_top = false;
    screen_top = Utils::worldToDisplay(data.Position, view, &clip_top);
    if (!clip_top) goto pulse_static;

    for (int i = 0; i < waveCount; ++i)
    {
        float phase = fmodf(time + i, 3.0f) * (1.0f / 3.0f);
        float world_radius = phase * maxRadius;
        if (world_radius < 0.15f) continue;

        float alpha = 1.0f - phase;
        alpha *= alpha;

        float pulse = sinf(time * 4.0f + i * 1.5f) * 0.2f + 0.8f;

        int baseR = (int)(esp.pulseColor[0] * 255 * pulse);
        int baseG = (int)(esp.pulseColor[1] * 255 * pulse);
        int baseB = (int)(esp.pulseColor[2] * 255 * pulse);

        for (int circle = 0; circle < 2; ++circle)
        {
            float r = world_radius * (0.9f + circle * 0.2f);
            float a = alpha * (1.0f - circle * 0.3f);
            if (a <= 0.02f) continue;

            ImU32 col = IM_COL32(baseR, baseG, baseB, (int)(a * 120));

            ImVec2 prev;
            bool prevVis = false;

            const int segs = 16;
            for (int s = 0; s <= segs; ++s)
            {
                float ang = (float)s * (2.0f * 3.1415926f / segs);
                Vector3 p = player_feet;
                p.x += cosf(ang) * r;
                p.z += sinf(ang) * r;

                bool vis = false;
                ImVec2 cur = Utils::worldToDisplay(p, view, &vis);

                if (vis && prevVis)
                    dl->AddLine(prev, cur, col, 1.0f);

                prev = cur;
                prevVis = vis;
            }
        }

        for (int ray = 0; ray < 6; ++ray)
        {
            float ang = (float)ray * (2.0f * 3.1415926f / 6.0f);

            Vector3 gp = player_feet;
            gp.x += cosf(ang) * world_radius * 0.8f;
            gp.z += sinf(ang) * world_radius * 0.8f;
            gp.y += sinf(time * 3.0f + ang * 2.0f) * 0.15f;

            bool clip_bottom = false;
            ImVec2 screen_bottom = Utils::worldToDisplay(gp, view, &clip_bottom);
            if (!clip_bottom) continue;

            float ray_alpha = alpha * (0.7f + sinf(time * 5.0f + ray) * 0.3f);
            if (ray_alpha <= 0.05f) continue;

            ImU32 ray_col = IM_COL32(
                (int)(esp.pulseColor[0] * 255),
                (int)(esp.pulseColor[1] * 255),
                (int)(esp.pulseColor[2] * 255),
                (int)(ray_alpha * 150)
            );

            dl->AddLine(screen_top, screen_bottom, ray_col, 1.0f);
            dl->AddCircleFilled(
                screen_bottom,
                2.0f + sinf(time * 6.0f) * 0.8f,
                IM_COL32(
                    (int)(esp.pulseColor[0] * 255),
                    (int)(esp.pulseColor[1] * 255),
                    (int)(esp.pulseColor[2] * 255),
                    (int)(alpha * 100)
                )
            );
        }
    }

pulse_static:
    {
        bool clip_static = false;
        ImVec2 static_center = Utils::worldToDisplay(player_feet, view, &clip_static);
        if (clip_static)
        {
            float static_alpha = 0.3f + sinf(time * 2.0f) * 0.1f;
            dl->AddCircle(
                static_center,
                2.0f,
                IM_COL32(
                    (int)(esp.pulseColor[0] * 255),
                    (int)(esp.pulseColor[1] * 255),
                    (int)(esp.pulseColor[2] * 255),
                    (int)(static_alpha * 80)
                ),
                12,
                1.0f
            );
        }
    }
}

if (esp.nickName) {
    auto playerNickName = Utils::getPlayerNickName(player);
    string nickName = playerNickName.asUtf8();
    if (!nickName.empty()) {
        float ns = ImClamp(esp.nameScale, 0.4f, 1.5f);
        SetWindowFontScale(ns);
        SetNextWindowPos(ImVec2{10000, 10000});

        ImVec2 textSize = CalcTextSize(nickName.c_str());
        ImVec2 textPosition(
            (data.boxHead + data.boxFeet) * 0.5f - textSize.x * 0.5f,
            data.headScreenPosition.y - textSize.y - 2.0f
        );

        ImColor nameColor(esp.nickNameColor[0], esp.nickNameColor[1], esp.nickNameColor[2]);

        float glow = ImClamp(esp.nameGlowStrength, 0.0f, 40.0f);

        if (glow > 0.0f) {
            PushFont(esp_fonts);
            for (int i = 1; i <= (int)glow; ++i) {
                float t = 1.0f - (float)i / glow;
                float alpha = t * t * 0.8f;
                ImU32 glowCol = ImGui::GetColorU32(ImVec4(nameColor.Value.x, nameColor.Value.y, nameColor.Value.z, alpha));
                float off = i * 0.6f;
                GetBackgroundDrawList()->AddText(ImVec2(textPosition.x - off, textPosition.y), glowCol, nickName.c_str());
                GetBackgroundDrawList()->AddText(ImVec2(textPosition.x + off, textPosition.y), glowCol, nickName.c_str());
                GetBackgroundDrawList()->AddText(ImVec2(textPosition.x, textPosition.y - off), glowCol, nickName.c_str());
                GetBackgroundDrawList()->AddText(ImVec2(textPosition.x, textPosition.y + off), glowCol, nickName.c_str());
            }
            PopFont();
        }

        if (esp.nameOutline)
        {
            ImU32 outlineCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
                esp.nameOutlineColor[0], esp.nameOutlineColor[1], esp.nameOutlineColor[2], 0.9f));
            GetBackgroundDrawList()->AddText(ImVec2(textPosition.x - 1, textPosition.y + 1), outlineCol, nickName.c_str());
            GetBackgroundDrawList()->AddText(ImVec2(textPosition.x + 1, textPosition.y + 1), outlineCol, nickName.c_str());
            GetBackgroundDrawList()->AddText(ImVec2(textPosition.x - 1, textPosition.y - 1), outlineCol, nickName.c_str());
            GetBackgroundDrawList()->AddText(ImVec2(textPosition.x + 1, textPosition.y - 1), outlineCol, nickName.c_str());
        }

        GetBackgroundDrawList()->AddText(textPosition, (ImU32)nameColor, nickName.c_str());
    }
}


if (esp.hitlogs) {
    static uintptr_t lastLocalPlayer = 0;
    static std::unordered_map<uintptr_t, int> lastHealth;
    static float lastUpdateTime = 0.0f;
    uintptr_t local_player = Utils::getLocalPlayer();

    float currentTime = ImGui::GetTime();
    float deltaTime = ImGui::GetIO().DeltaTime;
    
    if (deltaTime > 0.1f) deltaTime = 0.016f;

    if (local_player != lastLocalPlayer) {
        lastHealth.clear();
        lastLocalPlayer = local_player;
        hits.clear();
    }
    
    if (!hits.empty() && (currentTime - lastUpdateTime > 0.016f || !ImGui::GetIO().WantCaptureMouse)) {
        lastUpdateTime = currentTime;
        
        ImVec2 screen = ImGui::GetIO().DisplaySize;
        float baseY = screen.y * 0.35f;

        ImU32 textColor = IM_COL32((int)(esp.hitlogColor[0] * 255), (int)(esp.hitlogColor[1] * 255), (int)(esp.hitlogColor[2] * 255), 255);
        ImU32 backgroundColor = IM_COL32(20, 20, 30, (int)(esp.hitlogBgAlpha * 255));

        std::vector<size_t> toRemove;
        
        for (size_t i = 0; i < hits.size(); i++) {
            HitInfo& hit = hits[i];
            hit.timeLeft -= deltaTime;
            
            if (hit.offsetY > 0) {
                hit.offsetY -= deltaTime * 300.0f;
                if (hit.offsetY < 0) hit.offsetY = 0;
            }

            if (hit.timeLeft <= 0.0f) {
                toRemove.push_back(i);
                continue;
            }

            float alpha = hit.timeLeft < 1.0f ? hit.timeLeft : 1.0f;

            ImU32 finalTextCol = IM_COL32(
                (int)(esp.hitlogColor[0] * 255),
                (int)(esp.hitlogColor[1] * 255),
                (int)(esp.hitlogColor[2] * 255),
                (int)(alpha * 255)
            );
            
            ImU32 finalBgCol = IM_COL32(20, 20, 30, (int)(alpha * esp.hitlogBgAlpha * 255));
            
            char textBuffer[128];
            sprintf(textBuffer, "%s  -%i", hit.name.c_str(), hit.damage);
            
            ImVec2 textSize = ImGui::CalcTextSize(textBuffer);

            float padding = 14.0f;
            float containerWidth = textSize.x + padding * 2;
            float containerHeight = textSize.y + padding;
            
            ImVec2 pos(screen.x * 0.5f - containerWidth * 0.5f, baseY + hit.offsetY);

            ImDrawList* drawList = GetBackgroundDrawList();
            drawList->AddRectFilled(pos, ImVec2(pos.x + containerWidth, pos.y + containerHeight), finalBgCol, esp.hitlogRounding);
            
            if (esp.hitlogBorderThick > 0) {
                drawList->AddRect(pos, ImVec2(pos.x + containerWidth, pos.y + containerHeight), finalTextCol, esp.hitlogRounding, 0, esp.hitlogBorderThick);
            }

            ImVec2 textPos(pos.x + containerWidth * 0.5f - textSize.x * 0.5f, pos.y + containerHeight * 0.5f - textSize.y * 0.5f);
            
       /*     if (esp.hitlogOutline) {
                ImU32 outlineCol = IM_COL32(0, 0, 0, (int)(alpha * 255));
                drawList->AddText(ImVec2(textPos.x - 1, textPos.y - 1), outlineCol, textBuffer);
                drawList->AddText(ImVec2(textPos.x + 1, textPos.y - 1), outlineCol, textBuffer);
                drawList->AddText(ImVec2(textPos.x - 1, textPos.y + 1), outlineCol, textBuffer);
                drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), outlineCol, textBuffer);
            }*/
            
            drawList->AddText(textPos, finalTextCol, textBuffer);

            baseY += containerHeight + 6.0f;
        }
        
        for (int i = toRemove.size() - 1; i >= 0; i--) {
            hits.erase(hits.begin() + toRemove[i]);
        }
    }
    
    if (currentTime - lastUpdateTime > 0.05f) {
        for (size_t i = 0; i < players.size(); i++) {
            uintptr_t player = players[i].ptr;
            if (!player || player == local_player) continue;
            
            int currentHealth = Utils::getProp<int>(player, "health");
            if (currentHealth <= 0 || currentHealth > 1000) {
                auto it = lastHealth.find(player);
                if (it != lastHealth.end()) {
                    int damage = it->second;
                    if (damage > 0 && damage < 500) {
                        char nameBuf[64];
                        std::string name = Utils::getPlayerNickName(player).asUtf8();
                        if (name.empty()) strcpy(nameBuf, "Enemy");
                        else strcpy(nameBuf, name.c_str());
                        
                        bool exists = false;
                        for (const auto& h : hits) {
                            if (h.damage == damage && strcmp(h.name.c_str(), nameBuf) == 0 && h.timeLeft > 4.5f) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists && hits.size() < 8) {
                            hits.push_back({std::string(nameBuf), damage, 5.0f, 30.0f});
                        }
                    }
                }
                lastHealth.erase(player);
                continue;
            }

            auto it = lastHealth.find(player);
            if (it != lastHealth.end()) {
                int damage = it->second - currentHealth;
                if (damage > 0 && damage < 500) {
                    char nameBuf[64];
                    std::string name = Utils::getPlayerNickName(player).asUtf8();
                    if (name.empty()) strcpy(nameBuf, "Enemy");
                    else strcpy(nameBuf, name.c_str());
                    
                    bool exists = false;
                    for (const auto& h : hits) {
                        if (h.damage == damage && strcmp(h.name.c_str(), nameBuf) == 0 && h.timeLeft > 4.5f) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists && hits.size() < 8) {
                        hits.push_back({std::string(nameBuf), damage, 5.0f, 30.0f});
                    }
                }
            }
            
            lastHealth[player] = currentHealth;
        }
        lastUpdateTime = currentTime;
    }
}

                        if (ioss) {
                bool iios = rpm<uintptr_t>(player + 0x138);
                    SetWindowFontScale(0.7);
                    SetNextWindowPos(ImVec2{10000, 10000});
                    ImVec2 textSize = CalcTextSize("IOS");
                    ImVec2 textPosition((data.boxHead + data.boxFeet) * 0.5f - textSize.x * 0.5f, data.headScreenPosition.y - textSize.y);
                  if (iios)  Utils::addOutlineText(textPosition, ImColor(255, 255, 255), "IOS");
                
            }

           if (esp.money) {
                int playerMoney = Utils::getProp<int>(player, "money");
                string moneyText = std::to_string(playerMoney) + "$";
                SetWindowFontScale(0.27);
                SetNextWindowPos(ImVec2{10000, 10000});
                ImVec2 textPosition(data.boxFeet + data.width + 8, data.headScreenPosition.y);
                Utils::addOutlineText(textPosition, ImColor(esp.moneyColor[0], esp.moneyColor[1], esp.moneyColor[2]), moneyText.c_str());
            }

            if (esp.distance) {
                int distanceToEnemy = Utils::getDistance(Utils::getPlayerLocation(localPlayer), Utils::getPlayerLocation(player));
                string distanceText = std::to_string(distanceToEnemy) + "M";
                SetWindowFontScale(0.27);
                SetNextWindowPos(ImVec2{10000, 10000});
                ImVec2 textPosition(data.boxFeet + data.width + 8, data.headScreenPosition.y + 12);
                Utils::addOutlineText(textPosition, ImColor(esp.distanceColor[0], esp.distanceColor[1], esp.distanceColor[2]), distanceText.c_str());
            }

           if (esp.ping) {
                int playerPing = Utils::getProp<int>(player, "ping");
                if (playerPing > 0) {
                    string pingText = std::to_string(playerPing) + "ms";
                    SetWindowFontScale(0.27);
                    SetNextWindowPos(ImVec2{10000, 10000});
                    ImVec2 textPosition(data.boxFeet + data.width + 8, data.headScreenPosition.y + 24);
                    Utils::addOutlineText(textPosition, ImColor(esp.pingColor[0], esp.pingColor[1], esp.pingColor[2]), pingText.c_str());
                }
            }
            
            
            if (esp.ammo) {
                int playerAmmo = Utils::getProp<int>(player, "ammo");
                if (playerAmmo > 0) {
                    string ammoText = std::to_string(playerAmmo);
                    SetWindowFontScale(0.4);
                    SetNextWindowPos(ImVec2{10000, 10000});
                    ImVec2 textPosition((data.boxHead + data.boxFeet) * 0.5f - CalcTextSize(ammoText.c_str()).x * 0.5f, data.feetScreenPosition.y + 5);
                    Utils::addOutlineText(textPosition, ImColor(255, 215, 0), ammoText.c_str());
                }
            }
       }
       }
    }
};

bool isFiring() {
        auto PlayerControls = rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(proc.base + 133128528) + 0x58) + 0xB8) + 0x0);
        bool shotB = false, shootA = false, ShootS = false;
        if (PlayerControls > 0) {
            auto TouchController = rpm<uintptr_t>(PlayerControls + oxorany(0x38));
            if (TouchController > 0) {
                auto _shootButton = rpm<uintptr_t>(TouchController + oxorany(0x60));
                auto _shootArea = rpm<uintptr_t>(TouchController + oxorany(0x58));
                auto _shootShift = rpm<uintptr_t>(TouchController + oxorany(0x68));
                if (_shootButton > 0) {
                    shotB = rpm<bool>(_shootButton + oxorany(0xA8));
                }
                if (_shootArea > 0) {
                    shootA = rpm<bool>(_shootArea + oxorany(0x70));
                }
                if (_shootShift > 0) {
                    ShootS = rpm<bool>(_shootShift + oxorany(0xA8));
                }
            }
        }
        return (shotB || shootA || ShootS);
    }

        bool fisting() {
        auto PlayerControls = rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(rpm<uintptr_t>(proc.base + 133128528) + 0x58) + 0xB8) + 0x0);
        bool shotB = false, shootA = false, ShootS = false;
        if (PlayerControls > 0) {
            auto TouchController = rpm<uintptr_t>(PlayerControls + oxorany(0x38));
            if (TouchController > 0) {
                auto _shootButton = rpm<uintptr_t>(TouchController + oxorany(0x60));
                auto _shootArea = rpm<uintptr_t>(TouchController + oxorany(0x58));
                auto _shootShift = rpm<uintptr_t>(TouchController + oxorany(0x68));
                if (_shootButton > 0) {
                    wpm<bool>(_shootButton + oxorany(0xA8), true);
                }
                if (_shootArea > 0) {
                    wpm<bool>(_shootArea + oxorany(0x70), true);
                }
                if (_shootShift > 0) {
                    wpm<bool>(_shootShift + oxorany(0xA8), true);
                }
            }
        }
        return (shotB || shootA || ShootS);
    }
    
class Aim {
public:
    void aimBot() {
        if (!aimbot.enable && !silentaim) return;

        uintptr_t localPlayer = Utils::getLocalPlayer();
        if (!localPlayer) return;

        aimbot.maxDistance = aimbot.shortRangeOnly ? 40.0f : 9990.0f;

        float scaledFovDegrees = aimbot.fovRadius * (130.0f / 360.0f);
        scaledFovDegrees = std::max(10.0f, scaledFovDegrees);
        float fovRadians = scaledFovDegrees * (3.14159f / 180.0f);
        float effectiveRadius = tanf(fovRadians / 2) * displayY / 2;

        ImVec2 screenCenter(displayX / 2.f, displayY / 2.f);

        bool hasAnyTargetInFov = false;
        bool hasVisibleTarget   = false;
        bool hasInvisibleTarget = false;

        uintptr_t playersList = Utils::getPlayersList();
        if (playersList) {
            int count = Utils::getPlayersCount();
            if (count > 0 && count <= 64) {
                uintptr_t playerArray = rpm<uintptr_t>(playersList + 0x18);
                if (playerArray) {
                    Utils::PlayerList tempList;
                    tempList.update(playerArray, count);

                    Vector3 localPos = Utils::getPlayerLocation(localPlayer);
                    Matrix view = Utils::getViewMatrix(localPlayer);
                    uint8_t localTeam = Utils::getLocalTeam();

                    for (size_t i = 0; i < tempList.size(); ++i) {
                        auto& p = tempList[i];
                        if (!p.ptr || rpm<uint8_t>(p.ptr + 0x79) == localTeam) continue;

                        Vector3 pos;
                        if (!GetBonePosition(p.ptr, aimbot.aimTarget, pos)) continue;
                        pos.y += aimbot.yOffset;

                        float dist = Vector3::Distance(pos, localPos);
                        if (dist > aimbot.maxDistance) continue;

                        bool onScreen;
                        ImVec2 scr = Utils::worldToDisplay(pos, view, &onScreen);
                        if (!onScreen) continue;

                        float dx = scr.x - screenCenter.x;
                        float dy = scr.y - screenCenter.y;
                        if (sqrtf(dx*dx + dy*dy) > effectiveRadius) continue;

                        hasAnyTargetInFov = true;

                        auto occ = rpm<uintptr_t>(p.ptr + 0xB0);
                        bool vis = occ && rpm<int>(occ + 0x34) == 2 && rpm<int>(occ + 0x38) != 1;

                        if (vis)   hasVisibleTarget   = true;
                        else       hasInvisibleTarget = true;
                    }
                }
            }
        }

    //    if (!drawFovCircle) goto aim_logic;

        ImU32 outlineColor;
        ImU32 fillColor;

        if (hasAnyTargetInFov) {
            if (!aimbot.visibleCheck || hasVisibleTarget) {
                outlineColor = IM_COL32(aimbot.fovVisibleColor[0]*255, aimbot.fovVisibleColor[1]*255, aimbot.fovVisibleColor[2]*255, 220);
                fillColor    = IM_COL32(aimbot.fovVisibleColor[0]*255, aimbot.fovVisibleColor[1]*255, aimbot.fovVisibleColor[2]*255, (int)(aimbot.fovFillOpacity * 255));
            }
            else {
                outlineColor = IM_COL32(aimbot.fovInvisibleColor[0]*255, aimbot.fovInvisibleColor[1]*255, aimbot.fovInvisibleColor[2]*255, 180);
                fillColor    = IM_COL32(aimbot.fovInvisibleColor[0]*255, aimbot.fovInvisibleColor[1]*255, aimbot.fovInvisibleColor[2]*255, (int)(aimbot.fovFillOpacity * 255));
            }
        }
        else {
            if (!aimbot.visibleCheck) {
                outlineColor = IM_COL32(aimbot.fovVisibleColor[0]*255, aimbot.fovVisibleColor[1]*255, aimbot.fovVisibleColor[2]*255, 140);
                fillColor    = IM_COL32(aimbot.fovVisibleColor[0]*255, aimbot.fovVisibleColor[1]*255, aimbot.fovVisibleColor[2]*255, (int)(aimbot.fovFillOpacity * 255));
            }
            else {
                outlineColor = IM_COL32(aimbot.fovInvisibleColor[0]*255, aimbot.fovInvisibleColor[1]*255, aimbot.fovInvisibleColor[2]*255, 140);
                fillColor    = IM_COL32(aimbot.fovInvisibleColor[0]*255, aimbot.fovInvisibleColor[1]*255, aimbot.fovInvisibleColor[2]*255, (int)(aimbot.fovFillOpacity * 255));
            }
        }

        auto bg = GetBackgroundDrawList();

        if (aimbot.fovFillEnabled) {
            bg->AddCircleFilled(screenCenter, effectiveRadius, fillColor, 64);
        }

        bg->AddCircle(screenCenter, effectiveRadius, outlineColor, 64, aimbot.highline ? 2.8f : 1.6f);

    aim_logic:

        if (!playersList) return;
        int playersCount = Utils::getPlayersCount();
        if (playersCount <= 0 || playersCount > 64) return;

        uintptr_t playerArray = rpm<uintptr_t>(playersList + 0x18);
        if (!playerArray) return;

        Vector3 localPosition = Utils::getPlayerLocation(localPlayer);
        if (localPosition == Vector3{0,0,0}) return;

        Matrix view = Utils::getViewMatrix(localPlayer);
        Utils::PlayerList playerList;
        playerList.update(playerArray, playersCount);

        float closestDistance = FLT_MAX;
        Vector3 bestAngles{};
        uintptr_t bestTarget = 0;
        uint8_t localTeam = Utils::getLocalTeam();

        static uintptr_t lastValidTarget = 0;
        static Vector3 lastSmoothedAngles{};

        uintptr_t aimController = rpm<uintptr_t>(localPlayer + 0x80);
        if (!aimController) return;
        uintptr_t aimingData = rpm<uintptr_t>(aimController + 0x90);
        if (!aimingData) return;

        Vector3 currentAngles = rpm<Vector3>(aimingData + 0x18);
        currentAngles = normalizeAngles(currentAngles);

        uintptr_t a1 = rpm<uintptr_t>(localPlayer + 0x88);
        uintptr_t a2 = rpm<uintptr_t>(a1 + 0xA0);
        bool fire = rpm<int>(a2 + 0xF0);
        if (firrrr && !fire) return;

        bool targetStillValid = false;

        if (lastValidTarget) {
            for (const auto& player : playerList.players) {
                if (player.ptr == lastValidTarget) {
                    if (rpm<uint8_t>(player.ptr + 0x79) == localTeam) break;

                    auto objectOccludee = rpm<uintptr_t>(player.ptr + 0xB8);
                    bool visible = objectOccludee > 0 && rpm<int>(objectOccludee + 0x34) == 2 && rpm<int>(objectOccludee + 0x38) != 1;

                    if (aimbot.visibleCheck && !visible) break;

                    Vector3 enemyPosition;
                    if (!GetBonePosition(player.ptr, aimbot.aimTarget, enemyPosition)) break;
                    enemyPosition.y += aimbot.yOffset;

                    float distance = Vector3::Distance(enemyPosition, localPosition);
                    if (distance < 0.1f || distance > aimbot.maxDistance) break;

                    bool onScreen;
                    ImVec2 screenPosition = Utils::worldToDisplay(enemyPosition, view, &onScreen);
                    if (!onScreen) break;

                    float dx = screenPosition.x - screenCenter.x;
                    float dy = screenPosition.y - screenCenter.y;
                    float currentRadius = sqrtf(dx * dx + dy * dy);
                    if (currentRadius > effectiveRadius) break;

                    bestTarget = player.ptr;
                    bestAngles = calculateBestAngles(enemyPosition, localPosition);
                    bestAngles = normalizeAngles(bestAngles);
                    closestDistance = distance;
                    targetStillValid = true;
                    break;
                }
            }
        }

        if (!targetStillValid) {
            for (size_t i = 0; i < playerList.size(); i++) {
                Utils::Player& player = playerList[i];
                if (!player.ptr || rpm<uint8_t>(player.ptr + 0x79) == localTeam) continue;

                auto objectOccludee = rpm<uintptr_t>(player.ptr + 0xB8);
                bool visible = objectOccludee > 0 && rpm<int>(objectOccludee + 0x34) == 2 && rpm<int>(objectOccludee + 0x38) != 1;

                if (aimbot.visibleCheck && !visible) continue;

                Vector3 enemyPosition;
                if (!GetBonePosition(player.ptr, aimbot.aimTarget, enemyPosition)) continue;
                enemyPosition.y += aimbot.yOffset;

                float distance = Vector3::Distance(enemyPosition, localPosition);
                if (distance < 0.1f || distance > aimbot.maxDistance) continue;

                bool onScreen;
                ImVec2 screenPosition = Utils::worldToDisplay(enemyPosition, view, &onScreen);
                if (!onScreen) continue;

                float dx = screenPosition.x - screenCenter.x;
                float dy = screenPosition.y - screenCenter.y;
                float currentRadius = sqrtf(dx * dx + dy * dy);
                if (currentRadius > effectiveRadius) continue;

                Vector3 targetAngles = calculateBestAngles(enemyPosition, localPosition);
                targetAngles = normalizeAngles(targetAngles);

                float deltaYaw = fmod(targetAngles.y - currentAngles.y + 180, 360) - 180;
                float deltaPitch = targetAngles.x - currentAngles.x;
                float angleDiff = sqrtf(deltaYaw * deltaYaw + deltaPitch * deltaPitch);
                float fovAngleThreshold = std::max(aimbot.fovRadius / 2, 45.0f);

                if (angleDiff < fovAngleThreshold && distance < closestDistance) {
                    closestDistance = distance;
                    bestTarget = player.ptr;
                    bestAngles = targetAngles;
                }
            }
        }

        if (bestTarget) {
            lastValidTarget = bestTarget;
        } else {
            lastValidTarget = 0;
            lastSmoothedAngles = currentAngles;
        }

        if (lastValidTarget) {
            for (const auto& player : playerList.players) {
                if (player.ptr == lastValidTarget) {
                    Vector3 enemyPosition;
                    if (!GetBonePosition(player.ptr, aimbot.aimTarget, enemyPosition)) {
                        lastValidTarget = 0; break;
                    }
                    enemyPosition.y += aimbot.yOffset;

                    auto objectOccludee = rpm<uintptr_t>(player.ptr + 0xB8);
                    bool visible = objectOccludee > 0 && rpm<int>(objectOccludee + 0x34) == 2 && rpm<int>(objectOccludee + 0x38) != 1;
                    if (aimbot.visibleCheck && !visible) { lastValidTarget = 0; break; }

                    bool onScreen;
                    ImVec2 screenPosition = Utils::worldToDisplay(enemyPosition, view, &onScreen);
                    if (!onScreen || sqrtf(powf(screenPosition.x - screenCenter.x, 2) + powf(screenPosition.y - screenCenter.y, 2)) > effectiveRadius * 2.0f) {
                        lastValidTarget = 0; break;
                    }

                    bestAngles = calculateBestAngles(enemyPosition, localPosition);
                    bestAngles = normalizeAngles(bestAngles);

                    if (silentaim) {
                        // Continuous silent aim. AimingData has two angle vectors:
                        //   0x18 curAimAngle (also drives the visible camera)
                        //   0x24 curEulerAngles (used for the networked shot)
                        // Writing only 0x24 should land bullets on the target while
                        // the camera stays put. "Silent mode" lets you pick.
                        if (silent_field == 0 || silent_field == 2)
                            wpm<Vector3>(aimingData + 0x18, bestAngles);
                        if (silent_field == 1 || silent_field == 2)
                            wpm<Vector3>(aimingData + 0x24, bestAngles);
                        if (silent_autofire) setFireInput(true);
                    } else {
                        float smoothingFactor = 0.1f;
                        Vector3 smoothed = lerpAngles(lastSmoothedAngles, bestAngles, smoothingFactor);
                        smoothed = normalizeAngles(smoothed);
                        lastSmoothedAngles = smoothed;

                        wpm<Vector3>(aimingData + 0x18, smoothed);
                        wpm<Vector3>(aimingData + 0x24, smoothed);

                        if (tg) {
                            float currentTime = ImGui::GetTime();
                            if (currentTime - aimbot.lastTriggerTime >= aimbot.triggerDelay) {
                                setFireInput(true);
                                aimbot.lastTriggerTime = currentTime;
                            }
                        }
                    }
                    break;
                }
            }
        }

        // ---- HvH diagnostic (throttled, written to /data/local/tmp/hvh.txt) ----
        if (silentaim) {
            static float lastDbg = 0;
            float now = ImGui::GetTime();
            if (now - lastDbg >= 1.0f) {
                lastDbg = now;
                uintptr_t cls = rpm<uintptr_t>(proc.base + 0x911C7A0);
                uintptr_t sf  = cls ? rpm<uintptr_t>(cls + 0x60) : 0;
                FILE* f = fopen("/data/local/tmp/hvh.txt", "w");
                if (f) {
                    fprintf(f, "base=0x%lx\n", (unsigned long)proc.base);
                    fprintf(f, "localPlayer=0x%lx aimingData=0x%lx\n",
                            (unsigned long)localPlayer, (unsigned long)aimingData);
                    fprintf(f, "lastValidTarget=0x%lx  (target found: %s)\n",
                            (unsigned long)lastValidTarget, lastValidTarget ? "YES" : "no");
                    {
                        uintptr_t wc = rpm<uintptr_t>(localPlayer + 0x88);
                        uintptr_t gn = wc ? rpm<uintptr_t>(wc + 0xA0) : 0;
                        uint8_t ss = gn ? rpm<uint8_t>(gn + 0x148) : 0;
                        fprintf(f, "gun=0x%lx shootState(0x148)=%d  (firing if ==3)\n",
                                (unsigned long)gn, (int)ss);
                    }
                    fprintf(f, "cls=0x%lx static_fields=0x%lx\n", (unsigned long)cls, (unsigned long)sf);
                    // try several field offsets to find GameController->PlayerControls->PlayerInputs
                    for (uintptr_t off = 0x0; off <= 0x20 && sf; off += 0x8) {
                        uintptr_t gc = rpm<uintptr_t>(sf + off);
                        uintptr_t pc = gc ? rpm<uintptr_t>(gc + 0x2A0) : 0;
                        uintptr_t pi = pc ? rpm<uintptr_t>(pc + 0xA0) : 0;
                        fprintf(f, "  field+0x%lx: gc=0x%lx pc=0x%lx pi=0x%lx %s\n",
                                (unsigned long)off, (unsigned long)gc, (unsigned long)pc,
                                (unsigned long)pi, pi ? "<-- candidate" : "");
                    }
                    fclose(f);
                }
            }
        }
    }

private:
    // Drive the real fire input the game reads (PlayerInputs.IsToFire), so the
    // shot goes through the game's own pipeline (ammo/recoil/visibility checks).
    void setFireInput(bool v) {
        uintptr_t cls = rpm<uintptr_t>(proc.base + 0x911C7A0);
        if (!cls) return;
        uintptr_t sf = rpm<uintptr_t>(cls + 0x60);     // static_fields
        if (!sf) return;
        uintptr_t gc = rpm<uintptr_t>(sf + 0x8);       // GameController instance
        if (!gc) return;
        uintptr_t pc = rpm<uintptr_t>(gc + 0x2A0);     // PlayerControls
        if (!pc) return;
        uintptr_t pi = rpm<uintptr_t>(pc + 0xA0);      // PlayerInputs
        if (pi) wpm<bool>(pi + 0x21, v);               // IsToFire
        uintptr_t pi2 = rpm<uintptr_t>(pc + 0xA8);
        if (pi2) wpm<bool>(pi2 + 0x21, v);
    }

    bool GetBonePosition(uintptr_t playerPtr, int boneIndex, Vector3 &outPos) {
        uint64_t viewi = rpm<uint64_t>(playerPtr + 0x48);
        if (!viewi) return false;
        uint64_t bipedmap = rpm<uint64_t>(viewi + 0x48);
        if (!bipedmap) return false;

        uintptr_t boneAddress = 0;
        switch (boneIndex) {
            case 0: boneAddress = 0x20; break;
            case 1: boneAddress = 0x30; break;
            case 2: boneAddress = 0x88; break;
            default: boneAddress = 0x20; break;
        }

        uint64_t bonePtr = rpm<uint64_t>(bipedmap + boneAddress);
        if (!bonePtr) return false;

        outPos = getPosition(bonePtr);
        return true;
    }
};
}
