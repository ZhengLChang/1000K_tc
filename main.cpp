#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

class Long_TCP_TEST{
  public:
    Long_TCP_TEST(const char url[]="172.16.0.255", const int port=8081, const char pwd[]="751f621ea5c8f930", const char iv[]="2624750004598718");
    virtual ~Long_TCP_TEST();
    void print(){std::cout << pwd << std::endl << iv << std::endl;}
  private:
    char *pwd;
    char *iv;
    char *url;
    int port;
    int connFd;
    void link_to_acm();
    void send_json_to_acm();
    int tcp_conn();
};

Long_TCP_TEST::Long_TCP_TEST(const char url[], const int port, const char pwd[], const char iv[]);
  int urlLen = std::strlen(url);
  int pwdLen = std::strlen(pwd);
  int ivLen = std::strlen(iv);
  this->url = new char[urlLen + 1];
  this->port = port;
  this->pwd = new char[pwdLen + 1];
  this->iv = new char[ivLen + 1];
  this->connFd = -1;
  std::strncpy(this->pwd, pwd, pwdLen + 1);
  std::strncpy(this->iv, iv, ivLen + 1);
  std::strncpy(this->url, url, urlLen + 1);
}
Long_TCP_TEST::~Long_TCP_TEST(){
  delete []url;
  delete []pwd;
  delete []iv;
}
void Long_TCP_TEST::link_to_acm(){
  if((this->connFd = tcp_conn()) == -1){
    std::cout << "conn to " << url << "error" << std::endl;
    return ;
  }
}
void Long_TCP_TEST::send_json_to_acm(){
}
int Long_TCP_TEST::tcp_conn(){
  struct addrinfo hints, *res = NULL, *ressave = NULL;
  int n, sockfd;
  int so_optval;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  n = getaddrinfo(this->url, this->port, &hints, &res);
  if (n < 0)
  {
    goto ERR;
  }
  ressave = res;
  sockfd = -1;
  for (; res; res = res->ai_next){
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0){
      continue;
    }
    if(0 == connect(sockfd, res->ai_addr, res->ai_addrlen)){
      break;
    }
    if(sockfd >= 0){
      close(sockfd);
      sockfd = -1;
    }
  }
  if(ressave){
    freeaddrinfo(ressave);
    ressave = NULL;
  }
  return sockfd;
ERR:
  if(ressave){
    freeaddrinfo(ressave);
    ressave = NULL;
  }
  return sockfd;
}

int main(){
  Long_TCP_TEST test;
  test.print();
  return 0;
}
