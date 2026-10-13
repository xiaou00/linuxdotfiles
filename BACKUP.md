# Dotfiles backup

配置快照：2026-10-13。沿用此前 `~/dotfiles` → `xiaou00/linuxdotfiles`
的备份方式，直接提交配置文件；GitHub 的 Download ZIP 可下载整个快照。

## 目录对应关系

- `backup-manifest.json` 中 `config_paths` 列出的路径对应 `~/.config/`。
- `home_paths` 列出的路径对应家目录，例如 `.bashrc`、`.local/bin/`、
  `.local/share/` 和选中的壁纸。
- 根目录原有的 JPG/PNG 是旧备份保留的参考壁纸。
- `desktop-settings/` 是 dconf 导出的桌面主题、字体、光标和窗口偏好，
  不是要直接复制到 `~/.config` 的目录。
- `packages/` 记录 Arch Linux 显式安装的软件包，供恢复环境时参考。

本次收录了 Hyprland（当前使用 `hyprland.lua`）、Waybar、Alacritty、
共享主题与切换脚本、Fish/Bash、Kitty、输入法主题、GTK/Qt/KDE、通知与
启动器、Firefox/Zen 自定义样式、Qutebrowser、VS Code/Code OSS、
IntelliJ 的编辑器偏好、本地脚本及其他用户应用配置。准确范围以清单为准。
已从 Aseprite 和 Krita 的副本中去除最近文件列表，原始配置未改动。

## 单独备份的 Neovim

继续沿用独立仓库：<https://github.com/xiaou00/NeoVimDotfiles>。
本次核对的提交为 `b7c300bcbadb4cc1dc45392e21d91d87c7804609`，已在远端。
主仓库不重复复制 `nvim/`。

## 恢复

先安装需要的应用，再按照清单把所需路径复制到目标位置；覆盖前备份
目标目录。例如，下面只恢复 Hyprland，备份目录使用当前时间命名：

```bash
backup_dir="$HOME/.local/state/dotfiles-restore/$(date +%Y%m%d-%H%M%S)"
mkdir -p "$backup_dir" "$HOME/.config"
if [ -e "$HOME/.config/hypr" ]; then
    cp -a "$HOME/.config/hypr" "$backup_dir/"
fi
cp -a hypr/. "$HOME/.config/hypr/"
```

Hyprland 还依赖 `theme/` 和对应的终端配置。Firefox/Zen 的 profile 名字
因机器而异，只将备份中的 `chrome/` 和 `user.js` 放入正确的 profile。
配置里的 `/home/xiaou0`、显示器名、设备名和软件安装路径需按新机器调整。
这份快照保留现状，不自动安装软件或修复原配置中失效的路径。

恢复 dconf 偏好时，也先导出目标机器的现有值，再执行：

```bash
dconf load /org/gnome/desktop/interface/ < desktop-settings/interface.ini
dconf load /org/gnome/desktop/wm/preferences/ < desktop-settings/window-manager.ini
```

## 后续更新

按 `backup-manifest.json` 的路径从本机重新复制配置；保留清单中的排除项，
并更新 dconf 导出、软件包列表和快照日期。检查 `git diff` 和待提交文件，
确认没有敏感内容后再提交、推送。不要把整个家目录或 `.config` 直接打包。

凭据、私钥、登录态、Cookie、聊天记录、浏览器历史、用户词库、缓存、
日志、备份副本、运行锁文件和嵌套 Git 元数据不属于此配置快照。
SSH/GPG、代理订阅、AI 工具账户状态和 `/etc` 系统配置也未收录。
`.gitignore` 是辅助排除规则；更新配置时仍需检查文件内部是否含有密钥。
