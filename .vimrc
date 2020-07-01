"  " Colors & User Interface
set tabstop=3
set shiftwidth=3
set secure
set smarttab
set noswapfile
set et
set nu
set wrap
set ai
set cin
set showmatch
set colorcolumn=80
highlight ColorColumn ctermbg=DarkGrey
highlight SpellBad term=standout ctermfg=0 ctermbg=11 guifg=Blue guibg=Yellow
highlight LineNr ctermfg=DarkGrey guifg=Black
highlight PreProc ctermfg=Green guifg=Green
highlight Function ctermfg=Red guifg=Red
highlight Comment ctermfg=0 guifg=#3eb489
highlight Identifier term=underline cterm=bold ctermfg=Red guifg=Red
set hlsearch
set incsearch
set ignorecase
set lz
set list
set ffs=unix,dos,mac
set clipboard=unnamedplus,unnamed
set clipboard+=unnamed
let g:ycm_global_ycm_extra_conf = '~/.vim/plugged/YouCompleteMe/.ycm_c-c++_conf.py' 
map <C-n> :NERDTreeToggle<CR>
map <C-p> :YcmCompleter GetType<CR>
call plug#begin('~/.vim/plugged')
Plug 'https://github.com/junegunn/vim-github-dashboard.git'
Plug 'Valloric/YouCompleteMe', { 'do': './install.py --clang-complete --java-complete' }
Plug 'preservim/nerdtree'
Plug 'junegunn/fzf'
call plug#end()
