#define __debug_all__
#include "server.hpp"
#include "database.hpp"
#include <iostream>
#include <locale>

using namespace cattus::server;
using namespace cattus::command;
using namespace cattus::db;

void commands() {

	CommandMethod("EchoCommand", {
		std::cout << args.get<int>("time").value_or(-1);
		response.update(args);
		return CommandStatus::Sucess;
	});
		
	CommandMethod("RevelationCommand", {
		return CommandStatus::Sucess;
	});

	CommandMethod("GetUsers", {
		std::string query = "select * from users where id = ";
		query.append(args.get<std::string>("userId").value());
		cattus::db::Data &&data = db.execute(query.data());
		response.update(data.getJson());
		return CommandStatus::Sucess;
	});
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");
	ConnectionPool::start("localhost", "5432", "feneco_database", "postgres", "1234");
	commands();

	Server server("192.168.0.77", "50500");

}