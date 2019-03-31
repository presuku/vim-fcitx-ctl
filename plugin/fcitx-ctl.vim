" vim:set sts=2 sw=2 tw=0 et:

" fcitx-ctl.vim based on uimfep-vim.vim, uim-ctlso.vim and uim-ctl.vim.

let s:save_cpo = &cpo
set cpo&vim

scriptencoding utf-8

" Check imactivatefunc (and imstatusfunc)
if !exists('&imactivatefunc')
  finish
endif

if exists("g:loaded_vim_fcitx_ctl")
  finish
endif
let g:loaded_vim_fcitx_ctl = 1

let g:fcitx_ctl_dll_path = expand("<sfile>:p:h") . "/" . get(g:, "fcitx_ctl_dll", "fcitx-ctl.so")

if filereadable(g:fcitx_ctl_dll_path)
  augroup FcitxHelperInit
    au!
    autocmd InsertEnter * let s:err = libcallnr(g:fcitx_ctl_dll_path, 'load', g:fcitx_ctl_dll_path) | autocmd! FcitxHelperInit
  augroup END

  augroup FcitxHelperFinalize
    au!
    autocmd VimLeave * call libcallnr(g:fcitx_ctl_dll_path, "unload", 0)
  augroup END

  set imactivatefunc=fcitx_ctl#im_status_set
  set imstatusfunc=fcitx_ctl#im_status_get
endif

let &cpo = s:save_cpo
unlet s:save_cpo

