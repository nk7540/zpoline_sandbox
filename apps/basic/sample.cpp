#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>

const size_t MAX_PATH_SIZE = 4096;
const char DELIM = '/';

void process_component(std::string component_path, int &fd)
{
    std::string target_path;
    std::vector<std::string> target_components;

    char temp[MAX_PATH_SIZE];
    int length = readlink(component_path.c_str(), temp, sizeof(temp) - 1);
    // event: readlink

    if (length != -1)
    {
        temp[length] = '\0';
        target_path = temp;
    }
    else
    {
        target_path = component_path;
    }

    std::istringstream stream(target_path);
    std::string component;
    while (std::getline(stream, component, DELIM))
    {
        assert(component.find("/") == std::string::npos);
        fd = openat(fd, component.c_str(), O_NOFOLLOW);
        // event: open_nonsym
        // event: next_nonsym
    }
}

int safe_open(const std::string &pathname, int mode)
{
    int fd;
    size_t pos = 0;

    if (pathname[0] == '/')
    {
        // event: absolute
        fd = open("/", mode);
        // event: root_opened
        pos = 1;
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
        std::cerr << "Error opening directory" << std::endl;
        return -1;
    }

    std::string component_path = pathname.substr(pos);
    std::istringstream stream(component_path);
    std::string component;
    while (std::getline(stream, component, DELIM))
    {
        process_component(component, fd);
        // event: next_component
    }
    // event: fully_traversed
    // assert: property.txt
    return fd;
}

int main()
{
    std::string pathname = "./main.c";
    int fd = safe_open(pathname, O_RDONLY);
    std::cout << fd << std::endl;
}
