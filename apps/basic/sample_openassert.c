#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#define MAX_PATH_SIZE 4096
#define DELIM "/"

void process_component(char *component_path, int *fd)
{
    char target_path[MAX_PATH_SIZE];
    char *target;
    char *target_component;
    int length;

    length = readlink(component_path, target_path, sizeof(target_path) - 1);
    // event: readlink

    if (length != -1)
    {
        target_path[length] = '\0';
        target = target_path;
    }
    else
    {
        target = component_path;
    }

    char *saveptr2;
    target_component = __strtok_r(target, DELIM, &saveptr2);
    while (target_component != NULL)
    {
        assert(strchr(target_component, "/") == NULL);
        *fd = openat(*fd, target_component, __O_NOFOLLOW);
        // event: open_nonsym
        target_component = __strtok_r(NULL, DELIM, &saveptr2);
        // event: next_nonsym
    }
}

int safe_open(char *pathname, int mode)
{
    int fd;
    char *component;

    if (pathname[0] == '/')
    {
        // event: absolute
        fd = open("/", mode);
        // event: root_opened
        pathname++;
        // event: made_relative
    }
    else
    {
        // event: not_absolute
        fd = open(".", mode);
        // event: cwd_opened
    }

    if (fd == -1)
    {
        perror("Error opening directory");
        return 1;
    }

    char *saveptr;
    component = __strtok_r(pathname, DELIM, &saveptr);
    while (component != NULL)
    {
        process_component(component, &fd);
        component = __strtok_r(NULL, DELIM, &saveptr);
        // event: next_component
    }
    // event: fully_traversed
    // assert: property.txt
    return fd;
}

int main()
{
    int fd;
    char pathname[] = "./main.c";
    fd = safe_open(pathname, O_RDONLY);
    printf("%d\n", fd);
}
