" vim:set sts=2 sw=2 tw=0 et:

" uim-ctl-imaf.vim based on uimfep-vim.vim, uim-ctlso.vim and uim-ctl.vim.

let s:save_cpo = &cpo
set cpo&vim

scriptencoding utf-8

function! fcitx_ctl#im_status_set(active)
  if has("patch-8.1.0053")
    let arg = string(a:active)
  else
    let arg = a:active
  endif
  call libcallnr(g:fcitx_ctl_dll_path, 'im_set', arg)
endfunction

function! fcitx_ctl#im_status_get()
  return libcallnr(g:fcitx_ctl_dll_path, 'is_im_enable', 0)
endfunction

let &cpo = s:save_cpo
unlet s:save_cpo

