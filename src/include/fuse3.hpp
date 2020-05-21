#ifndef __FUSE3_7Z__FUSE3_HPP__
#define __FUSE3_7Z__FUSE3_HPP__

#define FUSE_USE_VERSION 35

#include <fuse.h> // <fuse3/fuse.h>
#include <fuse_lowlevel.h>

#include "7zip.hpp"

#include <cstddef>
#include <memory>
#include <string>

class Fuse {
public:
	~Fuse();
	Fuse(int argc, char** argv);

	ssize_t            execute(sevenzip::IArchive* archive);
	const std::string& path() const;

private:
	class Params;
	class Operations;

	struct fuse*                fuse;
	std::unique_ptr<Params>     _params;
	std::unique_ptr<Operations> _operations;
};

#endif
