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
#include <signal.h>
#include <cstdio>
#include <errno.h>
#include "acm_encryption.h"
#include "json.h"
#include "encode_decode.h"
#include <assert.h>
#include "mac_genarate.hpp"
#define ARRAY_STR_LEN(x) x, x ? sizeof(x) - 1: 0
#define xclose(fd) do{  if(fd >= 0){ \
						close(fd); \
						fd = -1; \
						} \
				}while(0)
#define xfree(p) do{ \
	if(p){ \
    free(p); \
    p = NULL; \
	} \
  }while(0)
class Long_TCP_TEST{
  public:
	static int is_end;
	static void signal_handler(int sig);
	static void *thread_proc(void *);
	static int sock_peek (int fd, char *buf, int bufsize);
	static int sock_read (int fd, char *buf, int bufsize);
	static int sock_write (int fd, const char *buf, const int bufsize);
	static int tcp_conn(const char *hostname, const char *service);
	static int dump_json(Long_TCP_TEST *_this, JsonNode *json);
    Long_TCP_TEST(const int plusNum = 0, const char url[]="172.16.0.225", const char server[]="8081", const char pwd[]="751f621ea5c8f930",
    		const char iv[]="26247500045987==", const char baseMac[]="00:01:02:03:04:09");
    virtual ~Long_TCP_TEST();
	int maxNum(int a, int b);
    void print(){std::cout << pwd << std::endl << iv << std::endl;}
    int link_to_acm();
    void login_to_server();
    void echo_to_server();
  private:
    char *pwd;
    char *iv;
    char *url;
    char *server;
    int connFd;
	int stacksize;
	pthread_attr_t attr;
	pthread_t thread;
	MacGenerate *mac;
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
Long_TCP_TEST::Long_TCP_TEST(const int macPlusNum, const char url[], const char server[], const char pwd[],
		const char iv[], const char baseMac[]){
  int urlLen = std::strlen(url);
  int pwdLen = std::strlen(pwd);
  int ivLen = std::strlen(iv);
  int serverLen = std::strlen(server);
  this->url = new char[urlLen + 1];
  this->server = new char[serverLen + 1];
  this->pwd = new char[pwdLen + 1];
  this->iv = new char[ivLen + 1];
  this->connFd = -1;
  this->stacksize = 20480;
  if(pthread_attr_init(&this->attr) != 0 ||
		  pthread_attr_setstacksize(&this->attr, this->stacksize) != 0){
	  std::cout << "pthread_attr_init error: " << strerror(errno) << std::endl;
	  Long_TCP_TEST::is_end = 1;
	  return;
  }
  mac = new MacGenerate(macPlusNum, baseMac);
  std::strncpy(this->pwd, pwd, pwdLen + 1);
  std::strncpy(this->iv, iv, ivLen + 1);
  std::strncpy(this->url, url, urlLen + 1);
  std::strncpy(this->server, server, serverLen + 1);
  if(pthread_create(&thread, &attr, thread_proc, this) != 0){
	  std::cout << "pthread_create error: " << strerror(errno) << std::endl;
  }
}
Long_TCP_TEST::~Long_TCP_TEST(){
  delete []url;
  delete []server;
  delete []pwd;
  delete []iv;
  delete mac;
  pthread_attr_destroy(&this->attr);
  if(this->connFd > 0){
	  close(this->connFd);
	  this->connFd = -1;
  }
}
void Long_TCP_TEST::login_to_server(){
	char mac_addr[64] = {0};
	JsonNode *mainJson = json_new_object();
	JsonNode *dataJson = json_new_object();
    int i = 0;
    /*ipv4*/
    char ip_address[128] = "";
    char subnet_mask[128] = "";
    char gateway[128] = "";
    char primary_dns[128] = "";
    char secondary_dns[128] = "";
    char model_name[128] = "";


	JSON_INSERT_STRING(mainJson, "type", "login");

	JSON_INSERT_STRING(dataJson, "mac", this->mac->get());
	/*Upload Status*/
	JSON_INSERT_STRING (dataJson, "firmware_version", "2.6.8.ada91");
	JSON_INSERT_STRING (dataJson, "hardware_version", "2");

	JSON_INSERT_STRING (dataJson, "product_name", "A2xLTE");
	JSON_INSERT_STRING (dataJson, "product_name_real", "A20LTE");

	json_insert_number (dataJson, "network_type", 1);

	/*ipv4*/
	json_insert_number (dataJson, "wan_type", 0);
	/*ip_address*/
	JSON_INSERT_STRING (dataJson, "ip_address", "172.16.0.99");
	/*subnet_mask*/
	JSON_INSERT_STRING (dataJson, "subnet_mask", "255.255.0.0");
	/*gateway*/
	JSON_INSERT_STRING (dataJson, "gateway", "172.16.0.1");
	/*primary_dns*/
	JSON_INSERT_STRING (dataJson, "primary_dns", "8.8.8.8");
	/*secondary_dns*/
	JSON_INSERT_STRING (dataJson, "secondary_dns", "");
	/*WIFIMac*/
	JSON_INSERT_STRING (dataJson, "WIFIMac", "");



	JSON_INSERT_STRING (dataJson, "ipv4", "172.16.0.99");

	json_insert_number(dataJson, "account_num", 33 - 1);

	json_insert_pair_into_object(mainJson, "data", dataJson);
	dump_json(this, mainJson);

	return ;
}
void Long_TCP_TEST::echo_to_server(){

	char mac_addr[64] = {0};
	if(this->connFd < 0){
		return;
	}
	JsonNode *mainJson = json_new_object();
	JsonNode *dataJson = json_new_object();
    int i = 0;
    /*ipv4*/
    char ip_address[128] = "";
    char subnet_mask[128] = "";
    char gateway[128] = "";
    char primary_dns[128] = "";
    char secondary_dns[128] = "";
    char model_name[128] = "";


	JSON_INSERT_STRING(mainJson, "type", "echo");

	json_insert_pair_into_object(mainJson, "data", dataJson);
	dump_json(this, mainJson);

	return ;

}
int Long_TCP_TEST::maxNum(int a, int b){
	return (a >= b ? a:b);
}
int Long_TCP_TEST::link_to_acm(){
  if(this->connFd < 0){
	  if((this->connFd = tcp_conn(url, server)) == -1){
		  std::cout << "conn to " << url << "error: " << strerror(errno) << std::endl;
		  Long_TCP_TEST::is_end = 1;
		  return -1;
	  }else{
		  this->login_to_server();
	  }
  }
  return this->connFd;
}
void *Long_TCP_TEST::thread_proc(void *argu){
	fd_set fdset;
	int maxFd = 0;
	int ret = 0;
  int i = 0;
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
			if(ret == 0){/*timeout*/
			}else if(ret > 0){
				if(FD_ISSET(_this->connFd, &fdset)){
					char buf[2048] = "";
					int rdlen = -1;
					int pklen = -1;
					char *endofmsg = NULL;
					char *decode_aes_data = NULL;
					int decode_aes_data_out_len = 0;
					memset(buf, 0, sizeof(buf));
					pklen = Long_TCP_TEST::sock_peek(_this->connFd, buf, sizeof(buf) - 1);
					if(pklen < 0){
						std::cout << "recv error: " << strerror(errno) << std::endl;
						if(_this->connFd >= 0){
							close(_this->connFd);
							_this->connFd = -1;
						}
		                continue;
					  }else if(pklen == 0){
						  std::cout << "Server Close the Socket" << std::endl;
		                  if(_this->connFd > 0){
								close(_this->connFd);
								_this->connFd = -1;
		                  }
		                  continue;
					  }else if((endofmsg = strchr(buf, (int)'\n')) == NULL){
						  continue;
					  }

		              rdlen = Long_TCP_TEST::sock_read (_this->connFd, buf, (int)(endofmsg - buf) + 1);
		              if(rdlen < 0){
		            	  std::cout << "read error: " << strerror(errno) << std::endl;
		            	  if(_this->connFd >= 0){
		            		  close(_this->connFd);
		            		  _this->connFd = -1;
		            	  }
		            	  continue;
		              }
		              buf[rdlen] = '\0';

		              decode_aes_data = (char *)acm_decode((uint8_t*)_this->iv, (uint8_t*)_this->pwd, (const char *)buf, (unsigned int *)&decode_aes_data_out_len);
                  free(decode_aes_data);
		         //     std::cout << "getMsg:\n" << decode_aes_data << std::endl;
				}
			}else{
				std::cout << "select error occur: " << strerror(errno) << std::endl;
			}
 //     _this->login_to_server();
      i++; 
      if(i % 99 == 0){
        i = 0;
			  _this->echo_to_server();
      }
		}
	}
}
int Long_TCP_TEST::dump_json(Long_TCP_TEST *_this, JsonNode *json){
	char *p_str = json_encode(json);
	int n = 0;
	char *data_by_aes = NULL;
	assert(p_str != NULL); /*Just for Debug*/
	if(_this->connFd < 0){
		goto ERROR;
	}
	data_by_aes = acm_encode((uint8_t*)_this->iv, (uint8_t *)_this->pwd, (const uint8_t *)p_str, strlen(p_str));
  strcat(data_by_aes, "\n");
	if(_this->connFd >= 0 &&
			Long_TCP_TEST::sock_write(_this->connFd, data_by_aes, strlen(data_by_aes)) <= 0){
		goto ERROR;
	}
  /*
	if(_this->connFd >= 0 &&
			Long_TCP_TEST::sock_write(_this->connFd, ARRAY_STR_LEN("\n")) <= 0){
		std::cout << "sock_write error: " <<  strerror(errno) << std::endl;
		goto ERROR;
	}
  */
	xfree(p_str);
	xfree(data_by_aes);
	if(json){
		json_delete(json);
	}
	return 0;
ERROR:
	xclose(_this->connFd);
	xfree(p_str);
	xfree(data_by_aes);
	if(json){
		json_delete(json);
	}
	return -1;
}
int Long_TCP_TEST::sock_peek (int fd, char *buf, int bufsize){
	  int res;
	  do{
	    res = recv (fd, buf, bufsize, MSG_PEEK);
	  }while (res == -1 && errno == EINTR);
	  return res;
}
int Long_TCP_TEST::sock_read (int fd, char *buf, int bufsize){
	  int res;
	  do{
	    res = read (fd, buf, bufsize);
	  }while (res == -1 && errno == EINTR);
	  return res;
}
int Long_TCP_TEST::sock_write (int fd, const char *buf, const int bufsize){
	  int res;
	  do{
	    res = write(fd, buf, bufsize);
	  }while(res == -1 && errno == EINTR);
	  return res;
}
int Long_TCP_TEST::tcp_conn(const char *hostname, const char *service){
  struct addrinfo hints, *res = NULL, *ressave = NULL;
  int n, sockfd;
  int so_optval;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  n = getaddrinfo(hostname, service, &hints, &res);
  if (n < 0){
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
//  Long_TCP_TEST test(1), test1(2);
  int i = 0;
  signal(SIGINT, Long_TCP_TEST::signal_handler);
  signal(SIGPIPE, Long_TCP_TEST::signal_handler);
  for(i = 0; i < 10000; i++){
    if(i % 100 == 0){
      std::cout << i << std::endl;
    }
    usleep(500 * 1000);
    if(Long_TCP_TEST::is_end){
      break;
    }
    new Long_TCP_TEST(i);
  }
//  test.print();
  while(!Long_TCP_TEST::is_end){
	  pause();
  }
  return 0;
}
