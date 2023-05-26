#ifndef INCLUDE_SERVER_H
#define INCLUDE_SERVER_H

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
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
namespace dbg = cattus::debug;

class Session : std::enable_shared_from_this<Session>{

};

class Server {
    beast::net::io_context* ioc;
    asio::ip::tcp::acceptor* acceptor;
    asio::ssl::context* ctx;

public:
    Server(const char* addr, const char* port) {
        auto           _addr = beast::net::ip::make_address(addr);
        unsigned short _port = atoi(port);
        unsigned int   _threads = std::thread::hardware_concurrency();

        ioc = new beast::net::io_context(_threads);

        ctx = new asio::ssl::context(asio::ssl::context::sslv23_server);

        ctx->set_options(
            asio::ssl::context::default_workarounds |
            asio::ssl::context::sslv23 |
            asio::ssl::context::no_sslv2
        );

        ctx->use_certificate_chain_file("CACert.pem");

        ctx->use_private_key_file("CAKey.pem", asio::ssl::context::pem);

        asio::ip::tcp::endpoint endpoint(_addr, _port);
        acceptor = new asio::ip::tcp::acceptor(*ioc);
        acceptor->open(endpoint.protocol());

        acceptor->set_option(beast::net::socket_base::reuse_address(true));
        
        acceptor->bind(endpoint);

        acceptor->listen(beast::net::socket_base::max_listen_connections);

        acceptor->async_accept(ioc->get_executor(), std::bind_front(&Server::handle, this));
    }

    ~Server() {
        delete acceptor;
        delete ctx;
        delete ioc;
    }

    void handle(beast::error_code ec, asio::ip::tcp::socket socket) {

        // cria o fluxo ssl em torno do socket
        beast::ssl_stream<asio::ip::tcp::socket&> stream(socket, *ctx);

        // realiza o handshake
        stream.handshake(asio::ssl::stream_base::server, ec);

        if (ec) {
            dbg::error(ec.message().data());
            return;
        }

        // le a requisi��o
        beast::http::request<beast::http::string_body> request;
        beast::http::response<beast::http::string_body> response;
        CommandResult result;

        // monta o buffer
        beast::flat_buffer buffer(8192);

        // le a requisi��o
        beast::http::read(stream, buffer, request);


        // recupera o nome do comando
        std::string command_name;
        // monta os dados para o comando
        CommandData args;
        try {
            std::string command_args = request.at("Arguments");
            command_name = request.at("Command");
            args.update(command_args);
        }
        catch (std::exception e) {
            dbg::error("header incompleto ou incorreto");
            dbg::error(e.what());
            response.result(beast::http::status::bad_request);
            response.keep_alive(false);
            response.reason("header incompleto ou incorreto");
            beast::http::write(stream, response);
            return;
        }

        
        if (!command_name.size()) {
            dbg::error("comando nao especificado");
            response.result(beast::http::status::bad_request);
            response.keep_alive(false);
            response.reason("comando nao especificado");
            beast::http::write(stream, response);
            return;
        }
        
        // busca pelo comando
        Command &command = Command::find(command_name.data());
        if (!&command) {
            dbg::error("comando nao encontrado");
            response.result(beast::http::status::not_found);
            response.keep_alive(false);
            response.reason("comando nao encontrado");
            beast::http::write(stream, response);
            return;
        }

        // executa o comando
        try {
            result = command.Command::run(command_name.data(), args);
        }
        catch (std::exception e) {
            dbg::error(e.what());
            response.result(beast::http::status::internal_server_error);
            response.keep_alive(false);
            response.body() = e.what();
            response.set(beast::http::field::content_type, "text/text");
            response.content_length(response.body().size());
            beast::http::write(stream, response);
            return;
        }

        // verifica o resultado do comando
        if (result == CommandResult::Error) {
            dbg::error("falha ao executar comando");
            response.result(beast::http::status::not_acceptable);
            response.keep_alive(false);
            response.reason("falha ao executar comando");
            beast::http::write(stream, response);
            return;
        }

        // configura o response
        response.version(request.version());
        response.keep_alive(false);
        response.result(beast::http::status::ok);
        response.reason("sucesso");
        response.set(beast::http::field::server, "GrayCattus");
        response.set(beast::http::field::content_type, "text/json");

        // adiciona os dados do comando
        response.body() = command.getResponse();
        response.content_length(response.body().size());
        
        // envia o response
        beast::http::write(stream, response);
    }

    void run() {
        ioc->run();
        return;
        while (true) {
            try {
                // handle();
            } catch (std::exception e){
                dbg::error(e.what());
            }
        }
    }
};

}
}
#endif //INCLUDE_SERVER_H