#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mosquitto/libmosquittopp.h>

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#endif

static int run = -1;
static int af_preference = -1;
static const char* host = NULL;

class mosquittopp_test : public mosqpp::mosquittopp
{
public:
	mosquittopp_test(const char *id);

	void on_connect(int rc);
	void on_disconnect(int rc);
};

mosquittopp_test::mosquittopp_test(const char *id) : mosqpp::mosquittopp(id)
{
}


void mosquittopp_test::on_connect(int rc)
{
	if(rc){
		exit(1);
	}else{
		disconnect();
	}
}


void mosquittopp_test::on_disconnect(int rc)
{
	run = rc;
}


int main(int argc, char *argv[])
{
	mosquittopp_test *mosq;

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

	mosqpp::lib_init();

	mosq = new mosquittopp_test("con-address-family-test");

    mosq->int_option(MOSQ_OPT_ADDRESS_FAMILY, af_preference);
    int rc = mosq->connect(host, port, 60);

	if (MOSQ_ERR_SUCCESS != rc) {
        return rc;
    }

	while(run == -1){
		mosq->loop();
	}
	delete mosq;

	mosqpp::lib_cleanup();

	return run;
}
