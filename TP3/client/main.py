import grpc
import multiprocessing
import sys
import logging

logging.basicConfig(level=logging.DEBUG)
import os

from clientRPC import tp3_pb2, tp3_pb2_grpc
import datetime
import time

filePath = os.path.abspath(__file__)
textPath = os.path.dirname(os.path.dirname(filePath))  # creates file in ../..


def write(text):
    print("write called")
    with open(f"{textPath}/results.txt", "a+") as f:
        f.write(text)


def handleClientRequests(requests, seconds, id, writefunc):
    with grpc.insecure_channel("localhost:50051") as channel:
        stub = tp3_pb2_grpc.Tp3RPCStub(channel)
        for _ in range(requests):
            message = tp3_pb2.ClientMessage(ProcessId=id)
            granted = stub.Request(message)
            if granted.granted:
                writefunc(f"Process Id: {id} - {datetime.datetime.now()}\n")
                time.sleep(seconds)
                stub.Release(message)


def main():
    if len(sys.argv) != 4:
        print(
            "Usage: python3 main.py <noOfProcesses> <noOfRequestsPerProcess> <secondsToSleep>"
        )
        sys.exit(1)
    n_processes, r_requests, k_seconds = [int(x) for x in sys.argv[1:]]

    processes = []
    for i in range(n_processes):
        p = multiprocessing.Process(
            target=handleClientRequests, args=(r_requests, k_seconds, i, write)
        )
        processes.append(p)

    for p in processes:
        p.start()

    for p in processes:
        p.join()

    print("done")


if __name__ == "__main__":
    main()
