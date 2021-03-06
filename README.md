# Simba

I don't like Samba, thus I created Simba, a tiny simple SMB client written in C++ and based on [libdsm](https://github.com/videolabs/libdsm).

![logo](logo.png)

The client implements only some essential file/directory operations.
The most relevant supported operations are upload and download of files.

The client supports also a GTK3 gui (all operations are implemented except move).

## Usage
```
$ ./simba -h                                     
Lightweight SMB client
Usage:
  Simba [OPTION...]
  -g, --gui           Show GTK3 gui
  -h, --help          Show this help menu
  -s, --server arg    SMB Server IP Address
  -d, --domain arg    Domain/Workgroup
  -a, --share arg     Share name
  -u, --username arg  Username


Example:
$ ./simba -s 127.0.0.1 -d example -a myfs -u test            
Insert password:
Logged in 
> help
Commands:
ls <path>	List files and directories
get <path>	Donwload the specified file to current directory
put <local path>,<remote path>	Upload the specified local file to remote path
rm <path>	Remove the specified file
mv <old path>,<new path>	Move the specified file from oldpath to newpath
mkdir <path>	create a new directory
rmdir <path>	Remove a directory
```

## Compile

Compile `libdsm` first, then you can use `make` to compile the command-line client or `make gui` to include also the gtk based gui support.
