
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
    }

    ~Server() {
        delete acceptor;
        delete ctx;
        delete ioc;
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
            beast::http::request<beast::http::string_body> request;
            beast::http::response<beast::http::string_body> response;
            CommandResult result;

            // monta o buffer
            beast::flat_buffer buffer(8192);

            // le a requisição
            beast::http::read(stream, buffer, request);

            std::string command_args = request.at("args");

            // monta os dados para o comando
            CommandData args;
            try {
                args.update(command_args);
            }
            catch (std::exception e) {
                cmn::error(e.what());
                response.result(beast::http::status::bad_request);
                response.keep_alive(false);
                response.reason(e.what());
                beast::http::write(stream, response);
                continue;
            }

            // recupera o nome do comando
            std::string command_name = request.at("command");
            if (!command_name.size()) {
                cmn::error("comando nao especificado");
                response.result(beast::http::status::bad_request);
                response.keep_alive(false);
                response.reason("comando nao especificado");
                beast::http::write(stream, response);
                continue;
            }
            
            // busca pelo comando
            Command &command = Command::find(command_name.data());
            if (!&command) {
                cmn::error("comando nao encontrado");
                response.result(beast::http::status::not_found);
                response.keep_alive(false);
                response.reason("comando nao encontrado");
                beast::http::write(stream, response);
                continue;
            }

            // executa o comando
            try {
                result = command.Command::run(command_name.data(), args);
            }
            catch (std::exception e) {
                cmn::error(e.what());
                response.result(beast::http::status::internal_server_error);
                response.keep_alive(false);
                response.body() = e.what();
                response.set(beast::http::field::content_type, "text/text");
                response.content_length(response.body().size());
                beast::http::write(stream, response);
                continue;
            }

            // verifica o resultado do comando
            if (result == CommandResult::Error) {
                cmn::error("falha ao executar comando");
                response.result(beast::http::status::not_acceptable);
                response.keep_alive(false);
                response.reason("falha ao executar comando");
                beast::http::write(stream, response);
                continue;
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
    }
};

}
}