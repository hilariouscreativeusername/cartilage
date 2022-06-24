#pragma once

#include <cstdint>
#include <ostream>
#include <memory>
#include <type_traits>
#include <vector>

namespace cartilage {

struct Message {

  struct Header {
    uint32_t message_type;
    uint32_t message_size = 0; // size of the whole message = sizeof(message_header) + body.size()
  };

  Header header;
  std::vector<uint8_t> body;

  friend std::ostream& operator << (std::ostream& os, const Message& msg) {
    os << "ID: " << msg.header.message_type << " Size: " << msg.header.message_size;
    return os;
  }

  template <typename T> friend Message& operator << (Message& msg, const T& data) {
    size_t i = msg.body.size();
    msg.body.resize(msg.body.size() + sizeof(T));
    std::memcpy(msg.body.data() + i, &data, sizeof(T));
    msg.header.message_size = (uint32_t)(sizeof(Header) + msg.body.size());
    return msg;
  }

  template <typename T> friend Message& operator >> (Message& msg, T& data) {
    size_t i = msg.body.size() - sizeof(T);
    std::memcpy(&data, msg.body.data() + i, sizeof(T));
    msg.body.resize(i);
    msg.header.message_size = (uint32_t)(sizeof(Header) + msg.body.size());
    return msg;
  }
};

class Connection;

struct OwnedMessage {
  std::shared_ptr<Connection> remote = nullptr;
  Message msg;

  friend std::ostream& operator << (std::ostream& os, const OwnedMessage& msg) {
    os << msg.msg;
    return os;
  }
};

}
