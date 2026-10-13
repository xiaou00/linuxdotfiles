-- Keep every file-type glyph in the global foreground while preserving names.
function Entity:icon()
    local icon = th.icon:match(self._file)
    if not icon then
        return ""
    end
    return ui.Line(icon.text .. " "):fg("#f2f2f2")
end
