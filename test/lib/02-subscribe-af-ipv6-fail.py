#!/usr/bin/env python3

# Test whether a client fails to connect to broker when address family is mismatched

# The broker is started and uses AF_INET6 address family. The client is then started 
# with -4 arg to tell it to set MOSQ_OPT_AF_PREFERENCE to AF_INET. 
# 
# Expected behavior is that the client fails to connect, in which case, the test passes.


from mosq_test_helper import *

port = mosq_test.get_lib_port()

addr = "::1"
family = socket.AF_INET6

rc = 1
keepalive = 60
connect_packet = mosq_test.gen_connect("subscribe-af-test", keepalive=keepalive)
connack_packet = mosq_test.gen_connack(rc=0)

disconnect_packet = mosq_test.gen_disconnect()

mid = 1
subscribe_packet = mosq_test.gen_subscribe(mid, "af/test", 0)
suback_packet = mosq_test.gen_suback(mid, 0)

sock = socket.socket(family, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.settimeout(5)
sock.bind((addr, port))
sock.listen(5)

client_args = sys.argv[1:]
env = dict(os.environ)
env['LD_LIBRARY_PATH'] = '../../lib:../../lib/cpp'
try:
    pp = env['PYTHONPATH']
except KeyError:
    pp = ''
env['PYTHONPATH'] = '../../lib/python:'+pp
client = mosq_test.start_client(filename=sys.argv[1].replace('/', '-'), cmd=client_args, env=env, port=port)
try:
    (conn, address) = sock.accept()
    conn.settimeout(10)

    mosq_test.do_receive_send(conn, connect_packet, connack_packet, "connect")
    mosq_test.do_receive_send(conn, subscribe_packet, suback_packet, "subscribe")
    mosq_test.expect_packet(conn, "disconnect", disconnect_packet)
    rc = mosq_test.TestError.TEST_FAIL # if test gets this far, then client connected with ipv4, so test fails

    conn.close()
except (mosq_test.TestError, socket.timeout):
    rc = mosq_test.TestError.TEST_PASS
    pass
finally:
    client.terminate()
    client.wait()
    sock.close()

exit(rc)
