--[[  ERA Remote Spy  ============================================================
  Finds the game's fire / hit remote so a real silent-aim can be built.

    1) Press  ● RECORD
    2) Fire ONE shot at an enemy (up close)
    3) Press  ■ STOP
    4) Read the captured remote(s) in the window — or press COPY and send them.

  Logs every RemoteEvent:FireServer / RemoteFunction:InvokeServer while recording,
  with the remote's full path and its arguments (so we can see the target / hit pos).
  Needs an executor with hookmetamethod + getnamecallmethod (Delta, Codex, Synapse,
  Script-Ware, Fluxus, etc.).
===============================================================================]]

local Players = game:GetService("Players")
local UIS     = game:GetService("UserInputService")
local LocalPlayer = Players.LocalPlayer

-- ---------------------------------------------------------------- state
local recording = false
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

-- serialize one argument for display
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
    Size = UDim2.new(0, 470, 0, 350), BackgroundColor3 = BG, BorderSizePixel = 0 }, gui)
corner(win, 12)
new("UIStroke", { Color = Color3.new(1,1,1), Transparency = 0.9, Thickness = 1 }, win)

-- title bar
local bar = new("Frame", { Size = UDim2.new(1, 0, 0, 40), BackgroundColor3 = BG2, BorderSizePixel = 0 }, win)
corner(bar, 12)
new("Frame", { Size = UDim2.new(1, 0, 0, 12), Position = UDim2.new(0, 0, 1, -12), BackgroundColor3 = BG2, BorderSizePixel = 0 }, bar)
new("TextLabel", { Size = UDim2.new(1, -170, 1, 0), Position = UDim2.new(0, 14, 0, 0), BackgroundTransparency = 1,
    Text = "ERA Remote Spy", TextXAlignment = Enum.TextXAlignment.Left, TextColor3 = TXT,
    Font = Enum.Font.GothamBold, TextSize = 15 }, bar)
local status = new("TextLabel", { Size = UDim2.new(0, 110, 1, 0), Position = UDim2.new(1, -150, 0, 0), BackgroundTransparency = 1,
    Text = "idle", TextXAlignment = Enum.TextXAlignment.Right, TextColor3 = SUB,
    Font = Enum.Font.GothamMedium, TextSize = 12 }, bar)
local closeBtn = new("TextButton", { Size = UDim2.new(0, 26, 0, 26), Position = UDim2.new(1, -32, 0.5, -13),
    BackgroundColor3 = BG, Text = "×", TextColor3 = SUB, Font = Enum.Font.GothamBold, TextSize = 16 }, bar)
corner(closeBtn, 7)

-- drag
do
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

-- output list
local scroll = new("ScrollingFrame", { Position = UDim2.new(0, 12, 0, 48), Size = UDim2.new(1, -24, 1, -102),
    BackgroundColor3 = BG2, BorderSizePixel = 0, ScrollBarThickness = 4, ScrollBarImageColor3 = ACCENT,
    CanvasSize = UDim2.new(0, 0, 0, 0), AutomaticCanvasSize = Enum.AutomaticSize.Y }, win)
corner(scroll, 8)
local out = new("TextLabel", { Size = UDim2.new(1, -16, 0, 0), Position = UDim2.new(0, 8, 0, 8),
    AutomaticSize = Enum.AutomaticSize.Y, BackgroundTransparency = 1, Text = "", TextXAlignment = Enum.TextXAlignment.Left,
    TextYAlignment = Enum.TextYAlignment.Top, TextColor3 = TXT, Font = Enum.Font.Code, TextSize = 13, TextWrapped = true }, scroll)

-- buttons
local btnRow = new("Frame", { Size = UDim2.new(1, -24, 0, 38), Position = UDim2.new(0, 12, 1, -46), BackgroundTransparency = 1 }, win)
new("UIListLayout", { FillDirection = Enum.FillDirection.Horizontal, Padding = UDim.new(0, 8),
    SortOrder = Enum.SortOrder.LayoutOrder }, btnRow)
local function mkBtn(text, col, ord)
    local b = new("TextButton", { Size = UDim2.new(0, 103, 1, 0), BackgroundColor3 = col or BG2, Text = text,
        TextColor3 = TXT, Font = Enum.Font.GothamBold, TextSize = 13, AutoButtonColor = true, LayoutOrder = ord }, btnRow)
    corner(b, 8)
    return b
end
local recBtn  = mkBtn("● RECORD", ACCENT, 1)
local stopBtn = mkBtn("■ STOP",  BG2, 2)
local clrBtn  = mkBtn("CLEAR",   BG2, 3)
local copyBtn = mkBtn("COPY",    BG2, 4)

-- ---------------------------------------------------------------- render
local function refresh()
    if #order == 0 then
        out.Text = "No remotes captured.\n\n1) Press  ● RECORD\n2) Fire ONE shot at an enemy (up close)\n3) Press  ■ STOP\n4) Read / COPY the remote below and send it."
        return
    end
    local lines = {}
    for _, key in ipairs(order) do
        local e = log[key]
        lines[#lines + 1] = ("[%dx]  %s\n%s\nargs: %s"):format(e.count, e.method, e.path, e.args ~= "" and e.args or "(none)")
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

-- ---------------------------------------------------------------- hook
if hookmetamethod and getnamecallmethod then
    local old
    old = hookmetamethod(game, "__namecall", function(self, ...)
        if recording then
            local ok, method = pcall(getnamecallmethod)
            if ok and (method == "FireServer" or method == "InvokeServer") then
                -- skip our own calls; only log the game's remotes (fired when you shoot)
                if not (checkcaller and checkcaller()) then
                    local packed = table.pack(...)
                    task.spawn(function() pcall(capture, self, method, packed) end)
                end
            end
        end
        return old(self, ...)
    end)
else
    out.Text = "This executor has no hookmetamethod / getnamecallmethod.\nRemote spy needs a full executor (Delta, Codex, Synapse, Script-Ware, Fluxus...)."
end

-- ---------------------------------------------------------------- buttons
recBtn.MouseButton1Click:Connect(function()  recording = true;  status.Text = "● recording"; status.TextColor3 = ACCENT end)
stopBtn.MouseButton1Click:Connect(function() recording = false; status.Text = "stopped";     status.TextColor3 = SUB end)
clrBtn.MouseButton1Click:Connect(function()  log, order = {}, {}; refresh() end)
copyBtn.MouseButton1Click:Connect(function()
    local cb = setclipboard or toclipboard or writeclipboard or (syn and syn.write_clipboard)
    if cb then pcall(cb, out.Text); status.Text = "copied ✓"; status.TextColor3 = ACCENT
    else status.Text = "no clipboard"; status.TextColor3 = SUB end
end)
closeBtn.MouseButton1Click:Connect(function() recording = false; gui:Destroy() end)

refresh()
print("[ERA] Remote Spy loaded — RECORD, shoot once, STOP.")
