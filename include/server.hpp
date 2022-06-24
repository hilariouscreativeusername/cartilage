#pragma once

#include <cstdio>
#include <deque>
#include <limits>
#include <memory>
#include <vector>

#include "asio_wrapper.hpp"
#include "connection.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "tsqueue.hpp"

namespace cartilage {

/*
 * Server class
 * Pure virtual class - intended usage is to subclass and provide overrides
 * for OnClientConnect, OnClientDisconnect, and OnMessage
 */

class Server {
public:
  Server(uint16_t port) : acceptor_(context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) { }

  virtual ~Server() {
    Stop();
  }

  bool Start() {
    try {
      WaitForClientConnection();
      context_thread_ = std::thread([this]() { context_.run(); });

    }
    catch (std::exception& e) {
      CR_LOG_ERROR("Server exception: %s\n", e.what());
      return false;
    }

    CR_LOG_INFO("Server started\n");
    return true;
  }

  void Stop() {
    context_.stop();
    if (context_thread_.joinable()) {
      context_thread_.join();
    }
    CR_LOG_INFO("Server stopped\n");
  }

// Async functions
public:
  void WaitForClientConnection() {
    acceptor_.async_accept(
      [this](std::error_code ec, asio::ip::tcp::socket sock) {
        if (!ec) {
          CR_LOG_INFO("Server reports new connection: %s\n", sock.remote_endpoint().address().to_string().c_str());
          std::shared_ptr<Connection> new_connection = std::make_shared<Connection>(
            Connection::Owner::kServer, context_, std::move(sock), in_queue_);
          
          // The user has the opportunity to reject the new connection, perhaps because the max number of connected
          // clients has been exceeded or because the IP is blacklisted 
          if (OnClientConnect(new_connection)) {
            connections_.push_back(std::move(new_connection));
            connections_.back()->ConnectToClient(id_counter_++);
            CR_LOG_INFO("Server accepted connection\n\tID #%u was allocated\n", connections_.back()->GetID());
          
          }
          else {
            CR_LOG_INFO("Server rejected connection: OnClientConnect returned false\n");
          }

        }
        else {
          CR_LOG_WARN("Server connection exception: %s\n", ec.message().c_str());
        }

        WaitForClientConnection();
      });
  }

  void Send(std::shared_ptr<Connection> client, const Message& msg) {
    if (client && client->IsConnected()) {
      client->Send(msg);
    }
    else {
      OnClientDisconnect(client);
      client.reset();
      connections_.erase(std::remove(connections_.begin(), connections_.end(), client), connections_.end());
    }
  }

  void SendToAll(const Message& msg, std::shared_ptr<Connection> ignore = nullptr) {
    bool invalid_client_found = false;

    for (auto& client : connections_) {
      if (client && client->IsConnected()) {
        if (client != ignore) {
          client->Send(msg);
        }
      }
      else {
        OnClientDisconnect(client);
        client.reset();
        invalid_client_found = true;
      }
    }

    if (invalid_client_found) {
      connections_.erase(std::remove(connections_.begin(), connections_.end(), nullptr), connections_.end());
    }
  }

  void ProcessMessages(bool wait = false, size_t max_messages = std::numeric_limits<size_t>::max()) {
    if (wait) in_queue_.wait();

    size_t message_count = 0;
    while (message_count < max_messages && !in_queue_.empty()) {
      auto msg = in_queue_.pop_front();
      OnMessage(msg.remote, msg.msg);
      ++message_count;
    }
  }

protected:
  // Called once when a client connects. Connection can be rejected (perhaps because the ip is blacklisted or there are
  // too many connected clients) by returning false
  virtual bool OnClientConnect(std::shared_ptr<Connection> client) = 0;

  // Called once on client disconnect. Intended to be used for cleanup
  virtual void OnClientDisconnect(std::shared_ptr<Connection> client) = 0;

  // Called when message arrives from client
  virtual void OnMessage(std::shared_ptr<Connection> client, Message& msg) = 0;

protected:
  tsqueue<OwnedMessage> in_queue_;
  std::deque<std::shared_ptr<Connection>> connections_;

  asio::io_context context_;
  std::thread context_thread_;

  asio::ip::tcp::acceptor acceptor_;

  // Client identifier to avoid sending IPs all over the place and accidentally doxxing people
  uint32_t id_counter_ = 10000;
};

}
