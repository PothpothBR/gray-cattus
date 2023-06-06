#ifndef INCLUDE_COMMAND_H
#define INCLUDE_COMMAND_H

#include <unordered_map>
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
	static std::unordered_map<std::string, Command> command_table;
	std::string name;
	CommandCallable &command;
	CommandData response;

public:
	Command(std::string name, CommandCallable *command): command(std::move(*command)) {
		command = nullptr;
		this->name = name;
		command_table.emplace(this->name, *this);
	}

	static Command& find(std::string name) {
		return command_table.at(name);
	}

	CommandResult run(CommandData& args) {
		dbg::log("%s%s", name.data(), args.serialize().data());
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
		return find(name).run(args);
	}

	CommandResult operator()(CommandData args) {
		response.clear();
		return command(args, response);
	}

	bool isNull() {
		return this == nullptr;
	}

	static void freeCommands() {
		/*for (Command& command : command_table) {
			delete &command;
		}*/
	}

	std::string getResponse() {
		return response.serialize();
	}
};
std::unordered_map<std::string, Command> Command::command_table;

#endif //INCLUDE_COMMAND_H