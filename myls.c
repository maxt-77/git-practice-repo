#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

/* Returns 1 if the extension of name is equal to ext (ext without the dot) */
int hasExtension(const char *name, const char *ext)
{
    const char *dot = strrchr(name, '.');

    if(dot == NULL)
        return 0;

    return strcmp(dot + 1, ext) == 0;
}

int listDirectory(const char *dirName, int onlyFiles, int onlyDirs, const char *ext)
{
    DIR *dp = NULL;
    struct dirent *entry = NULL;
    struct stat st;
    char path[1024];
    int isFile, isDir;

    /* Open the directory */
    errno = 0;
    dp = opendir(dirName);
    if(dp == NULL)
    {
        perror ("Error opening directory!");
        return EXIT_FAILURE;
    }

    /* Go through all the entries of the directory */
    while((entry = readdir(dp)) != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dirName, entry->d_name);
        if(stat(path, &st) == -1)
        {
            perror ("Error getting file information!");
            continue;
        }

        isFile = S_ISREG(st.st_mode);
        isDir = S_ISDIR(st.st_mode);

        /* -f: only files; -d: only directories (both: files and directories) */
        if(onlyFiles && !onlyDirs && !isFile)
            continue;
        if(onlyDirs && !onlyFiles && !isDir)
            continue;

        /* -e ext: only files with extension ext */
        if(ext != NULL && (!isFile || !hasExtension(entry->d_name, ext)))
            continue;

        printf("%s\n", entry->d_name);
    }

    closedir(dp);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int opt, i;
    int onlyFiles = 0, onlyDirs = 0;
    char *ext = NULL;
    int ret = EXIT_SUCCESS;

    while ((opt = getopt(argc, argv, "fde:")) != -1)
    {
        switch (opt)
        {
            case 'f':
                onlyFiles = 1;
                break;
            case 'd':
                onlyDirs = 1;
                break;
            case 'e':
                ext = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-f] [-d] [-e ext] [dir ...]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /* Without directories as arguments, list the current directory */
    if (optind >= argc)
    {
        if(listDirectory(".", onlyFiles, onlyDirs, ext) != EXIT_SUCCESS)
            ret = EXIT_FAILURE;
        return ret;
    }

    for (i = optind ; i < argc ; i++)
    {
        if(argc - optind > 1)
        {
            if(i > optind)
                printf("\n");
            printf("%s:\n", argv[i]);
        }

        if(listDirectory(argv[i], onlyFiles, onlyDirs, ext) != EXIT_SUCCESS)
            ret = EXIT_FAILURE;
    }

    return ret;
}
