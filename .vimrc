" Vim counterpart of ~/.config/nvim: appearance, mappings and abbreviations.
set nocompatible
set encoding=utf-8
scriptencoding utf-8
let mapleader = ' '

runtime config/options.vim
filetype plugin indent on
syntax enable
colorscheme nvim_port
runtime config/mappings.vim
runtime config/abbreviations.vim
runtime config/snippets.vim

augroup NvimPort
  autocmd!
  autocmd VimEnter,TermResponse * call nvim_port#probe_terminal_sync()
  autocmd FocusGained,CursorHold,CursorHoldI * call nvim_port#sync_theme()
  autocmd InsertLeave,TextChanged,CursorHold * call nvim_port#autosave()
  autocmd BufRead,BufNewFile *.sage setlocal filetype=python
  autocmd FileType typst setlocal commentstring=//\ %s
augroup END
if exists('g:nvim_port_theme_timer')
  call timer_stop(g:nvim_port_theme_timer)
endif
if has('timers')
  let g:nvim_port_theme_timer = timer_start(1000, function('nvim_port#sync_theme'), {'repeat': -1})
endif
command! -nargs=? NvimTheme call nvim_port#set_theme(<q-args>)
command! NvimPortHelp execute 'edit ' . fnameescape(expand('~/.vim/doc/nvim-port.txt'))
