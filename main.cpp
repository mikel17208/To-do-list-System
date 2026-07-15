#include <iostream>   // console input/output (cin, cout)
#include <vector>     // dynamic array (std::vector)
#include <fstream>    // reading/writing files
#include <sstream>    // parsing/building strings via streams
#include <string>     // std::string text handling
#include <limits>     // clearing invalid input (numeric_limits)
#include <algorithm>  // utilities like find_if

// Represents one to-do item: an id, its text, and whether it's done.
class Task
{
private:
    int id;                   // task's unique number
    std::string description;  // task's text
    bool completed;            // whether the task is done

public:
    // creates a Task; completed defaults to false if not given
    Task(int id, std::string desc, bool completed = false)
        // sets the task's fields from the given values
        : id(id), description(std::move(desc)), completed(completed) {}

    int getId() const { return id; }
    const std::string& getDescription() const { return description; }
    bool isCompleted() const { return completed; }

    void markCompleted() { completed = true; } // the only way to mark a task done

    // Goes through the description one character at a time.
    // Replaces risky characters (| \n \) with safe two-char codes (\p \n \\)
    // so they can't be mistaken for the file's real separators when saved.
    static std::string encode(const std::string& s)
    {
        std::string out;          // will hold the safe, encoded version
        out.reserve(s.size());    // pre-allocate space for speed

        for (char c : s)          // check each character one by one
        {
            if (c == '|') out += "\\p";        // disguise real pipe as \p
            else if (c == '\n') out += "\\n";  // disguise newline as \n
            else if (c == '\\') out += "\\\\"; // disguise backslash as double backslash
            else out += c;                     // normal character, keep as-is
        }

        return out; // give back the fully encoded, file-safe string
    }

    // Reverses encode(): turns the safe \p, \n, \\ codes back into the real characters
    static std::string decode(const std::string& s)
    {
        std::string out;                      // will hold the restored, original text
        out.reserve(s.size());                // pre-allocate space for speed

        for (size_t i = 0; i < s.size(); ++i) // i = 0 to start, loop while i < length, move forward by 1 each time
        {
            if (s[i] == '\\' && i + 1 < s.size()) // current char is a backslash, and there's a character after it
            {
                char next = s[i + 1];             // look at the character right after the backslash

                if (next == 'p') { out += '|'; ++i; continue; }   // \p means a real pipe, restore it, skip ahead
                if (next == 'n') { out += '\n'; ++i; continue; }  // \n means a real newline, restore it, skip ahead
                if (next == '\\') { out += '\\'; ++i; continue; } // \\ means a real backslash, restore it, skip ahead
            }

            out += s[i]; // not part of a disguise code, copy the character as-is
        }

        return out; // give back the fully decoded, original text
    }

    // Builds this task's data into one safe line of text for the save file
    std::string toFileString() const
    {
        // description is encoded here so a real "|" in the text can't be
        // mistaken for a field separator when the file is read back.
        return std::to_string(id) + "|" + encode(description) + "|" + (completed ? "1" : "0");
    }

    void display() const // prints this task as one readable line to the console
    {
        std::cout << "[" << id << "] "              // task's id in brackets
                  << (completed ? "[x] " : "[ ] ")   // checkbox: x if done, empty if not
                  << description << std::endl;       // task text, then move to next line
    }
};

// Manages the collection of tasks: add, view, complete, delete,
// and load/save them to a file so they persist between runs.
class ToDoList
{
private:
    std::vector<Task> tasks; // the list holding all current tasks
    int nextId;               // the id to assign to the next task added

public:
    // prepares a brand-new, empty to-do list; nextId starts at 1
    ToDoList() : nextId(1) {}

    bool addTask(const std::string& description)
    {
        // Find where the real text starts, skipping leading whitespace
        size_t start = description.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            // Nothing but whitespace (or empty) — reject it
            std::cout << "Task description cannot be empty.\n";
            return false;
        }

        // Find where the real text ends, skipping trailing whitespace
        size_t end = description.find_last_not_of(" \t\r\n");
        std::string trimmed = description.substr(start, end - start + 1); // just the real text, no extra spaces

        tasks.emplace_back(nextId++, trimmed); // add task with next id, then increment it
        std::cout << "Task added!\n";
        return true; // success
    }

    void viewTasks() const
    {
        if (tasks.empty())
        {
            // Nothing to show, let the user know and stop here
            std::cout << "No tasks available.\n";
            return;
        }

        // Print every task in the list, one per line
        for (const auto& t : tasks)
        {
            t.display();
        }
    }

    void markTaskCompleted(int id)
    {
        for (auto& t : tasks) // non-const: we need to modify the actual task, not a copy
        {
            if (t.getId() == id) // check if this task's id matches the one we're looking for
            {
                t.markCompleted();                        // found it, mark as done
                std::cout << "Task marked as completed.\n";
                return; // found and done, stop searching
            }
        }
        std::cout << "Task not found.\n"; // looped through everything, no match
    }

    void deleteTask(int id)
    {
        // Search the list for the first task whose id matches
        auto it = std::find_if(
            tasks.begin(), tasks.end(),   // search the whole list, start to end
            [id](const Task& t) {         // [id]: lets this mini-function use the outer id
                                           // (const Task& t): find_if passes each task here as t
                return t.getId() == id;   // true = this task matches, stop here
            }
        );

        if (it != tasks.end()) // it points to a real task, not the "end" marker — so a match was found
        {
            tasks.erase(it);   // remove the task at that position from the list
            std::cout << "Task deleted.\n";
        }
        else // it reached tasks.end() without matching anything
        {
            std::cout << "Task not found.\n";
        }
    }

    bool saveToFile(const std::string& filename) const
    {
        std::ofstream file(filename); // try to open/create the file for writing

        if (!file) // opening failed (e.g. disk full, no permission)
        {
            std::cout << "Warning: could not save tasks to '" << filename << "'.\n";
            return false; // report failure, nothing was written
        }

        for (const auto& t : tasks) // write every task, one per line
        {
            file << t.toFileString() << "\n";
        }

        return true; // save completed successfully
    }

    void loadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file) return; // No existing file yet, that's fine.

        tasks.clear();       // start fresh before loading
        std::string line;
        int maxId = 0;        // tracks highest id seen, to set nextId afterward
        int lineNum = 0;      // tracks which line we're on, for error messages

        while (std::getline(file, line)) // read one line at a time until end of file
        {
            ++lineNum;
            if (line.empty()) continue; // skip blank lines

            // split the line "id|description|completed" into its 3 fields
            std::stringstream ss(line);
            std::string idStr, desc, completedStr;

            std::getline(ss, idStr, '|');
            std::getline(ss, desc, '|');
            std::getline(ss, completedStr, '|');

            try
            {
                int id = std::stoi(idStr);                          // text -> number (can throw)
                bool completed = (completedStr == "1");             // "1" -> true, anything else -> false
                tasks.emplace_back(id, Task::decode(desc), completed); // decode restores the real text
                if (id > maxId) maxId = id;
            }
            catch (const std::exception&) // stoi failed: line is malformed/corrupted
            {
                std::cout << "Warning: skipping malformed line " << lineNum
                          << " in '" << filename << "'.\n";
            }
        }

        nextId = maxId + 1; // next new task continues numbering after the highest id loaded
    }
};

// Reads an integer from std::cin safely.
// Clears the fail state and discards bad input so a bad menu choice
// doesn't cause an infinite loop.
bool readInt(int& out)
{
    std::cin >> out;              // try to read a number
    if (std::cin.fail())          // input wasn't a valid number
    {
        std::cin.clear();         // reset the stream's error state
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard the bad input
        return false;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard leftover input (e.g. trailing enter)
    return true;
}

void showMenu() // prints the list of options the user can choose from
{
    std::cout << "\n--- TO-DO LIST ---\n";
    std::cout << "1. Add Task\n";
    std::cout << "2. View Tasks\n";
    std::cout << "3. Mark Task Completed\n";
    std::cout << "4. Delete Task\n";
    std::cout << "5. Exit\n";
    std::cout << "Choose: ";
}

int main()
{
    const std::string filename = "tasks.txt";
    ToDoList todo;
    todo.loadFromFile(filename); // load any previously saved tasks on startup

    int choice = 0;

    do
    {
        showMenu();
        if (!readInt(choice)) // user typed something that wasn't a number
        {
            std::cout << "Invalid input. Please enter a number.\n";
            continue; // skip this loop, show the menu again
        }

        switch (choice)
        {
            case 1: // add a new task
            {
                std::string desc;
                std::cout << "Enter task description: ";
                std::getline(std::cin, desc);
                if (todo.addTask(desc))
                {
                    todo.saveToFile(filename); // persist immediately
                }
                break;
            }
            case 2: // show all tasks
                todo.viewTasks();
                break;
            case 3: // mark a task as completed
            {
                int id;
                std::cout << "Enter task ID: ";
                if (readInt(id))
                {
                    todo.markTaskCompleted(id);
                    todo.saveToFile(filename);
                }
                else
                {
                    std::cout << "Invalid task ID.\n";
                }
                break;
            }
            case 4: // delete a task
            {
                int id;
                std::cout << "Enter task ID: ";
                if (readInt(id))
                {
                    todo.deleteTask(id);
                    todo.saveToFile(filename);
                }
                else
                {
                    std::cout << "Invalid task ID.\n";
                }
                break;
            }
            case 5: // exit — just breaks out of the switch, loop condition below handles the actual exit
                break;
            default: // choice wasn't 1-5
                std::cout << "Invalid choice. Please choose 1-5.\n";
                break;
        }

    } while (choice != 5); // keep looping until the user chooses to exit

    todo.saveToFile(filename); // final save on exit, just in case
    std::cout << "Tasks saved. Goodbye!\n";

    return 0;
}