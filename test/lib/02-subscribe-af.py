#!/usr/bin/env python3

# Test whether a client succeeds/fails to connect to broker when address family is same/mismatched

from mosq_test_helper import *

def do_test(conn, data):
    connect_packet = mosq_test.gen_connect("subscribe-af-test")
    connack_packet = mosq_test.gen_connack(rc=0)

    disconnect_packet = mosq_test.gen_disconnect()

    mid = 1
    subscribe_packet = mosq_test.gen_subscribe(mid, "af/test", 0)
    suback_packet = mosq_test.gen_suback(mid, 2)

    publish_packet = mosq_test.gen_publish("af/test", 0, "message")

    mosq_test.do_receive_send(conn, connect_packet, connack_packet, "connect")
    mosq_test.do_receive_send(conn, subscribe_packet, suback_packet, "subscribe")
    conn.send(publish_packet)
    mosq_test.expect_packet(conn, "disconnect", disconnect_packet)

# IPv4 
# Tell client to connect to 127.0.0.1 using AF_INET, with broker listening on AF_INET (PASS)
mosq_test.client_test("c/02-subscribe-af-client.test", ["-4"], do_test, None, sock_family=socket.AF_INET)
# Tell client to connect to ::1 using AF_INET6, with broker listening on AF_INET (FAIL)
try:
    mosq_test.client_test("c/02-subscribe-af-client.test", ["-6"], do_test, None, sock_family=socket.AF_INET)
except SystemExit:
    print("test passes, expected timeout\n")

# IPv6 
# Tell client to connect to ::1 using AF_INET6, with broker listening on AF_INET6 (PASS)
mosq_test.client_test("c/02-subscribe-af-client.test", ["-6"], do_test, None, sock_family=socket.AF_INET6)
# Tell client to connect to 127.0.0.1 using AF_INET, with broker listening on AF_INET6 (FAIL)
try:
    mosq_test.client_test("c/02-subscribe-af-client.test", ["-4"], do_test, None, sock_family=socket.AF_INET6)
except SystemExit:
    print("test passes, expected timeout\n")
