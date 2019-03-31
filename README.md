# vim-fcitx-ctl

This is fcitx (input method) control script for Vim with imactivefunc / imstatusfunc.

Vim script of this plugin is based on [vim-uim-ctrl-imaf.vim](https://github.com/presuku/vim-uim-ctl-imaf.vim) and c-lang part is based on [fcitx-remote](https://gitlab.com/fcitx/fcitx/blob/master/src/module/dbus/dbusremote.c).  
([vim-uim-ctrl-imaf.vim](https://github.com/presuku/vim-uim-ctl-imaf.vim) is based on [uim-ctl](https://code.google.com/p/vim-soko/source/browse/trunk/uim-ctl) and [uim-ctlso](https://github.com/koron/imcsc-vim).)


## How to install

* Download
  ```sh
  $ cd ~/.vim/<path to plugin dir>
  $ git clone https://github.com/presuku/vim-fcitx-ctl.git
  ```

* Build
  ```sh
  $ sudo apt install libdbus-1-dev fcitx-libs-dev
  $ cd ~/.vim/<path to plugin dir>/vim-fcitx-ctl
  $ make
  ```

* Setting of fcitx
  ```
  echo 'FCITX_NO_PREEDIT_APPS=""' | sudo tee -a /etc/environment
  ```

* vim-plug
  ```viml
  Plug 'presuku/vim-fcitx-ctl'
  ```

## LICENSE

All scripts are distributed under one of the Vim, MIT or BSD licenses.

