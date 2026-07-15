int currentConfigIndex = 0;
int createTypeIndex = 0;
static const char* cfgItems[128];

bool cfg_save_btn   = false;
bool cfg_load_btn   = false;
bool cfg_create_btn = false;
bool cfg_delete_btn = false;

bool kickplayers;

bool autoRefill, maxPenetration, instantThrow, infiniteGrenades, noFuse, maxRadius, noSelfDamage = false;

bool fakel, isLagging;
bool norec, fly, crouch, knif, fastgr;
bool antigr, fastb, bombd;
bool mbf;

bool set_pause;
bool set_win_team;
bool set_mmr = false;
bool steal_flag = false;
bool set_rank = false;
bool set_clan_tag = false;
bool set_bot = false;
bool set_ping = false;
bool set_money = false;
bool set_mvp = false;
bool cencmatchmm = false;
bool spoof_uid = false;
bool setkills = false;
bool fastwin = false;
bool setassists = false;
bool set_time = false;
bool set_round_start_time = false;
bool set_round = false;
bool setdeath = false;
bool moneyhacked = false;
bool sethp = false;
bool killall = false;

bool high_jump;
bool antiaim;
// Anti-aim configuration
int   aa_yaw_mode   = 2;      // 0 = Off, 1 = Static, 2 = Spin, 3 = Jitter
float aa_yaw_value  = 180.0f; // static offset / jitter range (degrees)
float aa_spin_speed = 12.0f;  // spin step per tick (degrees)
int   aa_pitch_mode = 1;      // 0 = Off, 1 = Down, 2 = Up, 3 = Zero, 4 = Jitter
float aa_pitch_value = 89.0f; // pitch magnitude (degrees)
bool  aa_body = true;         // also rotate the visible body
// Third person (CS:GO-style camera pull-back)
bool  thirdperson = false;
float thirdperson_distance = 2.0f;
bool  silentaim = false; // HvH: silent aim + auto-fire on visible targets
bool  silent_autofire = false; // also auto-pull the trigger (experimental)
int   silent_field = 1; // which AimingData angle to write: 0=camera(0x18) 1=shoot(0x24) 2=both
const char* silentFieldItems[] = { "Camera (0x18)", "Shoot (0x24)", "Both" };
bool autowin;
bool firerat = false;
bool airjump = false;
bool fastreload = false;
float sky_color[3] = { 0.05f, 0.05f, 0.1f };
ImColor colorsky = ImColor(sky_color[0], sky_color[1], sky_color[2]);
bool sky = false;

bool bunnyhope = false;
float hope_value = 1;
bool camerafov = false;
float fov_value = 70;
bool norecoil = false;
bool aspect = false;
float aspect_value = 2.0f;
float high_value = 20.0;
bool scorehack, armorhack, setammo, infbuytime, sethl9998, damagehacks, setdamage, onlyhead;
bool set_score = false;
int score_value = 250;
int healthvalls = 200;
int deathval = 200;
int assistsval = 200;
int killsval = 200;
int timeval = 60;
int roundval = 1;
int pingval = 30;

bool vssc = true;
bool enablespi = false;

bool snowflake = true;
float flakespeed = 2.5f;
bool background = true;
static float scrollBarWidth = 20.0f, tabAlpha, backgroundOpacity = 75.0f;
static int pageId, activeTab;

static float accentColor[3] = { 0.0f, 0.75f, 1.0f };

bool tanki = false;
int tankit = 0;

const char* aimTargetItems[] = { "Head", "Spine", "Hip" };
const char* aaYawItems[] = { "Off", "Static", "Spin", "Jitter" };
const char* aaPitchItems[] = { "Off", "Down", "Up", "Zero", "Jitter" };

struct {
    bool enable = false;
    bool shortRangeOnly = false;
    bool drawFovCircle = false;
    bool fovFillEnabled = false;
    float fovFillOpacity = 0.18f;
    float yOffset = -1.4f;
    bool visibleCheck = false;
    float fovRadius = 200.0f, maxDistance = 99999999.0f;
    float smoothing = 0.2f;
    float fovColor[3] = {1.0f, 1.0f, 1.0f};
    float highline = 3.5f;
    float triggerDelay = 0.3f;
    float lastTriggerTime = 0.0f;
    float fovVisibleColor[3]   = { 0.0f, 0.9f, 0.7f };
    float fovInvisibleColor[3] = { 1.0f, 0.25f, 0.05f };
    int aimTarget = 0;
} aimbot;

struct {
    bool enabl, box3d, money, distance, ping, ammo, isvis;
    bool line = false;
    bool box = false;
    bool armor = false;
    bool health = false;
    bool nickName = false;
    bool pulseWave = false;
    bool offscreenArrow = false;
    bool foto;
    float offscreenArrowSize = 25.0f;
    float offscreenArrowRadius = 0.45f;
    bool offscreenArrowGlow = true;
    float offscreenArrowGlowStrength = 5.0f;
    float offscreenArrowColor[3] = { 0.0f, 0.85f, 1.0f };
    float pulseColor[3] = { 0.0f, 0.9f, 1.0f };
    float pulseSpeed = 4.0f;
    float pulseMaxRadius = 80.0f;
    float pulseThickness = 2.5f;
    int pulseCount = 6;
    float pulseFadeStart = 0.6f;
    bool hitlogs = false;
    float hitlogColor[3] = { 1.0f, 0.2f, 0.1f };
    float hitlogBgAlpha = 0.25f;
    float hitlogBorderThick = 2.0f;
    float hitlogRounding = 8.0f;
    float moneyColor[3] = { 0.3f, 1.0f, 0.45f };
    float distanceColor[3] = { 0.65f, 0.85f, 1.0f };
    float pingColor[3] = { 1.0f, 0.75f, 0.1f };

    bool hpBar = false;
    bool hpGradient = false;
    float hpbarscaless = 4.0f;
    float hpGlowStrength = 6.0f;
    ImVec4 hpColor  = ImVec4(0.15f, 1.0f, 0.4f, 1.0f);
    ImVec4 hpColor1 = ImVec4(0.15f, 1.0f, 0.4f, 1.0f);
    ImVec4 hpColor2 = ImVec4(0.85f, 0.08f, 0.05f, 1.0f);
    bool hpAdaptiveColor = false;
    int hpBarStyle = 0;
    float hpBarRounding = 3.0f;

    bool armorGradient = false;
    ImVec4 armorColor  = ImVec4(0.0f, 0.6f, 1.0f, 1.0f);
    ImVec4 armorColor1 = ImVec4(0.25f, 0.8f, 1.0f, 1.0f);
    ImVec4 armorColor2 = ImVec4(0.0f, 0.25f, 0.75f, 1.0f);
    bool armorAdaptiveColor = false;
    float armorBarRounding = 3.0f;
    float boxGlowStrength = 5.0f;
    float lineGlowStrength = 4.0f;
    float armorThickness = 4.0f;
    float armorGlowStrength = 4.0f;
    float armorOffset = 6.0f;

    float nameGlowStrength = 0.0f;
    float rounding = 2.0f;
    float nickNameColor[3] = { 0.55f, 1.0f, 0.95f };
    bool nameOutline = true;
    float nameOutlineColor[3] = { 0.0f, 0.0f, 0.0f };
    float nameScale = 0.75f;

    float espColor[4] = { 0.0f, 0.88f, 1.0f, 1.0f };
    float espVColor[4] = { 0.15f, 1.0f, 0.5f, 1.0f };
    float espIColor[4] = { 1.0f, 0.12f, 0.0f, 1.0f };
    float espGlobalAlpha = 1.0f;

    bool enable, boxFilled, name, healthBarAdaptive, healthBarGradient, armorBarAdaptive, armorBarGradient;
    float boxFilledOpacity = 12.0f;
    float boxOutlineThickness = 2.0f;
    float healthBarColor[3] = { 0.15f, 0.9f, 0.35f };
    float healthBarGradientTopColor[3] = { 0.0f, 1.0f, 0.4f };
    float healthBarGradientBottomColor[3] = { 1.0f, 0.2f, 0.05f };
    float armorBarColor[3] = { 0.05f, 0.55f, 1.0f };
    float armorBarGradientRightColor[3] = { 0.0f, 0.3f, 0.8f };
    float armorBarGradientLeftColor[3] = { 0.3f, 0.85f, 1.0f };
    float espLineThickness = 2.0f;
    int snaplineStyle = 0;
    int lineEndStyle = 0;

    bool skeleton = false;
    float skeletonColor[3] = { 0.4f, 0.2f, 1.0f };
    float skeletonThickness = 1.8f;
    bool skeletonOutline = true;
    float skeletonOutlineThickness = 3.2f;
    bool skeletonOnlyVisible = false;
    float skeletonGlow = 3.0f;
    float skeletonScale = 0.9f;
    bool skeletonJointDots = false;
    float skeletonJointSize = 2.5f;
    float skeletonJointColor[3] = { 0.0f, 0.9f, 1.0f };
    float skeletonJointGlow = 0.0f;
    int headCircleStyle = 0;
    float headCircleSize = 5.0f;

    int boxStyle = 0;
    float cornerLength = 0.25f;

    bool weapon = false;
    bool weaponOutline = true;
    float weaponColor[3] = { 1.0f, 0.6f, 0.05f };
    float weaponGlowStrength = 3.0f;
    float weaponGlowColor[3] = { 1.0f, 0.5f, 0.0f };
    bool weaponicon = false;

    bool bullettrace = false;
    float bullettraceColor[3] = { 1.0f, 0.5f, 0.0f };
    float bullettraceGlowColor[3] = { 1.0f, 0.3f, 0.0f };
    float bullettraceThickness = 2.2f;
    float bullettraceGlow = 10.0f;
    float bullettraceTime = 2.0f;
    float bullettraceHitSize = 5.0f;
    int bullettraceStyle = 1;
    int bullettraceHitStyle = 0;
    bool bullettraceNeonCore = true;
    bool bullettraceFromCrosshair = true;
} esp;


bool boxf;
float boxfv = 0.0f;

bool ioss;
bool firrrr;
bool tg;
bool invismode, skiptable;
bool bexplos, bplant, bimpulse, bradius, bdamage;

bool aafk;
bool tpeses;
bool head_scale = false;
float headScaleValue = 1.0f;
bool strafe = false;
float strafe_speed = 1.0f;
bool speedhack = false;

bool gdetonate, gnodamage, dmgh, gUnlimitedGrenades = false;

bool damageHack;

struct {
    bool hideid,
    hideclantag = false;
} spoofers;

struct {
    bool reversammo,
    infinityammo,
    wallshoot,
    onehitkill,
    firerate,
    fastknife,
    jumpnorecoil,
    norecoil = false;
} weapon;

bool arms_position = false;
float ARMSX, ARMSY, ARMSZ = 0.0f;

bool grenadeset;
bool setm9s;
bool kerambitset;
bool jcomandoset;
bool butterflyset;
bool flipset;
bool kunaiset;
bool scorpionset;
bool tantoset;
bool duggerset;
bool kukriset;
bool stilletoset;
bool mantisset;
bool fangset;
bool stingset;
bool gloveset;
bool g22set;
bool uspset;
bool p350set;
bool beretasset;
bool tec9set;
bool fivesevenset;
bool deagleset;
bool ump45set;
bool mp7set;
bool p90set;
bool mp5set;
bool akrset;
bool akr12set;
bool m4set;
bool m4a1set;
bool m16set;
bool famasset;
bool fnfalset;
bool awmset;
bool m40set;
bool m110set;
bool sm1014set;
bool fabmset;
bool m60set;
bool valset;
bool mac10set;
bool spasset;
bool akimbouziset;
bool medalset;
bool avatarset;
bool mallardset;

float playerscaleval = 1.0f;
bool player_scale;

float teleportStep = 5.0f;

bool tpForward = false;
bool tpBackward = false;
bool tpLeft = false;
bool tpRight = false;
bool tpUp = false;
bool tpDown = false;

bool force_team_tr, force_team_ct;

bool set_medalsc, set_avatar;

int patternval = 240;
int srarrackval = 1000;

bool set_pattern, set_statrack;

bool moneyhack_a, infinity_buy_a, buy_anywhere_a;

bool clumsy = false;
bool islag = false;

bool hahahalol228 = true;

bool set_score_all = false;
int score_all_v = 200;
bool hide_id_all = false;
bool hide_clan_tag_all = false;
bool set_death_all = false;
int death_all_v = 200;
bool set_money_all = false;
int money_all_v = 10000;
bool set_ping_all = false;
int ping_all_vl = 1;
bool set_kills_all = false;
int kills_all_v = 200;
bool fake_avatar_all = false;
bool fake_medal_all = false;

bool airstafe = false;
bool autowin_tr = false;
bool autowin_ct = false;
bool skip_warmup = false;

bool anticheat = true;
