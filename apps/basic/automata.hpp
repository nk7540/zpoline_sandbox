#include <stdbool.h>
#include <string>

// extern "C" void generate_event(const char *event_name);
// extern "C" bool accepts_sequence(const char *filename);

extern "C" void generate_event(const std::string &event_name);
extern "C" bool accepts_sequence(const std::string &automaton_file);
