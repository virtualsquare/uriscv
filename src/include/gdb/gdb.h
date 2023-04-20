#include "uriscv/machine.h"
#include <cstring>
#include <netinet/in.h>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class GDBServer {
public:
  GDBServer(Machine *mac);
  void StartServer();

  static std::string GetChecksum(const std::string &msg) {
    uint checksum = 0;
    for (const char &c : msg) {
      checksum += static_cast<uint>(c);
    }
    checksum &= 0xff;
    char buffer[4];
    snprintf(buffer, 3, "%02x", checksum);
    return buffer;
  }

  static inline std::string EncodeReply(const std::string msg) {
    const int len = msg.length() + 4 + 1;
    char buffer[1024];
    snprintf(buffer, len, "$%s#%s", msg.c_str(), (GetChecksum(msg)).c_str());
    return buffer;
  }

  std::string ReadData(const std::string &msg);

private:
  Machine *mac;

  std::string ReadRegisters();
  std::string SendMemory(std::string &msg);
  static inline std::string GetMsg(const std::string &msg);
};
