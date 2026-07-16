--[[  ERA Roblox Script v9.0  ==================================================
      "Claude" edition 🦀  — glass/blur menu, sounds, animations, config system.

  Features
    Combat   : Aimbot (Camera/Silent, Hold/Toggle, FOV, smooth, prediction,
               team/visible/wall checks, sticky), Hitbox Expander.
    Weapon   : Infinite Ammo, Fast Fire        (best-effort, game-dependent).
    Visuals  : ESP  (Highlight chams through walls, team colour).
    Movement : NoClip Fly (fly with the normal joystick/WASD, follows the camera,
               passes through walls), WalkSpeed, JumpPower.
    Utility  : Anti-Kick (best-effort, needs an executor with hookmetamethod).
    UI       : tabbed glass menu, background blur, animated gradients, sounds,
               config save/load, mobile buttons (hold 3s to move them).

  Reliability: Aimbot/NoClip-Fly/Hitbox/ESP/WalkSpeed/Jump = reliable client-side.
               Infinite Ammo/Fast Fire/Anti-Kick = best-effort, depend on the
               game and the executor. Nothing here is a false promise.
===============================================================================]]

local Players      = game:GetService("Players")
local RunService   = game:GetService("RunService")
local UIS          = game:GetService("UserInputService")
local TweenService = game:GetService("TweenService")
local Lighting     = game:GetService("Lighting")
local SoundService  = game:GetService("SoundService")
local HttpService  = game:GetService("HttpService")
local Workspace    = workspace
local Stats        = game:FindService("Stats")
local LocalPlayer  = Players.LocalPlayer
local function cam() return Workspace.CurrentCamera end

-- ============================ CONFIG (persisted) ===============
local Config = {
    -- Aimbot
    AimOn = false, AimMethod = "Camera", AimMode = "Toggle",
    AimKey = Enum.UserInputType.MouseButton2,
    FOV = 300, Smoothness = 0.6, Prediction = 0.0, TargetPart = "Head",
    AimTarget = "Closest",   -- "Closest" (snap to nearest enemy) | "Crosshair" (nearest to aim, within FOV)
    TeamCheck = false, VisibleCheck = false, Wallshot = false, StickyTarget = true,
    ShowFOV = true, FOVRainbow = false, AimDebug = true,
    -- Hitbox
    HitboxOn = false, HitboxSize = 10, HitboxPart = "HumanoidRootPart",
    -- Weapon (best-effort)
    InfAmmoOn = false, FastFireOn = false, FireDelay = 0.03,
    -- HvH / Rage (enabled from load per request)
    AutoFire = true, AutoFireDelay = 0.08, SilentAim = true,
    AntiAimOn = false, AntiAimMode = "Spin", AntiAimSpeed = 20,
    ThirdPerson = false, ThirdPersonZoom = 12,
    -- ESP
    ESPOn = false, ESPTeamColor = true, ESPColor = Color3.fromRGB(217, 119, 87),
    -- Self / movement
    InvisOn = false,
    NoClipOn = false, NoClipSpeed = 60, WalkOn = false, WalkSpeed = 16, JumpOn = false, JumpPower = 50,
    -- Utility
    AntiKickOn = false,
    -- Interface  (Claude 🦀 style)
    Accent = Color3.fromRGB(217, 119, 87), MenuKey = Enum.KeyCode.RightControl,
    UIScale = 1, UISounds = true, MenuBlur = true, WatermarkAnim = true,
    ShowAimBtn = true, ShowNoClipBtn = true,
    ConfigName = "default",
}

-- ============================ THEME (Claude) ===================
local Theme = {
    Bg = Color3.fromRGB(23, 21, 19), Bg2 = Color3.fromRGB(31, 28, 25),
    Panel = Color3.fromRGB(40, 36, 32), PanelHi = Color3.fromRGB(50, 45, 40),
    Track = Color3.fromRGB(66, 60, 54), Text = Color3.fromRGB(240, 236, 228),
    Sub = Color3.fromRGB(162, 154, 142), Stroke = Color3.fromRGB(255, 252, 245),
    Good = Color3.fromRGB(122, 184, 122), Bad = Color3.fromRGB(214, 99, 91),
}

-- ============================ HELPERS ==========================
local function new(cls, props, parent)
    local o = Instance.new(cls)
    if props then for k, v in pairs(props) do o[k] = v end end
    if parent then o.Parent = parent end
    return o
end
local function corner(p, r) return new("UICorner", { CornerRadius = UDim.new(0, r or 8) }, p) end
local function stroke(p, c, t, tr) return new("UIStroke",
    { Color = c or Theme.Stroke, Thickness = t or 1, Transparency = tr or 0,
      ApplyStrokeMode = Enum.ApplyStrokeMode.Border }, p) end
local function gradient(p, a, b, rot) return new("UIGradient",
    { Color = ColorSequence.new(a, b), Rotation = rot or 90 }, p) end
local function TI(t, s, d) return TweenInfo.new(t or 0.18,
    s or Enum.EasingStyle.Quart, d or Enum.EasingDirection.Out) end
local function tween(o, props, t, s, d) local tw = TweenService:Create(o, TI(t, s, d), props); tw:Play(); return tw end
local function isTouch() return UIS.TouchEnabled and not UIS.MouseEnabled end

-- accent-tracked objects (recolored instantly when accent changes)
local accentObjects = {}
local function accent(obj, prop) obj[prop] = Config.Accent; table.insert(accentObjects, { obj, prop }); return obj end

-- widget registry for config load -> UI sync
local widgets = {}   -- flag -> { set = fn }
local loading = false -- true while applying a loaded config (suppress sound/notif spam)

-- ============================ SOUND ============================
local Sounds = {}
local function mkSound(id, vol)
    local s = new("Sound", { SoundId = "rbxassetid://" .. id, Volume = vol or 0.5 }, SoundService)
    return s
end
pcall(function()
    Sounds.open   = mkSound(6042053626, 0.45)   -- soft click/open
    Sounds.close  = mkSound(6042053626, 0.30)
    Sounds.toggle = mkSound(9080709694, 0.35)   -- tick
    Sounds.hover  = mkSound(9080709694, 0.15)
end)
local function playSound(name)
    if not Config.UISounds or loading then return end
    local s = Sounds[name]
    if s then pcall(function() SoundService:PlayLocalSound(s) end) end
end

-- ============================ ROOT GUI =========================
local function mountGui()
    local g = new("ScreenGui", { Name = "ERA_v9", ResetOnSpawn = false, IgnoreGuiInset = true,
        ZIndexBehavior = Enum.ZIndexBehavior.Sibling, DisplayOrder = 999 })
    local ok = pcall(function() g.Parent = (gethui and gethui()) or game:GetService("CoreGui") end)
    if not ok or not g.Parent then pcall(function() g.Parent = LocalPlayer:WaitForChild("PlayerGui") end) end
    return g
end
local gui = mountGui()

-- background blur (glass "shader" behind the menu)
local menuBlur = new("BlurEffect", { Size = 0, Enabled = true }, Lighting)

-- ================= centralized drag + slider input =============
local dragState, activeSlider
UIS.InputChanged:Connect(function(i)
    if i.UserInputType ~= Enum.UserInputType.MouseMovement and i.UserInputType ~= Enum.UserInputType.Touch then return end
    if dragState then
        local d = i.Position - dragState.start
        local sp = dragState.pos
        dragState.frame.Position = UDim2.new(sp.X.Scale, sp.X.Offset + d.X, sp.Y.Scale, sp.Y.Offset + d.Y)
    end
    if activeSlider then activeSlider(i.Position.X) end
end)
UIS.InputEnded:Connect(function(i)
    if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then
        dragState, activeSlider = nil, nil
    end
end)
local function makeDraggable(frame, handle)
    handle = handle or frame
    handle.InputBegan:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then
            dragState = { frame = frame, start = i.Position, pos = frame.Position }
        end
    end)
end

-- ============================ NOTIFICATIONS ====================
local notifHolder = new("Frame", { AnchorPoint = Vector2.new(1, 0), Position = UDim2.new(1, -16, 0, 16),
    Size = UDim2.new(0, 260, 1, -32), BackgroundTransparency = 1 }, gui)
new("UIListLayout", { Padding = UDim.new(0, 8), HorizontalAlignment = Enum.HorizontalAlignment.Right,
    VerticalAlignment = Enum.VerticalAlignment.Top, SortOrder = Enum.SortOrder.LayoutOrder }, notifHolder)
local function Notify(text, dur, col)
    if loading then return end
    dur = dur or 2.4
    local card = new("Frame", { Size = UDim2.new(1, 0, 0, 0), AutomaticSize = Enum.AutomaticSize.Y,
        BackgroundColor3 = Theme.Bg2, BackgroundTransparency = 1, ClipsDescendants = true }, notifHolder)
    corner(card, 10); stroke(card, Theme.Stroke, 1, 0.9)
    new("Frame", { Size = UDim2.new(0, 3, 1, 0), BackgroundColor3 = col or Config.Accent, BorderSizePixel = 0 }, card)
    local lbl = new("TextLabel", { Size = UDim2.new(1, -22, 0, 0), Position = UDim2.new(0, 14, 0, 0),
        BackgroundTransparency = 1, Text = text, TextWrapped = true, TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 13, TextXAlignment = Enum.TextXAlignment.Left,
        AutomaticSize = Enum.AutomaticSize.Y }, card)
    new("UIPadding", { PaddingTop = UDim.new(0, 10), PaddingBottom = UDim.new(0, 10) }, lbl)
    card.Position = UDim2.new(1, 0, 0, 0)
    tween(card, { BackgroundTransparency = 0.05 }, 0.25, Enum.EasingStyle.Back)
    task.delay(dur, function()
        if card and card.Parent then tween(card, { BackgroundTransparency = 1 }, 0.25); task.wait(0.26); card:Destroy() end
    end)
end

-- ============================ WINDOW ===========================
local WIN_W, WIN_H, RAIL_W = 566, 400, 140
local window = new("Frame", { Name = "Window", AnchorPoint = Vector2.new(0.5, 0.5),
    Position = UDim2.new(0.5, 0, 0.5, 0), Size = UDim2.new(0, WIN_W, 0, WIN_H),
    BackgroundColor3 = Theme.Bg, BorderSizePixel = 0, Visible = false }, gui)
corner(window, 16); stroke(window, Theme.Stroke, 1, 0.84)
local uiScale = new("UIScale", { Scale = 1 }, window)
new("ImageLabel", { Size = UDim2.new(1, 46, 1, 46), Position = UDim2.new(0, -23, 0, -23),
    BackgroundTransparency = 1, Image = "rbxassetid://6014261993", ImageColor3 = Color3.new(0, 0, 0),
    ImageTransparency = 0.35, ScaleType = Enum.ScaleType.Slice, SliceCenter = Rect.new(49, 49, 450, 450),
    ZIndex = 0 }, window)

-- title bar with animated accent gradient (shimmer)
local titleBar = new("Frame", { Size = UDim2.new(1, 0, 0, 46), BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, window)
corner(titleBar, 16)
new("Frame", { Size = UDim2.new(1, 0, 0, 16), Position = UDim2.new(0, 0, 1, -16),
    BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, titleBar)
local accentBar = accent(new("Frame", { Size = UDim2.new(1, -24, 0, 2), Position = UDim2.new(0, 12, 1, -2),
    BorderSizePixel = 0 }, titleBar), "BackgroundColor3")
local accentGrad = gradient(accentBar, Config.Accent, Color3.fromRGB(255, 200, 150), 0)
local logo = new("TextLabel", { Size = UDim2.new(0, 28, 1, 0), Position = UDim2.new(0, 14, 0, 0),
    BackgroundTransparency = 1, Text = "🦀", Font = Enum.Font.GothamBold, TextSize = 18 }, titleBar)
new("TextLabel", { Size = UDim2.new(0, 120, 1, 0), Position = UDim2.new(0, 44, 0, 0), BackgroundTransparency = 1,
    Text = "ERA", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
    Font = Enum.Font.GothamBold, TextSize = 18 }, titleBar)
new("TextLabel", { Size = UDim2.new(0, 60, 1, 0), Position = UDim2.new(0, 84, 0, 0), BackgroundTransparency = 1,
    Text = "v9 🦀", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Sub,
    Font = Enum.Font.GothamMedium, TextSize = 12 }, titleBar)
local closeBtn = new("TextButton", { Size = UDim2.new(0, 28, 0, 28), Position = UDim2.new(1, -40, 0.5, -14),
    BackgroundColor3 = Theme.Panel, Text = "✕", TextColor3 = Theme.Text, Font = Enum.Font.GothamBold,
    TextSize = 13, AutoButtonColor = false }, titleBar)
corner(closeBtn, 8)
makeDraggable(window, titleBar)

-- animated shimmer on the accent bar
task.spawn(function()
    while gui.Parent do
        if Config.WatermarkAnim then tween(accentGrad, { Rotation = 25 }, 1.6, Enum.EasingStyle.Sine, Enum.EasingDirection.InOut); task.wait(1.6)
            tween(accentGrad, { Rotation = -25 }, 1.6, Enum.EasingStyle.Sine, Enum.EasingDirection.InOut); task.wait(1.6)
        else task.wait(0.5) end
    end
end)

-- rail
local rail = new("Frame", { Size = UDim2.new(0, RAIL_W, 1, -46), Position = UDim2.new(0, 0, 0, 46),
    BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, window)
new("Frame", { Size = UDim2.new(0, 1, 1, -16), Position = UDim2.new(1, -1, 0, 8), BackgroundColor3 = Theme.Stroke,
    BackgroundTransparency = 0.9, BorderSizePixel = 0 }, rail)
local railList = new("Frame", { Size = UDim2.new(1, 0, 1, -44), Position = UDim2.new(0, 0, 0, 8),
    BackgroundTransparency = 1 }, rail)
new("UIListLayout", { Padding = UDim.new(0, 6), SortOrder = Enum.SortOrder.LayoutOrder,
    HorizontalAlignment = Enum.HorizontalAlignment.Center }, railList)
new("TextLabel", { Size = UDim2.new(1, -16, 0, 28), Position = UDim2.new(0, 8, 1, -34), BackgroundTransparency = 1,
    Text = "🦀 Claude style", TextColor3 = Theme.Sub, Font = Enum.Font.Gotham, TextSize = 11 }, rail)

local content = new("Frame", { Size = UDim2.new(1, -RAIL_W, 1, -46), Position = UDim2.new(0, RAIL_W, 0, 46),
    BackgroundTransparency = 1 }, window)

-- ============================ TAB SYSTEM =======================
local Tabs, currentTab   -- forward-declared for applyAccent
local function applyAccent(col)
    Config.Accent = col
    for _, e in ipairs(accentObjects) do if e[1] and e[1].Parent then pcall(function() e[1][e[2]] = col end) end end
    pcall(function() accentGrad.Color = ColorSequence.new(col, Color3.fromRGB(255, 200, 150)) end)
    if Tabs then for _, t in ipairs(Tabs) do
        if t == currentTab then t.icon.TextColor3 = col end
        pcall(function() t.page.ScrollBarImageColor3 = col end)
    end end
end

Tabs, currentTab = {}, nil
local function selectTab(tab)
    if currentTab == tab then return end
    for _, t in ipairs(Tabs) do
        local sel = t == tab
        t.page.Visible = sel
        tween(t.btn, { BackgroundColor3 = sel and Theme.Panel or Theme.Bg2 }, 0.15)
        tween(t.label, { TextColor3 = sel and Theme.Text or Theme.Sub }, 0.15)
        t.icon.TextColor3 = sel and Config.Accent or Theme.Sub
        t.indic.Visible = sel
    end
    currentTab = tab
end
local function addTab(name, icon)
    local btn = new("TextButton", { Size = UDim2.new(1, -16, 0, 36), BackgroundColor3 = Theme.Bg2, Text = "",
        AutoButtonColor = false, LayoutOrder = #Tabs + 1 }, railList)
    corner(btn, 9)
    local indic = accent(new("Frame", { Size = UDim2.new(0, 3, 0, 18), Position = UDim2.new(0, 0, 0.5, -9),
        BorderSizePixel = 0, Visible = false }, btn), "BackgroundColor3")
    corner(indic, 2)
    local ico = new("TextLabel", { Size = UDim2.new(0, 24, 1, 0), Position = UDim2.new(0, 12, 0, 0),
        BackgroundTransparency = 1, Text = icon or "•", TextColor3 = Theme.Sub, Font = Enum.Font.GothamBold, TextSize = 15 }, btn)
    local lbl = new("TextLabel", { Size = UDim2.new(1, -42, 1, 0), Position = UDim2.new(0, 40, 0, 0),
        BackgroundTransparency = 1, Text = name, TextXAlignment = Enum.TextXAlignment.Left,
        TextColor3 = Theme.Sub, Font = Enum.Font.GothamMedium, TextSize = 13 }, btn)
    local page = new("ScrollingFrame", { Size = UDim2.new(1, 0, 1, 0), BackgroundTransparency = 1, Visible = false,
        BorderSizePixel = 0, ScrollBarThickness = 3, ScrollBarImageColor3 = Config.Accent,
        ScrollBarImageTransparency = 0.5, CanvasSize = UDim2.new(0, 0, 0, 0),
        AutomaticCanvasSize = Enum.AutomaticSize.Y, ScrollingDirection = Enum.ScrollingDirection.Y }, content)
    new("UIListLayout", { Padding = UDim.new(0, 9), SortOrder = Enum.SortOrder.LayoutOrder }, page)
    new("UIPadding", { PaddingTop = UDim.new(0, 12), PaddingBottom = UDim.new(0, 12),
        PaddingLeft = UDim.new(0, 14), PaddingRight = UDim.new(0, 12) }, page)
    local tab = { btn = btn, page = page, icon = ico, label = lbl, indic = indic }
    table.insert(Tabs, tab)
    btn.MouseButton1Click:Connect(function() selectTab(tab); playSound("hover") end)
    if #Tabs == 1 then selectTab(tab) end
    return page
end

-- ============================ COMPONENTS =======================
local order = 0
local function nextOrder() order = order + 1; return order end
local function section(page, text)
    return new("TextLabel", { Size = UDim2.new(1, 0, 0, 16), BackgroundTransparency = 1, Text = string.upper(text),
        TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Sub, Font = Enum.Font.GothamBold,
        TextSize = 11, LayoutOrder = nextOrder() }, page)
end
local function hint(page, text)
    return new("TextLabel", { Size = UDim2.new(1, 0, 0, 30), BackgroundTransparency = 1, TextWrapped = true,
        Text = text, TextColor3 = Theme.Sub, Font = Enum.Font.Gotham, TextSize = 11,
        TextXAlignment = Enum.TextXAlignment.Left, LayoutOrder = nextOrder() }, page)
end
local function row(page, h)
    local r = new("Frame", { Size = UDim2.new(1, 0, 0, h or 44), BackgroundColor3 = Theme.Panel,
        BorderSizePixel = 0, LayoutOrder = nextOrder() }, page)
    corner(r, 10); stroke(r, Theme.Stroke, 1, 0.94)
    return r
end

-- Toggle bound to a Config flag
local function Toggle(page, text, flag, onChange)
    local r = row(page, 44)
    new("TextLabel", { Size = UDim2.new(1, -80, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
        Text = text, TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 14 }, r)
    local track = new("Frame", { Size = UDim2.new(0, 46, 0, 24), Position = UDim2.new(1, -60, 0.5, -12),
        BackgroundColor3 = Theme.Track, BorderSizePixel = 0 }, r)
    corner(track, 12)
    local knob = new("Frame", { Size = UDim2.new(0, 18, 0, 18), Position = UDim2.new(0, 3, 0.5, -9),
        BackgroundColor3 = Color3.new(1, 1, 1), BorderSizePixel = 0 }, track)
    corner(knob, 9)
    local st = Config[flag] and true or false
    local function render(anim)
        local tc = st and Config.Accent or Theme.Track
        local kp = st and UDim2.new(1, -21, 0.5, -9) or UDim2.new(0, 3, 0.5, -9)
        if anim then tween(track, { BackgroundColor3 = tc }, 0.16); tween(knob, { Position = kp }, 0.16, Enum.EasingStyle.Back)
        else track.BackgroundColor3 = tc; knob.Position = kp end
    end
    render(false)
    local function set(v, silent)
        st = v and true or false; Config[flag] = st; render(true)
        if not silent then playSound("toggle"); if onChange then onChange(st) end
        elseif onChange then onChange(st) end
    end
    widgets[flag] = { set = set }
    local hit = new("TextButton", { Size = UDim2.new(1, 0, 1, 0), BackgroundTransparency = 1, Text = "" }, r)
    hit.MouseButton1Click:Connect(function() set(not st) end)
    r.MouseEnter:Connect(function() tween(r, { BackgroundColor3 = Theme.PanelHi }, 0.12) end)
    r.MouseLeave:Connect(function() tween(r, { BackgroundColor3 = Theme.Panel }, 0.12) end)
    if st and onChange then onChange(true) end
    return { set = set, get = function() return st end }
end

local function Slider(page, text, flag, min, max, decimals, onChange)
    local r = row(page, 52)
    new("TextLabel", { Size = UDim2.new(1, -80, 0, 20), Position = UDim2.new(0, 14, 0, 6), BackgroundTransparency = 1,
        Text = text, TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 14 }, r)
    local val = accent(new("TextLabel", { Size = UDim2.new(0, 64, 0, 20), Position = UDim2.new(1, -76, 0, 6),
        BackgroundTransparency = 1, Text = "", TextXAlignment = Enum.TextXAlignment.Right,
        Font = Enum.Font.GothamBold, TextSize = 13 }, r), "TextColor3")
    local bar = new("Frame", { Size = UDim2.new(1, -28, 0, 6), Position = UDim2.new(0, 14, 0, 36),
        BackgroundColor3 = Theme.Track, BorderSizePixel = 0 }, r)
    corner(bar, 3)
    local fill = accent(new("Frame", { Size = UDim2.new(0, 0, 1, 0), BorderSizePixel = 0 }, bar), "BackgroundColor3")
    corner(fill, 3)
    local knob = new("Frame", { Size = UDim2.new(0, 14, 0, 14), Position = UDim2.new(1, -7, 0.5, -7),
        BackgroundColor3 = Color3.new(1, 1, 1), BorderSizePixel = 0, ZIndex = 2 }, fill)
    corner(knob, 7); accent(stroke(knob, Config.Accent, 2), "Color")
    local function fmt(v) local m = 10 ^ (decimals or 0); return tostring(math.floor(v * m + 0.5) / m) end
    local function apply(v, anim, silent)
        v = math.clamp(v, min, max); Config[flag] = v
        local a = (v - min) / (max - min)
        if anim then tween(fill, { Size = UDim2.new(a, 0, 1, 0) }, 0.08) else fill.Size = UDim2.new(a, 0, 1, 0) end
        val.Text = fmt(v); if onChange then onChange(v) end
    end
    apply(Config[flag], false)
    widgets[flag] = { set = function(v) apply(v, true) end }
    local function fromX(x) apply(min + (max - min) * math.clamp((x - bar.AbsolutePosition.X) / math.max(1, bar.AbsoluteSize.X), 0, 1), false) end
    local hit = new("TextButton", { Size = UDim2.new(1, 0, 0, 28), Position = UDim2.new(0, 0, 0, 26),
        BackgroundTransparency = 1, Text = "" }, r)
    hit.InputBegan:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then
            activeSlider = fromX; fromX(i.Position.X)
        end
    end)
    return { set = function(v) apply(v, true) end, get = function() return Config[flag] end }
end

local activeDropdown
local function Dropdown(page, text, flag, options, onChange)
    local r = row(page, 44)
    new("TextLabel", { Size = UDim2.new(0.5, -14, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
        Text = text, TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 14 }, r)
    local sel = new("TextButton", { Size = UDim2.new(0.5, -24, 0, 30), Position = UDim2.new(0.5, 10, 0.5, -15),
        BackgroundColor3 = Theme.Bg2, Text = tostring(Config[flag]) .. "  ▾", TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 13, AutoButtonColor = false, ZIndex = 3 }, r)
    corner(sel, 8); stroke(sel, Theme.Stroke, 1, 0.92)
    local listFrame
    local function close()
        if listFrame then listFrame:Destroy(); listFrame = nil end
        if activeDropdown == close then activeDropdown = nil end
    end
    local function choose(opt, silent)
        Config[flag] = opt; sel.Text = tostring(opt) .. "  ▾"
        if not silent then playSound("toggle") end
        if onChange then onChange(opt) end
    end
    widgets[flag] = { set = function(v) choose(v, true) end }
    sel.MouseButton1Click:Connect(function()
        if listFrame then close(); return end
        if activeDropdown then activeDropdown() end
        activeDropdown = close
        -- parent to `window` so it inherits UIScale; position in window-local unscaled
        -- coords, and flip above the button if it would overflow the viewport bottom.
        local s = uiScale.Scale
        local ap, sz = sel.AbsolutePosition, sel.AbsoluteSize
        local wp = window.AbsolutePosition
        local hAbs = (#options * 30 + 8) * s
        local yAbs = ap.Y + sz.Y + 2
        if yAbs + hAbs > gui.AbsoluteSize.Y then yAbs = ap.Y - hAbs - 2 end
        listFrame = new("Frame", { Size = UDim2.fromOffset(sz.X / s, #options * 30 + 8),
            Position = UDim2.fromOffset((ap.X - wp.X) / s, (yAbs - wp.Y) / s),
            BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0, ZIndex = 50 }, window)
        corner(listFrame, 8); stroke(listFrame, Theme.Stroke, 1, 0.9)
        new("UIListLayout", { Padding = UDim.new(0, 2), SortOrder = Enum.SortOrder.LayoutOrder }, listFrame)
        new("UIPadding", { PaddingTop = UDim.new(0, 4), PaddingBottom = UDim.new(0, 4),
            PaddingLeft = UDim.new(0, 4), PaddingRight = UDim.new(0, 4) }, listFrame)
        for _, opt in ipairs(options) do
            local ob = new("TextButton", { Size = UDim2.new(1, 0, 0, 28),
                BackgroundColor3 = opt == Config[flag] and Theme.Panel or Theme.Bg2, Text = tostring(opt),
                TextColor3 = opt == Config[flag] and Config.Accent or Theme.Text, Font = Enum.Font.GothamMedium,
                TextSize = 13, AutoButtonColor = false, ZIndex = 51 }, listFrame)
            corner(ob, 6)
            ob.MouseButton1Click:Connect(function() choose(opt); close() end)
        end
    end)
    return { get = function() return Config[flag] end }
end
UIS.InputBegan:Connect(function(i, gp)
    if gp or not activeDropdown then return end
    if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then activeDropdown() end
end)

local function Keybind(page, text, flag, onChange)
    local r = row(page, 44)
    new("TextLabel", { Size = UDim2.new(1, -120, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
        Text = text, TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 14 }, r)
    local function keyName(k) return typeof(k) == "EnumItem" and k.Name or "None" end
    local kbtn = new("TextButton", { Size = UDim2.new(0, 100, 0, 28), Position = UDim2.new(1, -112, 0.5, -14),
        BackgroundColor3 = Theme.Bg2, Text = keyName(Config[flag]), TextColor3 = Theme.Text, Font = Enum.Font.GothamMedium,
        TextSize = 12, AutoButtonColor = false }, r)
    corner(kbtn, 8); stroke(kbtn, Theme.Stroke, 1, 0.92)
    widgets[flag] = { set = function(v) kbtn.Text = keyName(v) end }
    local listening = false
    kbtn.MouseButton1Click:Connect(function() listening = true; kbtn.Text = "..."; tween(kbtn, { BackgroundColor3 = Config.Accent }, 0.12) end)
    UIS.InputBegan:Connect(function(i)
        if not listening then return end
        local k
        if i.KeyCode ~= Enum.KeyCode.Unknown then k = i.KeyCode
        elseif i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.MouseButton2 then k = i.UserInputType end
        if k then Config[flag] = k; listening = false; kbtn.Text = keyName(k); tween(kbtn, { BackgroundColor3 = Theme.Bg2 }, 0.12); if onChange then onChange(k) end end
    end)
    return { get = function() return Config[flag] end }
end

local function Button(page, text, col, cb)
    local b = new("TextButton", { Size = UDim2.new(1, 0, 0, 40), BackgroundColor3 = col or Theme.Panel,
        Text = text, TextColor3 = Theme.Text, Font = Enum.Font.GothamBold, TextSize = 14,
        AutoButtonColor = false, LayoutOrder = nextOrder() }, page)
    corner(b, 10)
    b.MouseButton1Click:Connect(function() playSound("hover"); cb() end)
    return b
end

-- ============================ FEATURE LOGIC ====================
local function others() local t = {} for _, p in ipairs(Players:GetPlayers()) do if p ~= LocalPlayer then t[#t+1] = p end end return t end
local function enemyCheck(p)
    if not Config.TeamCheck then return true end
    return not (p.Team ~= nil and LocalPlayer.Team ~= nil and p.Team == LocalPlayer.Team)
end
local function charParts(p)
    local c = p.Character; if not c then return end
    return c, c:FindFirstChildWhichIsA("Humanoid"), c:FindFirstChild("HumanoidRootPart")
end

-- ---- Invisibility ----
local function setInvisible(char, on)
    if not char then return end
    for _, v in ipairs(char:GetDescendants()) do
        if v:IsA("BasePart") or v:IsA("Decal") then
            if on then if v:GetAttribute("ERA_t") == nil then v:SetAttribute("ERA_t", v.Transparency) end; v.Transparency = 1
            else v.Transparency = v:GetAttribute("ERA_t") or 0 end
        end
    end
end

-- Drawing helper (used by the aimbot FOV circle; guarded if the executor lacks Drawing)
local hasDrawing = Drawing ~= nil
local function newDraw(t, props)
    if not hasDrawing then return nil end
    local ok, d = pcall(function() return Drawing.new(t) end)
    if not ok then return nil end
    if props then for k, v in pairs(props) do pcall(function() d[k] = v end) end end
    return d
end

-- ---- ESP (simple Highlight chams — the proven-working version) ----
local espFolder = new("Folder", { Name = "ERA_ESP" }, gui.Parent or gui)
local highlights = {}   -- player -> Highlight
local function espColorFor(p)
    return (Config.ESPTeamColor and p.TeamColor and p.TeamColor.Color) or Config.ESPColor
end
local function removeEsp(p) if highlights[p] then highlights[p]:Destroy(); highlights[p] = nil end end
local function addEsp(p)
    if p == LocalPlayer or not p.Character then return end
    local hl = highlights[p]
    if hl then hl.Adornee = p.Character; hl.FillColor = espColorFor(p); return end
    highlights[p] = new("Highlight", {
        FillTransparency = 0.5, OutlineTransparency = 0,
        FillColor = espColorFor(p), OutlineColor = Color3.new(1, 1, 1),
        DepthMode = Enum.HighlightDepthMode.AlwaysOnTop, Adornee = p.Character }, espFolder)
end
local function refreshEsp()
    for _, p in ipairs(others()) do if Config.ESPOn then addEsp(p) else removeEsp(p) end end
end
local function clearAllEsp() for p in pairs(highlights) do removeEsp(p) end end
-- light maintenance loop: keeps adornee/colour fresh on respawn & team change
local espConn
espConn = RunService.Heartbeat:Connect(function()
    if not Config.ESPOn then return end
    for _, p in ipairs(others()) do
        if p.Character then addEsp(p) else removeEsp(p) end
    end
end)

-- ---- Aimbot ----
local aiming, aimToggleState, aimMobile, lockedTarget = false, false, false, nil
local aimDbg   -- on-screen debug label (created in the UI section); shows player/target state
local function partOf(p)
    local c = p.Character; if not c then return end
    -- named head/target part first
    local head = c:FindFirstChild(Config.TargetPart) or c:FindFirstChild("Head")
    if head and head:IsA("BasePart") then return head end
    -- custom rig: the TOPMOST part is almost always the head → clean aim point
    local top, topY
    for _, v in ipairs(c:GetDescendants()) do
        if v:IsA("BasePart") and (not topY or v.Position.Y > topY) then top, topY = v, v.Position.Y end
    end
    if top then return top end
    local hum = c:FindFirstChildWhichIsA("Humanoid")
    return (hum and hum.RootPart) or c.PrimaryPart
end
local function isAlive(p)
    -- Targetable if the character is present with a body part. We intentionally do NOT
    -- gate on Humanoid.Health: many games use custom health and keep Humanoid.Health at 0,
    -- which was making the aimbot skip perfectly valid targets. Dead players are removed/
    -- respawned so they won't have parts anyway.
    local c = p.Character
    if not c or not c.Parent then return false end
    local h = c:FindFirstChildWhichIsA("Humanoid")
    if h and h.Health <= 0 and h.MaxHealth > 0 then return false end   -- only skip if it's a real, dead Humanoid
    return c:FindFirstChildWhichIsA("BasePart", true) ~= nil
end
local function visibleTo(part)
    if Config.Wallshot or not Config.VisibleCheck then return true end
    local rp = RaycastParams.new(); rp.FilterType = Enum.RaycastFilterType.Exclude
    rp.FilterDescendantsInstances = { LocalPlayer.Character }
    local from = cam().CFrame.Position
    local dir = part.Position - from
    local hit = Workspace:Raycast(from, dir, rp)
    if not hit then return true end
    if part.Parent and hit.Instance:IsDescendantOf(part.Parent) then return true end
    -- near-miss forgiveness: blocky rigs raycast slightly in front of the aim part
    return hit.Distance >= dir.Magnitude - 6
end
local function aimAt(part)
    if Config.Prediction > 0 then return part.Position + part.AssemblyLinearVelocity * Config.Prediction end
    return part.Position
end
local function pickTarget()
    local C = cam(); local center = Vector2.new(C.ViewportSize.X / 2, C.ViewportSize.Y / 2)
    local closest = Config.AimTarget == "Closest"
    -- sticky: keep the current target if it's still valid & on screen (reduces flicker)
    if Config.StickyTarget and lockedTarget and lockedTarget.Parent and enemyCheck(lockedTarget) and isAlive(lockedTarget) then
        local part = partOf(lockedTarget)
        if part then
            local sp, on = C:WorldToViewportPoint(part.Position)
            if on and visibleTo(part) and (Vector2.new(sp.X, sp.Y) - center).Magnitude <= Config.FOV then
                return part
            end
        end
        lockedTarget = nil
    end
    local best, bestPart, bestScore = nil, nil, math.huge
    for _, p in ipairs(others()) do
        if enemyCheck(p) and isAlive(p) then
            local part = partOf(p)
            if part and visibleTo(part) then
                local sp, on = C:WorldToViewportPoint(part.Position)
                local score
                if on then
                    local sd = (Vector2.new(sp.X, sp.Y) - center).Magnitude
                    if sd <= Config.FOV then
                        -- must be on screen AND inside the FOV circle (the enemy you're aiming near).
                        -- Closest = nearest of those by 3D distance; Crosshair = nearest to the crosshair.
                        score = closest and (C.CFrame.Position - part.Position).Magnitude or sd
                    end
                end
                if score and score < bestScore then best, bestPart, bestScore = p, part, score end
            end
        end
    end
    lockedTarget = best
    return bestPart
end
local fovCircle = newDraw("Circle", { Thickness = 1.6, NumSides = 64, Filled = false, Visible = false })
local rainbowHue = 0
-- Auto Fire: shoots the equipped Tool via Tool:Activate() (works on PC AND mobile) once the
-- crosshair is on the target. Silent Aim flicks the camera onto the head for the shot frame.
local lastFire = 0
local function tryAutoFire(part, C)
    local sp, on = C:WorldToViewportPoint(part.Position)
    if not on then return end
    local centered = (Vector2.new(sp.X, sp.Y) - Vector2.new(C.ViewportSize.X / 2, C.ViewportSize.Y / 2)).Magnitude
    if not Config.SilentAim and centered > 45 then return end   -- wait for the lock unless silent flick
    local now = os.clock()
    if now - lastFire < math.max(0.03, Config.AutoFireDelay) then return end
    lastFire = now
    if Config.SilentAim then C.CFrame = CFrame.new(C.CFrame.Position, aimAt(part)) end  -- flick onto the head
    local char = LocalPlayer.Character
    local tool = char and char:FindFirstChildOfClass("Tool")
    if tool then pcall(function() tool:Activate() end)
    elseif mouse1click then pcall(function() mouse1click() end) end
end
RunService:BindToRenderStep("ERA_Aim", Enum.RenderPriority.Camera.Value + 1, function(dt)
    local C = cam()
    if fovCircle then
        fovCircle.Visible = Config.AimOn and Config.ShowFOV
        fovCircle.Radius = Config.FOV
        if Config.FOVRainbow then rainbowHue = (rainbowHue + dt * 0.2) % 1; fovCircle.Color = Color3.fromHSV(rainbowHue, 0.8, 1)
        else fovCircle.Color = Config.Accent end
        fovCircle.Position = Vector2.new(C.ViewportSize.X / 2, C.ViewportSize.Y / 2)
    end
    if aimDbg then
        local cnt = 0
        for _, p in ipairs(others()) do if p.Character and isAlive(p) then cnt = cnt + 1 end end
        aimDbg.Visible = Config.AimDebug and (Config.AimOn or Config.AutoFire)
        aimDbg.Text = string.format("AIM %s  |  enemies: %d  |  target: %s  |  %s",
            (Config.AimOn or Config.AutoFire) and "ON" or "off", cnt,
            (lockedTarget and lockedTarget.Parent) and lockedTarget.Name or "NONE", Config.AimMethod)
    end
    if not Config.AimOn and not Config.AutoFire then aiming = false; return end
    if Config.AimOn then
        if Config.AimMode == "Always" then aiming = true
        elseif Config.AimMode == "Toggle" then aiming = aimToggleState
        else
            local k = Config.AimKey
            if typeof(k) == "EnumItem" and k.EnumType == Enum.UserInputType then aiming = UIS:IsMouseButtonPressed(k)
            elseif typeof(k) == "EnumItem" and k.EnumType == Enum.KeyCode then aiming = UIS:IsKeyDown(k)
            else aiming = false end
        end
    else aiming = false end
    if aimMobile then aiming = true end   -- mobile AIM button works regardless of the desktop master toggle
    local engage = aiming or Config.AutoFire   -- Auto Fire aims + shoots by itself (rage)
    if not engage then lockedTarget = nil; return end
    local part = pickTarget(); if not part then return end
    local goal = aimAt(part)
    if Config.AimMethod == "Camera" or not mousemoverel then   -- Silent falls back to Camera if the executor lacks mousemoverel
        local a = 1 - (1 - math.clamp(Config.Smoothness, 0.05, 1)) ^ (dt * 60)   -- frame-rate independent lerp
        C.CFrame = C.CFrame:Lerp(CFrame.new(C.CFrame.Position, goal), a)
        pcall(function() C.Focus = CFrame.new(goal) end)   -- some first-person games follow Camera.Focus
    else
        local sp, on = C:WorldToViewportPoint(goal)
        if on then mousemoverel((sp.X - C.ViewportSize.X / 2) * Config.Smoothness, (sp.Y - C.ViewportSize.Y / 2) * Config.Smoothness) end
    end
    if Config.AutoFire then tryAutoFire(part, C) end
end)

-- ---- Anti-Aim (rotates the replicated HumanoidRootPart so enemy aimbots mis-track) ----
local antiAimConn
local function setAntiAim(on)
    Config.AntiAimOn = on
    if on and not antiAimConn then
        antiAimConn = RunService.Heartbeat:Connect(function()
            local _, hum, root = charParts(LocalPlayer)
            if not root or not hum then return end
            if Config.NoClipOn then return end   -- NoClip Fly's gyro owns rotation while flying
            hum.AutoRotate = false
            local look = cam().CFrame.LookVector
            local camYaw = math.atan2(-look.X, -look.Z)   -- where the camera faces
            local t = os.clock()
            local off
            local m = Config.AntiAimMode
            if m == "Spin" then off = (t * Config.AntiAimSpeed) % (math.pi * 2)
            elseif m == "Jitter" then off = (math.floor(t * Config.AntiAimSpeed * 2) % 2 == 0) and 1.2 or -1.2
            elseif m == "Backwards" then off = math.pi
            elseif m == "Left" then off = math.pi / 2
            elseif m == "Right" then off = -math.pi / 2
            else off = 0 end
            root.CFrame = CFrame.new(root.Position) * CFrame.Angles(0, camYaw + off, 0)
        end)
    elseif not on and antiAimConn then
        antiAimConn:Disconnect(); antiAimConn = nil
        local _, hum = charParts(LocalPlayer); if hum then hum.AutoRotate = true end
    end
end

-- ---- Third Person (best-effort: forces classic camera + zoom-out, fights re-locks) ----
local tpConn, tpOrigMode, tpOrigMin, tpOrigMax
local function setThirdPerson(on)
    Config.ThirdPerson = on
    if on then
        if tpOrigMode == nil then
            tpOrigMode = LocalPlayer.CameraMode
            tpOrigMin, tpOrigMax = LocalPlayer.CameraMinZoomDistance, LocalPlayer.CameraMaxZoomDistance
        end
        pcall(function()
            LocalPlayer.CameraMode = Enum.CameraMode.Classic
            LocalPlayer.CameraMinZoomDistance = 0.5
            LocalPlayer.CameraMaxZoomDistance = Config.ThirdPersonZoom
        end)
        if not tpConn then tpConn = RunService.Heartbeat:Connect(function()
            if not Config.ThirdPerson then return end
            if LocalPlayer.CameraMode ~= Enum.CameraMode.Classic then pcall(function() LocalPlayer.CameraMode = Enum.CameraMode.Classic end) end
            if LocalPlayer.CameraMaxZoomDistance < Config.ThirdPersonZoom then pcall(function() LocalPlayer.CameraMaxZoomDistance = Config.ThirdPersonZoom end) end
        end) end
    else
        if tpConn then tpConn:Disconnect(); tpConn = nil end
        if tpOrigMode ~= nil then pcall(function()
            LocalPlayer.CameraMode = tpOrigMode
            LocalPlayer.CameraMinZoomDistance = tpOrigMin
            LocalPlayer.CameraMaxZoomDistance = tpOrigMax
        end) end
    end
end

-- ---- NoClip Fly ----
-- One feature: fly by moving the normal joystick/WASD (direction follows the
-- camera, so looking up/down changes altitude) AND pass through walls. Idle = hover.
local nfBV, nfBG, nfConn, nfClip = nil, nil, nil, {}
local function stopNoClipFly()
    if nfConn then nfConn:Disconnect(); nfConn = nil end
    if nfBV then nfBV:Destroy(); nfBV = nil end
    if nfBG then nfBG:Destroy(); nfBG = nil end
    for part in pairs(nfClip) do if part and part.Parent then part.CanCollide = true end end
    nfClip = {}
    local _, hum = charParts(LocalPlayer); if hum then hum.PlatformStand = false end
end
local function startNoClipFly()
    local c, hum, root = charParts(LocalPlayer)
    if not root then return end
    stopNoClipFly()
    nfBV = new("BodyVelocity", { MaxForce = Vector3.new(1,1,1) * 1e5, Velocity = Vector3.zero, P = 1e4 }, root)
    nfBG = new("BodyGyro", { MaxTorque = Vector3.new(1,1,1) * 4e5, P = 1e4, D = 500, CFrame = cam().CFrame }, root)
    if hum then hum.PlatformStand = true end
    local vel = Vector3.zero
    nfConn = RunService.RenderStepped:Connect(function(dt)
        if not Config.NoClipOn or not nfBV or not nfBG then return end
        local cc = LocalPlayer.Character
        if cc then for _, v in ipairs(cc:GetDescendants()) do if v:IsA("BasePart") and v.CanCollide then nfClip[v] = true; v.CanCollide = false end end end
        local _, hum2 = charParts(LocalPlayer)
        local C = cam()
        local goal = Vector3.zero
        if hum2 and hum2.MoveDirection.Magnitude > 0 then
            -- how much the joystick pushes forward/right relative to the camera (flattened)
            local md = hum2.MoveDirection
            local ff = Vector3.new(C.CFrame.LookVector.X, 0, C.CFrame.LookVector.Z); if ff.Magnitude > 0 then ff = ff.Unit end
            local fr = Vector3.new(C.CFrame.RightVector.X, 0, C.CFrame.RightVector.Z); if fr.Magnitude > 0 then fr = fr.Unit end
            -- rebuild using the FULL camera vectors so pitch (look up/down) controls altitude
            local move = C.CFrame.LookVector * md:Dot(ff) + C.CFrame.RightVector * md:Dot(fr)
            if move.Magnitude > 0 then goal = move.Unit * Config.NoClipSpeed end
        end
        vel = vel:Lerp(goal, math.clamp(dt * 10, 0, 1))
        nfBV.Velocity = vel
        nfBG.CFrame = C.CFrame
    end)
end
local function setNoClip(on)
    Config.NoClipOn = on
    if on then startNoClipFly() else stopNoClipFly() end
end

-- ---- WalkSpeed / JumpPower ----
local moveConn
local function ensureMoveConn()
    if moveConn then return end
    moveConn = RunService.Heartbeat:Connect(function()
        local _, hum = charParts(LocalPlayer)
        if not hum then return end
        if Config.WalkOn then hum.WalkSpeed = Config.WalkSpeed end
        if Config.JumpOn then hum.JumpPower = Config.JumpPower; hum.UseJumpPower = true end
    end)
end

-- ---- Hitbox Expander ----
-- Hitbox expander (client-side aim assist). Size-only, so it stays a LOCAL,
-- non-replicated change to another player's part: nothing visible (no Transparency),
-- no physics change (no CanCollide/Massless) => far less for an anti-cheat to flag.
local hitboxOrig, hitboxConn = {}, nil   -- part -> original Size
local function restoreHitboxes()
    for part, sz in pairs(hitboxOrig) do if part and part.Parent then pcall(function() part.Size = sz end) end end
    hitboxOrig = {}
end
local function setHitbox(on)
    Config.HitboxOn = on
    if on and not hitboxConn then
        hitboxConn = RunService.Heartbeat:Connect(function()
            local want = Vector3.new(Config.HitboxSize, Config.HitboxSize, Config.HitboxSize)
            for _, p in ipairs(others()) do
                if enemyCheck(p) and p.Character then
                    local part = p.Character:FindFirstChild(Config.HitboxPart)
                    if part and part:IsA("BasePart") then
                        if hitboxOrig[part] == nil then hitboxOrig[part] = part.Size end
                        if part.Size ~= want then pcall(function() part.Size = want end) end  -- only write on change
                    end
                end
            end
        end)
    elseif not on and hitboxConn then hitboxConn:Disconnect(); hitboxConn = nil; restoreHitboxes() end
end

-- ---- Infinite Ammo / Fast Fire (best-effort) ----
local AMMO_NAMES = { "Ammo", "ammo", "MagAmmo", "CurrentAmmo", "Bullets", "bullets", "Clip", "Rounds", "Mag" }
local FIRE_NAMES = { "FireRate", "Firerate", "Cooldown", "cooldown", "Delay", "delay", "ShotDelay", "ReloadTime" }
local function eachTool(fn)
    local c = LocalPlayer.Character
    if c then for _, t in ipairs(c:GetChildren()) do if t:IsA("Tool") then fn(t) end end end
    local bp = LocalPlayer:FindFirstChildOfClass("Backpack")
    if bp then for _, t in ipairs(bp:GetChildren()) do if t:IsA("Tool") then fn(t) end end end
end
local ammoConn
local function setInfAmmo(on)
    Config.InfAmmoOn = on
    if on and not ammoConn then
        ammoConn = RunService.Heartbeat:Connect(function()
            eachTool(function(t)
                for _, nm in ipairs(AMMO_NAMES) do
                    local v = t:FindFirstChild(nm) or (t:FindFirstChild("Configuration") and t.Configuration:FindFirstChild(nm))
                    if v and (v:IsA("IntValue") or v:IsA("NumberValue")) then v.Value = 999 end
                    if type(t:GetAttribute(nm)) == "number" then t:SetAttribute(nm, 999) end
                end
            end)
        end)
    elseif not on and ammoConn then ammoConn:Disconnect(); ammoConn = nil end
end
local function setFastFire(on)
    Config.FastFireOn = on
    if on then task.spawn(function()
        while Config.FastFireOn do
            eachTool(function(t)
                for _, nm in ipairs(FIRE_NAMES) do
                    local v = t:FindFirstChild(nm) or (t:FindFirstChild("Configuration") and t.Configuration:FindFirstChild(nm))
                    if v and (v:IsA("IntValue") or v:IsA("NumberValue")) then v.Value = Config.FireDelay end
                    if type(t:GetAttribute(nm)) == "number" then t:SetAttribute(nm, Config.FireDelay) end
                end
            end)
            task.wait(0.4)
        end
    end) end
end

-- ---- Anti-Kick (best-effort; needs executor hooks) ----
local antiKickActive, antiKickInstalled = false, false
local function setAntiKick(on)
    Config.AntiKickOn = on
    antiKickActive = on                       -- runtime gate the hook checks
    if on and not antiKickInstalled then      -- install hooks exactly once
        antiKickInstalled = true
        pcall(function()
            if hookmetamethod and getnamecallmethod then
                local mt_old
                mt_old = hookmetamethod(game, "__namecall", function(self, ...)
                    local method = getnamecallmethod()
                    if antiKickActive and self == LocalPlayer and method == "Kick" then return nil end
                    return mt_old(self, ...)
                end)
            end
        end)
        pcall(function()
            if hookfunction and LocalPlayer.Kick then
                hookfunction(LocalPlayer.Kick, function() while antiKickActive do task.wait(999) end end)
            end
        end)
        if not (hookmetamethod or hookfunction) then
            Notify("Anti-Kick needs an executor with hooks", 3.5, Theme.Bad)
        else Notify("Anti-Kick active (best-effort)", 2, Theme.Good) end
    end
end

-- ============================ CONFIG FILE ======================
local CFG_DIR, CFG_EXT = "ERA_Claude", ".json"
local function cfgPath(name) return CFG_DIR .. "/" .. (name or Config.ConfigName) .. CFG_EXT end
local function serialize()
    local t = {}
    for k, v in pairs(Config) do
        if typeof(v) == "Color3" then t[k] = { __t = "Color3", r = math.floor(v.R*255+0.5), g = math.floor(v.G*255+0.5), b = math.floor(v.B*255+0.5) }
        elseif typeof(v) == "EnumItem" then t[k] = { __t = "Enum", e = tostring(v.EnumType), n = v.Name }
        else t[k] = v end
    end
    return HttpService:JSONEncode(t)
end
local function deserializeInto(json)
    local ok, t = pcall(function() return HttpService:JSONDecode(json) end)
    if not ok or type(t) ~= "table" then return false end
    for k, v in pairs(t) do
        if type(v) == "table" and v.__t == "Color3" then Config[k] = Color3.fromRGB(v.r, v.g, v.b)
        elseif type(v) == "table" and v.__t == "Enum" then
            pcall(function()
                if v.e == "Enum.KeyCode" then Config[k] = Enum.KeyCode[v.n]
                elseif v.e == "Enum.UserInputType" then Config[k] = Enum.UserInputType[v.n] end
            end)
        else Config[k] = v end
    end
    return true
end
local function fileReady() return writefile and readfile and isfile end
local function saveConfig(name)
    if not fileReady() then Notify("No file API in this executor", 2.5, Theme.Bad); return end
    pcall(function() if makefolder and (not (isfolder and isfolder(CFG_DIR))) then makefolder(CFG_DIR) end end)
    local ok = pcall(function() writefile(cfgPath(name), serialize()) end)
    Notify(ok and ("Saved config: " .. (name or Config.ConfigName)) or "Save failed", 2, ok and Theme.Good or Theme.Bad)
end
local function syncWidgets()
    loading = true
    for flag, w in pairs(widgets) do pcall(function() w.set(Config[flag]) end) end
    loading = false
    applyAccent(Config.Accent); uiScale.Scale = Config.UIScale
end
local function loadConfig(name)
    if not fileReady() then Notify("No file API in this executor", 2.5, Theme.Bad); return end
    if not isfile(cfgPath(name)) then Notify("Config not found: " .. (name or Config.ConfigName), 2.5, Theme.Bad); return end
    local ok, data = pcall(function() return readfile(cfgPath(name)) end)
    if ok and deserializeInto(data) then syncWidgets(); Notify("Loaded config: " .. (name or Config.ConfigName), 2, Theme.Good)
    else Notify("Load failed", 2, Theme.Bad) end
end

local aimBtn, ncBtn   -- mobile buttons (created later; forward-declared so Settings toggles can hide them)
-- ============================ BUILD TABS =======================
local combat = addTab("Combat",  "⌖")
local rage   = addTab("Rage",    "🔥")
local weapon = addTab("Weapon",  "▤")
local visual = addTab("Visuals", "◉")
local moveT  = addTab("Movement","➤")
local utilT  = addTab("Utility", "🛡")
local setT   = addTab("Settings","⚙")

-- Combat
section(combat, "Aimbot")
Toggle(combat, "AimBot", "AimOn", function(on) if not on then aimToggleState = false; aimMobile = false end; Notify(on and "Aimbot ON" or "Aimbot OFF", 1.3, on and Theme.Good or Theme.Bad) end)
Dropdown(combat, "Method", "AimMethod", { "Camera", "Silent" }, function(v) if v == "Silent" and not mousemoverel then Notify("Silent needs mousemoverel", 3, Theme.Bad) end end)
Dropdown(combat, "Mode", "AimMode", { "Hold", "Toggle", "Always" }, function() aimToggleState = false end)
Keybind(combat, "Aim Key", "AimKey")
Slider(combat, "FOV", "FOV", 40, 500, 0)
Slider(combat, "Smoothness", "Smoothness", 0.02, 1, 2)
Slider(combat, "Prediction", "Prediction", 0, 0.3, 3)
Dropdown(combat, "Target Part", "TargetPart", { "Head", "HumanoidRootPart", "Torso", "UpperTorso" })
Dropdown(combat, "Target", "AimTarget", { "Closest", "Crosshair" })
section(combat, "Checks")
Toggle(combat, "Team Check", "TeamCheck")
Toggle(combat, "Visible Check", "VisibleCheck")
Toggle(combat, "Wallshot (ignore walls)", "Wallshot")
Toggle(combat, "Sticky Target", "StickyTarget")
Toggle(combat, "Show FOV Circle", "ShowFOV")
Toggle(combat, "FOV Rainbow", "FOVRainbow")
Toggle(combat, "Aim Debug (on-screen)", "AimDebug")
section(combat, "Hitbox")
Toggle(combat, "Hitbox Expander", "HitboxOn", function(on) setHitbox(on) end)
Slider(combat, "Hitbox Size", "HitboxSize", 3, 30, 0)
Dropdown(combat, "Hitbox Part", "HitboxPart", { "HumanoidRootPart", "Head", "Torso" }, function() restoreHitboxes() end)

-- Rage / HvH
section(rage, "Auto Fire")
Toggle(rage, "Auto Fire (shoots what it sees)", "AutoFire")
Toggle(rage, "Silent Aim (flick on shot)", "SilentAim")
Slider(rage, "Fire Delay", "AutoFireDelay", 0.03, 0.6, 3)
hint(rage, "Auto-fires the equipped weapon (Tool:Activate) at the nearest target in FOV. Set FOV/Target Part in the Combat tab. Client-hit games (like this one) register the head shot.")
section(rage, "Anti-Aim")
Toggle(rage, "Anti-Aim", "AntiAimOn", function(on) setAntiAim(on) end)
Dropdown(rage, "Mode", "AntiAimMode", { "Spin", "Backwards", "Jitter", "Left", "Right" })
Slider(rage, "Spin Speed", "AntiAimSpeed", 2, 60, 0)
hint(rage, "Rotates your character so enemy aimbots mis-track. Fights the game's own rotation — may look jittery and can affect movement. Game-dependent.")
section(rage, "Camera")
Toggle(rage, "Third Person", "ThirdPerson", function(on) setThirdPerson(on) end)
Slider(rage, "3rd Person Zoom", "ThirdPersonZoom", 5, 30, 0)

-- Weapon
section(weapon, "Best-effort (game-dependent)")
hint(weapon, "Only works if the game keeps ammo/fire values on the client. Server-authoritative games ignore these.")
Toggle(weapon, "Infinite Ammo", "InfAmmoOn", function(on) setInfAmmo(on) end)
Toggle(weapon, "Fast Fire", "FastFireOn", function(on) setFastFire(on) end)
Slider(weapon, "Fire Delay", "FireDelay", 0.0, 0.3, 3)

-- Visuals
section(visual, "ESP")
Toggle(visual, "Enable ESP", "ESPOn", function(on) if on then refreshEsp() else clearAllEsp() end end)
Toggle(visual, "Team Color", "ESPTeamColor", function() for _, p in ipairs(others()) do if highlights[p] then highlights[p].FillColor = espColorFor(p) end end end)
hint(visual, "Chams highlight through walls. Colour = team colour (toggle off for a fixed colour).")
section(visual, "Self")
Toggle(visual, "Invisibility", "InvisOn", function(on) if LocalPlayer.Character then setInvisible(LocalPlayer.Character, on) end end)

-- Movement
section(moveT, "NoClip Fly")
Toggle(moveT, "NoClip Fly", "NoClipOn", function(on) setNoClip(on) end)
Slider(moveT, "Fly Speed", "NoClipSpeed", 16, 300, 0)
hint(moveT, "Fly by moving your normal joystick / WASD — you go where the camera looks (look up/down to change height), and pass through walls. Idle = hover. Mobile: one NOCLIP button (hold 3s to move it).")
section(moveT, "Speed / Jump")
Toggle(moveT, "WalkSpeed", "WalkOn", function() ensureMoveConn() end)
Slider(moveT, "WalkSpeed value", "WalkSpeed", 16, 200, 0)
Toggle(moveT, "JumpPower", "JumpOn", function() ensureMoveConn() end)
Slider(moveT, "JumpPower value", "JumpPower", 50, 300, 0)

-- Utility
section(utilT, "Protection")
Toggle(utilT, "Anti-Kick", "AntiKickOn", function(on) setAntiKick(on) end)
hint(utilT, "Best-effort: blocks client Kick namecalls. Needs an executor with hookmetamethod; server bans are not affected.")

-- Settings
section(setT, "Interface")
Keybind(setT, "Menu Key", "MenuKey", function(k) if typeof(k) == "EnumItem" and k.EnumType == Enum.KeyCode then Config.MenuKey = k end end)
Toggle(setT, "UI Sounds", "UISounds")
Toggle(setT, "Background Blur", "MenuBlur", function(on) if not on then tween(menuBlur, { Size = 0 }, 0.2) elseif window.Visible then tween(menuBlur, { Size = 14 }, 0.2) end end)
Toggle(setT, "Watermark Animation", "WatermarkAnim")
section(setT, "Mobile Buttons")
Toggle(setT, "Show AIM Button", "ShowAimBtn", function(on) if aimBtn then aimBtn.Visible = on end end)
Toggle(setT, "Show NOCLIP Button", "ShowNoClipBtn", function(on) if ncBtn then ncBtn.Visible = on end end)
hint(setT, "Menu opens by tapping the watermark (top-left).")
local accents = { { "Claude 🦀", Color3.fromRGB(217,119,87) }, { "Blue", Color3.fromRGB(0,170,255) },
    { "Purple", Color3.fromRGB(150,90,255) }, { "Pink", Color3.fromRGB(255,45,120) }, { "Green", Color3.fromRGB(46,204,113) } }
local accRow = row(setT, 44)
new("TextLabel", { Size = UDim2.new(0, 90, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
    Text = "Accent", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
    Font = Enum.Font.GothamMedium, TextSize = 14 }, accRow)
for idx, a in ipairs(accents) do
    local sw = new("TextButton", { Size = UDim2.new(0, 24, 0, 24), Position = UDim2.new(0, 104 + (idx-1)*30, 0.5, -12),
        BackgroundColor3 = a[2], Text = "", AutoButtonColor = false }, accRow)
    corner(sw, 12); stroke(sw, Theme.Stroke, 1, 0.7)
    sw.MouseButton1Click:Connect(function() applyAccent(a[2]); playSound("toggle"); Notify("Accent: " .. a[1], 1.1, a[2]) end)
end
local uiScaleSlider = Slider(setT, "UI Scale", "UIScale", 0.7, 1.3, 2, function(v) uiScale.Scale = v end)
section(setT, "Config System")
Button(setT, "💾  Save Config", Theme.Panel, function() saveConfig() end)
Button(setT, "📂  Load Config", Theme.Panel, function() loadConfig() end)
if not fileReady() then hint(setT, "This executor has no file API (writefile/readfile) — config save/load unavailable.") end
section(setT, "Session")
Button(setT, "Unload ERA", Theme.Bad, function() ERA_UNLOAD() end)

-- ============================ WATERMARK ========================
local wm = new("Frame", { Position = UDim2.new(0, 20, 0, 20), Size = UDim2.new(0, 300, 0, 32),
    BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, gui)
corner(wm, 9); stroke(wm, Theme.Stroke, 1, 0.86)
local wmBar = accent(new("Frame", { Size = UDim2.new(0, 3, 1, -12), Position = UDim2.new(0, 7, 0, 6), BorderSizePixel = 0 }, wm), "BackgroundColor3")
gradient(wmBar, Config.Accent, Color3.fromRGB(255, 200, 150), 90)
local wmCrab = new("TextLabel", { Size = UDim2.new(0, 20, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
    Text = "🦀", Font = Enum.Font.GothamBold, TextSize = 14 }, wm)
local wmTxt = new("TextLabel", { Size = UDim2.new(1, -40, 1, 0), Position = UDim2.new(0, 36, 0, 0), BackgroundTransparency = 1,
    Text = "ERA v9", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
    Font = Enum.Font.GothamMedium, TextSize = 12 }, wm)
makeDraggable(wm)
-- On-screen aim debug (shows whether the bot sees enemies / picks a target)
aimDbg = new("TextLabel", { Name = "AimDbg", AnchorPoint = Vector2.new(0.5, 0), Position = UDim2.new(0.5, 0, 0, 6),
    Size = UDim2.new(0, 380, 0, 22), BackgroundColor3 = Theme.Bg2, BackgroundTransparency = 0.2,
    Text = "AIM", TextColor3 = Theme.Text, Font = Enum.Font.GothamMedium, TextSize = 12, Visible = false }, gui)
corner(aimDbg, 6); stroke(aimDbg, Theme.Stroke, 1, 0.85)
local wmFrameConn
task.spawn(function()
    local frames = 0
    wmFrameConn = RunService.RenderStepped:Connect(function() frames += 1 end)
    while gui.Parent do
        task.wait(1)
        local fps = frames; frames = 0
        local ping = "-"
        pcall(function() if Stats and Stats.Network and Stats.Network.ServerStatsItem then
            ping = math.floor(Stats.Network.ServerStatsItem["Data Ping"]:GetValue()) .. "ms" end end)
        wmTxt.Text = string.format("ERA v9  •  %d fps  •  %s  •  %s", fps, ping, LocalPlayer.Name)
        if Config.WatermarkAnim then tween(wmCrab, { Rotation = 12 }, 0.3); task.wait(0.15); tween(wmCrab, { Rotation = -12 }, 0.3) end
    end
end)

-- (No floating button — the menu opens by tapping the watermark, wired below.)

-- ============================ MOBILE CONTROLS ==================
-- AIM (hold to aim) + NOCLIP (tap to toggle).
-- Move a button: open the menu and drag it, or (tap buttons) hold it 3 seconds.
local mob = new("Frame", { AnchorPoint = Vector2.new(1, 1), Position = UDim2.new(1, -14, 1, -20),
    Size = UDim2.new(0, 210, 0, 210), BackgroundTransparency = 1, Visible = isTouch() }, gui)
local function mobButton(txt, pos, base)
    local b = new("TextButton", { Size = UDim2.new(0, 60, 0, 60), Position = pos, BackgroundColor3 = base or Theme.Bg2,
        Text = txt, TextColor3 = Theme.Text, Font = Enum.Font.GothamBold, TextSize = 13, AutoButtonColor = false,
        BackgroundTransparency = 0.12 }, mob)
    corner(b, 30); stroke(b, Theme.Stroke, 1, 0.7)
    return b
end
-- generic: onHold/onRelease for hold-buttons, onTap for tap-buttons; 3s hold => drag
local function bindMobile(btn, opts)
    local startP, startBtnP, dragMode, active, moved
    btn.InputBegan:Connect(function(i)
        if i.UserInputType ~= Enum.UserInputType.Touch and i.UserInputType ~= Enum.UserInputType.MouseButton1 then return end
        startP, startBtnP, dragMode, active, moved = i.Position, btn.Position, false, true, false
        if window.Visible then
            -- menu open = edit/move mode: any button drags immediately (no aim/toggle)
            dragMode = true
            tween(btn, { BackgroundColor3 = Config.Accent, BackgroundTransparency = 0 }, 0.12)
        else
            if opts.onHold then opts.onHold() end
            if not opts.onHold then          -- tap buttons: also enter drag by holding 3s
                task.delay(3, function()
                    if active and not moved and not window.Visible then
                        dragMode = true
                        tween(btn, { BackgroundColor3 = Config.Accent, BackgroundTransparency = 0 }, 0.15)
                        Notify("Move the button, then release", 1.5, Config.Accent)
                    end
                end)
            end
        end
    end)
    UIS.InputChanged:Connect(function(i)
        if not active then return end
        if i.UserInputType ~= Enum.UserInputType.Touch and i.UserInputType ~= Enum.UserInputType.MouseMovement then return end
        local d = i.Position - startP
        if d.Magnitude > 16 then moved = true end   -- tolerant of touch jitter so a tap still counts
        if dragMode then btn.Position = UDim2.new(startBtnP.X.Scale, startBtnP.X.Offset + d.X, startBtnP.Y.Scale, startBtnP.Y.Offset + d.Y) end
    end)
    btn.InputEnded:Connect(function(i)
        if i.UserInputType ~= Enum.UserInputType.Touch and i.UserInputType ~= Enum.UserInputType.MouseButton1 then return end
        active = false
        if dragMode then dragMode = false; tween(btn, { BackgroundColor3 = opts.base or Theme.Bg2, BackgroundTransparency = 0.12 }, 0.15)
        else
            if opts.onRelease then opts.onRelease() end
            if not moved and opts.onTap then opts.onTap() end
        end
    end)
end
aimBtn = mobButton("AIM", UDim2.new(1, -66, 1, -66), Config.Accent)
ncBtn  = mobButton("NOCLIP", UDim2.new(1, -136, 1, -66), Theme.Bg2)
aimBtn.Visible = Config.ShowAimBtn
ncBtn.Visible  = Config.ShowNoClipBtn
-- Mobile AIM: a simple TAP-TOGGLE that drives `aimMobile` (works in any Aim Mode).
-- Tap once = aim on, tap again = off. It also auto-enables the master AimBot toggle.
bindMobile(aimBtn, { base = Config.Accent, onTap = function()
    if not Config.AimOn and widgets.AimOn then widgets.AimOn.set(true) end
    aimMobile = not aimMobile
    aimBtn.BackgroundTransparency = aimMobile and 0 or 0.12
    Notify(aimMobile and "Aim ON" or "Aim OFF", 1.2, aimMobile and Theme.Good or Theme.Bad)
end })
bindMobile(ncBtn,  { base = Theme.Bg2, onTap = function() local w = widgets.NoClipOn; if w then w.set(not Config.NoClipOn) end end })

-- ============================ OPEN / CLOSE =====================
local menuOpen = false
local function setMenu(o)
    menuOpen = o
    if o then
        playSound("open")
        window.Visible = true
        uiScale.Scale = Config.UIScale * 0.9; window.BackgroundTransparency = 1
        tween(uiScale, { Scale = Config.UIScale }, 0.24, Enum.EasingStyle.Back)
        tween(window, { BackgroundTransparency = 0 }, 0.18)
        if Config.MenuBlur then tween(menuBlur, { Size = 14 }, 0.25) end
    else
        playSound("close")
        if activeDropdown then activeDropdown() end
        tween(menuBlur, { Size = 0 }, 0.2)
        tween(uiScale, { Scale = Config.UIScale * 0.94 }, 0.14)
        local t = tween(window, { BackgroundTransparency = 1 }, 0.14)
        t.Completed:Connect(function() if not menuOpen then window.Visible = false; uiScale.Scale = Config.UIScale end end)
    end
end
closeBtn.MouseButton1Click:Connect(function() setMenu(false) end)
-- Open the menu by TAPPING the watermark (tap vs drag disambiguated).
do
    local downP, moved
    wm.InputBegan:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then
            downP, moved = i.Position, false
        end
    end)
    UIS.InputChanged:Connect(function(i)
        if downP and (i.UserInputType == Enum.UserInputType.MouseMovement or i.UserInputType == Enum.UserInputType.Touch) then
            if (i.Position - downP).Magnitude > 6 then moved = true end
        end
    end)
    wm.InputEnded:Connect(function(i)
        if (i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch) and downP then
            if not moved then setMenu(not menuOpen) end
            downP = nil
        end
    end)
end
UIS.InputBegan:Connect(function(i, gp)
    -- Aim toggle must fire even when the game consumes the button (e.g. RMB = aim-down-sights,
    -- which makes gameProcessed=true). So handle it BEFORE the gp early-return.
    if Config.AimOn and Config.AimMode == "Toggle" then
        local k = (i.KeyCode ~= Enum.KeyCode.Unknown and i.KeyCode) or i.UserInputType
        if k == Config.AimKey then
            aimToggleState = not aimToggleState
            Notify(aimToggleState and "Aim ON" or "Aim OFF", 1, aimToggleState and Theme.Good or Theme.Bad)
        end
    end
    if gp then return end
    if i.KeyCode == Config.MenuKey then setMenu(not menuOpen) end
end)

-- ============================ UNLOAD ==========================
function ERA_UNLOAD()
    pcall(function() RunService:UnbindFromRenderStep("ERA_Aim") end)
    Config.ESPOn = false
    if espConn then espConn:Disconnect() end
    pcall(function() espFolder:Destroy() end)
    setNoClip(false); setHitbox(false); setInfAmmo(false); Config.FastFireOn = false; antiKickActive = false
    Config.AutoFire = false; setAntiAim(false); setThirdPerson(false)
    if moveConn then moveConn:Disconnect() end
    if wmFrameConn then wmFrameConn:Disconnect() end
    clearAllEsp()
    if fovCircle then pcall(function() fovCircle:Remove() end) end
    pcall(function() menuBlur:Destroy() end)
    if LocalPlayer.Character then setInvisible(LocalPlayer.Character, false) end
    Notify("ERA unloaded 🦀", 1.4)
    task.delay(0.5, function() if gui then gui:Destroy() end end)
end

-- ============================ HOOKS ===========================
local function hookPlayer(p)
    if p == LocalPlayer then return end
    p.CharacterAdded:Connect(function() task.wait(0.4); if Config.ESPOn then addEsp(p) end end)
    p.CharacterRemoving:Connect(function() removeEsp(p) end)
end
for _, p in ipairs(Players:GetPlayers()) do hookPlayer(p) end
Players.PlayerAdded:Connect(hookPlayer)
Players.PlayerRemoving:Connect(function(p) removeEsp(p) end)

LocalPlayer.CharacterAdded:Connect(function(char)
    task.wait(0.5)
    if Config.InvisOn then setInvisible(char, true) end
    if Config.NoClipOn then setNoClip(true) end
    if Config.WalkOn or Config.JumpOn then ensureMoveConn() end
end)

-- auto-load config on start (before responsive scale)
local loadedConfig = false
pcall(function() if fileReady() and isfile(cfgPath("default")) then loadedConfig = deserializeInto(readfile(cfgPath("default"))); syncWidgets() end end)

-- Enforce reliable HvH capture settings so a stale saved config can't break target capture.
loading = true
for k, v in pairs({ TeamCheck = false, VisibleCheck = false, AimTarget = "Closest" }) do
    Config[k] = v
    if widgets[k] then pcall(function() widgets[k].set(v) end) end
end
loading = false

-- responsive first scale (only when no saved config, so a saved UI Scale persists)
if not loadedConfig then
    local vp = cam().ViewportSize
    uiScaleSlider.set(math.clamp(math.min(vp.X / 660, vp.Y / 480), 0.7, 1))
end

Notify("ERA v9 🦀 loaded — tap the watermark (top-left) or press " .. (typeof(Config.MenuKey) == "EnumItem" and Config.MenuKey.Name or "MenuKey"), 4, Config.Accent)
if Config.AutoFire then Notify("🔥 Auto Fire is ON (Rage tab) — it aims & shoots enemies in FOV", 5, Theme.Bad) end
print("[ERA] v9 Claude edition loaded 🦀")
