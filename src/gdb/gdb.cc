/*
 * Gianmaria Rovelli - 2023
 *
 * Reference
 * https://www.chciken.com/tlmboy/2022/04/03/gdb-z80.html
 *
 */

#include "gdb/gdb.h"
#include "uriscv/arch.h"
#include "uriscv/processor.h"
#include "uriscv/utility.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#define PORT 8080

#define emptyReply "$#00"

const std::string OKReply = GDBServer::EncodeReply("OK");

#define qSupportedMsg                                                          \
  "qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;"       \
  "vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;"     \
  "memory-tagging+"
const std::string qSupportedReply =
    GDBServer::EncodeReply("swbreak+;hwbreak+;");

#define vMustReplyMsg "vMustReplyEmpty"

const std::string questionMarkReply = GDBServer::EncodeReply("S05");

#define qAttachedMsg "qAttached"
const std::string qAttachedReply = GDBServer::EncodeReply("S05");

#define vContMsg "vCont?"

GDBServer::GDBServer(Machine *mac) { this->mac = mac; }

inline std::string GDBServer::getMsg(const std::string &msg) {
  static std::regex reg(R"(.*\$(.*)#.*)");
  std::smatch matches;
  regex_match(msg, matches, reg);
  if (matches.size() <= 1)
    return "";

  return matches[1].str();
}

std::string GDBServer::readRegisters() {

  std::string res = "";

  Processor *cpu = mac->getProcessor(0);

  for (unsigned int i = 0; i < cpu->kNumCPURegisters; i++) {
    char r[1024] = {0};
    snprintf(r, 1024, "%08x", htonl(cpu->regRead(i)));
    res += r;
  }
  char pc[9] = {0};
  snprintf(pc, 9, "%08x", htonl(cpu->getPC()));
  res += pc;

  return EncodeReply(res);
}

std::string GDBServer::sendMemory(std::string &msg) {
  /* deletes 'm' */
  msg.erase(0, 1);

  std::string delimiter = ",";
  uint addr;
  uint size;
  std::stringstream sa, ss;

  uint pos = msg.find(delimiter);
  sa << std::hex << msg.substr(0, pos);
  sa >> addr;
  msg.erase(0, pos + delimiter.length());
  ss << std::hex << msg.substr(0, msg.find(delimiter));
  ss >> size;

  std::string res = "";

  uint instr = 0;
  for (uint i = 0; i < size; i += WORD_SIZE) {
    // if (mac->getProcessor(0)->mapVirtual(addr + i, &phyPC, EXEC)) {
    // } else if (mac->getBus()->InstrRead(currPhysPC, &currInstr, this)) {
    // }
    if (!mac->getBus()->InstrReadGDB(addr + i, &instr, mac->getProcessor(0))) {
      char r[1024] = {0};
      snprintf(r, 1024, "%08x", htonl(instr));
      res += r;
    }
  }

  if (res == "")
    return emptyReply;

  return EncodeReply(res);
}

inline bool GDBServer::checkBreakpoint(const uint &addr) {
  return std::find(breakpoints.begin(), breakpoints.end(), addr) !=
         breakpoints.end();
}
inline void GDBServer::addBreakpoint(const uint &addr) {
  if (std::find(breakpoints.begin(), breakpoints.end(), addr) ==
      breakpoints.end()) {
    breakpoints.push_back(addr);
  }
}
inline void GDBServer::removeBreakpoint(const uint &addr) {
  auto new_end = remove(breakpoints.begin(), breakpoints.end(), addr);
  (void)new_end;
}

uint GDBServer::parseBreakpoint(std::string &msg) {
  /* deletes 'Z0,' or 'z0,'  */
  msg.erase(0, 3);

  uint addr;
  std::stringstream sa;

  sa << std::hex << msg;
  sa >> addr;

  return addr;
}

std::string GDBServer::ReadData(const std::string &msg) {
  std::string body = getMsg(msg);
  if (body == "")
    return emptyReply;

  if (strcmp(body.c_str(), qSupportedMsg) == 0) {
    return qSupportedReply;
  } else if (strcmp(body.c_str(), qAttachedMsg) == 0) {
    return qAttachedReply;
  } else if (strcmp(body.c_str(), vMustReplyMsg) == 0) {
    return emptyReply;
  } else if (strcmp(body.c_str(), "?") == 0) {
    return questionMarkReply;
  } else if (strcmp(body.c_str(), "g") == 0) {
    return readRegisters();
  } else if (body.c_str()[0] == 'm') {
    return sendMemory(body);
  } else if (body.c_str()[0] == 'c') {
    do {
      bool stopped = false;
      mac->step(&stopped);
    } while (!checkBreakpoint(mac->getProcessor(0)->getPC()));

    return qAttachedReply;
  } else if (body.c_str()[0] == 'z') {
    uint addr = parseBreakpoint(body);
    removeBreakpoint(addr);
    return OKReply;
  } else if (body.c_str()[0] == 'Z') {
    uint addr = parseBreakpoint(body);
    addBreakpoint(addr);
    return OKReply;
  } else if (strcmp(body.c_str(), vContMsg) == 0) {
    /* Not supported */
    return emptyReply;
  }

  return emptyReply;
}

void GDBServer::StartServer() {
  DEBUGMSG("[GDB] Starting GDB Server\n");

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  printf("[GDB] GDB connection accepted\n");

  while (true) {
    /* clean the buffer */
    valread = read(new_socket, buffer, 1024);
    if (valread != -1) {
      if (strcmp(buffer, "+") != 0 && strcmp(buffer, "") != 0) {
        DEBUGMSG("[GDB][->] %s\n", buffer);

        /* + is the ACK */
        std::string reply = "+" + ReadData(buffer);
        DEBUGMSG("[GDB][<-] %s\n\n", reply.c_str());

        send(new_socket, reply.c_str(), (reply).size(), 0);
        usleep(200);
        memset(buffer, 0, 1024);
      }
    }
  }

  // closing the connected socket
  close(new_socket);
  // closing the listening socket
  shutdown(server_fd, SHUT_RDWR);
}
