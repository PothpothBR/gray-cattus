#define __debug_all__
#include "server.hpp"
#include "database.hpp"
#include <iostream>
#include <locale>

using namespace cattus::server;
using namespace cattus::command;
using namespace cattus::db;

void commands() {

	Command("EchoCommand", [](CommandData& args, CommandData& response, Connection &db) {
		std::cout << args.get<int>("time").value_or(-1);
	response.update(args);
	return CommandStatus::Sucess;
		});
		
	Command("RevelationCommand", [](CommandData& args, CommandData& response, Connection &db) {
	return CommandStatus::Sucess;
		});

	Command("GetUsers", [](CommandData& args, CommandData& response, Connection &db) {
		
		std::string query = "select * from users where id = ";
	query.append(args.get<std::string>("userId").value());
	cattus::db::Data &&data = db.execute(query.data());
	//dbg::info("%i", data.get<int>("id"));
	response.update(data.getJson());
	return CommandStatus::Sucess;
		});
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");
	ConnectionPool::start("localhost", "5432", "feneco_database", "postgres", "chopinzinho0202");
	commands();

	Server server("192.168.0.77", "50500");

}