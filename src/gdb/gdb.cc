/*
 * Gianmaria Rovelli - 2023
 *
 * Reference
 * https://www.chciken.com/tlmboy/2022/04/03/gdb-z80.html
 */

#include "gdb/gdb.h"
#include "uriscv/arch.h"
#include "uriscv/const.h"
#include "uriscv/machine.h"
#include "uriscv/processor.h"
#include "uriscv/utility.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#define PORT 8080

#define emptyReply "$#00"

const std::string OKReply = GDBServer::EncodeReply("OK");

#define qSupportedMsg                                                          \
  "qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-" \
  "events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;memory-"    \
  "tagging+"
const std::string qSupportedReply = GDBServer::EncodeReply("swbreak+;");

#define vMustReplyMsg "vMustReplyEmpty"

const std::string questionMarkReply = GDBServer::EncodeReply("S05");

#define qAttachedMsg "qAttached"
const std::string qAttachedReply = GDBServer::EncodeReply("1");

#define vContMsg "vCont?"

pthread_mutex_t continue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t continue_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t stopped_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bp_mutex = PTHREAD_MUTEX_INITIALIZER;

GDBServer::GDBServer(Machine *mac) {
  this->killed = false;
  this->stopped = false;
  this->mac = mac;
}

inline std::string GDBServer::getMsg(const std::string &msg) {
  static std::regex reg(R"(.*\$(.*)#.*)");
  std::smatch matches;
  regex_match(msg, matches, reg);
  if (matches.size() <= 1)
    return "";

  return matches[1].str();
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

std::string GDBServer::writeRegister(std::string &msg) {
  /* deletes 'P' */
  msg.erase(0, 1);

  std::string delimiter = "=";
  uint rn;
  uint rv;
  std::stringstream sa, ss;

  uint pos = msg.find(delimiter);
  sa << std::hex << msg.substr(0, pos);
  sa >> rn;
  printf("%d\n", rn);
  msg.erase(0, pos + delimiter.length());
  ss << std::hex << msg.substr(0, msg.find(delimiter));
  ss >> rv;

  if (rn == mac->getProcessor(0)->kNumCPURegisters)
    mac->getProcessor(0)->setPC(rv);
  else
    mac->getProcessor(0)->regWrite(rn, rv);

  return OKReply;
}

std::string GDBServer::writeRegisters(std::string &msg) {
  /* deletes 'G' */
  msg.erase(0, 1);
  Processor *cpu = mac->getProcessor(0);

  std::stringstream sa;
  uint r;

  for (unsigned int i = 0; i < cpu->kNumCPURegisters; i++) {
    sa << std::hex << msg.substr(i * BYTELEN, i * BYTELEN + BYTELEN);
    sa >> r;
    cpu->regWrite(i, htonl(r));
  }
  sa << std::hex
     << msg.substr(cpu->kNumCPURegisters * BYTELEN,
                   cpu->kNumCPURegisters * BYTELEN + BYTELEN);
  sa >> r;
  cpu->setPC(htonl(r));

  return OKReply;
}

bool GDBServer::Step() {
  bool stopped = false;
  mac->step(&stopped);
  return stopped;
}
bool GDBServer::CheckBreakpoint() {
  return checkBreakpoint(mac->getProcessor(0)->getPC());
}
inline bool GDBServer::checkBreakpoint(const uint &addr) {
  pthread_mutex_lock(&bp_mutex);
  bool r = std::find(breakpoints.begin(), breakpoints.end(), addr) !=
           breakpoints.end();
  pthread_mutex_unlock(&bp_mutex);
  return r;
}
inline void GDBServer::addBreakpoint(const uint &addr) {
  pthread_mutex_lock(&bp_mutex);
  if (std::find(breakpoints.begin(), breakpoints.end(), addr) ==
      breakpoints.end()) {
    breakpoints.push_back(addr);
  }
  pthread_mutex_unlock(&bp_mutex);
}
inline void GDBServer::removeBreakpoint(const uint &addr) {
  pthread_mutex_lock(&bp_mutex);
  auto new_end = remove(breakpoints.begin(), breakpoints.end(), addr);
  (void)new_end;
  pthread_mutex_unlock(&bp_mutex);
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
  if (msg.c_str()[0] == 0x3) {
    pthread_mutex_lock(&stopped_mutex);
    stopped = true;
    pthread_mutex_unlock(&stopped_mutex);
    return OKReply;
  }

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
  } else if (body.c_str()[0] == 'k') {
    killed = true;
    return OKReply;
  } else if (body.c_str()[0] == 'P') {
    writeRegister(body);
    return OKReply;
  } else if (body.c_str()[0] == 'G') {
    writeRegisters(body);
    return OKReply;
  } else if (body.c_str()[0] == 'c') {

    pthread_mutex_lock(&continue_mutex);
    pthread_cond_signal(&continue_cond);
    pthread_mutex_unlock(&continue_mutex);

    // return qAttachedReply;
    return OKReply;
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

void *MachineStep(void *vargp) {

  GDBServer *gdb = (GDBServer *)vargp;

  while (true) {

    pthread_mutex_lock(&continue_mutex);
    pthread_cond_wait(&continue_cond, &continue_mutex);
    pthread_mutex_unlock(&continue_mutex);

    bool s = false;
    do {
      gdb->Step();

      pthread_mutex_lock(&stopped_mutex);
      s = gdb->IsStopped();
      pthread_mutex_unlock(&stopped_mutex);
    } while (!gdb->CheckBreakpoint() && !s);

    if (!s) {
      pthread_mutex_lock(&stopped_mutex);
      gdb->Stop();
      pthread_mutex_unlock(&stopped_mutex);
    }
  }

  pthread_exit((void *)0);
}

void GDBServer::sendMsg(const int &socket, const std::string &msg) {
  /* + is the ACK */
  std::string reply = "+" + msg;
  DEBUGMSG("[GDB][<-] %s\n\n", reply.c_str());

  send(socket, reply.c_str(), (reply).size(), 0);
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

  for (;;) {

    pthread_t tid;

    pthread_create(&tid, NULL, MachineStep, (this));
    mac->getProcessor(0)->Reset(MCTL_DEFAULT_BOOT_PC, MCTL_DEFAULT_BOOT_SP);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    DEBUGMSG("[GDB] GDB connection accepted\n");

    struct pollfd fds[1];
    int timeout = 100;
    fds[0].fd = new_socket;
    fds[0].events = POLLIN;

    while (!killed) {

      /* clean the buffer */
      valread = poll(fds, 1, timeout);
      if (valread > 0 && (read(new_socket, buffer, 1024)) != 0) {

        if (strcmp(buffer, "+") != 0 && strcmp(buffer, "") != 0) {
          DEBUGMSG("[GDB][->] %s (%x)\n", buffer, buffer[0]);

          std::string d = ReadData(buffer);
          sendMsg(new_socket, d);
        }
      }
      usleep(200);
      memset(buffer, 0, 1024);

      pthread_mutex_lock(&stopped_mutex);
      if (stopped) {
        stopped = false;
        pthread_mutex_unlock(&stopped_mutex);
        sendMsg(new_socket, questionMarkReply);
      } else
        pthread_mutex_unlock(&stopped_mutex);
    }

    DEBUGMSG("[GDB] GDB Session terminated\n");

    pthread_kill(tid, SIGTERM);

    // closing the connected socket
    close(new_socket);
  }

  // closing the listening socket
  shutdown(server_fd, SHUT_RDWR);
}
