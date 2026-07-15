# To-Do List (C++)

A simple command-line to-do list application written in C++. Tasks are stored
in memory while the program runs and persisted to a local file
(`tasks.txt`) so they survive between runs.

## Features

- Add tasks
- View all tasks with their completion status
- Mark tasks as completed
- Delete tasks
- Automatic save/load to `tasks.txt`
- Safe against common input mistakes (invalid menu choices, non-numeric
  IDs, empty descriptions, corrupted save-file lines)
- Task descriptions can safely contain `|`, newlines, or backslashes
  without corrupting the save file

## Requirements

- A C++17-compatible compiler (e.g. `g++` 9 or newer, `clang++`)

## Build

```bash
g++ -std=c++17 -Wall -Wextra -o todo_list todo_list.cpp
```

## Run

```bash
./todo_list
```

On startup, the program loads any existing tasks from `tasks.txt` in the
current directory. If the file doesn't exist yet, it starts with an empty
list.

## Usage

You'll see a menu like this:

```
--- TO-DO LIST ---
1. Add Task
2. View Tasks
3. Mark Task Completed
4. Delete Task
5. Exit
Choose:
```

Enter the number for the action you want, and follow the prompts.

| Option | Action |
|---|---|
| 1 | Add a new task (enter a description when prompted) |
| 2 | View all current tasks with their ID and status |
| 3 | Mark a task as completed by its ID |
| 4 | Delete a task by its ID |
| 5 | Exit the program |

Tasks are saved automatically after every add, complete, or delete action,
and again on exit.

## File Format

Tasks are stored in `tasks.txt`, one per line, in the format:

```
id|description|completed
```

- `id` — a unique integer
- `description` — the task text, with `|`, newlines, and backslashes
  escaped so they can't be confused with the field separator
- `completed` — `1` if done, `0` if not

Example:

```
1|Buy milk|0
2|Walk the dog|1
```

You generally shouldn't need to edit this file by hand, but the program
will safely skip and warn about any malformed lines it encounters rather
than crashing.

## Project Structure

```
todo_list.cpp   # entire application: Task class, ToDoList class, and main()
tasks.txt       # created automatically at runtime to store your tasks
```

## Notes

- This is a single-user, local console application — it does not support
  concurrent access from multiple running instances writing to the same
  file at once.
- Built and tested with a Linux `g++` toolchain; should also work on
  macOS/Windows with a C++17 compiler.
