#!/usr/bin/env python3
"""Run with python3; all writes and reloads are isolated from the desktop."""

import json
import os
from pathlib import Path
import pty
import select
import shutil
import subprocess
import tempfile
import time
import tomllib
import unittest

SOURCE = Path(__file__).resolve().parent
COMMAND = Path.home() / ".local/bin/themecolor"


class ThemeColorTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="themecolor-test-")
        self.addCleanup(self.temp.cleanup)
        self.home = Path(self.temp.name)
        self.config = self.home / ".config"
        self.theme = self.config / "theme"
        self.theme.mkdir(parents=True)
        for name in ("apply-theme.sh", "theme-color.py", "background-defaults.json", "palette.json", "bashrc"):
            shutil.copy2(SOURCE / name, self.theme / name)
        # Tests remain deterministic after the real desktop color has changed.
        self.defaults = json.loads((self.theme / "background-defaults.json").read_text())
        palette = json.loads((self.theme / "palette.json").read_text())
        for mode in self.defaults:
            palette[mode].update(self.defaults[mode])
        self.put("theme/palette.json", json.dumps(palette))
        self.put("theme/mode", "dark\n")
        self.put("alacritty/theme.toml", "")
        self.put("gtk-3.0/colors.css", "@define-color theme_base_color_breeze #151515;\n")
        self.put("gtk-4.0/colors.css", "")
        self.put("fish/conf.d/theme.fish", "")
        self.put("starship.toml", '[palettes.system]\nbg = "#151515"\n')
        self.put("quickshell/nucleus-shell/config/colors.json", "{}")
        self.put("yazi/theme.toml", '[which]\nmask = { bg = "#151515" }\n'
                 '[confirm]\nbtn_yes = { fg = "#151515", bg = "#ff4444" }\n'
                 '[status]\noverall = { fg = "#d0d0d0", bg = "#242424" }\n')
        self.put("cava/config", "[color]\nbackground = '#151515'\ngradient_color_1 = '#3a3a3a'\n")
        self.put("zathura/zathurarc", 'set default-bg "#151515"\nset statusbar-bg "#1e1e1e"\n')
        self.put("silicon/themes/obsidian-ember.tmTheme",
                 '<dict><key>background</key><string>#151515</string></dict>\n')
        self.put("zen/profiles.ini", '[Profile0]\nIsRelative=1\nPath=Profile with spaces\n')
        self.put("zen/Profile with spaces/chrome/userChrome.css", "/* keep user styles */\n")
        self.put("zen/Profile with spaces/user.js", 'user_pref("custom.setting", 42);\n')
        self.environment = dict(os.environ, HOME=str(self.home), XDG_CONFIG_HOME=str(self.config),
                                XDG_DATA_HOME=str(self.home / ".local/share"), THEME_NO_RELOAD="1")

    def put(self, relative, text):
        path = self.config / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def run_command(self, *args, ok=True):
        result = subprocess.run(["bash", str(COMMAND), *args], env=self.environment,
                                capture_output=True, text=True)
        if ok:
            self.assertEqual(result.returncode, 0, result.stderr)
        return result

    def apply(self, mode):
        result = subprocess.run(["bash", str(self.theme / "apply-theme.sh"), mode],
                                env=self.environment, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    def palette(self):
        return json.loads((self.theme / "palette.json").read_text())

    def check_outputs(self, color):
        self.assertEqual(self.run_command().stdout.strip(), color)
        with (self.config / "alacritty/theme.toml").open("rb") as stream:
            terminal = tomllib.load(stream)["colors"]
        self.assertEqual(terminal["primary"]["background"], color)
        surfaces = [terminal["selection"], terminal["line_indicator"], terminal["footer_bar"],
                    *terminal["search"].values(), *terminal["hints"].values()]
        for surface in surfaces:
            background = surface["background"]
            foreground = surface.get("foreground", surface.get("text"))
            self.assertNotEqual(background, color)
            self.assertGreaterEqual(self.contrast(foreground, background), 4.5)
        for role in ("cursor", "vi_mode_cursor"):
            self.assertGreaterEqual(self.contrast(terminal[role]["cursor"], color), 4.5)
            self.assertGreaterEqual(self.contrast(terminal[role]["text"], terminal[role]["cursor"]), 4.5)
        self.assertNotEqual(terminal["search"]["matches"]["background"],
                            terminal["search"]["focused_match"]["background"])
        self.assertIn(f"@define-color theme_bg {color};", (self.theme / "colors.css").read_text())
        self.assertIn(f"background {color}\n", (self.theme / "kitty.conf").read_text())
        self.assertIn(f"theme_base_color_breeze {color};", (self.config / "gtk-3.0/colors.css").read_text())
        self.assertEqual(json.loads((self.config / "quickshell/nucleus-shell/config/colors.json").read_text())["background"], color)
        self.assertIn(f"--system-theme-bg: {color};", (self.config / "zen/Profile with spaces/chrome/system-theme-chrome.css").read_text())
        self.assertIn(f"background = '{color}'", (self.config / "cava/config").read_text())
        self.assertIn(f'set default-bg "{color}"', (self.config / "zathura/zathurarc").read_text())
        self.assertIn(f'<key>background</key><string>{color}</string>', (self.config / "silicon/themes/obsidian-ember.tmTheme").read_text())
        with (self.config / "yazi/theme.toml").open("rb") as stream:
            yazi = tomllib.load(stream)
        self.assertEqual(yazi["which"]["mask"]["bg"], color)
        self.assertEqual(yazi["confirm"]["btn_yes"]["fg"], color)

    @staticmethod
    def contrast(foreground, background):
        def luminance(value):
            channels = [int(value[i:i + 2], 16) / 255 for i in (1, 3, 5)]
            return sum((c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4) * weight
                       for c, weight in zip(channels, (0.2126, 0.7152, 0.0722)))
        fg, bg = luminance(foreground), luminance(background)
        return (max(fg, bg) + 0.05) / (min(fg, bg) + 0.05)

    @unittest.skipUnless(shutil.which("nvim"), "Neovim is not installed")
    def test_alacritty_and_nvim_selection_colors_agree(self):
        environment = dict(self.environment, NVIM_TEST_SOURCE=str(SOURCE.parent / "nvim"))
        lua = ('vim.opt.rtp:prepend(vim.env.NVIM_TEST_SOURCE); '
               'local colors = require("theme.palette").load("dark"); '
               'io.write(vim.json.encode(colors))')
        for background in ("#001015", "#204830", "#f7f5f2", "#000000", "#ffffff"):
            self.run_command(background)
            result = subprocess.run(["nvim", "--headless", "-u", "NONE", "-i", "NONE",
                                     "-c", "lua " + lua, "-c", "qa!"],
                                    env=environment, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue(result.stdout, result.stderr)
            nvim = json.loads(result.stdout)
            terminal = tomllib.loads((self.config / "alacritty/theme.toml").read_text())["colors"]
            self.assertEqual(terminal["selection"]["background"], nvim["text_selection"])
            self.assertEqual(terminal["selection"]["text"], nvim["selection_fg"])
            self.assertEqual(terminal["search"]["matches"]["background"], nvim["search"])
            self.assertEqual(terminal["line_indicator"]["foreground"], nvim["cursorline_fg"])
            self.assertEqual(terminal["line_indicator"]["background"], nvim["bg1"])

    def test_repeated_changes_and_semantic_color_collisions(self):
        original = self.palette()["dark"]
        for color in ("#202830", "#336699", "#ff4444", "#ffffff", "#000000", "#151515"):
            self.run_command(color)
            self.check_outputs(color)
            for role, value in original.items():
                if not role.startswith("bg"):
                    self.assertEqual(self.palette()["dark"][role], value)
        self.run_command("AbCdEf")
        self.check_outputs("#abcdef")

    def test_modes_persist_separately_and_reset(self):
        self.run_command("#123456")
        self.apply("toggle")
        self.check_outputs("#f7f5f2")
        self.run_command("#ebdab9")
        self.apply("toggle")
        self.check_outputs("#123456")
        self.run_command("--reset")
        self.check_outputs("#151515")
        for mode in ("dark", "nvim_dark"):
            for key, value in self.defaults[mode].items():
                self.assertEqual(self.palette()[mode][key], value)
        self.apply("light")
        self.check_outputs("#ebdab9")
        self.run_command("--reset")
        self.check_outputs("#f7f5f2")

    def test_bad_arguments_do_not_change_palette(self):
        before = (self.theme / "palette.json").read_bytes()
        for color in ("#abc", "#12345678", "zzzzzz", "#12;abc", "--bad", "#123456\n"):
            self.assertNotEqual(self.run_command(color, ok=False).returncode, 0)
            self.assertEqual((self.theme / "palette.json").read_bytes(), before)
        self.assertNotEqual(self.run_command("123456", "654321", ok=False).returncode, 0)

    def test_zen_customizations_preserved_and_imports_not_duplicated(self):
        self.run_command("#123456")
        self.run_command("#234567")
        profile = self.config / "zen/Profile with spaces"
        chrome = (profile / "chrome/userChrome.css").read_text()
        self.assertIn("/* keep user styles */", chrome)
        self.assertEqual(chrome.count("@import"), 1)
        prefs = (profile / "user.js").read_text()
        self.assertIn('user_pref("custom.setting", 42);', prefs)
        self.assertEqual(prefs.count("toolkit.legacyUserProfileCustomizations.stylesheets"), 1)
        self.assertFalse((profile / "prefs.js").exists())
        content = (profile / "chrome/system-theme-content.css").read_text()
        self.assertIn('@-moz-document url("about:newtab")', content)
        self.assertNotIn("url-prefix", content)

    def test_interactive_bash_accepts_unquoted_hash(self):
        binary = self.home / ".local/bin/themecolor"
        binary.parent.mkdir(parents=True)
        shutil.copy2(COMMAND, binary)
        binary.chmod(0o755)
        # Separate lines force parsing after the interactive_comments setting changes.
        result = subprocess.run(["bash", "--noprofile", "--norc", "-i"],
                                input='source "$HOME/.config/theme/bashrc"\nthemecolor #253647\nexit\n',
                                env=self.environment, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.check_outputs("#253647")

    def fish_terminal(self):
        """Use a PTY: Fish only retains interactive input when reading a terminal."""
        binary = self.home / ".local/bin/themecolor"
        binary.parent.mkdir(parents=True)
        shutil.copy2(COMMAND, binary)
        binary.chmod(0o755)
        function = self.config / "fish/functions/themecolor.fish"
        function.parent.mkdir(parents=True)
        shutil.copy2(SOURCE.parent / "fish/functions/themecolor.fish", function)
        pid, fd = pty.fork()
        if pid == 0:
            environment = dict(self.environment, TERM="dumb", fish_history="",
                               PATH=str(binary.parent) + os.pathsep + os.environ["PATH"])
            os.execvpe("fish", ["fish", "--no-config", "-i", "-C",
                               'set --prepend fish_function_path "$HOME/.config/fish/functions"; '
                               "function fish_prompt; printf 'THEMECOLOR_TEST> '; end"], environment)

        def cleanup():
            try:
                os.write(fd, b"exit\r")
                deadline = time.monotonic() + 3
                while time.monotonic() < deadline:
                    if os.waitpid(pid, os.WNOHANG)[0]:
                        return
                    select.select([fd], [], [], 0.05)
                os.kill(pid, 15)
                os.waitpid(pid, 0)
            finally:
                os.close(fd)

        self.addCleanup(cleanup)

        def read_prompt(executed=False):
            output = b""
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline:
                if select.select([fd], [], [], 0.1)[0]:
                    output += os.read(fd, 65536)
                    # TERM=dumb redraws the prompt while typing: only accept a
                    # prompt after a newline when waiting for command execution.
                    tail = output.split(b"\n", 1)[-1] if executed else output
                    if (not executed or b"\n" in output) and b"THEMECOLOR_TEST> " in tail:
                        return output.decode(errors="replace")
            self.fail(f"Fish prompt timeout: {output!r}")

        read_prompt()

        def execute(line):
            os.write(fd, line.encode() + b"\r")
            return read_prompt(executed=True)

        return execute

    @unittest.skipUnless(shutil.which("fish"), "Fish is not installed")
    def test_interactive_fish_accepts_raw_quoted_and_plain_colors(self):
        execute = self.fish_terminal()
        for line, expected in (("themecolor #204830", "#204830"),
                               ("themecolor '#123456'", "#123456"),
                               ("themecolor 345678", "#345678")):
            execute(line)
            self.check_outputs(expected)
        execute("themecolor --reset")
        self.check_outputs("#151515")

    @unittest.skipUnless(shutil.which("fish"), "Fish is not installed")
    def test_interactive_fish_query_and_comments_do_not_reapply_history(self):
        execute = self.fish_terminal()
        execute("themecolor #204830")
        before = (self.theme / "generation").read_bytes()
        for line in ("themecolor", "themecolor # show current", "echo ignored #204830",
                     "themecolor; echo ignored #204830"):
            execute(line)
            self.assertEqual((self.theme / "generation").read_bytes(), before)
        result = execute("themecolor #bad")
        self.assertIn("颜色必须是六位十六进制", result)
        self.assertEqual((self.theme / "generation").read_bytes(), before)


if __name__ == "__main__":
    unittest.main(verbosity=2)
