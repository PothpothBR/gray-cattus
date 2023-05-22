#define __debug_all__
#include <iostream>
#include <vector>

typedef int CommandResult;
typedef void* CommandArgs;
typedef CommandResult(*CommandCallable)(CommandArgs);

class Command {
	static std::vector<Command*> command_table;
	const char* name;
	CommandCallable command;

public:
	Command(Command&&) = delete;
	Command(Command&) = delete;

	Command(const char* name, CommandCallable command) {
		this->name = name;
		this->command = command;
		command_table.push_back(this);
	}

	static Command& find(const char* name) {
		for (Command* command : command_table) {
			for (char* c = const_cast<char*>(command->name), *n = const_cast<char*>(name); *c == *n; c++, n++) {
				if (*c == '\0') {
					return *command;
				}
			}
		}
		return *(Command*)0;
	}

	static CommandResult run(const char* name, CommandArgs args) {
		for (Command* command : command_table) {
			for (
				char* c = const_cast<char*>(command->name), *n = const_cast<char*>(name);
				*c == *n;
				c++, n++
			) {
				if (*c == '\0') {
					return command->command(args);
				}
			}
		}
		return -1;
	}

	CommandResult operator()(CommandArgs args) {
		return command(args);
	}

	bool isNull() {
		return this == nullptr;
	}

	static void freeCommands() {
		for (Command* command : command_table) {
			delete command;
		}
	}
};
std::vector<Command*> Command::command_table;

void sla() {
	new Command("Teste", [](CommandArgs args) {
		std::cout << "Exec: Teste" << std::endl;
		return (CommandResult)0;
	});
}

int main(int argc, char *argv[]) {

	sla();
	std::cout << "Result " << Command::run("Teste", 0);

	Command::freeCommands();
}