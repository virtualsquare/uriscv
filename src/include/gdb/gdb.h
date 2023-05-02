#include "uriscv/machine.h"
#include <cstring>
#include <netinet/in.h>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

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
  bool CheckBreakpoint();
  bool Step();

  inline void Stop() { stopped = true; };
  bool IsStopped() const { return stopped; };

private:
  bool killed, stopped;
  Machine *mac;
  std::vector<uint> breakpoints;

  std::string readRegisters();
  std::string writeRegister(std::string &msg);
  std::string writeRegisters(std::string &msg);
  std::string sendMemory(std::string &msg);
  static inline std::string getMsg(const std::string &msg);

  uint parseBreakpoint(std::string &msg);

  inline bool checkBreakpoint(const uint &addr);
  inline void addBreakpoint(const uint &addr);
  inline void removeBreakpoint(const uint &addr);

  void sendMsg(const int &socket, const std::string &msg);
};

typedef struct thread_arg_struct {
  GDBServer *gdb;
  int socket;
} thread_arg_t;
