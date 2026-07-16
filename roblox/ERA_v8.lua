--[[  ERA Roblox Script v8.0  ==================================================
    Redesigned UI + reworked Fly + working Aimbot (Camera-lock & Silent/mouse)
    + Hitbox Expander + best-effort Infinite Ammo / Fast Fire / Wallshot.
    Mobile + PC.  Drop-in.  Delta / any Drawing-capable executor.

    RELIABILITY NOTE (read once):
      * Aimbot, Fly, Hitbox Expander, ESP, Invis, NoClip  -> pure client-side, reliable.
      * Infinite Ammo / Fast Fire                         -> GAME-DEPENDENT best-effort.
        They scan common Tool/value patterns; server-authoritative games ignore them.
      * Wallshot = aimbot targets through walls (skips the visibility ray). Whether the
        SHOT lands through the wall still depends on the game's hit registration.
===============================================================================]]

local Players      = game:GetService("Players")
local RunService   = game:GetService("RunService")
local UIS          = game:GetService("UserInputService")
local TweenService = game:GetService("TweenService")
local Workspace    = workspace
local Stats        = game:FindService("Stats")
local LocalPlayer  = Players.LocalPlayer
local function cam() return Workspace.CurrentCamera end

-- ============================ CONFIG ============================
local Config = {
    -- Aimbot
    AimMethod    = "Camera",           -- "Camera" | "Silent"
    AimMode      = "Hold",             -- "Hold" | "Toggle"
    AimKey       = Enum.UserInputType.MouseButton2, -- hold key (PC). Mobile uses on-screen btn.
    FOV          = 140,
    Smoothness   = 0.22,               -- 0.05 slow/smooth .. 0.6 snappy
    TargetPart   = "Head",             -- Head | HumanoidRootPart | Torso | UpperTorso
    TeamCheck    = true,
    Wallshot     = false,              -- target through walls (skip visibility ray)
    Prediction   = 0.0,                -- velocity lead (0 = off)
    StickyTarget = true,
    ShowFOV      = true,
    -- Visuals
    ESPColor     = Color3.fromRGB(255, 45, 85),
    ESPTeamColor = true,
    ESPBox       = false,
    -- Movement
    FlySpeed     = 60,
    NoClip       = false,
    -- Weapon (best-effort)
    InfAmmo      = false,
    FastFire     = false,
    FireDelay    = 0.03,
    -- Hitbox
    HitboxSize   = 10,
    HitboxPart   = "HumanoidRootPart",
    -- Interface
    Accent       = Color3.fromRGB(0, 170, 255),
    MenuKey      = Enum.KeyCode.RightControl,
    UIScale      = 1,
}

local State = { Aim = false, ESP = false, Invis = false, Fly = false,
                Hitbox = false, InfAmmo = false, FastFire = false }

-- ============================ THEME ============================
local Theme = {
    Bg = Color3.fromRGB(17, 18, 23), Bg2 = Color3.fromRGB(25, 27, 34),
    Panel = Color3.fromRGB(32, 35, 44), PanelHi = Color3.fromRGB(40, 44, 55),
    Track = Color3.fromRGB(57, 61, 74), Text = Color3.fromRGB(236, 238, 244),
    Sub = Color3.fromRGB(150, 156, 170), Stroke = Color3.fromRGB(255, 255, 255),
    Good = Color3.fromRGB(46, 204, 113), Bad = Color3.fromRGB(231, 76, 60),
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

-- theme-tracked accent objects (so changing accent recolors everything)
local accentObjects = {}   -- { obj, prop }
local function accent(obj, prop) obj[prop] = Config.Accent; table.insert(accentObjects, { obj, prop }); return obj end
local Tabs, currentTab   -- forward-declared so applyAccent can recolor tabs/scrollbars
local function applyAccent(col)
    Config.Accent = col
    for _, e in ipairs(accentObjects) do if e[1] and e[1].Parent then pcall(function() e[1][e[2]] = col end) end end
    if Tabs then for _, t in ipairs(Tabs) do
        if t == currentTab then t.icon.TextColor3 = col end
        pcall(function() t.page.ScrollBarImageColor3 = col end)
    end end
end

-- ============================ ROOT GUI =========================
local function mountGui()
    local g = new("ScreenGui", { Name = "ERA_v8", ResetOnSpawn = false, IgnoreGuiInset = true,
        ZIndexBehavior = Enum.ZIndexBehavior.Sibling, DisplayOrder = 999 })
    local ok = pcall(function() g.Parent = (gethui and gethui()) or game:GetService("CoreGui") end)
    if not ok or not g.Parent then pcall(function() g.Parent = LocalPlayer:WaitForChild("PlayerGui") end) end
    return g
end
local gui = mountGui()

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
local notifHolder = new("Frame", { AnchorPoint = Vector2.new(1, 1), Position = UDim2.new(1, -16, 1, -16),
    Size = UDim2.new(0, 250, 1, -32), BackgroundTransparency = 1 }, gui)
new("UIListLayout", { Padding = UDim.new(0, 8), HorizontalAlignment = Enum.HorizontalAlignment.Right,
    VerticalAlignment = Enum.VerticalAlignment.Bottom, SortOrder = Enum.SortOrder.LayoutOrder }, notifHolder)
local function Notify(text, dur, col)
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
    tween(card, { BackgroundTransparency = 0.05 }, 0.2)
    task.delay(dur, function()
        if card and card.Parent then tween(card, { BackgroundTransparency = 1 }, 0.25); task.wait(0.26); card:Destroy() end
    end)
end

-- ============================ WINDOW ===========================
local WIN_W, WIN_H, RAIL_W = 560, 388, 138
local window = new("Frame", { Name = "Window", AnchorPoint = Vector2.new(0.5, 0.5),
    Position = UDim2.new(0.5, 0, 0.5, 0), Size = UDim2.new(0, WIN_W, 0, WIN_H),
    BackgroundColor3 = Theme.Bg, BorderSizePixel = 0, Visible = false }, gui)
corner(window, 14); stroke(window, Theme.Stroke, 1, 0.86)
local uiScale = new("UIScale", { Scale = 1 }, window)   -- single UIScale (responsive + open pop)

new("ImageLabel", { Size = UDim2.new(1, 40, 1, 40), Position = UDim2.new(0, -20, 0, -20),
    BackgroundTransparency = 1, Image = "rbxassetid://6014261993", ImageColor3 = Color3.new(0, 0, 0),
    ImageTransparency = 0.4, ScaleType = Enum.ScaleType.Slice, SliceCenter = Rect.new(49, 49, 450, 450),
    ZIndex = 0 }, window)

-- title bar
local titleBar = new("Frame", { Size = UDim2.new(1, 0, 0, 44), BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, window)
corner(titleBar, 14)
new("Frame", { Size = UDim2.new(1, 0, 0, 16), Position = UDim2.new(0, 0, 1, -16),
    BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, titleBar)
local logoDot = accent(new("Frame", { Size = UDim2.new(0, 10, 0, 10), Position = UDim2.new(0, 16, 0.5, -5),
    BorderSizePixel = 0 }, titleBar), "BackgroundColor3")
corner(logoDot, 5)
new("TextLabel", { Size = UDim2.new(0, 60, 1, 0), Position = UDim2.new(0, 34, 0, 0), BackgroundTransparency = 1,
    Text = "ERA", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
    Font = Enum.Font.GothamBold, TextSize = 17 }, titleBar)
new("TextLabel", { Size = UDim2.new(0, 60, 1, 0), Position = UDim2.new(0, 74, 0, 0), BackgroundTransparency = 1,
    Text = "v8.0", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Sub,
    Font = Enum.Font.GothamMedium, TextSize = 12 }, titleBar)
local closeBtn = new("TextButton", { Size = UDim2.new(0, 28, 0, 28), Position = UDim2.new(1, -38, 0.5, -14),
    BackgroundColor3 = Theme.Panel, Text = "✕", TextColor3 = Theme.Text, Font = Enum.Font.GothamBold,
    TextSize = 13, AutoButtonColor = false }, titleBar)
corner(closeBtn, 8)
makeDraggable(window, titleBar)

-- rail
local rail = new("Frame", { Size = UDim2.new(0, RAIL_W, 1, -44), Position = UDim2.new(0, 0, 0, 44),
    BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, window)
new("Frame", { Size = UDim2.new(0, 1, 1, -16), Position = UDim2.new(1, -1, 0, 8), BackgroundColor3 = Theme.Stroke,
    BackgroundTransparency = 0.9, BorderSizePixel = 0 }, rail)
local railList = new("Frame", { Size = UDim2.new(1, 0, 1, -44), Position = UDim2.new(0, 0, 0, 8),
    BackgroundTransparency = 1 }, rail)
new("UIListLayout", { Padding = UDim.new(0, 6), SortOrder = Enum.SortOrder.LayoutOrder,
    HorizontalAlignment = Enum.HorizontalAlignment.Center }, railList)
new("TextLabel", { Size = UDim2.new(1, -16, 0, 28), Position = UDim2.new(0, 8, 1, -34), BackgroundTransparency = 1,
    Text = "made with ♥", TextColor3 = Theme.Sub, Font = Enum.Font.Gotham, TextSize = 11 }, rail)

local content = new("Frame", { Size = UDim2.new(1, -RAIL_W, 1, -44), Position = UDim2.new(0, RAIL_W, 0, 44),
    BackgroundTransparency = 1 }, window)

-- ============================ TAB SYSTEM =======================
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
    local btn = new("TextButton", { Size = UDim2.new(1, -16, 0, 38), BackgroundColor3 = Theme.Bg2, Text = "",
        AutoButtonColor = false, LayoutOrder = #Tabs + 1 }, railList)
    corner(btn, 9)
    local indic = accent(new("Frame", { Size = UDim2.new(0, 3, 0, 18), Position = UDim2.new(0, 0, 0.5, -9),
        BorderSizePixel = 0, Visible = false }, btn), "BackgroundColor3")
    corner(indic, 2)
    local ico = new("TextLabel", { Size = UDim2.new(0, 24, 1, 0), Position = UDim2.new(0, 12, 0, 0),
        BackgroundTransparency = 1, Text = icon or "•", TextColor3 = Theme.Sub, Font = Enum.Font.GothamBold, TextSize = 15 }, btn)
    local lbl = new("TextLabel", { Size = UDim2.new(1, -42, 1, 0), Position = UDim2.new(0, 40, 0, 0),
        BackgroundTransparency = 1, Text = name, TextXAlignment = Enum.TextXAlignment.Left,
        TextColor3 = Theme.Sub, Font = Enum.Font.GothamMedium, TextSize = 14 }, btn)
    local page = new("ScrollingFrame", { Size = UDim2.new(1, 0, 1, 0), BackgroundTransparency = 1, Visible = false,
        BorderSizePixel = 0, ScrollBarThickness = 3, ScrollBarImageColor3 = Config.Accent,
        ScrollBarImageTransparency = 0.5, CanvasSize = UDim2.new(0, 0, 0, 0),
        AutomaticCanvasSize = Enum.AutomaticSize.Y, ScrollingDirection = Enum.ScrollingDirection.Y }, content)
    new("UIListLayout", { Padding = UDim.new(0, 9), SortOrder = Enum.SortOrder.LayoutOrder }, page)
    new("UIPadding", { PaddingTop = UDim.new(0, 12), PaddingBottom = UDim.new(0, 12),
        PaddingLeft = UDim.new(0, 14), PaddingRight = UDim.new(0, 12) }, page)
    local tab = { btn = btn, page = page, icon = ico, label = lbl, indic = indic }
    table.insert(Tabs, tab)
    btn.MouseButton1Click:Connect(function() selectTab(tab) end)
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
local function row(page, h)
    local r = new("Frame", { Size = UDim2.new(1, 0, 0, h or 44), BackgroundColor3 = Theme.Panel,
        BorderSizePixel = 0, LayoutOrder = nextOrder() }, page)
    corner(r, 10); stroke(r, Theme.Stroke, 1, 0.94)
    return r
end

local function Toggle(page, text, default, cb)
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
    local st = default and true or false
    local function render(anim)
        local tc = st and Config.Accent or Theme.Track
        local kp = st and UDim2.new(1, -21, 0.5, -9) or UDim2.new(0, 3, 0.5, -9)
        if anim then tween(track, { BackgroundColor3 = tc }, 0.16); tween(knob, { Position = kp }, 0.16, Enum.EasingStyle.Back)
        else track.BackgroundColor3 = tc; knob.Position = kp end
    end
    render(false)
    local api = {}
    function api.set(v, silent) st = v and true or false; render(true); if not silent then cb(st) end end
    function api.get() return st end
    local hit = new("TextButton", { Size = UDim2.new(1, 0, 1, 0), BackgroundTransparency = 1, Text = "" }, r)
    hit.MouseButton1Click:Connect(function() api.set(not st) end)
    r.MouseEnter:Connect(function() tween(r, { BackgroundColor3 = Theme.PanelHi }, 0.12) end)
    r.MouseLeave:Connect(function() tween(r, { BackgroundColor3 = Theme.Panel }, 0.12) end)
    if default then cb(true) end
    return api
end

local function Slider(page, text, min, max, default, decimals, cb)
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
    local value = default
    local function apply(v, anim)
        v = math.clamp(v, min, max); value = v
        local a = (v - min) / (max - min)
        if anim then tween(fill, { Size = UDim2.new(a, 0, 1, 0) }, 0.08) else fill.Size = UDim2.new(a, 0, 1, 0) end
        val.Text = fmt(v); cb(v)
    end
    apply(default, false)
    local function fromX(x) apply(min + (max - min) * math.clamp((x - bar.AbsolutePosition.X) / math.max(1, bar.AbsoluteSize.X), 0, 1), false) end
    local hit = new("TextButton", { Size = UDim2.new(1, 0, 0, 28), Position = UDim2.new(0, 0, 0, 26),
        BackgroundTransparency = 1, Text = "" }, r)
    hit.InputBegan:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then
            activeSlider = fromX; fromX(i.Position.X)
        end
    end)
    return { set = function(v) apply(v, true) end, get = function() return value end }
end

local activeDropdown   -- close() fn of the currently-open dropdown (shared, mutually exclusive)
local function Dropdown(page, text, options, default, cb)
    local r = row(page, 44)
    new("TextLabel", { Size = UDim2.new(0.5, -14, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
        Text = text, TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 14 }, r)
    local sel = new("TextButton", { Size = UDim2.new(0.5, -24, 0, 30), Position = UDim2.new(0.5, 10, 0.5, -15),
        BackgroundColor3 = Theme.Bg2, Text = tostring(default) .. "  ▾", TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 13, AutoButtonColor = false, ZIndex = 3 }, r)
    corner(sel, 8); stroke(sel, Theme.Stroke, 1, 0.92)
    local listFrame, current = nil, default
    local function close()
        if listFrame then listFrame:Destroy(); listFrame = nil end
        if activeDropdown == close then activeDropdown = nil end
    end
    sel.MouseButton1Click:Connect(function()
        if listFrame then close(); return end
        if activeDropdown then activeDropdown() end          -- close any other open dropdown
        activeDropdown = close
        -- parent to top-level gui so the ScrollingFrame can't clip it and sibling rows can't cover it
        local ap, sz = sel.AbsolutePosition, sel.AbsoluteSize
        listFrame = new("Frame", { Size = UDim2.fromOffset(sz.X, #options * 30 + 8),
            Position = UDim2.fromOffset(ap.X, ap.Y + sz.Y + 2),
            BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0, ZIndex = 50 }, gui)
        corner(listFrame, 8); stroke(listFrame, Theme.Stroke, 1, 0.9)
        new("UIListLayout", { Padding = UDim.new(0, 2), SortOrder = Enum.SortOrder.LayoutOrder }, listFrame)
        new("UIPadding", { PaddingTop = UDim.new(0, 4), PaddingBottom = UDim.new(0, 4),
            PaddingLeft = UDim.new(0, 4), PaddingRight = UDim.new(0, 4) }, listFrame)
        for _, opt in ipairs(options) do
            local ob = new("TextButton", { Size = UDim2.new(1, 0, 0, 28),
                BackgroundColor3 = opt == current and Theme.Panel or Theme.Bg2, Text = tostring(opt),
                TextColor3 = opt == current and Config.Accent or Theme.Text, Font = Enum.Font.GothamMedium,
                TextSize = 13, AutoButtonColor = false, ZIndex = 51 }, listFrame)
            corner(ob, 6)
            ob.MouseButton1Click:Connect(function() current = opt; sel.Text = tostring(opt) .. "  ▾"; cb(opt); close() end)
        end
    end)
    return { get = function() return current end, close = close }
end
-- outside-click / touch dismiss for the open dropdown (button/option clicks are gameProcessed)
UIS.InputBegan:Connect(function(i, gp)
    if gp or not activeDropdown then return end
    if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then
        activeDropdown()
    end
end)

local function Keybind(page, text, defaultKey, cb)
    local r = row(page, 44)
    new("TextLabel", { Size = UDim2.new(1, -120, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
        Text = text, TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
        Font = Enum.Font.GothamMedium, TextSize = 14 }, r)
    local key = defaultKey
    local function keyName(k)
        if typeof(k) == "EnumItem" then return k.Name end
        return "None"
    end
    local kbtn = new("TextButton", { Size = UDim2.new(0, 100, 0, 28), Position = UDim2.new(1, -112, 0.5, -14),
        BackgroundColor3 = Theme.Bg2, Text = keyName(key), TextColor3 = Theme.Text, Font = Enum.Font.GothamMedium,
        TextSize = 12, AutoButtonColor = false }, r)
    corner(kbtn, 8); stroke(kbtn, Theme.Stroke, 1, 0.92)
    local listening = false
    kbtn.MouseButton1Click:Connect(function() listening = true; kbtn.Text = "..."; tween(kbtn, { BackgroundColor3 = Config.Accent }, 0.12) end)
    UIS.InputBegan:Connect(function(i)
        if not listening then return end
        local k
        if i.KeyCode ~= Enum.KeyCode.Unknown then k = i.KeyCode
        elseif i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.MouseButton2 then k = i.UserInputType end
        if k then key = k; listening = false; kbtn.Text = keyName(k); tween(kbtn, { BackgroundColor3 = Theme.Bg2 }, 0.12); if cb then cb(k) end end
    end)
    return { get = function() return key end }
end

local function Button(page, text, col, cb)
    local b = new("TextButton", { Size = UDim2.new(1, 0, 0, 40), BackgroundColor3 = col or Theme.Panel,
        Text = text, TextColor3 = Theme.Text, Font = Enum.Font.GothamBold, TextSize = 14,
        AutoButtonColor = false, LayoutOrder = nextOrder() }, page)
    corner(b, 10)
    b.MouseButton1Click:Connect(cb)
    return b
end

-- ============================ FEATURE LOGIC ====================
local function alivePlayers()
    local t = {}
    for _, p in ipairs(Players:GetPlayers()) do if p ~= LocalPlayer then table.insert(t, p) end end
    return t
end
local function enemyCheck(p)
    if not Config.TeamCheck then return true end
    return not (p.Team ~= nil and LocalPlayer.Team ~= nil and p.Team == LocalPlayer.Team)
end

-- ---- Invisibility (restores original transparency) ----
local function setInvisible(char, on)
    if not char then return end
    for _, v in ipairs(char:GetDescendants()) do
        if v:IsA("BasePart") or v:IsA("Decal") then
            if on then
                if v:GetAttribute("ERA_t") == nil then v:SetAttribute("ERA_t", v.Transparency) end
                v.Transparency = 1
            else v.Transparency = v:GetAttribute("ERA_t") or 0 end
        end
    end
end

-- ---- ESP (Highlight + optional box) ----
local espFolder = new("Folder", { Name = "ERA_ESP" }, gui)
local highlights = {}
local function removeESP(p) if highlights[p] then highlights[p]:Destroy(); highlights[p] = nil end end
local function addESP(p)
    if p == LocalPlayer or not p.Character then return end
    if highlights[p] then highlights[p].Adornee = p.Character; return end
    highlights[p] = new("Highlight", { FillTransparency = 0.55, OutlineTransparency = 0,
        FillColor = (Config.ESPTeamColor and p.TeamColor and p.TeamColor.Color) or Config.ESPColor,
        OutlineColor = Color3.new(1, 1, 1), Adornee = p.Character }, espFolder)
end
local function refreshESP() for _, p in ipairs(alivePlayers()) do if State.ESP then addESP(p) else removeESP(p) end end end

-- ---- Aimbot ----
local aiming = false           -- current aim-active state
local aimToggleState = false   -- for Toggle mode
local aimMobile = false        -- on-screen AIM button held
local lockedTarget = nil       -- sticky target

local function partOf(p)
    local c = p.Character
    if not c then return nil end
    return c:FindFirstChild(Config.TargetPart) or c:FindFirstChild("HumanoidRootPart") or c:FindFirstChild("Head")
end
local function isAlive(p)
    local c = p.Character
    local h = c and c:FindFirstChildOfClass("Humanoid")
    return h ~= nil and h.Health > 0
end
local function visibleTo(part)
    if Config.Wallshot then return true end
    local c = LocalPlayer.Character
    local rp = RaycastParams.new()
    rp.FilterType = Enum.RaycastFilterType.Exclude
    rp.FilterDescendantsInstances = { c }
    local from = cam().CFrame.Position
    local hit = Workspace:Raycast(from, part.Position - from, rp)
    return (not hit) or (part.Parent and hit.Instance:IsDescendantOf(part.Parent))
end
local function aimPos(part)
    if Config.Prediction > 0 then
        local v = part.AssemblyLinearVelocity
        return part.Position + v * Config.Prediction
    end
    return part.Position
end
local function pickTarget()
    local C = cam()
    local vp = C.ViewportSize
    local center = Vector2.new(vp.X / 2, vp.Y / 2)
    -- keep sticky target if still valid & inside FOV
    if Config.StickyTarget and lockedTarget then
        local p = lockedTarget
        if p.Parent and isAlive(p) and enemyCheck(p) then
            local part = partOf(p)
            if part then
                local sp, on = C:WorldToViewportPoint(part.Position)
                if on and (Vector2.new(sp.X, sp.Y) - center).Magnitude <= Config.FOV and visibleTo(part) then
                    return part
                end
            end
        end
        lockedTarget = nil
    end
    local best, bestPart, bestDist = nil, nil, Config.FOV
    for _, p in ipairs(alivePlayers()) do
        if enemyCheck(p) and isAlive(p) then
            local part = partOf(p)
            if part then
                local sp, on = C:WorldToViewportPoint(part.Position)
                if on then
                    local d = (Vector2.new(sp.X, sp.Y) - center).Magnitude
                    if d < bestDist and visibleTo(part) then best, bestPart, bestDist = p, part, d end
                end
            end
        end
    end
    lockedTarget = best
    return bestPart
end

-- FOV circle (Drawing)
local fovCircle
if Drawing then pcall(function()
    fovCircle = Drawing.new("Circle"); fovCircle.Thickness = 1.6; fovCircle.NumSides = 64
    fovCircle.Filled = false; fovCircle.Visible = false
end) end

RunService:BindToRenderStep("ERA_Aim", Enum.RenderPriority.Camera.Value + 1, function()
    local C = cam()
    if fovCircle then
        fovCircle.Visible = State.Aim and Config.ShowFOV
        fovCircle.Radius = Config.FOV; fovCircle.Color = Config.Accent
        fovCircle.Position = Vector2.new(C.ViewportSize.X / 2, C.ViewportSize.Y / 2)
    end
    if not State.Aim then aiming = false; return end
    -- compute activation (Hold key/mouse, Toggle, or mobile AIM button)
    if Config.AimMode == "Toggle" then
        aiming = aimToggleState
    else -- Hold
        local k = Config.AimKey
        if typeof(k) == "EnumItem" and k.EnumType == Enum.UserInputType then
            aiming = UIS:IsMouseButtonPressed(k)
        elseif typeof(k) == "EnumItem" and k.EnumType == Enum.KeyCode then
            aiming = UIS:IsKeyDown(k)
        else
            aiming = false
        end
    end
    if aimMobile then aiming = true end
    if not aiming then lockedTarget = nil; return end
    local part = pickTarget()
    if not part then return end
    local goal = aimPos(part)
    if Config.AimMethod == "Camera" then
        local target = CFrame.new(C.CFrame.Position, goal)
        C.CFrame = C.CFrame:Lerp(target, math.clamp(Config.Smoothness, 0.02, 1))
    else -- Silent: nudge the mouse toward the target's screen position
        if mousemoverel then
            local sp, on = C:WorldToViewportPoint(goal)
            if on then
                local cx, cy = C.ViewportSize.X / 2, C.ViewportSize.Y / 2
                local dx, dy = (sp.X - cx) * Config.Smoothness, (sp.Y - cy) * Config.Smoothness
                mousemoverel(dx, dy)
            end
        end
    end
end)

-- ---- Fly (reworked: smooth accel, camera-relative, mobile up/down, noclip) ----
local flyEnabled = false
local flyBV, flyBG, flyConn
local flyUp, flyDown = false, false   -- mobile buttons
local curVel = Vector3.zero
local function getRoot()
    local c = LocalPlayer.Character
    return c and (c:FindFirstChild("HumanoidRootPart") or c:FindFirstChild("Torso")), c and c:FindFirstChildOfClass("Humanoid")
end
local function startFly()
    local root, hum = getRoot()
    if not root then return end
    flyEnabled = true
    if flyBV then flyBV:Destroy() end
    if flyBG then flyBG:Destroy() end
    flyBV = new("BodyVelocity", { MaxForce = Vector3.new(1, 1, 1) * 1e5, Velocity = Vector3.zero, P = 1e4 }, root)
    flyBG = new("BodyGyro", { MaxTorque = Vector3.new(1, 1, 1) * 4e5, P = 1e4, D = 500, CFrame = cam().CFrame }, root)
    if hum then hum.PlatformStand = true end
    curVel = Vector3.zero
    if flyConn then flyConn:Disconnect() end
    flyConn = RunService.RenderStepped:Connect(function(dt)
        if not flyEnabled then return end
        local root2, hum2 = getRoot()
        if not root2 or not flyBV or not flyBG then return end
        local C = cam()
        local wish = Vector3.zero
        if hum2 and hum2.MoveDirection.Magnitude > 0 then
            wish = hum2.MoveDirection            -- already world-space, camera-relative from control
        end
        local up = 0
        if flyUp or UIS:IsKeyDown(Enum.KeyCode.Space) or UIS:IsGamepadButtonDown(Enum.UserInputType.Gamepad1, Enum.KeyCode.ButtonA) then up = up + 1 end
        if flyDown or UIS:IsKeyDown(Enum.KeyCode.LeftControl) or UIS:IsGamepadButtonDown(Enum.UserInputType.Gamepad1, Enum.KeyCode.ButtonB) then up = up - 1 end
        local dir = wish + Vector3.new(0, up, 0)
        if dir.Magnitude > 1 then dir = dir.Unit end
        local goalVel = dir * Config.FlySpeed
        -- smooth acceleration (exponential toward goal)
        local a = math.clamp(dt * 10, 0, 1)
        curVel = curVel:Lerp(goalVel, a)
        flyBV.Velocity = curVel
        flyBG.CFrame = C.CFrame
    end)
end
local function stopFly()
    flyEnabled = false
    if flyConn then flyConn:Disconnect(); flyConn = nil end
    if flyBV then flyBV:Destroy(); flyBV = nil end
    if flyBG then flyBG:Destroy(); flyBG = nil end
    local _, hum = getRoot()
    if hum then hum.PlatformStand = false end
end

-- ---- NoClip ----
local noclipConn
local function setNoClip(on)
    Config.NoClip = on
    if on and not noclipConn then
        noclipConn = RunService.Stepped:Connect(function()
            local c = LocalPlayer.Character
            if not c then return end
            for _, v in ipairs(c:GetDescendants()) do
                if v:IsA("BasePart") and v.CanCollide then v.CanCollide = false end
            end
        end)
    elseif not on and noclipConn then noclipConn:Disconnect(); noclipConn = nil end
end

-- ---- Hitbox Expander (client-side; helps local aim / client-hit games) ----
local hitboxOrig = {}   -- part -> original Size
local hitboxConn
local function restoreHitboxes()
    for part, o in pairs(hitboxOrig) do
        if part and part.Parent then
            part.Size = o.Size; part.Transparency = o.Transparency
            part.CanCollide = o.CanCollide; part.Massless = o.Massless
        end
    end
    hitboxOrig = {}
end
local function setHitbox(on)
    State.Hitbox = on
    if on and not hitboxConn then
        hitboxConn = RunService.Heartbeat:Connect(function()
            for _, p in ipairs(alivePlayers()) do
                if enemyCheck(p) and p.Character then
                    local part = p.Character:FindFirstChild(Config.HitboxPart)
                    if part and part:IsA("BasePart") then
                        if hitboxOrig[part] == nil then hitboxOrig[part] = { Size = part.Size, Transparency = part.Transparency, CanCollide = part.CanCollide, Massless = part.Massless } end
                        local s = Config.HitboxSize
                        part.Size = Vector3.new(s, s, s)
                        part.Transparency = 0.75
                        part.CanCollide = false
                        part.Massless = true
                    end
                end
            end
        end)
    elseif not on and hitboxConn then
        hitboxConn:Disconnect(); hitboxConn = nil; restoreHitboxes()
    end
end

-- ---- Infinite Ammo / Fast Fire (best-effort, GAME-DEPENDENT) ----
-- Scans equipped/backpack Tools for common ammo & cooldown value patterns.
local AMMO_NAMES = { "Ammo", "ammo", "MagAmmo", "CurrentAmmo", "Bullets", "bullets", "Clip", "Rounds", "Mag" }
local FIRE_NAMES = { "FireRate", "Firerate", "Cooldown", "cooldown", "Delay", "delay", "ShotDelay", "ReloadTime" }
local ammoConn
local function eachTool(fn)
    local c = LocalPlayer.Character
    if c then for _, t in ipairs(c:GetChildren()) do if t:IsA("Tool") then fn(t) end end end
    local bp = LocalPlayer:FindFirstChildOfClass("Backpack")
    if bp then for _, t in ipairs(bp:GetChildren()) do if t:IsA("Tool") then fn(t) end end end
end
local function setInfAmmo(on)
    State.InfAmmo = on
    if on and not ammoConn then
        ammoConn = RunService.Heartbeat:Connect(function()
            eachTool(function(t)
                local applied = false
                for _, nm in ipairs(AMMO_NAMES) do
                    local v = t:FindFirstChild(nm) or (t:FindFirstChild("Configuration") and t.Configuration:FindFirstChild(nm))
                    if v and (v:IsA("IntValue") or v:IsA("NumberValue")) then v.Value = 999; applied = true end
                    local a = t:GetAttribute(nm)
                    if a ~= nil and type(a) == "number" then t:SetAttribute(nm, 999); applied = true end
                end
            end)
        end)
    elseif not on and ammoConn then ammoConn:Disconnect(); ammoConn = nil end
end
local function setFastFire(on)
    State.FastFire = on
    -- one-shot lower of any fire-delay values; re-applies on tool change
    local function apply()
        if not State.FastFire then return end
        eachTool(function(t)
            for _, nm in ipairs(FIRE_NAMES) do
                local v = t:FindFirstChild(nm) or (t:FindFirstChild("Configuration") and t.Configuration:FindFirstChild(nm))
                if v and (v:IsA("IntValue") or v:IsA("NumberValue")) then v.Value = Config.FireDelay end
                local a = t:GetAttribute(nm)
                if a ~= nil and type(a) == "number" then t:SetAttribute(nm, Config.FireDelay) end
            end
        end)
    end
    if on then apply(); task.spawn(function() while State.FastFire do apply(); task.wait(0.5) end end) end
end

-- ============================ MOBILE CONTROLS ==================
-- On touch devices: on-screen AIM (hold) + Fly up/down buttons.
local mobileHolder = new("Frame", { AnchorPoint = Vector2.new(1, 1), Position = UDim2.new(1, -14, 1, -70),
    Size = UDim2.new(0, 160, 0, 160), BackgroundTransparency = 1, Visible = isTouch() }, gui)
local function mobBtn(txt, pos, col)
    local b = new("TextButton", { Size = UDim2.new(0, 62, 0, 62), Position = pos, BackgroundColor3 = col or Theme.Bg2,
        Text = txt, TextColor3 = Theme.Text, Font = Enum.Font.GothamBold, TextSize = 13, AutoButtonColor = false,
        BackgroundTransparency = 0.15 }, mobileHolder)
    corner(b, 31); stroke(b, Theme.Stroke, 1, 0.7)
    return b
end
local aimBtn  = mobBtn("AIM",  UDim2.new(1, -66, 1, -66))
local upBtn   = mobBtn("▲",    UDim2.new(0, 0, 0, 0))
local downBtn = mobBtn("▼",    UDim2.new(0, 0, 1, -66))
local function holdBtn(b, downFn, upFn)
    b.InputBegan:Connect(function(i) if i.UserInputType == Enum.UserInputType.Touch or i.UserInputType == Enum.UserInputType.MouseButton1 then downFn() end end)
    b.InputEnded:Connect(function(i) if i.UserInputType == Enum.UserInputType.Touch or i.UserInputType == Enum.UserInputType.MouseButton1 then upFn() end end)
end
holdBtn(aimBtn,  function() aimMobile = true end,  function() aimMobile = false end)
holdBtn(upBtn,   function() flyUp = true end,      function() flyUp = false end)
holdBtn(downBtn, function() flyDown = true end,    function() flyDown = false end)

-- ============================ BUILD TABS =======================
local combat = addTab("Combat",  "⌖")
local weapon = addTab("Weapon",  "▤")
local visual = addTab("Visuals", "◉")
local moveT  = addTab("Movement","➤")
local setT   = addTab("Settings","⚙")

-- Combat
section(combat, "Aimbot")
Toggle(combat, "AimBot", false, function(on) State.Aim = on; if not on then aimToggleState = false end; Notify(on and "Aimbot ON" or "Aimbot OFF", 1.4, on and Theme.Good or Theme.Bad) end)
Dropdown(combat, "Method", { "Camera", "Silent" }, Config.AimMethod, function(v) Config.AimMethod = v; if v == "Silent" and not mousemoverel then Notify("Silent needs mousemoverel (executor)", 3, Theme.Bad) end end)
Dropdown(combat, "Mode", { "Hold", "Toggle" }, Config.AimMode, function(v) Config.AimMode = v; aimToggleState = false end)
Keybind(combat, "Aim Key", Config.AimKey, function(k) Config.AimKey = k end)
Slider(combat, "FOV", 40, 500, Config.FOV, 0, function(v) Config.FOV = v end)
Slider(combat, "Smoothness", 0.02, 1, Config.Smoothness, 2, function(v) Config.Smoothness = v end)
Slider(combat, "Prediction", 0, 0.3, Config.Prediction, 3, function(v) Config.Prediction = v end)
Dropdown(combat, "Target Part", { "Head", "HumanoidRootPart", "Torso", "UpperTorso" }, Config.TargetPart, function(v) Config.TargetPart = v end)
section(combat, "Options")
Toggle(combat, "Team Check", Config.TeamCheck, function(on) Config.TeamCheck = on end)
Toggle(combat, "Wallshot (thru walls)", Config.Wallshot, function(on) Config.Wallshot = on end)
Toggle(combat, "Sticky Target", Config.StickyTarget, function(on) Config.StickyTarget = on end)
Toggle(combat, "Show FOV Circle", Config.ShowFOV, function(on) Config.ShowFOV = on end)
section(combat, "Hitbox")
Toggle(combat, "Hitbox Expander", false, function(on) setHitbox(on) end)
Slider(combat, "Hitbox Size", 3, 30, Config.HitboxSize, 0, function(v) Config.HitboxSize = v end)
Dropdown(combat, "Hitbox Part", { "HumanoidRootPart", "Head", "Torso" }, Config.HitboxPart, function(v) restoreHitboxes(); Config.HitboxPart = v end)

-- Weapon (best-effort)
section(weapon, "Best-effort (game-dependent)")
new("TextLabel", { Size = UDim2.new(1, 0, 0, 34), BackgroundTransparency = 1, TextWrapped = true,
    Text = "Works only if the game keeps ammo/fire values client-side. Server-authoritative games ignore these.",
    TextColor3 = Theme.Sub, Font = Enum.Font.Gotham, TextSize = 11, TextXAlignment = Enum.TextXAlignment.Left,
    LayoutOrder = nextOrder() }, weapon)
Toggle(weapon, "Infinite Ammo", false, function(on) setInfAmmo(on) end)
Toggle(weapon, "Fast Fire", false, function(on) setFastFire(on) end)
Slider(weapon, "Fire Delay", 0.0, 0.3, Config.FireDelay, 3, function(v) Config.FireDelay = v end)

-- Visuals
section(visual, "ESP")
Toggle(visual, "Highlight ESP", false, function(on) State.ESP = on; refreshESP() end)
Toggle(visual, "Team Color", Config.ESPTeamColor, function(on) Config.ESPTeamColor = on
    for p, hl in pairs(highlights) do hl.FillColor = (on and p.TeamColor and p.TeamColor.Color) or Config.ESPColor end end)
section(visual, "Self")
Toggle(visual, "Invisibility", false, function(on) State.Invis = on; if LocalPlayer.Character then setInvisible(LocalPlayer.Character, on) end end)

-- Movement
section(moveT, "Fly")
Toggle(moveT, "Fly", false, function(on) State.Fly = on; if on then startFly() else stopFly() end end)
Slider(moveT, "Fly Speed", 16, 250, Config.FlySpeed, 0, function(v) Config.FlySpeed = v end)
new("TextLabel", { Size = UDim2.new(1, 0, 0, 30), BackgroundTransparency = 1, TextWrapped = true,
    Text = "PC: WASD + Space (up) / LeftCtrl (down). Mobile: joystick + ▲ ▼ buttons.",
    TextColor3 = Theme.Sub, Font = Enum.Font.Gotham, TextSize = 11, TextXAlignment = Enum.TextXAlignment.Left,
    LayoutOrder = nextOrder() }, moveT)
section(moveT, "Misc")
Toggle(moveT, "NoClip", false, function(on) setNoClip(on) end)

-- Settings
section(setT, "Interface")
Keybind(setT, "Menu Key", Config.MenuKey, function(k) if typeof(k) == "EnumItem" and k.EnumType == Enum.KeyCode then Config.MenuKey = k end end)
local accents = { { "Blue", Color3.fromRGB(0,170,255) }, { "Purple", Color3.fromRGB(150,90,255) },
    { "Pink", Color3.fromRGB(255,45,120) }, { "Green", Color3.fromRGB(46,204,113) }, { "Orange", Color3.fromRGB(255,140,0) } }
local accRow = row(setT, 44)
new("TextLabel", { Size = UDim2.new(0, 90, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
    Text = "Accent", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
    Font = Enum.Font.GothamMedium, TextSize = 14 }, accRow)
for idx, a in ipairs(accents) do
    local sw = new("TextButton", { Size = UDim2.new(0, 24, 0, 24), Position = UDim2.new(0, 104 + (idx-1)*30, 0.5, -12),
        BackgroundColor3 = a[2], Text = "", AutoButtonColor = false }, accRow)
    corner(sw, 12); stroke(sw, Theme.Stroke, 1, 0.7)
    sw.MouseButton1Click:Connect(function() applyAccent(a[2]); Notify("Accent: " .. a[1], 1.1, a[2]) end)
end
local uiScaleSlider = Slider(setT, "UI Scale", 0.7, 1.3, Config.UIScale, 2, function(v) Config.UIScale = v; uiScale.Scale = v end)
section(setT, "Session")
Button(setT, "Unload ERA", Theme.Bad, function() ERA_UNLOAD() end)

-- ============================ WATERMARK ========================
local wm = new("Frame", { Position = UDim2.new(0, 20, 0, 20), Size = UDim2.new(0, 260, 0, 30),
    BackgroundColor3 = Theme.Bg2, BorderSizePixel = 0 }, gui)
corner(wm, 8); stroke(wm, Theme.Stroke, 1, 0.88)
accent(new("Frame", { Size = UDim2.new(0, 3, 1, -10), Position = UDim2.new(0, 6, 0, 5), BorderSizePixel = 0 }, wm), "BackgroundColor3")
local wmTxt = new("TextLabel", { Size = UDim2.new(1, -20, 1, 0), Position = UDim2.new(0, 16, 0, 0), BackgroundTransparency = 1,
    Text = "ERA v8", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = Theme.Text,
    Font = Enum.Font.GothamMedium, TextSize = 12 }, wm)
makeDraggable(wm)
task.spawn(function()
    local frames = 0
    RunService.RenderStepped:Connect(function() frames += 1 end)
    while gui.Parent do
        task.wait(1)
        local fps = frames; frames = 0
        local ping = "-"
        pcall(function() if Stats and Stats.Network and Stats.Network.ServerStatsItem then
            ping = math.floor(Stats.Network.ServerStatsItem["Data Ping"]:GetValue()) .. "ms" end end)
        wmTxt.Text = string.format("ERA v8  •  %d fps  •  %s  •  %s", fps, ping, LocalPlayer.Name)
    end
end)

-- ============================ FLOATING BUTTON ==================
local fab = new("TextButton", { Name = "FAB", Size = UDim2.new(0, 50, 0, 50), Position = UDim2.new(0, 20, 0.5, -25),
    Text = "", AutoButtonColor = false, BackgroundColor3 = Config.Accent }, gui)
corner(fab, 25); stroke(fab, Theme.Stroke, 1, 0.75)
new("TextLabel", { Size = UDim2.new(1, 0, 1, 0), BackgroundTransparency = 1, Text = "E",
    TextColor3 = Color3.new(1, 1, 1), Font = Enum.Font.GothamBold, TextSize = 24 }, fab)
accent(fab, "BackgroundColor3")
makeDraggable(fab)

-- ============================ OPEN / CLOSE =====================
local menuOpen = false
local function setMenu(o)
    menuOpen = o
    if o then
        window.Visible = true
        uiScale.Scale = Config.UIScale * 0.9; window.BackgroundTransparency = 1
        tween(uiScale, { Scale = Config.UIScale }, 0.22, Enum.EasingStyle.Back)
        tween(window, { BackgroundTransparency = 0 }, 0.18)
    else
        if activeDropdown then activeDropdown() end
        tween(uiScale, { Scale = Config.UIScale * 0.94 }, 0.14)
        local t = tween(window, { BackgroundTransparency = 1 }, 0.14)
        t.Completed:Connect(function() if not menuOpen then window.Visible = false; uiScale.Scale = Config.UIScale end end)
    end
end
fab.MouseButton1Click:Connect(function() setMenu(not menuOpen) end)
closeBtn.MouseButton1Click:Connect(function() setMenu(false) end)
UIS.InputBegan:Connect(function(i, gp)
    if gp then return end
    if i.KeyCode == Config.MenuKey then setMenu(not menuOpen) end
    -- Toggle-mode aim activation on key/mouse press
    if State.Aim and Config.AimMode == "Toggle" then
        local k = (i.KeyCode ~= Enum.KeyCode.Unknown and i.KeyCode) or i.UserInputType
        if k == Config.AimKey then aimToggleState = not aimToggleState end
    end
end)

-- ============================ UNLOAD ==========================
function ERA_UNLOAD()
    pcall(function() RunService:UnbindFromRenderStep("ERA_Aim") end)
    stopFly(); setNoClip(false); setHitbox(false); setInfAmmo(false); State.FastFire = false
    for p in pairs(highlights) do removeESP(p) end
    if fovCircle then pcall(function() fovCircle:Remove() end) end
    if LocalPlayer.Character then setInvisible(LocalPlayer.Character, false) end
    Notify("ERA unloaded", 1.4)
    task.delay(0.5, function() if gui then gui:Destroy() end end)
end

-- ============================ HOOKS ===========================
LocalPlayer.CharacterAdded:Connect(function(char)
    task.wait(0.4)
    if State.Invis then setInvisible(char, true) end
    if State.Fly then stopFly(); startFly() end
    if Config.NoClip then setNoClip(true) end
    if State.ESP then refreshESP() end
end)
local function hookPlayer(p)
    if p == LocalPlayer then return end
    p.CharacterAdded:Connect(function() task.wait(0.4); if State.ESP then addESP(p) end end)
    p.CharacterRemoving:Connect(function() removeESP(p) end)   -- drop dead adornee immediately
end
for _, p in ipairs(Players:GetPlayers()) do hookPlayer(p) end   -- cover players already in the server
Players.PlayerAdded:Connect(hookPlayer)
Players.PlayerRemoving:Connect(function(p) removeESP(p) end)

-- responsive first scale
do
    local vp = cam().ViewportSize
    uiScaleSlider.set(math.clamp(math.min(vp.X / 640, vp.Y / 470), 0.7, 1))
end

Notify("ERA v8 loaded — tap E or press " .. Config.MenuKey.Name, 3, Config.Accent)
print("[ERA] v8 loaded. Aimbot: hold " .. tostring(Config.AimKey) .. " | Menu: " .. Config.MenuKey.Name)
