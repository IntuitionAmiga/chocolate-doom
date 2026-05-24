#ifndef IEDOOM_SYS_STAT_H
#define IEDOOM_SYS_STAT_H

typedef unsigned int mode_t;
typedef long time_t;
typedef long off_t;

struct stat
{
    mode_t st_mode;
    off_t st_size;
    time_t st_mtime;
};

#define S_IFDIR 0040000
#define S_IFREG 0100000
#define S_ISDIR(m) (((m) & S_IFDIR) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFREG) == S_IFREG)

int stat(const char *path, struct stat *buf);
int mkdir(const char *path, mode_t mode);

#endif
