#include <iostream> // show input/output
#include <vector>  // for dynamic array (std::vector)
#include <fstream> // for reading/writing files
#include <sstream> // for parsing/building strings via streams
#include <string> // for std::string text handling
#include <limits> // for clearing invalid input (numeric_limits)
#include <algorithm> // for utilities like find_if

class Task
{
private:
    int id;
    std::string description;
    bool completed;

public:
    Task(int id, std::string desc, bool completed = false)
        : id(id), description(std::move(desc)), completed(completed) {}

    int getId() const { return id; }
    const std::string& getDescription() const { return description; }
    bool isCompleted() const { return completed; }

    void markCompleted() { completed = true; }

    // Encode "|" and "\n" inside the description so the file format
    // never gets corrupted by user input.
    static std::string encode(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            if (c == '|') out += "\\p";
            else if (c == '\n') out += "\\n";
            else if (c == '\\') out += "\\\\";
            else out += c;
        }
        return out;
    }

    static std::string decode(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '\\' && i + 1 < s.size())
            {
                char next = s[i + 1];
                if (next == 'p') { out += '|'; ++i; continue; }
                if (next == 'n') { out += '\n'; ++i; continue; }
                if (next == '\\') { out += '\\'; ++i; continue; }
            }
            out += s[i];
        }
        return out;
    }

    std::string toFileString() const
    {
        return std::to_string(id) + "|" + encode(description) + "|" + (completed ? "1" : "0");
    }

    void display() const
    {
        std::cout << "[" << id << "] "
                  << (completed ? "[x] " : "[ ] ")
                  << description << std::endl;
    }
};

class ToDoList
{
private:
    std::vector<Task> tasks;
    int nextId;

public:
    ToDoList() : nextId(1) {}

    bool addTask(const std::string& description)
    {
        // Trim leading/trailing whitespace
        size_t start = description.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            std::cout << "Task description cannot be empty.\n";
            return false;
        }
        size_t end = description.find_last_not_of(" \t\r\n");
        std::string trimmed = description.substr(start, end - start + 1);

        tasks.emplace_back(nextId++, trimmed);
        std::cout << "Task added!\n";
        return true;
    }

    void viewTasks() const
    {
        if (tasks.empty())
        {
            std::cout << "No tasks available.\n";
            return;
        }
        for (const auto& t : tasks)
        {
            t.display();
        }
    }

    void markTaskCompleted(int id)
    {
        for (auto& t : tasks)
        {
            if (t.getId() == id)
            {
                t.markCompleted();
                std::cout << "Task marked as completed.\n";
                return;
            }
        }
        std::cout << "Task not found.\n";
    }

    void deleteTask(int id)
    {
        auto it = std::find_if(tasks.begin(), tasks.end(),
                                [id](const Task& t) { return t.getId() == id; });
        if (it != tasks.end())
        {
            tasks.erase(it);
            std::cout << "Task deleted.\n";
        }
        else
        {
            std::cout << "Task not found.\n";
        }
    }

    bool saveToFile(const std::string& filename) const
    {
        std::ofstream file(filename);
        if (!file)
        {
            std::cout << "Warning: could not save tasks to '" << filename << "'.\n";
            return false;
        }
        for (const auto& t : tasks)
        {
            file << t.toFileString() << "\n";
        }
        return true;
    }

    void loadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file) return; // No existing file yet, that's fine.

        tasks.clear();
        std::string line;
        int maxId = 0;
        int lineNum = 0;

        while (std::getline(file, line))
        {
            ++lineNum;
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string idStr, desc, completedStr;

            std::getline(ss, idStr, '|');
            std::getline(ss, desc, '|');
            std::getline(ss, completedStr, '|');

            try
            {
                int id = std::stoi(idStr);
                bool completed = (completedStr == "1");
                tasks.emplace_back(id, Task::decode(desc), completed);
                if (id > maxId) maxId = id;
            }
            catch (const std::exception&)
            {
                std::cout << "Warning: skipping malformed line " << lineNum
                          << " in '" << filename << "'.\n";
            }
        }

        nextId = maxId + 1;
    }
};

// Reads an integer from std::cin safely. Returns false if input failed
// (e.g. user typed non-numeric text), after clearing the error state
// and discarding the bad input.
bool readInt(int& out)
{
    std::cin >> out;
    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return true;
}

void showMenu()
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
    todo.loadFromFile(filename);

    int choice = 0;

    do
    {
        showMenu();
        if (!readInt(choice))
        {
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice)
        {
            case 1:
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
            case 2:
                todo.viewTasks();
                break;
            case 3:
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
            case 4:
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
            case 5:
                break;
            default:
                std::cout << "Invalid choice. Please choose 1-5.\n";
                break;
        }

    } while (choice != 5);

    todo.saveToFile(filename);
    std::cout << "Tasks saved. Goodbye!\n";

    return 0;
}