#ifndef INCLUDE_COMMAND_H
#define INCLUDE_COMMAND_H

#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <boost/json.hpp>
#include "database.hpp"

namespace json = boost::json;
namespace dbg = cattus::debug;

class CommandData {
protected:
	json::object args;
public:
	CommandData() {}

	CommandData(std::string args) {
		if (!args.size()) return;
		try {
			this->args = json::parse(args).as_object();
		}
		catch (std::exception e) {
			dbg::error(e.what());
			throw std::runtime_error("Argumentos incorretos ou no formato errado");
		}
	}

	void get(std::string name, std::optional<std::string> &value) {
		value = static_cast<std::string>(args.at(name).as_string().c_str());
	}

	void get(std::string name, std::optional<int> &value) {
		value = static_cast<int>(args.at(name).as_int64());
	}

	void get(std::string name, std::optional<double> &value) {
		value = static_cast<double>(args.at(name).as_double());
	}

	void get(std::string name, std::optional<bool> &value) {
		value = static_cast<bool>(args.at(name).as_bool());
	}

	template<typename T>
	inline std::optional<T> get(std::string name) {
		std::optional<T> value;
		if (args.contains(name)) get(name, value);
		return value;
	}

	void put(std::string name, std::string value) {
		args.insert_or_assign(name, value);
	}

	void put(std::string name, int value) {
		args.insert_or_assign(name, value);
	}

	void put(std::string name, double value) {
		args.insert_or_assign(name, value);
	}

	void put(std::string name, bool value) {
		args.insert_or_assign(name, value);
	}

	void clear() {
		args.clear();
	}

	std::string serialize() {
		return json::serialize(args);
	}

	void update(json::object data) {
		args = data;
	}

	void update(CommandData data) {
		args = data.args;
	}

	void update(std::string args) {
		if (!args.size()) return;
		try {
			this->args = json::parse(args).as_object();
		}
		catch (std::exception e) {
			dbg::error(e.what());
			throw std::runtime_error("Argumentos incorretos ou no formato errado");
		}
	}
};

enum CommandResult {
	Error = -1,
	Sucess = 0
};

typedef CommandResult(CommandCallable)(CommandData&, CommandData&);

class Command {
	static std::vector<Command> command_table;
	const char* name;
	CommandCallable &command;
	CommandData response;

public:
	Command(const char* name, CommandCallable *command): command(std::move(*command)) {
		this->name = name;
		command_table.emplace_back(std::move(*this));
	}

	static Command& find(const char* name) {
		for (Command& command : command_table) {
			for (char* c = const_cast<char*>(command.name), *n = const_cast<char*>(name); *c == *n; c++, n++) {
				if (*c == '\0') {
					return command;
				}
			}
		}
		return *(Command*)0;
	}

	CommandResult run(CommandData& args) {
		dbg::log("%s%s", name, args.serialize().data());
		response.clear();
		cattus::db::global_conn.begin(); // fails wen multithread
		try {
			CommandResult res = command(args, response);
			cattus::db::global_conn.commit();
			return res;
		}
		catch (exception e) {
			cattus::db::global_conn.roolback();
			return CommandResult::Error;
		}
	}

	static CommandResult run(const char* name, CommandData& args) {
		for (Command& command : command_table) {
			for (
				char* c = const_cast<char*>(command.name), *n = const_cast<char*>(name);
				*c == *n;
				c++, n++
				) {
				if (*c == '\0') {
					return command.run(args);
				}
			}
		}
		return CommandResult::Error;
	}

	CommandResult operator()(CommandData args) {
		response.clear();
		return command(args, response);
	}

	bool isNull() {
		return this == nullptr;
	}

	static void freeCommands() {
		for (Command& command : command_table) {
			delete &command;
		}
	}

	std::string getResponse() {
		return response.serialize();
	}
};
std::vector<Command> Command::command_table;

#endif //INCLUDE_COMMAND_H