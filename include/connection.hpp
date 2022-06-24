#pragma once

#include <cstdint>
#include <memory>

#include "asio_wrapper.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "tsqueue.hpp"

namespace cartilage {

class Connection : public std::enable_shared_from_this<Connection> {
public:
  enum class Owner {
    kServer,
    kClient
  };

public:
  Connection(Owner parent, asio::io_context& context, asio::ip::tcp::socket sock, tsqueue<OwnedMessage>& in_queue)
    : context_(context), socket_(std::move(sock)), in_queue_(in_queue), owner_type_(parent) { }

  virtual ~Connection() { }

  uint32_t GetID() const {
    return id_;
  }

public:
  void ConnectToClient(uint32_t id = 0) {
    if (owner_type_ == Owner::kServer && socket_.is_open()) {
      id_ = id;

      ReadHeader();
    }
  }

  void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
    if (owner_type_ == Owner::kClient) {
      asio::async_connect(socket_, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
          if (!ec) {
            ReadHeader();

          }
          else {
            CR_LOG_WARN("Client failed to connect to server:\n\t%s\n", ec.message().c_str());
            socket_.close();
          }
        });
    }
  }

  void Disconnect() {
    if (IsConnected()) {
      asio::post(context_,
        [this]() {
          socket_.close();
        });
    }
  }

  bool IsConnected() const {
    return socket_.is_open();
  }

  void Send(const Message& msg) {
    asio::post(context_,
      [this, msg]() {
        const bool busy = !out_queue_.empty();
        out_queue_.push_back(msg);
        if (!busy) {
          WriteHeader();
        }
      });
  }

// Async functions
private:
  void ReadHeader() {
    asio::async_read(socket_, asio::buffer(&temp_in_.header, sizeof(Message::Header)),
      [this](std::error_code ec, std::size_t length) {
        if (!ec) {
          if (temp_in_.header.message_size > 0) {
            temp_in_.body.resize(temp_in_.header.message_size - sizeof(Message::Header));
            ReadBody();
          }
          else {
            AddToIncomingQueue();
          }

        }
        else {
          CR_LOG_WARN("#%u: Message header read failed:\n\t%s\n", id_, ec.message().c_str());
          socket_.close();
        }
      });
  }

  void ReadBody() {
    asio::async_read(socket_, asio::buffer(temp_in_.body.data(), temp_in_.body.size()),
      [this](std::error_code ec, std::size_t length) {
        if (!ec) {
          AddToIncomingQueue();
        }
        else {
          CR_LOG_WARN("#%u: Message body read failed\n\t%s\n", id_, ec.message().c_str());
          socket_.close();
        }
      });
  }

  void WriteHeader() {
    asio::async_write(socket_, asio::buffer(&out_queue_.front().header, sizeof(Message::Header)),
      [this](std::error_code ec, std::size_t length) {
        if (!ec) {
          if (out_queue_.front().body.size() > 0) {
            WriteBody();
          }
          else {
            out_queue_.pop_front();

            if (!out_queue_.empty()) {
              WriteHeader();
            }
          }
        }
        else {
          CR_LOG_WARN("#%u: Message header write failed\n\t%s\n", id_, ec.message().c_str());
          socket_.close();
        }
      });
  }

  void WriteBody() {
    asio::async_write(socket_, asio::buffer(out_queue_.front().body.data(), out_queue_.front().body.size()),
      [this](std::error_code ec, std::size_t length) {
        if (!ec) {
          out_queue_.pop_front();

          if (!out_queue_.empty()) {
            WriteHeader();
          }
        }
        else {
          CR_LOG_WARN("#%u: Message body write failed\n\t%s\n", id_, ec.message().c_str());
          socket_.close();
        }
      });
  }

  void AddToIncomingQueue() {
    if (owner_type_ == Owner::kServer) {
      in_queue_.push_back({ this->shared_from_this(), temp_in_ });
    }
    else {
      in_queue_.push_back({ nullptr, temp_in_ });
    }

    ReadHeader();
  }

protected:
  asio::ip::tcp::socket socket_;
  asio::io_context& context_;

  tsqueue<Message> out_queue_;
  tsqueue<OwnedMessage>& in_queue_;
  Message temp_in_;

  uint32_t id_ = 0;
  Owner owner_type_ = Owner::kServer;
};

}
