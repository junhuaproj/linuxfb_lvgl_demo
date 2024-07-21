/*
 * net.c
 *
 *  Created on: Jun 25, 2024
 *      Author: wang
 */


//#include "../inc/net.h"
#include "stock.h"
#include "cJSON.h"

struct stock_info* stocks=NULL;
int stock_count=0;
int stock_updated=0;
#if 0
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include "stock.h"
#include "cJSON.h"

//c++ -I /usr/local/include httpclient.cpp -o main  -L /usr/local/lib -lpthread
namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>



extern "C"  void request_url(const char* host,const char* port,const char* path,net_response_cb cb,void* data)
{
	try{
		const int version=11;
		net::io_context ioc;
		tcp::resolver resolver(ioc);
		beast::tcp_stream stream(ioc);
		auto const results = resolver.resolve(host, port);
		stream.connect(results);
		http::request<http::string_body> req{http::verb::get, path, version};
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		// Send the HTTP request to the remote host
		http::write(stream, req);

		// This buffer is used for reading and must be persisted
		beast::flat_buffer buffer;

		// Declare a container to hold the response
		//http::response<http::dynamic_body> res;
		http::response<http::string_body> res;

		// Receive the HTTP response
		http::read(stream, buffer, res);

		// Write the message to standard out
		//std::cout << res << std::endl;
		//res.body().
		//std::string body=boost::beast::buffers_to_string(res.body().data());
		//std::cout << res.body() << std::endl;
		cb(res.body().c_str(),res.body().length(),data);

		// Gracefully close the socket
		beast::error_code ec;
		stream.socket().shutdown(tcp::socket::shutdown_both, ec);

		// not_connected happens sometimes
		// so don't bother reporting it.
		//
		if(ec && ec != beast::errc::not_connected)
			throw beast::system_error{ec};
	}
	catch(std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	    //return EXIT_FAILURE;
	}
}
#else
extern "C" void request_url(const char* host,const char* port,const char* path,net_response_cb cb,void* data)
{

}
#endif












