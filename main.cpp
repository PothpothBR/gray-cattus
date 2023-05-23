#define __debug_all__
#include "server.hpp"
#include <iostream>
#include <locale>

using namespace cattus::server;

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");
	Command("EchoCommand", [](CommandData& args, CommandData& response) {
		std::cout << args.get<int>("time").value();
		response.update(args);
		return CommandResult::Sucess;
	});

	Server server("192.168.0.77", "50500");
	server.run();

}