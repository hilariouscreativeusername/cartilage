#include <cartilage.hpp>

class TestServer : public cartilage::Server {
public:
  TestServer(uint16_t port) : cartilage::Server(port) { }

protected:
  virtual bool OnClientConnect(std::shared_ptr<cartilage::Connection> client) override {
    return true;
  }

  virtual void OnClientDisconnect(std::shared_ptr<cartilage::Connection> client) override {
  }

  virtual void OnMessage(std::shared_ptr<cartilage::Connection> client, cartilage::Message& msg) override {
    switch (msg.header.message_type) {
      case 0:
        CR_LOG_INFO("#%u: ping\n", client->GetID());
        client->Send(msg);
        break;
    }
  }
};

int main() {
  TestServer server(65432);
  server.Start();

  while (true) {
    server.ProcessMessages();
  }
}
