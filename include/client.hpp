#pragma once

#include <string>
#include <thread>

#include "asio_wrapper.hpp"
#include "connection.hpp"
#include "logger.hpp"
#include "tsqueue.hpp"
#include "message.hpp"

namespace cartilage {

/*
 * Client class
 * Intended use case is to subclass this 
 */

class Client {
public:
  Client() = default;

  virtual ~Client() {
    Disconnect();
  }

public:
  bool Connect(const std::string& url, const uint16_t port) {
    try {
      asio::ip::tcp::resolver resolver(context_);
		  asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(url, std::to_string(port));

		  connection_ = std::make_unique<Connection>(Connection::Owner::kClient, context_, asio::ip::tcp::socket(context_), in_queue_);
      connection_->ConnectToServer(endpoints);
			context_thread_ = std::thread(
        [this]() { 
          context_.run();
        });

    }
    catch (std::exception& e) {
      CR_LOG_ERROR("Client Exception: %s\n", e.what());
      return false; 
    }

    return true; 
  }

  void Disconnect() {
    if (IsConnected()) {
      connection_->Disconnect();
    }

    context_.stop();
    if (context_thread_.joinable()) {
      context_thread_.join();
    }
    connection_.release();
  }

  bool IsConnected() const {
    if (connection_) {
      return connection_->IsConnected();
    }
    else {
      return false;
    }
  }

  void Send(const Message& msg) {
    if (IsConnected()) {
		  connection_->Send(msg);
    }
	}

  tsqueue<OwnedMessage>& IncomingMessages() {
    return in_queue_;
  }

protected:
  asio::io_context context_;
  std::thread context_thread_;
  std::unique_ptr<Connection> connection_;

private:
  tsqueue<OwnedMessage> in_queue_;
};

}
