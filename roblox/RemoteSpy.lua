--[[  ERA Remote Spy  ============================================================
  Finds the game's fire / hit remote so a real silent-aim can be built.

  Two modes:
    • LIST   — passively lists every RemoteEvent/Function in the game. No hooks,
               it only reads the instance tree. Try this first.
    • RECORD — live capture of FireServer/InvokeServer WITH arguments via a standard
               __namecall logger. Opt-in; only hooks when you press it. NOTE: some
               anti-cheats (e.g. Adonis) detect namecall hooks and will kick you. If
               that happens, use LIST instead — reading the tree can't be detected.

  Steps for live capture:  RECORD → fire ONE shot at an enemy → STOP → COPY.
  Needs an executor with hookmetamethod + getnamecallmethod (Delta, Codex, etc.).
===============================================================================]]

local Players = game:GetService("Players")
local UIS     = game:GetService("UserInputService")
local LocalPlayer = Players.LocalPlayer

-- ---------------------------------------------------------------- state
local recording, hookInstalled = false, false
local log, order = {}, {}     -- key -> {method, path, count, args} ; keys in capture order

-- ---------------------------------------------------------------- theme / helpers
local ACCENT = Color3.fromRGB(217, 119, 87)
local BG   = Color3.fromRGB(20, 19, 18)
local BG2  = Color3.fromRGB(30, 28, 26)
local TXT  = Color3.fromRGB(237, 233, 226)
local SUB  = Color3.fromRGB(150, 142, 132)

local function new(cls, props, parent)
    local o = Instance.new(cls)
    if props then for k, v in pairs(props) do o[k] = v end end
    if parent then o.Parent = parent end
    return o
end
local function corner(p, r) new("UICorner", { CornerRadius = UDim.new(0, r or 8) }, p) end

local function ser(v)
    local t = typeof(v)
    if t == "Instance" then return ("Instance<%s '%s'>"):format(v.ClassName, v.Name)
    elseif t == "Vector3" then return "Vector3(" .. tostring(v) .. ")"
    elseif t == "CFrame"  then return "CFrame@" .. tostring(v.Position)
    elseif t == "string"  then return '"' .. v .. '"'
    elseif t == "table"   then return "table"
    else return t .. "(" .. tostring(v) .. ")" end
end

-- ---------------------------------------------------------------- GUI
local gui = new("ScreenGui", { Name = "ERA_RemoteSpy", ResetOnSpawn = false, IgnoreGuiInset = true, DisplayOrder = 9999 })
pcall(function() gui.Parent = (gethui and gethui()) or game:GetService("CoreGui") end)
if not gui.Parent then pcall(function() gui.Parent = LocalPlayer:WaitForChild("PlayerGui") end) end

local win = new("Frame", { AnchorPoint = Vector2.new(0.5, 0.5), Position = UDim2.new(0.5, 0, 0.5, 0),
    Size = UDim2.new(0, 520, 0, 360), BackgroundColor3 = BG, BorderSizePixel = 0 }, gui)
corner(win, 12)
new("UIStroke", { Color = Color3.new(1,1,1), Transparency = 0.9, Thickness = 1 }, win)

local bar = new("Frame", { Size = UDim2.new(1, 0, 0, 40), BackgroundColor3 = BG2, BorderSizePixel = 0 }, win)
corner(bar, 12)
new("Frame", { Size = UDim2.new(1, 0, 0, 12), Position = UDim2.new(0, 0, 1, -12), BackgroundColor3 = BG2, BorderSizePixel = 0 }, bar)
new("TextLabel", { Size = UDim2.new(1, -190, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
    Text = "ERA Remote Spy", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = TXT,
    Font = Enum.Font.GothamBold, TextSize = 15 }, bar)
local status = new("TextLabel", { Size = UDim2.new(0, 130, 1, 0), Position = UDim2.new(1, -170, 0, 0), BackgroundTransparency = 1,
    Text = "safe mode", TextXAlignment = Enum.TextXAlignment.Right, TextColor3 = SUB,
    Font = Enum.Font.GothamMedium, TextSize = 12 }, bar)
local closeBtn = new("TextButton", { Size = UDim2.new(0, 26, 0, 26), Position = UDim2.new(1, -32, 0.5, -13),
    BackgroundColor3 = BG, Text = "×", TextColor3 = SUB, Font = Enum.Font.GothamBold, TextSize = 16 }, bar)
corner(closeBtn, 7)

do  -- drag
    local dragging, startP, startPos
    bar.InputBegan:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then
            dragging, startP, startPos = true, i.Position, win.Position
        end
    end)
    UIS.InputChanged:Connect(function(i)
        if dragging and (i.UserInputType == Enum.UserInputType.MouseMovement or i.UserInputType == Enum.UserInputType.Touch) then
            local d = i.Position - startP
            win.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + d.X, startPos.Y.Scale, startPos.Y.Offset + d.Y)
        end
    end)
    UIS.InputEnded:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 or i.UserInputType == Enum.UserInputType.Touch then dragging = false end
    end)
end

local scroll = new("ScrollingFrame", { Position = UDim2.new(0, 12, 0, 48), Size = UDim2.new(1, -24, 1, -102),
    BackgroundColor3 = BG2, BorderSizePixel = 0, ScrollBarThickness = 4, ScrollBarImageColor3 = ACCENT,
    CanvasSize = UDim2.new(0, 0, 0, 0), AutomaticCanvasSize = Enum.AutomaticSize.Y }, win)
corner(scroll, 8)
local out = new("TextLabel", { Size = UDim2.new(1, -16, 0, 0), Position = UDim2.new(0, 8, 0, 8),
    AutomaticSize = Enum.AutomaticSize.Y, BackgroundTransparency = 1, Text = "", TextXAlignment = Enum.TextXAlignment.Left,
    TextYAlignment = Enum.TextYAlignment.Top, TextColor3 = TXT, Font = Enum.Font.Code, TextSize = 13, TextWrapped = true }, scroll)

local btnRow = new("Frame", { Size = UDim2.new(1, -24, 0, 38), Position = UDim2.new(0, 12, 1, -46), BackgroundTransparency = 1 }, win)
new("UIListLayout", { FillDirection = Enum.FillDirection.Horizontal, Padding = UDim.new(0, 8),
    SortOrder = Enum.SortOrder.LayoutOrder }, btnRow)
local function mkBtn(text, col, ord)
    local b = new("TextButton", { Size = UDim2.new(0, 92, 1, 0), BackgroundColor3 = col or BG2, Text = text,
        TextColor3 = TXT, Font = Enum.Font.GothamBold, TextSize = 13, AutoButtonColor = true, LayoutOrder = ord }, btnRow)
    corner(b, 8)
    return b
end
local listBtn = mkBtn("LIST",     ACCENT, 1)
local recBtn  = mkBtn("● RECORD", BG2, 2)
local stopBtn = mkBtn("■ STOP",   BG2, 3)
local clrBtn  = mkBtn("CLEAR",    BG2, 4)
local copyBtn = mkBtn("COPY",     BG2, 5)

-- ---------------------------------------------------------------- render
local function refresh()
    if #order == 0 then
        out.Text = "Press  LIST  to list every remote (safe, no hook).\n\nFor live args:  ● RECORD  →  fire ONE shot at an enemy  →  ■ STOP  →  COPY.\n(RECORD installs a stealth hook only when pressed.)"
        return
    end
    local lines = {}
    for _, key in ipairs(order) do
        local e = log[key]
        local head = e.count > 0 and ("[%dx]  %s"):format(e.count, e.method) or e.method
        lines[#lines + 1] = head .. "\n" .. e.path .. "\nargs: " .. (e.args ~= "" and e.args or "(none)")
    end
    out.Text = table.concat(lines, "\n\n")
end

local function capture(remote, method, packed)
    local ok, path = pcall(function() return remote:GetFullName() end)
    if not ok then path = tostring(remote) end
    local parts = {}
    for i = 1, packed.n do parts[i] = i .. "=" .. ser(packed[i]) end
    local key = method .. " | " .. path
    local e = log[key]
    if not e then e = { method = method, path = path, count = 0, args = "" }; log[key] = e; order[#order + 1] = key end
    e.count += 1
    e.args = table.concat(parts, ", ")
    refresh()
end

-- ---------------------------------------------------------------- namecall logger (lazy, opt-in)
local function installHook()
    if hookInstalled then return true end
    if not (hookmetamethod and getnamecallmethod) then return false end
    local ok = pcall(function()
        local old
        old = hookmetamethod(game, "__namecall", function(self, ...)
            if recording then
                local ok2, m = pcall(getnamecallmethod)
                if ok2 and (m == "FireServer" or m == "InvokeServer") then
                    if not (checkcaller and checkcaller()) then
                        local packed = table.pack(...)
                        task.spawn(function() pcall(capture, self, m, packed) end)
                    end
                end
            end
            return old(self, ...)
        end)
    end)
    hookInstalled = ok
    return ok
end

-- ---------------------------------------------------------------- buttons
listBtn.MouseButton1Click:Connect(function()
    log, order = {}, {}
    for _, v in ipairs(game:GetDescendants()) do
        if v:IsA("RemoteEvent") or v:IsA("RemoteFunction") then
            local ok, path = pcall(function() return v:GetFullName() end)
            local key = v.ClassName .. " | " .. (ok and path or v.Name)
            if not log[key] then
                log[key] = { method = v.ClassName, path = ok and path or v.Name, count = 0,
                    args = "(fire once with RECORD to see arguments)" }
                order[#order + 1] = key
            end
        end
    end
    refresh()
    status.Text = ("%d remotes"):format(#order); status.TextColor3 = SUB
end)

recBtn.MouseButton1Click:Connect(function()
    if not installHook() then status.Text = "hook unavailable"; status.TextColor3 = SUB; return end
    recording = true; status.Text = "● recording"; status.TextColor3 = ACCENT
end)
stopBtn.MouseButton1Click:Connect(function() recording = false; status.Text = "stopped"; status.TextColor3 = SUB end)
clrBtn.MouseButton1Click:Connect(function() log, order = {}, {}; refresh() end)
copyBtn.MouseButton1Click:Connect(function()
    local cb = setclipboard or toclipboard or writeclipboard or (syn and syn.write_clipboard)
    if cb then pcall(cb, out.Text); status.Text = "copied ✓"; status.TextColor3 = ACCENT
    else status.Text = "no clipboard"; status.TextColor3 = SUB end
end)
closeBtn.MouseButton1Click:Connect(function() recording = false; gui:Destroy() end)

refresh()
print("[ERA] Remote Spy loaded — LIST is safe; RECORD hooks only when pressed.")
