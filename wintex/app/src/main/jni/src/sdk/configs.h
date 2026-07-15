using std::string;
using std::vector;
using namespace json;

static const string CFG_PATH = "/storage/emulated/0/#HulkCheat/";
static const string CFG_EXT  = ".json";

vector<string> ConfigList;
int selectedConfig = 0;
int createType = 0;

extern std::vector<int> savedSkins;

inline bool FileExists(const string& name)
{
    std::ifstream f(CFG_PATH + name + CFG_EXT);
    return f.good();
}

inline string MakeUniqueName(const string& base)
{
    int id = 1;
    while (true)
    {
        string name = base + std::to_string(id);
        if (!FileExists(name))
            return name;
        id++;
    }
}

inline void ScanConfigs()
{
    ConfigList.clear();

    DIR* dir = opendir(CFG_PATH.c_str());
    if (!dir) return;

    dirent* ent;
    while ((ent = readdir(dir)) != nullptr)
    {
        string file = ent->d_name;
        if (file.size() > CFG_EXT.size() &&
            file.substr(file.size() - CFG_EXT.size()) == CFG_EXT)
        {
            ConfigList.push_back(
                file.substr(0, file.size() - CFG_EXT.size())
            );
        }
    }
    closedir(dir);

    if (selectedConfig >= ConfigList.size())
        selectedConfig = 0;
}

inline void CreateConfig()
{
    const char* bases[] = { "Legit_", "Rage_", "Custom_" };
    string name = MakeUniqueName(bases[createType]);

    JSON j;
    j["skins_count"] = 0;

    std::ofstream f(CFG_PATH + name + CFG_EXT);
    f << j.dump();
    f.close();

    ScanConfigs();
}

inline void SaveConfigSelected()
{
    if (ConfigList.empty()) return;
    JSON config;
    
    config["skins_count"] = (int)savedSkins.size();
    for (int i = 0; i < savedSkins.size(); i++)
    config["skin_" + std::to_string(i)] = savedSkins[i];
    config["enablespi"] = enablespi;
    config["esp.box"] = esp.box;
    config["esp.box3d"] = esp.box3d;
    config["esp.line"] = esp.line;
    config["esp.health"] = esp.health;
    config["esp.nickName"] = esp.nickName;
    config["esp.money"] = esp.money;
    config["esp.distance"] = esp.distance;
    config["boxf"] = boxf;
    config["boxfv"] = boxfv;
    config["esp.hpbarscaless"] = esp.hpbarscaless;
    config["esp.espLineThickness"] = esp.espLineThickness;
    config["esp.ping"] = esp.ping;
    config["esp.isvis"] = esp.isvis;
    config["esp.espVColor.r"] = esp.espVColor[0];
    config["esp.espVColor.g"] = esp.espVColor[1];
    config["esp.espVColor.b"] = esp.espVColor[2];
    config["esp.espIColor.r"] = esp.espIColor[0];
    config["esp.espIColor.g"] = esp.espIColor[1];
    config["esp.espIColor.b"] = esp.espIColor[2];
    
    config["esp.offscreenArrow"] = esp.offscreenArrow;
    config["esp.offscreenArrowColor.r"] = esp.offscreenArrowColor[0];
    config["esp.offscreenArrowColor.g"] = esp.offscreenArrowColor[1];
    config["esp.offscreenArrowColor.b"] = esp.offscreenArrowColor[2];
    
    config["esp.pulseWave"] = esp.pulseWave;
    config["esp.pulseSpeed"] = esp.pulseSpeed;
    config["esp.pulseColor.r"] = esp.pulseColor[0];
    config["esp.pulseColor.g"] = esp.pulseColor[1];
    config["esp.pulseColor.b"] = esp.pulseColor[2];
    
    config["esp.hitlogs"] = esp.hitlogs;
    config["esp.hitlogColor.r"] = esp.hitlogColor[0];
    config["esp.hitlogColor.g"] = esp.hitlogColor[1];
    config["esp.hitlogColor.b"] = esp.hitlogColor[2];
    config["esp.weapon"] = esp.weapon;
    config["esp.weaponicon"] = esp.weaponicon;
    config["esp.weaponColor.r"] = esp.weaponColor[0];
    config["esp.weaponColor.g"] = esp.weaponColor[1];
    config["esp.weaponColor.b"] = esp.weaponColor[2];
    config["esp.skeleton"] = esp.skeleton;
    config["esp.skeletonColor.r"] = esp.skeletonColor[0];
    config["esp.skeletonColor.g"] = esp.skeletonColor[1];
    config["esp.skeletonColor.b"] = esp.skeletonColor[2];
    config["esp.bullettrace"] = esp.bullettrace;
    config["esp.bullettraceColor.r"] = esp.bullettraceColor[0];
    config["esp.bullettraceColor.g"] = esp.bullettraceColor[1];
    config["esp.bullettraceColor.b"] = esp.bullettraceColor[2];
    
    
    config["head_scale"] = head_scale;
    config["headScaleValue"] = headScaleValue;
    
    config["arms_position"] = arms_position;
    config["arms.x"] = ARMSX;
    config["arms.y"] = ARMSY;
    config["arms.z"] = ARMSZ;

    config["aimbot.enable"] = aimbot.enable;
    config["aimTarget"] = aimTarget;
    config["aimbot.smoothing"]  = aimbot.smoothing;
    config["aimbot.highline"]  = aimbot.highline;
    config["aimbot.fovRadius"]  = aimbot.fovRadius;
    
    config["triggerDelay"] = triggerDelay;
    config["esp.rounding"] = esp.rounding;
    
    config["fovVisibleColor.r"] = fovVisibleColor[0];
    config["fovVisibleColor.g"] = fovVisibleColor[1];
    config["fovVisibleColor.b"] = fovVisibleColor[2];
    config["fovInvisibleColor.r"] = fovInvisibleColor[0];
    config["fovInvisibleColor.g"] = fovInvisibleColor[1];
    config["fovInvisibleColor.b"] = fovInvisibleColor[2];
    
    config["esp.nickNameColor.r"] = esp.nickNameColor[0];
    config["esp.nickNameColor.g"] = esp.nickNameColor[1];
    config["esp.nickNameColor.b"] = esp.nickNameColor[2];
    
    config["esp.hpBar"] = esp.hpBar;
    config["esp.hpGradient"] = esp.hpGradient;
    config["esp.hpbarscaless"] = esp.hpbarscaless;
    config["esp.hpGlowStrength"] = esp.hpGlowStrength;

    config["esp.hpColor.r"] = esp.hpColor.x;
    config["esp.hpColor.g"] = esp.hpColor.y;
    config["esp.hpColor.b"] = esp.hpColor.z;

    config["esp.hpColor1.r"] = esp.hpColor1.x;
    config["esp.hpColor1.g"] = esp.hpColor1.y;
    config["esp.hpColor1.b"] = esp.hpColor1.z;

    config["esp.hpColor2.r"] = esp.hpColor2.x;
    config["esp.hpColor2.g"] = esp.hpColor2.y;
    config["esp.hpColor2.b"] = esp.hpColor2.z;
    
    config["esp.armor"] = esp.armor;
    config["esp.armorGradient"] = esp.armorGradient;
    config["esp.armorThickness"] = esp.armorThickness;
    config["esp.armorGlowStrength"] = esp.armorGlowStrength;
    config["esp.armorOffset"] = esp.armorOffset;

    config["esp.armorColor.r"] = esp.armorColor.x;
    config["esp.armorColor.g"] = esp.armorColor.y;
    config["esp.armorColor.b"] = esp.armorColor.z;

    config["esp.armorColor1.r"] = esp.armorColor1.x;
    config["esp.armorColor1.g"] = esp.armorColor1.y;
    config["esp.armorColor1.b"] = esp.armorColor1.z;

    config["esp.armorColor2.r"] = esp.armorColor2.x;
    config["esp.armorColor2.g"] = esp.armorColor2.y;
    config["esp.armorColor2.b"] = esp.armorColor2.z;

    config["esp.boxGlowStrength"] = esp.boxGlowStrength;
    config["esp.lineGlowStrength"] = esp.lineGlowStrength;
    config["esp.nameGlowStrength"] = esp.nameGlowStrength;
    config["esp.rounding"] = esp.rounding;

    config["esp.moneyColor.r"] = esp.moneyColor[0];
    config["esp.moneyColor.g"] = esp.moneyColor[1];
    config["esp.moneyColor.b"] = esp.moneyColor[2];


    config["esp.distanceColor.r"] = esp.distanceColor[0];
    config["esp.distanceColor.g"] = esp.distanceColor[1];
    config["esp.distanceColor.b"] = esp.distanceColor[2];

    config["esp.pingColor.r"] = esp.pingColor[0];
    config["esp.pingColor.g"] = esp.pingColor[1];
    config["esp.pingColor.b"] = esp.pingColor[2];

    config["esp.nickNameColor.r"] = esp.nickNameColor[0];
    config["esp.nickNameColor.g"] = esp.nickNameColor[1];
    config["esp.nickNameColor.b"] = esp.nickNameColor[2];
    
    config["esp.boxFilled"] = esp.boxFilled;
    config["esp.boxFilledOpacity"] = esp.boxFilledOpacity;

    config["esp.healthBarAdaptive"] = esp.healthBarAdaptive;
    config["esp.healthBarGradient"] = esp.healthBarGradient;
    config["esp.armorBarAdaptive"] = esp.armorBarAdaptive;
    config["esp.armorBarGradient"] = esp.armorBarGradient;

    config["esp.healthBarColor.r"] = esp.healthBarColor[0];
    config["esp.healthBarColor.g"] = esp.healthBarColor[1];
    config["esp.healthBarColor.b"] = esp.healthBarColor[2];

    config["esp.healthBarGradientTopColor.r"] = esp.healthBarGradientTopColor[0];
    config["esp.healthBarGradientTopColor.g"] = esp.healthBarGradientTopColor[1];
    config["esp.healthBarGradientTopColor.b"] = esp.healthBarGradientTopColor[2];

    config["esp.healthBarGradientBottomColor.r"] = esp.healthBarGradientBottomColor[0];
    config["esp.healthBarGradientBottomColor.g"] = esp.healthBarGradientBottomColor[1];
    config["esp.healthBarGradientBottomColor.b"] = esp.healthBarGradientBottomColor[2];

    config["esp.armorBarColor.r"] = esp.armorBarColor[0];
    config["esp.armorBarColor.g"] = esp.armorBarColor[1];
    config["esp.armorBarColor.b"] = esp.armorBarColor[2];

    config["esp.armorBarGradientLeftColor.r"]  = esp.armorBarGradientLeftColor[0];
    config["esp.armorBarGradientLeftColor.g"]  = esp.armorBarGradientLeftColor[1];
    config["esp.armorBarGradientLeftColor.b"]  = esp.armorBarGradientLeftColor[2];

    config["esp.armorBarGradientRightColor.r"] = esp.armorBarGradientRightColor[0];
    config["esp.armorBarGradientRightColor.g"] = esp.armorBarGradientRightColor[1];
    config["esp.armorBarGradientRightColor.b"] = esp.armorBarGradientRightColor[2];
       
    config["sky"] = sky;
    config["sky_color.r"] = sky_color[0];
    config["sky_color.g"] = sky_color[1];
    config["sky_color.b"] = sky_color[2];
    config["camerafov"] = camerafov;
    config["fov_value"] = fov_value;
    config["aspect"] = aspect;
    config["aspect_value"] = aspect_value;

    config["fastwin"] = fastwin;
    config["killall"] = killall;
    config["cencmatchmm"] = cencmatchmm;
    config["set_score"] = set_score;
    config["score_value"] = score_value;
    config["setdeath"] = setdeath;
    config["deathval"] = deathval;
    config["setassists"] = setassists;
    config["assistsval"] = assistsval;
    config["setkills"] = setkills;
    config["killsval"] = killsval;
    config["set_mvp"] = set_mvp;
    config["set_ping"] = set_ping;
    config["pingval"] = pingval;
    config["set_money"] = set_money;
    config["moneyhacked"] = moneyhacked;
    
    config["fly"] = fly;
    config["crouch"] = crouch;
    config["bunnyhope"] = bunnyhope;
    config["hope_value"] = hope_value;
    config["airjump"] = airjump;

    config["sethl9998"] = sethl9998;

    config["instantThrow"] = instantThrow;
    config["infiniteGrenades"] = infiniteGrenades;
    config["noFuse"] = noFuse;
    config["maxRadius"] = maxRadius;
    config["noSelfDamage"] = noSelfDamage;
    config["maxPenetration"] = maxPenetration;
    config["fastgr"] = fastgr;
    config["antigr"] = antigr;
    
    config["fakel"] = fakel;
    config["isLagging"] = isLagging;
    config["fastb"] = fastb;
    config["bombd"] = bombd;
    config["mbf"] = mbf;
    config["accentColor.r"] = accentColor[0];
    config["accentColor.g"] = accentColor[1];
    config["accentColor.b"] = accentColor[2];
    config["background"] = background;
    config["backgroundOpacity"] = backgroundOpacity;
    config["snowflake"] = snowflake;
    config["flakespeed"] = flakespeed;    
    config["dmgh"] = dmgh;
    config["aafk"] = aafk;
    config["invismode"] = invismode;
    config["tanki"] = tanki;
    config["tankit"] = tankit;
    config["vssc"] = vssc;
    config["firrrr"] = firrrr;
    config["tg"] = tg;
    config["bexplos"] = bexplos;
    config["bplant"] = bplant;
    config["bimpulse"] = bimpulse;
    config["bradius"] = bradius;
    config["bdamage"] = bdamage;
    config["gdetonate"] = gdetonate;
    config["gnodamage"] = gnodamage;
    config["gUnlimitedGrenades"] = gUnlimitedGrenades; 
    config["speedhack"] = speedhack;
    config["tpeses"] = tpeses;
    
    config["autowin"]     = autowin;
    config["high_jump"]   = high_jump;
    config["high_value"]  = high_value;
    config["antiaim"]     = antiaim;
    
    config["spoofers.hideid"] = spoofers.hideid;
    config["spoofers.hideclantag"] = spoofers.hideclantag;
    
    config["weapon.reversammo"] = weapon.reversammo;
    config["weapon.infinityammo"] = weapon.infinityammo;
    config["weapon.wallshoot"] = weapon.wallshoot;
    config["weapon.norecoil"] = weapon.norecoil;
    config["weapon.firerate"] = weapon.firerate;
    config["weapon.onehitkill"] = weapon.onehitkill;
    config["weapon.fastknife"] = weapon.fastknife;
    
    config["set_avatar"] = set_avatar;
    config["set_medalsc"] = set_medalsc;
    config["force_team_ct"] = force_team_ct;
    config["force_team_tr"] = force_team_tr;
    config["playerscaleval"] = playerscaleval;
    config["player_scale"] = player_scale;
    config["set_statrack"] = set_statrack;
    config["srarrackval"] = srarrackval;
    config["set_pattern"] = set_pattern;
    config["patternval"] = patternval;
    
    config["moneyhack_a"] = moneyhack_a;
    config["infinity_buy_a"] = infinity_buy_a;
    config["buy_anywhere_a"] = buy_anywhere_a;
    config["clumsy"] = clumsy;
    config["islag"] = islag;
    config["strafe"] = strafe;
    config["strafe_speed"] = strafe_speed;
    
    std::ofstream f(CFG_PATH + ConfigList[selectedConfig] + CFG_EXT);
    f << config.dump();
    f.close();
}

inline void LoadConfigSelected()
{
    if (ConfigList.empty()) return;

    std::ifstream f(CFG_PATH + ConfigList[selectedConfig] + CFG_EXT);
    if (!f.is_open()) return;

    string text;
    f.seekg(0, std::ios::end);
    text.resize(f.tellg());
    f.seekg(0, std::ios::beg);
    f.read(&text[0], text.size());
    f.close();

    JSON config = JSON::Load(text);
    
    savedSkins.clear();
    int count = config["skins_count"].ToInt();
    for (int i = 0; i < count; i++)
        savedSkins.push_back(
            config["skin_" + std::to_string(i)].ToInt()
        );

    ApplySavedSkins();

    enablespi = config["enablespi"].ToBool();
    esp.box = config["esp.box"].ToBool();
    esp.box3d = config["esp.box3d"].ToBool();
    esp.line = config["esp.line"].ToBool();
    esp.health = config["esp.health"].ToBool();
    esp.nickName = config["esp.nickName"].ToBool();
    esp.money = config["esp.money"].ToBool();
    esp.distance = config["esp.distance"].ToBool();
    boxf = config["boxf"].ToBool();
    boxfv = config["boxfv"].ToFloat();
    esp.hpbarscaless = config["esp.hpbarscaless"].ToFloat();
    esp.espLineThickness = config["esp.espLineThickness"].ToFloat();
    esp.ping = config["esp.ping"].ToBool();
    esp.isvis = config["esp.isvis"].ToBool();
    esp.espVColor[0] = config["esp.espVColor.r"].ToFloat();
    esp.espVColor[1] = config["esp.espVColor.g"].ToFloat();
    esp.espVColor[2] = config["esp.espVColor.b"].ToFloat();
    esp.espIColor[0] = config["esp.espIColor.r"].ToFloat();
    esp.espIColor[1] = config["esp.espIColor.g"].ToFloat();
    esp.espIColor[2] = config["esp.espIColor.b"].ToFloat();
    
    esp.nickNameColor[0] = config["esp.nickNameColor.r"].ToFloat();
    esp.nickNameColor[1] = config["esp.nickNameColor.g"].ToFloat();
    esp.nickNameColor[2] = config["esp.nickNameColor.b"].ToFloat();
    
    esp.hpBar = config["esp.hpBar"].ToBool();
    esp.hpGradient = config["esp.hpGradient"].ToBool();
    esp.hpbarscaless = config["esp.hpbarscaless"].ToFloat();
    esp.hpGlowStrength = config["esp.hpGlowStrength"].ToFloat();

    esp.hpColor.x = config["esp.hpColor.r"].ToFloat();
    esp.hpColor.y = config["esp.hpColor.g"].ToFloat();
    esp.hpColor.z = config["esp.hpColor.b"].ToFloat();

    esp.hpColor1.x = config["esp.hpColor1.r"].ToFloat();
    esp.hpColor1.y = config["esp.hpColor1.g"].ToFloat();
    esp.hpColor1.z = config["esp.hpColor1.b"].ToFloat();

    esp.hpColor2.x = config["esp.hpColor2.r"].ToFloat();
    esp.hpColor2.y = config["esp.hpColor2.g"].ToFloat();
    esp.hpColor2.z = config["esp.hpColor2.b"].ToFloat();
    
    esp.armor = config["esp.armor"].ToBool();
    esp.armorGradient = config["esp.armorGradient"].ToBool();
    esp.armorThickness = config["esp.armorThickness"].ToFloat();
    esp.armorGlowStrength = config["esp.armorGlowStrength"].ToFloat();
    esp.armorOffset = config["esp.armorOffset"].ToFloat();

    esp.armorColor.x = config["esp.armorColor.r"].ToFloat();
    esp.armorColor.y = config["esp.armorColor.g"].ToFloat();
    esp.armorColor.z = config["esp.armorColor.b"].ToFloat();

    esp.armorColor1.x = config["esp.armorColor1.r"].ToFloat();
    esp.armorColor1.y = config["esp.armorColor1.g"].ToFloat();
    esp.armorColor1.z = config["esp.armorColor1.b"].ToFloat();

    esp.armorColor2.x = config["esp.armorColor2.r"].ToFloat();
    esp.armorColor2.y = config["esp.armorColor2.g"].ToFloat();
    esp.armorColor2.z = config["esp.armorColor2.b"].ToFloat();

    esp.boxGlowStrength = config["esp.boxGlowStrength"].ToFloat();
    esp.lineGlowStrength = config["esp.lineGlowStrength"].ToFloat();
    esp.nameGlowStrength = config["esp.nameGlowStrength"].ToFloat();
    esp.rounding = config["esp.rounding"].ToFloat();

    esp.moneyColor[0] = config["esp.moneyColor.r"].ToFloat();
    esp.moneyColor[1] = config["esp.moneyColor.g"].ToFloat();
    esp.moneyColor[2] = config["esp.moneyColor.b"].ToFloat();

    esp.distanceColor[0] = config["esp.distanceColor.r"].ToFloat();
    esp.distanceColor[1] = config["esp.distanceColor.g"].ToFloat();
    esp.distanceColor[2] = config["esp.distanceColor.b"].ToFloat();

    esp.pingColor[0] = config["esp.pingColor.r"].ToFloat();
    esp.pingColor[1] = config["esp.pingColor.g"].ToFloat();
    esp.pingColor[2] = config["esp.pingColor.b"].ToFloat();

    esp.nickNameColor[0] = config["esp.nickNameColor.r"].ToFloat();
    esp.nickNameColor[1] = config["esp.nickNameColor.g"].ToFloat();
    esp.nickNameColor[2] = config["esp.nickNameColor.b"].ToFloat();

    esp.boxFilled = config["esp.boxFilled"].ToBool();
    esp.boxFilledOpacity = config["esp.boxFilledOpacity"].ToFloat();

    esp.healthBarAdaptive = config["esp.healthBarAdaptive"].ToBool();
    esp.healthBarGradient = config["esp.healthBarGradient"].ToBool();
    esp.armorBarAdaptive = config["esp.armorBarAdaptive"].ToBool();
    esp.armorBarGradient = config["esp.armorBarGradient"].ToBool();

    esp.healthBarColor[0] = config["esp.healthBarColor.r"].ToFloat();
    esp.healthBarColor[1] = config["esp.healthBarColor.g"].ToFloat();
    esp.healthBarColor[2] = config["esp.healthBarColor.b"].ToFloat();

    esp.healthBarGradientTopColor[0] = config["esp.healthBarGradientTopColor.r"].ToFloat();
    esp.healthBarGradientTopColor[1] = config["esp.healthBarGradientTopColor.g"].ToFloat();
    esp.healthBarGradientTopColor[2] = config["esp.healthBarGradientTopColor.b"].ToFloat();

    esp.healthBarGradientBottomColor[0] = config["esp.healthBarGradientBottomColor.r"].ToFloat();
    esp.healthBarGradientBottomColor[1] = config["esp.healthBarGradientBottomColor.g"].ToFloat();
    esp.healthBarGradientBottomColor[2] = config["esp.healthBarGradientBottomColor.b"].ToFloat();

    esp.armorBarColor[0] = config["esp.armorBarColor.r"].ToFloat();
    esp.armorBarColor[1] = config["esp.armorBarColor.g"].ToFloat();
    esp.armorBarColor[2] = config["esp.armorBarColor.b"].ToFloat();

    esp.armorBarGradientLeftColor[0]  = config["esp.armorBarGradientLeftColor.r"].ToFloat();
    esp.armorBarGradientLeftColor[1]  = config["esp.armorBarGradientLeftColor.g"].ToFloat();
    esp.armorBarGradientLeftColor[2]  = config["esp.armorBarGradientLeftColor.b"].ToFloat();

    esp.armorBarGradientRightColor[0] = config["esp.armorBarGradientRightColor.r"].ToFloat();
    esp.armorBarGradientRightColor[1] = config["esp.armorBarGradientRightColor.g"].ToFloat();
    esp.armorBarGradientRightColor[2] = config["esp.armorBarGradientRightColor.b"].ToFloat();
    
    triggerDelay = config["triggerDelay"].ToFloat();
    esp.rounding = config["esp.rounding"].ToFloat();
    
    esp.offscreenArrow = config["esp.offscreenArrow"].ToBool();
    esp.offscreenArrowColor[0] = config["esp.offscreenArrowColor.r"].ToFloat();
    esp.offscreenArrowColor[1] = config["esp.offscreenArrowColor.g"].ToFloat();
    esp.offscreenArrowColor[2] = config["esp.offscreenArrowColor.b"].ToFloat();
    
    esp.pulseWave = config["esp.pulseWave"].ToBool();
    esp.pulseSpeed = config["esp.pulseSpeed"].ToFloat();
    esp.pulseColor[0] = config["esp.pulseColor.r"].ToFloat();
    esp.pulseColor[1] = config["esp.pulseColor.g"].ToFloat();
    esp.pulseColor[2] = config["esp.pulseColor.b"].ToFloat();
    
    esp.hitlogs = config["esp.hitlogs"].ToBool();
    esp.hitlogColor[0] = config["esp.hitlogColor.r"].ToFloat();
    esp.hitlogColor[1] = config["esp.hitlogColor.g"].ToFloat();
    esp.hitlogColor[2] = config["esp.hitlogColor.b"].ToFloat();
    esp.weapon = config["esp.weapon"].ToBool();
    esp.weaponicon = config["esp.weaponicon"].ToBool();
    esp.weaponColor[0] = config["esp.weaponColor.r"].ToFloat();
    esp.weaponColor[1] = config["esp.weaponColor.g"].ToFloat();
    esp.weaponColor[2] = config["esp.weaponColor.b"].ToFloat();
    esp.skeleton = config["esp.skeleton"].ToBool();
    esp.skeletonColor[0] = config["esp.skeletonColor.r"].ToFloat();
    esp.skeletonColor[1] = config["esp.skeletonColor.g"].ToFloat();
    esp.skeletonColor[2] = config["esp.skeletonColor.b"].ToFloat();
    esp.bullettrace = config["esp.bullettrace"].ToBool();
    esp.bullettraceColor[0] = config["esp.bullettraceColor.r"].ToFloat();
    esp.bullettraceColor[1] = config["esp.bullettraceColor.g"].ToFloat();
    esp.bullettraceColor[2] = config["esp.bullettraceColor.b"].ToFloat();
    
    head_scale = config["head_scale"].ToBool();
    headScaleValue = config["headScaleValue"].ToFloat();
    
    arms_position = config["arms_position"].ToBool();
    ARMSX = config["arms.x"].ToFloat();
    ARMSY = config["arms.y"].ToFloat();
    ARMSZ = config["arms.z"].ToFloat();

    aimbot.enable = config["aimbot.enable"].ToBool();
    aimTarget = config["aimTarget"].ToInt();
    aimbot.smoothing = config["aimbot.smoothing"].ToFloat();
    aimbot.highline = config["aimbot.highline"].ToFloat();
    fovVisibleColor[0]   = config["fovVisibleColor.r"].ToFloat();
    fovVisibleColor[1]   = config["fovVisibleColor.g"].ToFloat();
    fovVisibleColor[2]   = config["fovVisibleColor.b"].ToFloat();
    fovInvisibleColor[0] = config["fovInvisibleColor.r"].ToFloat();
    fovInvisibleColor[1] = config["fovInvisibleColor.g"].ToFloat();
    fovInvisibleColor[2] = config["fovInvisibleColor.b"].ToFloat();
    aimbot.fovRadius = config["aimbot.fovRadius"].ToFloat();
    
    sky = config["sky"].ToBool();
    sky_color[0] = config["sky_color.r"].ToFloat();
    sky_color[1] = config["sky_color.g"].ToFloat();
    sky_color[2] = config["sky_color.b"].ToFloat();
    camerafov = config["camerafov"].ToBool();
    fov_value = config["fov_value"].ToFloat();
    aspect = config["aspect"].ToBool();
    aspect_value = config["aspect_value"].ToFloat();
    
    fastwin = config["fastwin"].ToBool();
    killall = config["killall"].ToBool();
    cencmatchmm = config["cencmatchmm"].ToBool();
    set_score = config["set_score"].ToBool();
    score_value = config["score_value"].ToInt();
    setdeath = config["setdeath"].ToBool();
    deathval = config["deathval"].ToInt();
    setassists = config["setassists"].ToBool();
    assistsval = config["assistsval"].ToInt();
    setkills = config["setkills"].ToBool();
    killsval = config["killsval"].ToInt();
    set_mvp = config["set_mvp"].ToBool();
    set_ping = config["set_ping"].ToBool();
    pingval = config["pingval"].ToInt();
    set_money = config["set_money"].ToBool();
    moneyhacked = config["moneyhacked"].ToBool();
    gUnlimitedGrenades = config["gUnlimitedGrenades"].ToBool();

    fly = config["fly"].ToBool();
    crouch = config["crouch"].ToBool();
    bunnyhope = config["bunnyhope"].ToBool();
    hope_value = config["hope_value"].ToFloat();
    airjump = config["airjump"].ToBool();

    sethl9998 = config["sethl9998"].ToBool();
    
    instantThrow = config["instantThrow"].ToBool();
    infiniteGrenades = config["infiniteGrenades"].ToBool();
    noFuse = config["noFuse"].ToBool();
    maxRadius = config["maxRadius"].ToBool();
    noSelfDamage = config["noSelfDamage"].ToBool();
    maxPenetration = config["maxPenetration"].ToBool();
    fastgr = config["fastgr"].ToBool();
    antigr = config["antigr"].ToBool();
    
    fakel = config["fakel"].ToBool();
    isLagging = config["isLagging"].ToBool();
    fastb = config["fastb"].ToBool();
    bombd = config["bombd"].ToBool();
    mbf = config["mbf"].ToBool();
    accentColor[0] = config["accentColor.r"].ToFloat();
    accentColor[1] = config["accentColor.g"].ToFloat();
    accentColor[2] = config["accentColor.b"].ToFloat();
    background = config["background"].ToBool();
    backgroundOpacity = config["backgroundOpacity"].ToFloat();
    snowflake = config["snowflake"].ToBool();
    flakespeed = config["flakespeed"].ToFloat();
    dmgh = config["dmgh"].ToBool();
    aafk = config["aafk"].ToBool();
    invismode = config["invismode"].ToBool();
    tanki = config["tanki"].ToBool();
    tankit = config["tankit"].ToInt();
    vssc = config["vssc"].ToBool();
    firrrr = config["firrrr"].ToBool();
    tg = config["tg"].ToBool();
    bexplos = config["bexplos"].ToBool();
    bplant = config["bplant"].ToBool();
    bimpulse = config["bimpulse"].ToBool();
    bradius = config["bradius"].ToBool();
    bdamage = config["bdamage"].ToBool();
    gdetonate = config["gdetonate"].ToBool();
    gdetonate = config["gdetonate"].ToBool();
    gnodamage = config["gnodamage"].ToBool();
    speedhack = config["speedhack"].ToBool();
    tpeses = config["tpeses"].ToBool();
    
    autowin = config["autowin"].ToBool();
    high_jump = config["high_jump"].ToBool();
    high_value = config["high_value"].ToFloat();
    antiaim = config["antiaim"].ToBool();
    
    spoofers.hideid = config["spoofers.hideid"].ToBool();
    spoofers.hideclantag = config["spoofers.hideclantag"].ToBool();
    
    weapon.reversammo = config["weapon.reversammo"].ToBool();
    weapon.infinityammo = config["weapon.infinityammo"].ToBool();
    weapon.wallshoot = config["weapon.wallshoot"].ToBool();
    weapon.norecoil = config["weapon.norecoil"].ToBool();
    weapon.firerate = config["weapon.firerate"].ToBool();
    weapon.onehitkill = config["weapon.onehitkill"].ToBool();
    weapon.fastknife = config["weapon.fastknife"].ToBool();
    
    set_avatar = config["set_avatar"].ToBool();
    set_medalsc = config["set_medalsc"].ToBool();
    force_team_ct = config["force_team_ct"].ToBool();
    force_team_tr = config["force_team_tr"].ToBool();
    playerscaleval = config["playerscaleval"].ToFloat();
    player_scale = config["player_scale"].ToBool();
    set_statrack = config["set_statrack"].ToBool();
    srarrackval = config["srarrackval"].ToInt();
    set_pattern = config["set_pattern"].ToBool();
    patternval = config["patternval"].ToInt();
    
    moneyhack_a = config["moneyhack_a"].ToBool();
    infinity_buy_a = config["infinity_buy_a"].ToBool();
    buy_anywhere_a = config["buy_anywhere_a"].ToBool();
    clumsy = config["clumsy"].ToBool();
    islag = config["islag"].ToBool();
    strafe = config["strafe"].ToBool();
    strafe_speed = config["strafe_speed"].ToFloat();
}

inline void DeleteConfigSelected()
{
    if (ConfigList.empty()) return;

    string path = CFG_PATH + ConfigList[selectedConfig] + CFG_EXT;
    remove(path.c_str());

    ScanConfigs();
}
