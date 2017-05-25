#ifndef PARSECLIPS_HPP
#define PARSECLIPS_HPP

#include <vector>

class Task;

using TaskList = std::vector<Task>;

class QIODevice;

int parseClips(TaskList& tasks, QIODevice& eingabe);

#endif
