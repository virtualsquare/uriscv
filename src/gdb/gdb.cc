/*
 * Gianmaria Rovelli - 2023
 *
 * Reference
 * https://www.chciken.com/tlmboy/2022/04/03/gdb-z80.html
 *
 */

#include "gdb/gdb.h"
#include "uriscv/processor.h"
#include "uriscv/utility.h"
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#define PORT 8080

#define emptyReply "$#00"
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

GDBServer::GDBServer(Machine *mac) { this->mac = mac; }

inline std::string GDBServer::GetMsg(const std::string &msg) {
  static std::regex reg(R"(.*\$(.*)#.*)");
  std::smatch matches;
  regex_match(msg, matches, reg);
  if (matches.size() <= 1)
    return "";

  return matches[1].str();
}

std::string GDBServer::ReadRegisters() {

  std::string res = "";

  Processor *cpu = mac->getProcessor(0);

  for (unsigned int i = 0; i < cpu->kNumCPURegisters; i++) {
    char r[1024] = {0};
    printf("reg %d -> %08x\n", i, cpu->regRead(i));
    snprintf(r, 1024, "%08x", htonl(cpu->regRead(i)));
    // res += r;
    res += "10000001";
  }
  res += "10000001";
  res += "10000001";
  res += "10000001";
  res += "10000001";
  char pc[9] = {0};
  snprintf(pc, 9, "%08x", (cpu->getPC()));
  // res += pc;
  res += "1fc00000";

  return EncodeReply(res);
}

std::string GDBServer::ReadData(const std::string &msg) {
  std::string body = GetMsg(msg);
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
    return ReadRegisters();
  }

  return emptyReply;
}

void GDBServer::StartServer() {
  printf("Starting GDB Server\n");

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

  while (true) {
    valread = read(new_socket, buffer, 1024);
    if (valread != -1) {
      printf("[->] %s\n", buffer);
      /* + is the ACK */
      std::string reply = "+" + ReadData(buffer);
      printf("[<-] %s\n", reply.c_str());

      send(new_socket, reply.c_str(), (reply).size(), 0);
      usleep(200);
    }
    /* clean the buffer */
    memset(buffer, 0, 1024);
  }

  // closing the connected socket
  close(new_socket);
  // closing the listening socket
  shutdown(server_fd, SHUT_RDWR);
}
