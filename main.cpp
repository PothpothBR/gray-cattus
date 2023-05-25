#define __debug_all__
#include "server.hpp"
#include "database.hpp"
#include <iostream>
#include <locale>

using namespace cattus::server;

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");

	Command("EchoCommand", [](CommandData& args, CommandData& response) {
		std::cout << args.get<int>("time").value_or(-1);
		response.update(args);
		return CommandResult::Sucess;
	});

	Command("RevelationCommand", [](CommandData& args, CommandData& response) {
		std::cout << "Lucas é gay";
		return CommandResult::Sucess;
	});

	Command("GetUsers", [](CommandData& args, CommandData& response) {

		//cattus::db::Connection conn("localhost", "5432", "feneco_database", "postgres", "chopinzinho0202");
		std::string query = "select * from users where id = ";
		query.append(args.get<std::string>("userId").value());
		cattus::db::Data *data = cattus::db::global_conn.execute(query.data());
		response.update(data->getJson());
		return CommandResult::Sucess;
	});

	Server server("192.168.0.77", "50500");
	server.run();

}