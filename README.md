# fuse3-p7zip

## Abstract

fuse3 file system that uses the p7zip library to mount archives (read only)

## Synopsis

```
$ fuse3-p7zip archive.7z /tmp/mnt
$ ls -la /tmp/mnt
$ fusermount -u /tmp/mnt
```
## Description

This application is an implementation of Filesystem in Userspace (FUSE) version 3 backed by 7zip library. It's able to
mount any archive supported by 7zip as a filesysterm mountpoint and have read only access to any file in this archive.

## Dependencies

* Linux
* fuse3
* p7zip

## Configuration

### Environment variables

* FUSE3_P7ZIP_LIBRARY

	Change where application searches for 7z.so library. By default this is `/usr/lib/p7zip/7z.so`

* FUSE3_P7ZIP_FORCE_ENCODING

	Some archives (especially those ones created on windows) have file names encoded in non-unicode encoding. This
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
	.img file - application will extract the whole partition to /tmp. Most of the time on modern linux it's a RAM based
	folder) To prevent this situation you can set this environment variable to a maximum allowed file size. Attempts
	opeining any file in archive bigger than this will end up in E2BIG error.

## Author

* Andrew Grechkin

## License

* GPL v3
