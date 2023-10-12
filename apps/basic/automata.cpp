#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include "automata.hpp"

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
    for (auto const &e : event_sequence)
        std::cout << e << std::endl;

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
    for (const auto &event : event_sequence)
    {
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
