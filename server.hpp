
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/json.hpp>
#include <thread>
#include "database.hpp"
#include "command.hpp"
#include <iostream>



namespace cattus {
namespace server {

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace json = boost::json;
namespace db = cattus::db;

class Server {
    beast::net::io_context* ioc;
    asio::ip::tcp::acceptor* acceptor;
    asio::ssl::context* ctx;

public:
    Server(const char* addr, const char* port) {
        auto _addr = beast::net::ip::make_address(addr);
        unsigned short _port = atoi(port);
        unsigned int thread_count = std::thread::hardware_concurrency();

        ioc = new beast::net::io_context(thread_count);

        ctx = new asio::ssl::context(asio::ssl::context::sslv23_server);

        ctx->set_options(
            asio::ssl::context::default_workarounds |
            asio::ssl::context::sslv23 |
            asio::ssl::context::no_sslv2
        );

        ctx->use_certificate_chain_file("CACert.pem");

        ctx->use_private_key_file("CAKey.pem", asio::ssl::context::pem);

        acceptor = new asio::ip::tcp::acceptor(*ioc, { _addr, _port });

        new Command("TestCommand", [](CommandArgs args) {
            cout << "calling command" << endl;
        return 0;
            });
    }

    ~Server() {
        delete acceptor;
    }

    void handle(
        beast::http::request<beast::http::dynamic_body>& request,
        beast::http::response<beast::http::dynamic_body>& response
    ) {
        db::Connection db("localhost", "5432", "feneco_database", "postgres", "chopinzinho0202");

        std::cout << endl << "sexo?: " << request.at("command");

        response.version(request.version());
        response.keep_alive(false);
        response.result(beast::http::status::ok);
        response.set(beast::http::field::server, "GrayCattus");

        response.set(beast::http::field::content_type, "text/json");

        db::Data* dt = db.execute("select * from products");

        json::object json_response;

        json_response.emplace("name", dt->getString("name"));

        beast::ostream(response.body()) << json::serialize(dt->getJsonData());
    }

    void run() {
        while (true) {
            beast::error_code ec;
            // cria o socket
            asio::ip::tcp::socket socket(*ioc);

            // aceita conexões
            acceptor->accept(socket);

            // cria o fluxo ssl em torno do socket
            beast::ssl_stream<asio::ip::tcp::socket&> stream(socket, *ctx);

            // realiza o handshake
            stream.handshake(asio::ssl::stream_base::server, ec);

            if (ec) {
                cmn::error(ec.message().data());
                continue;
            }

            // le a requisição
            beast::http::request<beast::http::dynamic_body> request;
            beast::http::response<beast::http::dynamic_body> response;

            beast::flat_buffer buffer(8192);
            beast::http::read(stream, buffer, request);

            handle(request, response);

            string command = request.at("command");

            Command::run(command.data(), 0);

            response.content_length(response.body().size());
            
            beast::http::write(stream, response);
        }
    }
};

}
}