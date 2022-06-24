#include <chrono>
#include <iostream>

#include <cartilage.hpp>

class BoxingClient : public cartilage::Client {
public:
  void PingServer() {
    std::cout << "Sending ping to server\n";

    cartilage::Message msg;
    msg.header.message_type = 0;

    auto now = std::chrono::high_resolution_clock::now();
    msg << now;

    Send(msg);
  }
};

int main() {
  BoxingClient client;
  client.Connect("127.0.0.1", 65432);

  for (int i = 0; i < 10; ++i) {
    client.PingServer();

    // Spin while we wait for the response
    while (client.IncomingMessages().empty()) { }

    cartilage::Message msg = client.IncomingMessages().pop_front().msg;
    switch (msg.header.message_type) {
      case 0:
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::time_point sent_time;
        msg >> sent_time;

        std::cout << "ping round trip time: " << (std::chrono::duration<double>(now - sent_time).count() * 1000) << "ms\n";
        break;
    }
  }

  std::cout << "Press enter to finish\n";
  std::cin.get();
}
