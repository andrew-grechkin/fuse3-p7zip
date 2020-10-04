if !exists("g:ale_configs_updated")
	let g:ale_configs_updated = 1
	let s:root = dir#git_root()
	let s:options = [
		\"-I /usr/include/fuse3",
		\"-I ".s:root."/include",
		\"-I ".s:root."/src/include",
		\"-I ".s:root."/3rdparty/p7zip",
		\"-I ".s:root."/3rdparty/p7zip/CPP",
		\"-I ".s:root."/3rdparty/p7zip/CPP/include_windows",
		\"-I ".s:root."/src/7zip/include",
		\"-D_FILE_OFFSET_BITS=64"
	\]

	let g:ale_cpp_clang_options      = g:ale_cpp_options . ' ' . join(s:options, ' ')
	let g:ale_cpp_clangcheck_options = join(map(s:options, {key, val -> "--extra-arg='" . val . "'"}), ' ')
	"let g:ale_cpp_clangd_options     = g:ale_cpp_clang_options
	let g:ale_cpp_gcc_options        = g:ale_cpp_clang_options
	let g:ale_cpp_clangtidy_options  = g:ale_cpp_clang_options
endif
