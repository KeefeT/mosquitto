#!/usr/bin/env python3

# Test whether a client succeeds/fails to connect to broker when address family is same/mismatched

from mosq_test_helper import *

def do_test(cmd, args, family, expect_failure):
    port = mosq_test.get_port()

    rc = 1
    connect_packet = mosq_test.gen_connect("con-address-family-test")
    connack_packet = mosq_test.gen_connack(rc=0)

    disconnect_packet = mosq_test.gen_disconnect()

    addr = "::1" if (family == socket.AF_INET6) else "127.0.0.1"
    sock = socket.socket(family, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.settimeout(5)
    sock.bind((addr, port))
    sock.listen(5)

    broker_args = [cmd, str(port)] + args
    env = mosq_test.env_add_ld_library_path()

    broker = mosq_test.start_client(cmd, broker_args, env)

    try:
        (conn, address) = sock.accept()
        conn.settimeout(5)

        mosq_test.expect_packet(conn, "connect", connect_packet)
        conn.send(connack_packet)

        mosq_test.expect_packet(conn, "disconnect", disconnect_packet)
        rc = 0

        conn.close()
    except mosq_test.TestError:
        pass
    except TimeoutError:
        if expect_failure:
            rc = 0
    finally:
        sock.close()
        if mosq_test.wait_for_subprocess(broker):
            print("test client not finished")
            rc=1
            exit(1)
        if rc:
            (o, e) = broker.communicate()
            print(o)
            print(e)
            print(f"Fail: {cmd} rc={rc}")
            exit(rc)

# IPv4
# Tell client to connect to 127.0.0.1 using AF_INET, with broker listening on AF_INET lo (PASS)
do_test("c/01-con-address-family.test", ["-4"], socket.AF_INET, False)
# Tell client to connect to ::1 using AF_INET6, with broker listening on AF_INET lo (FAIL)
try:
    do_test("c/01-con-address-family.test", ["-6"], socket.AF_INET, True)
except SystemExit:
    print("test passes, expected timeout\n")

# IPv6
# Tell client to connect to ::1 using AF_INET6, with broker listening on AF_INET6 lo (PASS)
do_test("c/01-con-address-family.test", ["-6"], socket.AF_INET6, False)
# Tell client to connect to 127.0.0.1 using AF_INET, with broker listening on AF_INET6 lo (FAIL)
try:
    do_test("c/01-con-address-family.test", ["-4"], socket.AF_INET6, True)
except SystemExit:
    print("test passes, expected timeout\n")