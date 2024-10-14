# VDDReader
A command line tool to extract and repack Puyo Box and Waku Puyo Dungeon's VDD files.

### Usage

```
$ vddreader --help
```
Views basic help screen.

```
$ vddreader --extract [filename.vdd]
```
Extract all of the files contained in `[filename.vdd]` to `'out'` folder, and export file information to `'vddinfo.txt'`

```
$ vddreader --repack [filename.vdd]
```
Repack all of the files contained in `'out'` folder into `[filename.vdd]`

```
$ vddreader --info [filename.vdd]
```
Only export file structure information of `[filename.vdd]` into `'vddinfo.txt'`

### Building

```
$ gcc vddreader.c -o vddreader -lm
```
Important: A folder named `'out'` must be included alongside with the executable, otherwise it wont work. I was simply too lazy to make the program create its own folder.
