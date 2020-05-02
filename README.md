# SoalShiftSISOP20_modul4_E12

## Soal 1.1
Pada awalnya dilakukan mapping untuk substitusi pada enkripsi dan dekripsi. Fungsi mapping ini akan membuat isi dari array map tiap index menjadi substitusi untuk enkripsi sebaliknya untuk revmap berisi substitusi untuk dekripsinya. Mapping ini dilakukan untuk mengefisiensikan program agar pada saat enkripsi maupun dekripsi tidak perlu melakukan searching ulang secara linear untuk tiap karakter melainkan berfungsi sebagai hash map untuk enkripsi dan dekripsi. Untuk implementasinya sebagai berikut.
```
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
```
Kemudian untuk fungsi enkripsi yaitu fungsi encryptName() dimana pada fungsi ini akan melakukan enkripsi pada path terakhir dari suatu full path, misalnya parameter inputnya adalah “encv1_hehe/testfile.jpg” maka bagian yang akan dienkripsi adalah “testfile.jpg”. Didalam fungsi encryptName() juga akan mencari extensi yang dimiliki oleh file yang ada menggunakan strrchr() dimana strrchr nantinya akan mereturn pointer pada karakter terakhir yang dicari sehingga bila ditemukan ekstensi maka bagian yang terenkripsi hingga karakter sebelum ‘.’ terakhir, sedangkan bila tidak ditemukan akan melakukan enkripsi hingga path terakhir. Hal yang sama juga berlaku untuk dekripsi hanya saja pada fungsi decryptName() hanya mengubah map menjadi revmap.
```
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
```
Untuk dekripsinya sebagai berikut.
```
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
```
Untuk mengecek apakah suatu path berada didalam folder enkripsi maka digunakan fungsi isEncryptName() dimana akan mereturn true bila path tersebut didalam folder enkripsi dan false untuk sebaliknya. Untuk implementasinya sebagai berikut.
```
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
```
Untuk melakukan enkripsi pada folder enkripsi, maka untuk setiap file dan direktori didalam direktori enkripsi akan dilakukan enkripsi ketika melakukan readdir. Pada readdir tinggal memasukkan path enkripsi pada buffer filler yang ada. Agar path dapat terbaca dengan folder yang asli maka sebelum melakukan enkripsi untuk tiap fungsi fuse dilakukan dekripsi terlebih dahulu.
```
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
```
Untuk log databasenya, diletakkan pada mkdir dan rename dimana bila direktori encrypt baru dibuat  maupun direname akan dicatat dalam log database. Untuk log databasenya kami membuat formatnya mirip untuk penyelesaian soal 1.4.
```

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

```

## Soal 1.4


Kendala :
1. Sudah, tetapi masih memungkinkan bug
2. Belum sempat mengerjakan. Masih bingung melakukan split file
3. Belum mengerti maksud soalnya
4. Untuk log sudah jadi, mungkin belum mengimplementasikan untuk semua command yang ada
