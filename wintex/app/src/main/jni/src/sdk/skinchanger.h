#include <vector>
#include <string>
#define oxorany

std::vector<int> savedSkins;

bool WriteSkinByIndex(int index, int new_id)
{
    auto a1 = rpm<uintptr_t>(
        rpm<uintptr_t>(
            rpm<uintptr_t>(
                rpm<uintptr_t>(proc.base + oxorany(0x91143F8)) + oxorany(0x190)
            ) + oxorany(0x198)
        ) + oxorany(0x0)
    );

    auto a2 = rpm<uintptr_t>(a1 + oxorany(0x100)); //0xF8
    int skins_s = rpm<int>(a2 + oxorany(0x20));

    if (index < 0 || index >= skins_s)
        return false;

    auto skins_list = rpm<uintptr_t>(a2 + oxorany(0x18));
    auto target_skin = rpm<uintptr_t>(
        skins_list + oxorany(0x30) + oxorany(0x18) * index
    );

    if (!target_skin)
        return false;

    wpm<int>(target_skin + oxorany(0x10), new_id);
    return true;
}

void ApplySavedSkins()
{
    for (int i = 0; i < savedSkins.size(); i++)
        WriteSkinByIndex(i, savedSkins[i]);
}

bool ChangeSkin(int new_id)
{
    for (int id : savedSkins)
        if (id == new_id)
            return false;

    savedSkins.push_back(new_id);

    ApplySavedSkins();

    return true;
}