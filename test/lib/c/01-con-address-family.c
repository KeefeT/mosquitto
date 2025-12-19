#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#endif

static int af_preference = -1;
static const char* host = NULL;
static int run = -1;

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    (void)mosq;
    (void)obj;

	if(rc){
		exit(1);
	}else{
		mosquitto_disconnect(mosq);
	}
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
    (void)mosq;
    (void)obj;

	run = rc;
}

int main(int argc, char *argv[])
{
	struct mosquitto *mosq;

    if(argc < 2){
        return 1;
    }

	int port = atoi(argv[1]);
    for(int i = 2; i < argc; i++){
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

	mosq = mosquitto_new("con-address-family-test", true, NULL);
	if(mosq == NULL){
		return 1;
	}
    mosquitto_int_option(mosq, MOSQ_OPT_ADDRESS_FAMILY, af_preference);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);

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
