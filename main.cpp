#define __debug_all__

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "database.h"

using namespace boost::beast;
using namespace boost::asio;
using namespace std;

int main(int argc, char* argv[]) {
    
    Database db;

    auto const addr = net::ip::make_address("192.168.0.77");
    unsigned short port = atoi("50500");

    net::io_context ctx(1);
    ip::tcp::acceptor acceptor(ctx, {addr, port});
    ip::tcp::socket socket(ctx);

    acceptor.accept(socket);

    http::request<http::dynamic_body> request;
    http::response<http::dynamic_body> response;

    flat_buffer buffer(8192);
    http::read(socket, buffer, request);

    response.version(request.version());
    response.keep_alive(false);
    response.result(http::status::ok);
    response.set(http::field::server, "GrayCattus");

    response.set(http::field::content_type, "text/html");

    database::Data* dt = db.execute("select nome from pessoa");

    boost::beast::ostream(response.body()) << "<html> <h1> " << dt->getString("nome") << " </h1> </html>";

    response.content_length(response.body().size());

    http::write(socket, response);
}