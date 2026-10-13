-- Hyprland 0.56 has no resize event. Sample geometry in Lua and redraw only
-- after it settles, without running a polling subprocess or changing opacity.
local M = {}
local configHome = os.getenv("XDG_CONFIG_HOME") or (os.getenv("HOME") .. "/.config")
local helper = configHome .. "/hypr/scripts/alacritty-refresh.py"
local quotedHelper = "'" .. helper:gsub("'", "'\\''") .. "'"
local resizeWatcher

local function isAlacritty(window)
    return window and window.mapped and window.class == "Alacritty"
end

local function refreshProcesses(pids)
    local args = {}
    for pid in pairs(pids) do
        if pid and pid > 0 then
            args[#args + 1] = string.format("%d", pid)
        end
    end
    if #args > 0 then
        hl.exec_cmd("python3 " .. quotedHelper .. " " .. table.concat(args, " "))
    end
end

function M.toggle_opacity()
    local window = hl.get_active_window()
    if not window then return end
    hl.dispatch(hl.dsp.window.tag({ tag = "manual-opaque" }))
    if isAlacritty(window) then
        -- Let the compositor apply the tag before requesting a fresh buffer.
        hl.timer(function()
            if isAlacritty(window) then
                refreshProcesses({ [window.pid] = true })
            end
        end, { timeout = 60, type = "oneshot" })
    end
end

function M.watch_resizes()
    if resizeWatcher then return resizeWatcher end
    -- No live outputs during --verify-config or early startup. Creating a timer
    -- there triggers a Hyprland 0.56 cleanup bug; hyprland.start retries later.
    if #hl.get_monitors() == 0 then return end
    local previous = {}
    resizeWatcher = hl.timer(function()
        local current, refresh = {}, {}
        for _, window in ipairs(hl.get_windows()) do
            if isAlacritty(window) then
                local size, address = window.size, window.address
                local state = previous[address]
                if not state then
                    -- New windows already draw their initial buffer.
                    state = { width = size.x, height = size.y, pending = false }
                elseif state.width ~= size.x or state.height ~= size.y then
                    state.width, state.height = size.x, size.y
                    state.pending = true
                elseif state.pending then
                    refresh[window.pid] = true
                    state.pending = false
                end
                current[address] = state
            end
        end
        previous = current
        refreshProcesses(refresh)
    end, { timeout = 100, type = "repeat" })
    return resizeWatcher
end

return M
