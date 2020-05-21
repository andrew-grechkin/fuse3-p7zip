function! s:find_git_root()
	return system('git rev-parse --show-toplevel 2> /dev/null')[:-2]
endfunction

if !exists("g:ale_configs_updated")
	let g:ale_configs_updated        = 1
	let root = s:find_git_root()
	"let g:ale_cpp_clang_options      = g:ale_cpp_options." -std=c++20 -I /usr/include/fuse3 -I ".expand('%:p:h')."/include -I ".expand('%:p:h')."/src/include -I ".expand('%:p:h')."/3rdparty/p7zip -I ".expand('%:p:h')."/3rdparty/p7zip/CPP -I ".expand('%:p:h')."/3rdparty/p7zip/CPP/include_windows -I ".expand('%:p:h')."/src/7zip/include -D_FILE_OFFSET_BITS=64"
	let g:ale_cpp_clang_options      = g:ale_cpp_options." -std=c++20 -I /usr/include/fuse3 -I ".root."/include -I ".root."/src/include -I ".root."/3rdparty/p7zip -I ".root."/3rdparty/p7zip/CPP -I ".root."/3rdparty/p7zip/CPP/include_windows -I ".root."/src/7zip/include -D_FILE_OFFSET_BITS=64"
	let g:ale_cpp_clangcheck_options = g:ale_cpp_clang_options
	let g:ale_cpp_clangd_options     = g:ale_cpp_clang_options
	let g:ale_cpp_gcc_options        = g:ale_cpp_clang_options
	let g:ale_cpp_clangtidy_options  = g:ale_cpp_clang_options
endif
