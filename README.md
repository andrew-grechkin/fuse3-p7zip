# fuse3-p7zip

[![Build status](https://github.com/andrew-grechkin/fuse3-p7zip/workflows/CMake/badge.svg)](https://github.com/andrew-grechkin/fuse3-p7zip/actions)

## Abstract

fuse3 file system that uses the p7zip library to mount archives (read only)

## Synopsis

```
$ fuse3-p7zip archive.7z /tmp/mnt
$ ls -la /tmp/mnt
$ fusermount -u /tmp/mnt
```

## Description

https://github.com/andrew-grechkin/fuse3-p7zip

This application is an implementation of Filesystem in Userspace (FUSE) version 3 backed by 7zip library. It's able to
mount any archive supported by 7zip as a filesystem mountpoint and have read only access to any file in this archive.

## Dependencies

* Linux
* fuse3
* p7zip

## Enforcing codepage by renaming archive

If archive name matches regex `m/[.] cp\d{3,} [.]/ix` (for example `cyrillic-created-on-windows.cp866.zip`) then matched
encoding `cp866` will be enforced for listing archived files/directories.

## Configuration

### Command line options

* -p `<PASSWORD>`
* --password=`<PASSWORD>`

Provide a password which will be used if archive is protected with a password.

* --passfile `</path/to/password/file>`

Provide a path to a file with a password which will be used if archive is protected with a password.

### Environment variables

* FUSE3_P7ZIP_LIBRARY

Change where application searches for 7z.so library. By default this is `/usr/lib/p7zip/7z.so`

* FUSE3_P7ZIP_PASSWORD

Provide a password which will be used if archive is protected with a password.

* FUSE3_P7ZIP_FORCE_ENCODING

Some archives (especially those ones created on windows) have file names encoded in non Unicode encoding. This
environment variable forces application to use alternative encoding to decode file names.

for example:

```
$ fuse3-p7zip cyrillic-created-on-windows.zip /tmp/mnt
$ ls /tmp/mnt
''$'\302\217''à¨¢¥â.txt'

$ fusermount -u /tmp/mnt

$ FUSE3_P7ZIP_FORCE_ENCODING=CP866 fuse3-p7zip cyrillic-created-on-windows.zip /tmp/mnt
$ ls /tmp/mnt
'Привет.txt'
```

* FUSE3_P7ZIP_MAX_OPEN_SIZE

The nature of 7z API does not allow to have random access to the content of a file in archive. This means that each
file should be extracted to a temporary folder on access. If this file is too big (for example it's partition in an
.img file - application will extract the whole partition to /tmp. Most of the time on modern Linux it's a RAM based
folder). To prevent this situation you can set this environment variable to a maximum allowed file size in bytes.
Attempts openining any file in archive bigger than this will end up in E2BIG error.

* FUSE3_P7ZIP_FORMATS

When opening a file the application tries all formats in the alphabetical order. Sometimes it's desired to open a file
with a format later in the list. This environment variable allows to override formats order.

for example (try open .iso file with Iso or Udf formats first and only fallback on APM and GPT formats)

```
FUSE3_P7ZIP_FORMATS="Iso:Udf:*:APM:GPT" fuse3-p7zip ubuntu-20.04-desktop-amd64.iso /tmp/mnt
```

To check the list of all available formats run `7z i` command

## Examples

### vifm config snippet (mount 7zip supported archive with fuse3-p7zip in vifm)

> I suggest to use [archivemount](https://github.com/cybernoid/archivemount) for all tar-based archives as priority and
> only fallback to p7zip for such archives

```vim
filetype *.tar,
	\*.tar.bz2,*.tbz2,*.tbz,*.tz2,*.tar.gz,*.tgz,*.taz,*.tar.lz,*.tlzip,*.tar.lzma,*.tlzma,*.tlz,
	\*.tar.lzop,*.tlzo,*.tlzop,*.tar.xz,*.txz,*.tar.zst,*.tzst,*.tar.Z,*.taZ,
	\*.bz2,*.gz,*.lz,*.lzma,*.lzop,*.xz,*.zst,*.Z,
	\*.jar,*.war,*.ear,*.oxt,*.apkg,*.lha,*.ar,*.cpio,*.rpm,*.deb,*.udeb,*.mtree,*.xar,
	\ {Mount with archivemount}
	\ FUSE_MOUNT|archivemount %SOURCE_FILE %DESTINATION_DIR,
filetype *.7z,
	\*.dsl.dz,*.tar,*.a,*.so,lib*.so.*,*.zip,*.ova,*.sfs,
	\*.apk,*.apm,*.ar,*.arj,*.cab,*.chm,*.cpio,*.cramfs,*.deb,*.dll,*.dmg,*.doc,*.esd,*.exe,
	\*.flv,*.hxs,*.img,*.iso,*.iso,*.jar,*.lib,*.macho,*.msi,*.msp,*.nsis,*.pkg,*.pmd,*.ppt,
	\*.qcow,*.qcow2,*.qcow2c,*.r00,*.rar,*.raw,*.rpm,*.squashfs,*.swf,*.swm,*.sys,*.vdi,*.vhd,*.vmdk,*.wim,*.xar,*.xls,
	\*.tar.bz2,*.tbz2,*.tbz,*.tz2,*.tar.gz,*.tgz,*.taz,*.tar.lz,*.tlzip,*.tar.lzma,*.tlzma,*.tlz,
	\*.tar.lzop,*.tlzo,*.tlzop,*.tar.xz,*.txz,*.tar.zst,*.tzst,*.tar.Z,*.taZ,
	\*.bz2,*.gz,*.lz,*.lzma,*.lzop,*.xz,*.zst,*.Z
	\ {Mount with fuse3-p7zip}
	\ FUSE_MOUNT|fuse3-p7zip %SOURCE_FILE %DESTINATION_DIR,
```

## Author

* Andrew Grechkin

## License

* GPL v3
