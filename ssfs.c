#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/xattr.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

int shift = 10;

static const char *dirpath = "/home/almond/Documents";
// key yang diberikan dapat membuat error karena mengandung '.';
//char key[] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";

char key[] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M<b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
char map[256], revmap[256];

void mapping(){
    //init()
    for(int i=0; i<256; ++i){
        map[i] = (char) i;
        revmap[i] = (char) i;
    }

    for(int i=0; i<strlen(key); i++){
        map[(int)*(key + i)] = *(key + (i + shift) % strlen(key) );
        revmap[(int)*(key + (i + shift) % strlen(key))] = *(key + i);
    }
}

void printlog(char* command , int level, char* path){
    FILE* file;
    file = fopen("/home/almond/fs.log", "a+");
    time_t epoch;
    struct tm* timestamp;

    time(&epoch);
    timestamp= localtime (&epoch);
    if(level == 0) fprintf(file, "INFO");
    else if (level == 1) fprintf(file, "WARNING");
    fprintf(file, "::%02d%02d%02d-%02d:%02d:%02d::%s::%s\n", (timestamp->tm_year+1900)%100, 
        timestamp->tm_mon+1, timestamp->tm_mday, timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec, command ,path + strlen(dirpath));
    fclose(file);
}


void printlog2(char* command , int level, char* path, char* path2){
    FILE* file;
    file = fopen("/home/almond/fs.log", "a+");
    time_t epoch;
    struct tm* timestamp;

    time(&epoch);
    timestamp= localtime (&epoch);
    if(level == 0) fprintf(file, "INFO");
    else if (level == 1) fprintf(file, "WARNING");
    fprintf(file, "::%02d%02d%02d-%02d:%02d:%02d::%s::%s::%s\n", (timestamp->tm_year+1900)%100, 
        timestamp->tm_mon+1, timestamp->tm_mday, timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec, command ,path + strlen(dirpath), path2 + strlen(dirpath));
    fclose(file);
}

void logDatabase(char *fpath){
    FILE* file;
    file = fopen("/home/almond/Documents/Database/log", "a+");
    time_t epoch;
    struct tm* timestamp;

    time(&epoch);
    timestamp= localtime (&epoch);
    fprintf(file, "ENCRYPTED_FOLDER::%02d%02d%02d-%02d:%02d:%02d::%s\n", (timestamp->tm_year+1900)%100, 
        timestamp->tm_mon+1, timestamp->tm_mday, timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec, fpath + strlen(dirpath));
    fclose(file);
}

//parameter cuma nama 1 file ;
void encryptName(char* path){
    if(!strcmp(path,".") || !strcmp(path,"..")) return;
    char* ext;
    ext = strrchr(path,'.');
    if(ext != NULL){
        for(char* itr=path; itr!=ext; ++itr){
            *itr = map[(int)(*itr)];
        }
    }
    else{
        for(int itr=0; itr < strlen(path) ; ++itr){
            path[itr] = map[(int)path[itr]];
        }
    }
}

void decryptName(char* path){
    if(!strcmp(path,".") || !strcmp(path,"..")) return;
    char* ext;
    ext = strrchr(path,'.');
    if(ext != NULL){
        for(char* itr=path; itr!=ext; ++itr){
            *itr = revmap[(int)(*itr)];
        }
    }
    else{
        for(int itr=0; itr < strlen(path) ; ++itr){
            path[itr] = revmap[(int)path[itr]];
        }
        printf("huhu : %s\n", path);
    }
}

// check apakah full path didalam folder encv1_
bool isEncryptName(char* path){
    char fullpath[1024];
    sprintf(fullpath, "%s%s", dirpath, path);
    char* pattern = "encv1_";
    
    char* last = strrchr(fullpath, '/');

    if(last){
        for(char* itr = fullpath; itr < last - strlen(pattern) ; ++itr){
            if(*itr == '/'){
                //printf("%s\n", itr + 1);
                if(!strncmp(itr + 1, pattern, strlen(pattern))) return true;
            }
        }
    }
    return false;
}

static int xmp_getattr(const char *path, struct stat *stbuf){
    int res;
    char fpath[1000];
    char name[1000];
    sprintf(name,"%s",path);
    sprintf(fpath, "%s%s",dirpath,name);
    if(isEncryptName(fpath)){
        char* temp;
        printf("getattr prev: %s\n", fpath);
        for(char* itr = fpath; itr < fpath + strlen(fpath) - 6; ++itr){
            if(*itr == '/' && !strncmp(itr + 1, "encv1_", 6)){
                temp = strchr(itr+1, '/');
                printf("hehe: %s\n", temp);
            }
        }
        decryptName(temp+1);
    }
    printf("getattr custom %s\n", fpath);
    res = lstat(fpath, stbuf);
    if (res != 0){
        return -ENOENT;
    }

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    int res;
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;
    char fpath[1000];
    char name[1000];
    if (strcmp(path, "/") == 0){
        sprintf(fpath, "%s", dirpath);
    }else{
        sprintf(name,"%s",path);
        if(isEncryptName(name)){
            char *temp = strrchr(name, '/');
            decryptName(temp + 1);
        }
        sprintf(fpath, "%s%s",dirpath,name);
    }

    printf("readdir dir: %s\n", fpath);
    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        char fullpathname[1000];
        sprintf(fullpathname, "%s/%s", fpath, de->d_name);
        char temp[1000];
        strcpy(temp,de->d_name);

        if(isEncryptName(fullpathname)){
            encryptName(temp);
        }

        res = (filler(buf, temp, &st, 0));
        if(res!=0) break;
    }
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
    printf("Begin read\n");
    char fpath[1000];
    char name[1000];
    if (strcmp(path, "/") == 0){
        sprintf(fpath, "%s", dirpath);
    }else{
        sprintf(name,"%s",path);
        sprintf(fpath, "%s%s",dirpath,name);
        if(isEncryptName(fpath)){
            char* temp;
            for(char* itr = fpath; itr < fpath + strlen(fpath) - 6; ++itr){
                if(*itr == '/' && !strncmp(itr + 1, "encv1_", 6)){
                    temp = strchr(itr+1, '/');
                }
            }
            decryptName(temp+1);
        }
    }
    int res = 0;
    int fd = 0 ;

    (void) fi;
    printf("fpath read : %s\n", fpath);
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    printf("End read\n");
    return res;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    char fpath[1024];
    sprintf(fpath,"%s%s",dirpath,path);
    char* newname = strrchr(fpath, '/');
    if(isEncryptName(fpath)){
        decryptName(newname + 1);
    }
    int res = mkdir(fpath, mode);
    if (res == -1)
        return -errno;

    if(!strncmp("encv1_", newname + 1, 6)){
        struct stat st = {0};
        if (stat("/home/almond/Documents/Database", &st) == -1) {
            mkdir("/home/almond/Documents/Database", 0777);
        }
        logDatabase(fpath);
    }

    printlog("MKDIR", 0, fpath);
    return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;
    char fpath[1024];
    sprintf(fpath,"%s%s",dirpath,path);
    char* newname = strrchr(fpath, '/');
    if(isEncryptName(fpath)){
        decryptName(newname + 1);
    }
    res = mknod(fpath, mode, rdev);
    if(res == -1)
        return -errno;
    printlog("CREAT", 0,fpath);

    return 0;
}

static int xmp_rename(const char *from, const char *to)
{
    int res;

    char from_fpath[1000];
    char to_fpath[1000];

    char name[1000];
    sprintf(name,"%s",from);
    sprintf(from_fpath, "%s%s",dirpath,name);
    sprintf(to_fpath,"%s%s",dirpath,to);
    
    char* temp;
    if(isEncryptName(from_fpath)){
        printf("getattr prev f: %s\n", from_fpath);
        for(char* itr = from_fpath; itr < from_fpath + strlen(from_fpath) - 6; ++itr){
            if(*itr == '/' && !strncmp(itr + 1, "encv1_", 6)){
                temp = strchr(itr+1, '/');
                printf("hehe: %s\n", temp);
            }
        }
        decryptName(temp+1);
    }

    res = rename(from_fpath, to_fpath);
    printf("to_fpath nya : %s\n", to_fpath);
    if(res == -1)
        return -errno;
    printlog2("RENAME", 0, from_fpath, to_fpath);


    char folderName[1000];
    sprintf(folderName, "%s", to_fpath);
    temp = strrchr(folderName,'/');
    if(!strncmp("encv1_", temp + 1, 6)){
        struct stat st = {0};
        if (stat("/home/almond/Documents/Database", &st) == -1) {
            mkdir("/home/almond/Documents/Database", 0777);
        }
        printf("hudashdsahda\n");
        logDatabase(to_fpath);
    }

    return 0;
}

static int xmp_unlink(const char *path)
{
    int res;
    char fpath[1024];
    sprintf(fpath,"%s%s",dirpath,path);
    char* newname = strrchr(fpath, '/');
    if(isEncryptName(fpath)){
        decryptName(newname + 1);
    }

    res = unlink(fpath);
    if (res == -1)
        return -errno;
    printlog("UNLINK", 1, fpath);

    return 0;
}

static int xmp_rmdir(const char *path)
{
    int res;
    char fpath[1024];
    sprintf(fpath,"%s%s",dirpath,path);
    char* newname = strrchr(fpath, '/');
    if(isEncryptName(fpath)){
        decryptName(newname + 1);
    }

    res = rmdir(fpath);
    if (res == -1)
        return -errno;
    printlog("RMDIR", 1, fpath);

    return 0;
}

 
static struct fuse_operations xmp_oper = {
    .getattr    = xmp_getattr,
    .readdir    = xmp_readdir,
    .read       = xmp_read,
    .mkdir      = xmp_mkdir,
    .mknod      = xmp_mknod, 
    .rename     = xmp_rename,
    .unlink     = xmp_unlink,
    .rmdir      = xmp_rmdir,
};
 
int main(int argc, char *argv[])
{
    umask(0);
    mapping();
    
    return fuse_main(argc, argv, &xmp_oper, NULL);
}