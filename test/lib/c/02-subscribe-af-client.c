#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

#ifdef WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <errno.h>
#endif

static int af_preference = -1;
static char* host = NULL;
static int run = -1;

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    (void)mosq;
    (void)obj;

	if(rc){
		exit(1);
	}else{
		mosquitto_subscribe(mosq, NULL, "af/test", 0);
	}
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
    (void)mosq;
    (void)obj;

	run = rc;
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    (void)mosq;
    (void)obj;
    (void)mid;
    (void)qos_count;
    (void)granted_qos;

    int fd = mosquitto_socket(mosq);
    struct sockaddr_storage ss;
#ifdef WIN32
    int slen = (int)sizeof(ss);
    if(fd != INVALID_SOCKET && getpeername(fd, (struct sockaddr *)&ss, &slen) == 0)
#else
    socklen_t slen = (socklen_t)sizeof(ss);
    if(fd >= 0 && getpeername(fd, (struct sockaddr *)&ss, &slen) == 0)
#endif
    {
        if(ss.ss_family != af_preference){
            exit(1);
        }
    }else{
        exit(1);
    }
    mosquitto_disconnect(mosq);
}

int main(int argc, char *argv[])
{
	struct mosquitto *mosq;

    if(argc < 2){
        return 1;
    }

	int port = atoi(argv[1]);
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-4")){
            af_preference = AF_INET;
            host = "127.0.0.1";
        }else if(!strcmp(argv[i], "-6")){
            af_preference = AF_INET6;
            host = "::1";
        }
    }

    if(af_preference < 0 || host == NULL){
        printf("invalid arg\n");
        return 1;
    }

	mosquitto_lib_init();

	mosq = mosquitto_new("subscribe-af-test", true, NULL);
	if(mosq == NULL){
		return 1;
	}
    mosquitto_int_option(mosq, MOSQ_OPT_ADDRESS_FAMILY, af_preference);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);
	mosquitto_subscribe_callback_set(mosq, on_subscribe);

	if (MOSQ_ERR_SUCCESS != mosquitto_connect(mosq, host, port, 60)) {
        printf("cannot connect to host %s:%d!", host, port);
        run = 0;
    }

	while(run == -1){
		mosquitto_loop(mosq, -1, 1);
	}

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return run;
}