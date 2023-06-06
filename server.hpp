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

#define assert_code if (ec) { dbg::error(ec.message().data()); return; }

namespace cattus {
namespace server {

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace json = boost::json;

namespace db = cattus::db;
namespace dbg = cattus::debug;

class Session : public std::enable_shared_from_this<Session>{
    beast::ssl_stream<beast::tcp_stream> stream;
    beast::flat_buffer buffer;
    beast::http::request<beast::http::string_body> request;
    beast::http::response<beast::http::string_body> response;

public:
    explicit Session(asio::ip::tcp::socket&& socket, boost::asio::ssl::context& ctx) : stream(std::move(socket), ctx) {}

    static auto spawn(asio::ip::tcp::socket&& socket, boost::asio::ssl::context& ctx) {
        auto session = std::make_shared<Session>(std::move(socket), ctx);
        beast::net::dispatch(session->stream.get_executor(), std::bind_front(&Session::handshake, session->shared_from_this()));
        return session;
    }

    void handshake() {
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        stream.async_handshake(asio::ssl::stream_base::server, std::bind_front(&Session::read, shared_from_this()));
    }

    void read(beast::error_code ec) {
        assert_code(ec);
        request = {};

        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        beast::http::async_read(stream, buffer, request, std::bind_front(&Session::handle, shared_from_this()));
    }

    void handle(beast::error_code ec, size_t transfered) {
        assert_code(ec);
        boost::ignore_unused(transfered);

        response = {};
        CommandResult result;

        std::string command_name;
        CommandData args;
        try {
            std::string command_args = request.at("Arguments");
            command_name = request.at("Command");
            args.update(command_args);
        }
        catch (std::exception e) {
            fail("header incompleto ou incorreto", beast::http::status::bad_request, &e);
            return;
        }


        if (!command_name.size()) {
            fail("comando nao especificado", beast::http::status::bad_request);
            return;
        }

        // busca pelo comando
        Command& command = Command::find(command_name.data());
        if (!&command) {
            fail("comando nao encontrado", beast::http::status::not_found);
            return;
        }

        // executa o comando
        try {
            result = command.Command::run(command_name.data(), args);
        }
        catch (std::exception e) {
            fail("falha ao executar comando", beast::http::status::internal_server_error, &e);
            return;
        }

        // verifica o resultado do comando
        if (result == CommandResult::Error) {
            fail("falha ao executar comando", beast::http::status::not_acceptable);
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
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        beast::http::async_write(stream, response, std::bind_front(&Session::shutdown, shared_from_this()));
    }

    void shutdown(beast::error_code ec, size_t transfered) {
        assert_code(ec);
        boost::ignore_unused(transfered);
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        stream.async_shutdown(std::bind_front(&Session::shutdown_clear, shared_from_this()));
    }

    void shutdown_clear(beast::error_code ec) {
        assert_code(ec);
    }

    void fail(string reason, beast::http::status status, exception *e=nullptr) {
        dbg::error(reason.data());
        if (e) dbg::error(e->what());

        response.result(status);
        response.keep_alive(false);
        response.reason(reason);
        if (e) response.body() = e->what();
        response.set(beast::http::field::content_type, "text/text");
        response.content_length(response.body().size());
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        beast::http::async_write(stream, response, std::bind_front(&Session::shutdown, shared_from_this()));
        return;
    }
};

class Listener : public std::enable_shared_from_this<Listener> {
    beast::net::io_context& ioc;
    asio::ip::tcp::acceptor acceptor;
    asio::ssl::context& ctx;

public:
    explicit Listener(beast::net::io_context& ioc, asio::ssl::context& ctx, asio::ip::tcp::endpoint endpoint) : ioc(ioc), ctx(ctx), acceptor(ioc) {
        acceptor.open(endpoint.protocol());
        acceptor.set_option(beast::net::socket_base::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen(beast::net::socket_base::max_listen_connections);
    }

    static auto spawn(beast::net::io_context& ioc, asio::ssl::context& ctx, asio::ip::tcp::endpoint endpoint) {
        auto listener = std::make_shared<Listener>(ioc, ctx, endpoint);
        listener->try_accept();
        return listener;
    }

    void try_accept() {
        acceptor.async_accept(beast::net::make_strand(ioc), std::bind_front(&Listener::accept, shared_from_this()));
    }

    void accept(beast::error_code ec, asio::ip::tcp::socket socket) {
        assert_code(ec);
        Session::spawn(std::move(socket), ctx);
        try_accept();
    }
};

class Server {
public:
    Server(const char* addr, const char* port) {
        auto           _addr = beast::net::ip::make_address(addr);
        unsigned short _port = atoi(port);
        unsigned int   _threads = std::thread::hardware_concurrency();

        dbg::info("threads em uso: %i", _threads);

        beast::net::io_context ioc(_threads);
        asio::ssl::context ctx(asio::ssl::context::sslv23_server);

        ctx.set_options(
            asio::ssl::context::default_workarounds |
            asio::ssl::context::sslv23 |
            asio::ssl::context::no_sslv2
        );

        ctx.use_certificate_chain_file("CACert.pem");

        ctx.use_private_key_file("CAKey.pem", asio::ssl::context::pem);

        asio::ip::tcp::endpoint endpoint(_addr, _port);

        Listener::spawn(ioc, ctx, endpoint);

        std::vector<std::jthread> pool;
        pool.reserve(_threads - 1);
        for (auto i = _threads - 1; i > 0; i--) {
            pool.emplace_back(
                [&ioc] { ioc.run(); }
            );
        }
        ioc.run();
    }
};

}
}
#endif //INCLUDE_SERVER_H