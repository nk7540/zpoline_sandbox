extern "C"
{
    // #include <string.h>
    // #include <stdlib.h>
    // #include <fcntl.h>
    // #include <unistd.h>
    // #include <sys/types.h>
    // #include <sys/stat.h>
    // #include <assert.h>
}
// #include <cstdio>
// #include <cstdlib>
// #include <cassert>
#include <fstream>
#include <iostream>
// #include <vector>
#include <string>
#include <map>
#include <set>

std::vector<std::string> event_sequence;
struct Transition
{
    std::string from;
    std::string event;
    std::string to;
};

void generate_event(const std::string &event_name)
{
    event_sequence.push_back(event_name);
}

bool accepts_sequence(const std::string &automaton_file)
{
    // for (auto const &e : event_sequence)
    //     std::cout << e << std::endl;

    std::ifstream file(automaton_file);

    if (!file.is_open())
    {
        std::cerr << "Failed to open the automaton file." << std::endl;
        return false;
    }

    std::map<std::string, std::map<std::string, std::string>> transitions; // from -> event -> to
    std::set<std::string> accept_states;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.substr(0, 6) == "ACCEPT")
        {
            accept_states.insert(line.substr(7)); // Extract state after the keyword ACCEPT
            continue;
        }

        Transition t;
        size_t pos1 = line.find(",");
        size_t pos2 = line.find(",", pos1 + 1);
        t.from = line.substr(0, pos1);
        t.event = line.substr(pos1 + 1, pos2 - pos1 - 1);
        t.to = line.substr(pos2 + 1);

        transitions[t.from][t.event] = t.to;
    }

    std::string current_state = "0"; // assuming initial state is "0"
    for (std::vector<std::string>::const_iterator it = event_sequence.begin(); it != event_sequence.end(); ++it)
    {
        const std::string &event = *it;
        if (transitions[current_state].find(event) != transitions[current_state].end())
        {
            current_state = transitions[current_state][event];
        }
        else
        {
            return false; // No transition for the event, hence sequence is not accepted
        }
    }

    // Check if the final state after processing the sequence is an accept state
    return accept_states.find(current_state) != accept_states.end();
}

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
    generate_event("readlink");

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
        *fd = openat(*fd, target_component, __O_NOFOLLOW);
        // event: open_nonsym
        generate_event("open_nonsym");
        target_component = __strtok_r(NULL, DELIM, &saveptr2);
        // event: next_nonsym
        generate_event("next_nonsym");
    }
}

int safe_open(char *pathname, int mode)
{
    int fd;
    char *component;

    if (pathname[0] == '/')
    {
        // event: absolute
        generate_event("absolute");
        fd = open("/", mode);
        // event: root_opened
        generate_event("root_opened");
        pathname++;
        // event: made_relative
        generate_event("made_relative");
    }
    else
    {
        // event: not_absolute
        generate_event("not_absolute");
        fd = open(".", mode);
        // event: cwd_opened
        generate_event("cwd_opened");
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
        generate_event("next_component");
    }
    // event: fully_traversed
    generate_event("fully_traversed");
    // assert: property.txt
    assert(accepts_sequence("property.txt"));
    return fd;
}

int main()
{
    int fd;
    char pathname[] = "./main.c";
    fd = safe_open(pathname, O_RDONLY);
    printf("%d\n", fd);
}
