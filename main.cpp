#define __debug_all__
#include "server.hpp"
#include <iostream>

using namespace cattus::server;

int main(int argc, char* argv[]) {

	Server server("192.168.0.77", "50500");
	server.run();

}