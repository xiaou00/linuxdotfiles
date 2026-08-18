-- Keep every file-type glyph white while preserving the file-name style.
function Entity:icon()
    local icon = self._file:icon()
    if not icon then
        return ""
    end
    return ui.Line(icon.text .. " "):fg("#f2f2f2")
end

