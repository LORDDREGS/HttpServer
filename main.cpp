#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>      
#include <iterator>     

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace fs = boost::filesystem;
using tcp = net::ip::tcp;

void handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
    if (req.method() == http::verb::get) {
        fs::path path = "./static" + std::string(req.target()); 
        if (path.string().back() == '/') path /= "index.html";

        if (fs::exists(path) && fs::is_regular_file(path)) {
            std::ifstream file(path.string(), std::ios::in | std::ios::binary);
            if (!file) {
                res.result(http::status::internal_server_error);
                res.set(http::field::content_type, "text/plain");
                res.body() = "Failed to open file";
                res.prepare_payload();
                return;
            }
            std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/html");
            res.body() = body;
            res.prepare_payload();
        } else {
            res.result(http::status::not_found);
            res.set(http::field::content_type, "text/plain");
            res.body() = "File not found";
            res.prepare_payload();
        }
    } else if (req.method() == http::verb::post) {
        res.result(http::status::ok);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Received POST data: " + req.body();
        res.prepare_payload();
    } else {
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Unsupported method";
        res.prepare_payload();
    }
}

void do_session(tcp::socket socket) {
    bool close = false;
    beast::error_code ec;

    beast::flat_buffer buffer;

    http::request<http::string_body> req;

    http::read(socket, buffer, req, ec);
    if (ec == http::error::end_of_stream)
        close = true;
    else if (ec)
        return;

    http::response<http::string_body> res;
    handle_request(std::move(req), res);

    http::write(socket, res, ec);
    if (ec)
        return;

    if (close) {
        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_send, ec);
    }
}

int main() {
    try {
        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), 8080}};

        std::cout << "HTTP Server running on port 8080" << std::endl;

        while (true) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);
            std::thread(do_session, std::move(socket)).detach(); // Передача функции напрямую
        }
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
