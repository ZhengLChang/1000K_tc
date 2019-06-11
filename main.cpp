#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include "json.h"
#include <sys/select.h>

class Long_TCP_TEST{
  public:
	static int is_end;
	static void signal_handler(int sig);
	static void *thread_proc(void *);
    Long_TCP_TEST(const char url[]="118.24.118.155", const int port=8081, const char pwd[]="751f621ea5c8f930", const char iv[]="2624750004598718");
    virtual ~Long_TCP_TEST();
	int maxNum(int a, int b);
    void print(){std::cout << pwd << std::endl << iv << std::endl;}
  private:
    char *pwd;
    char *iv;
    char *url;
    int port;
    int connFd;
	pthread_t thread;
    int link_to_acm();
    void send_json_to_acm();
    int tcp_conn();
};
int Long_TCP_TEST::is_end = 0;
void Long_TCP_TEST::signal_handler(int sig){
	switch(sig){
		case SIGINT:
			is_end = 1;
			break;
	}
	std::cout << "Get signal: " << sig << std::endl;
	return;
}
Long_TCP_TEST::Long_TCP_TEST(const char url[], const int port, const char pwd[], const char iv[]){
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
  if(pthread_create(&thread, NULL, thread_proc, this) != 0){
	  std::cout << "pthread_create error: " << strerror(errno) << std::endl;
  }
}
Long_TCP_TEST::~Long_TCP_TEST(){
  delete []url;
  delete []pwd;
  delete []iv;
  if(this->connFd > 0){
	  close(this->connFd);
	  this->connFd = -1;
  }
}
int Long_TCP_TEST::maxNum(int a, int b){
	return (a >= b ? a:b);
}
int Long_TCP_TEST::link_to_acm(){
  if(this->connFd < 0 && (this->connFd = tcp_conn()) == -1){
    std::cout << "conn to " << url << "error" << std::endl;
    return -1;
  }
  return this->connFd;
}
void *Long_TCP_TEST::thread_proc(void *argu){
	fd_set fdset;
	int maxFd = 0;
	int ret = 0;
	struct timeval selTimeout;
	Long_TCP_TEST *_this = (Long_TCP_TEST *)argu;
	while(!is_end){
		maxFd = 0;
		_this->link_to_acm();
		FD_ZERO(&fdset);
		if(_this->connFd > 0){
			FD_SET(_this->connFd, &fdset);
			maxFd = _this->maxNum(maxFd, _this->connFd + 1);
			selTimeout.tv_sec = 0;
			selTimeout.tv_usec = 100 * 1000;
			ret = select(maxFd, &fdset, NULL, NULL, &selTimeout);
			if(ret == 0){
				/*do something*/
			}else if(ret > 0){
				if(FD_ISSET(_this->connFd, fdset)){

				}
			}else{
				std::cout << "select error occur: " << strerror(errno) << std::endl;
			}
		}
	}
}
void Long_TCP_TEST::send_json(){
}
int Long_TCP_TEST::tcp_conn(){
  struct addrinfo hints, *res = NULL, *ressave = NULL;
  int n, sockfd;
  int so_optval;
  char serverStr[1024] = "";
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  snprintf(serverStr, sizeof(serverStr), "%d", this->port);
  n = getaddrinfo(this->url, serverStr, &hints, &res);
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
  signal(SIGINT, Long_TCP_TEST::signal_handler);
  signal(SIGPIPE, Long_TCP_TEST::signal_handler);
  test.print();
  return 0;
}
