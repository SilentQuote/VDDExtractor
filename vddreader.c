#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BYTE(x) (x & 0xFF)

struct vddfile{
    int address;
    int sector;
    int filesize;
    char *name;
};

//reads n bytes as a string.
//todo: replace fgetc() loop with fread()
char *getstr(int n, FILE *f){
    char *str = malloc(n+1);
    memset(str, '\0', n+1);
    if(!str) exit(25);
    int i;
    unsigned char c;
    for(i = 0; i < n && (c = fgetc(f)) != EOF; i++){
        str[i] = c;
    }
    return str;
}

//uses getstr() to convert integer stored in little endian into int.
unsigned int getint(FILE *f){
    char *bytes = getstr(4, f);
    return BYTE(bytes[0]) + (BYTE(bytes[1]) << 8) + (BYTE(bytes[2]) << 16) + (BYTE(bytes[3]) << 24);
}

//quick filestream check
void FileCheck(FILE *file){
    if(!file){
        printf("File open error.\n");
        exit(25);
    }
}

//repacks files into VDD
void RepackVDD(char *filename, int filecount, struct vddfile *vddfiles){
    char outpath[20] = "out/";
    FILE *out = fopen(strcat(outpath, filename), "wb");
    FileCheck(out);
    int i, j;
    int sectorindex = (int) ceil(((filecount * 24.0) + 4.0) / 2048.0);
    //recalculate filesize and sectors
    for(i = 0; i < filecount; i++){
        char inpath[20] = "out/";
        FILE *next = fopen(strcat(inpath, vddfiles[i].name), "rb");
        FileCheck(next);
        fseek(next, 0L, SEEK_END);
        vddfiles[i].filesize = ftell(next);
        vddfiles[i].sector = sectorindex;
        vddfiles[i].address = vddfiles[i].sector * 0x800;
        sectorindex += (int) ceil(vddfiles[i].filesize / 2048.0);
        fclose(next);
    }

    //write filecount
    fwrite(&filecount, sizeof(char), 4, out);
    for(i = 0; i < filecount; i++){
        //write file name
        fwrite(vddfiles[i].name, sizeof(char), 16, out);
        //write sector index
        fwrite(&vddfiles[i].sector, sizeof(char), 4, out);
        //write file size
        fwrite(&vddfiles[i].filesize, sizeof(char), 4, out);
    }
    //pack files
    for(i = 0; i < filecount; i++){
        //pad to sector
        while(ftell(out) < vddfiles[i].address){
            fputc(0, out);
        }
        char nextpath[20] = "out/";
        FILE *next = fopen(strcat(nextpath, vddfiles[i].name), "rb");
        FileCheck(next);
        for(j = 0; j < vddfiles[i].filesize; j++)
            fputc(fgetc(next), out);
    }
    //round off to next sector
    while(ftell(out) % 0x800 != 0){
        fputc(0, out);
    }
    fclose(out);
    printf("Wrote VDD to %s\n", strcat(outpath, filename));
}



void ExtractFiles(FILE *vdd, unsigned int filecount, struct vddfile *vddfiles){
    int i, j;
    
    for(i = 0; i < filecount; i++){
        char outpath[20] = "out/";
        FILE *newfile = fopen(strcat(outpath, vddfiles[i].name), "wb");
        FileCheck(newfile);
        fseek(vdd, (long) vddfiles[i].address, SEEK_SET);
        for(j = 0; j < vddfiles[i].filesize; j++){
            fputc(fgetc(vdd), newfile);
        }
        fclose(newfile);
    }
}


int main(int argc, char **argv){
    FILE *vdd, *out;
    char *vddname;
    char c;
    unsigned int fileindex = 4;
    unsigned int filecount;
    int i;
    struct vddfile *vddfiles;
    int argExtract = 0;
    int argRepack = 0;
    int argInfo = 0;
    int argHelp = 0;
    if(argc < 2){
        printf("Please provide arguments\n\nexamples:\n\nvddreader --extract [filename.vdd]\nvddreader --repack [filename.vdd]\n vddreader --info [filename.vdd]\nvddreader --help\n");
        exit(1);
    }
    argExtract = !strcmp(argv[1], "--extract") && argc == 3;
    argRepack = !strcmp(argv[1], "--repack") && argc == 3;
    argInfo = !strcmp(argv[1], "--info") && argc == 3;
    argHelp = !strcmp(argv[1], "--help");
    if(!argExtract && !argRepack && !argInfo && !argHelp){
        printf("Please provide valid arguments.\nvddreader --help\n");
        exit(1);
    } else if(argExtract || argRepack || argInfo){
        vddname = argv[2];
    } else if(argHelp){
        printf("VDDReader by jegneg\n\nExtract VDD files from Puyo Puyo Box and Waku Puyo Dungeon\n\nUsage:\n\nvddreader --extract [filename.vdd]\nExtract all of the files contained in [filename.vdd] to 'out' folder, and export file information to 'vddinfo.txt'\n\nvddreader --repack [filename.vdd]\nRepack all of the files contained in 'out' folder into [filename.vdd]\n\nvddreader --info [filename.vdd]\nOnly export file structure information of [filename.vdd] to 'vddinfo.txt'\n\nvddreader --help\nView this help.\n");
        exit(0);
    }
    printf("Processing file...\n");
    if(argInfo || argExtract){
        out = fopen("vddinfo.txt", "w");
    }
    vdd = fopen(vddname, "rb");
    FileCheck(vdd);
    filecount = getint(vdd);
    vddfiles = malloc(sizeof(struct vddfile) * filecount);
    if(!vddfiles){
        printf("Malloc Fail. Requested %lu bytes. filecount %u sizeof %u", (long) sizeof(struct vddfile) * (long) filecount, filecount, sizeof(struct vddfile) );
        exit(25);
    }
    if(argInfo || argExtract){
        fprintf(out, "%s\n", vddname);
        fprintf(out, "file count: %d\n", filecount);
    }
    for(i = 0; i < filecount; i++){
        vddfiles[i].name = getstr(16, vdd);
        vddfiles[i].sector = getint(vdd);
        vddfiles[i].address = vddfiles[i].sector * 2048;
        vddfiles[i].filesize = getint(vdd);
        if(argInfo || argExtract){
            fprintf(out, "%s at 0x%X\n\tsector:\t\t0x%X\n\taddress:\t0x%X\n\tfile size:\t0x%X bytes\n",
                vddfiles[i].name, fileindex, vddfiles[i].sector, vddfiles[i].address, vddfiles[i].filesize);
        }
        fileindex += 24;
    }
    if(argInfo || argExtract){
        printf("VDD Information written to vddinfo.txt.\n");
    }
    if(argExtract){
        printf("Extracting...\n");
        ExtractFiles(vdd, filecount, vddfiles);
    } else if(argRepack){
        printf("\nRepacking...\n");
        RepackVDD(vddname, filecount, vddfiles);
    }
    fclose(vdd);
    if(out) fclose(out);
    printf("\nDone.\n");
    free(vddfiles);
    return 0;
}