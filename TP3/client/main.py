import grpc
from rpc import tp3_pb2_grpc
from rpc import tp3_pb2


def run():
    with grpc.insecure_channel("localhost:50051") as channel:
        # Create Stub
        stub = tp3_pb2_grpc.Tp3RPCStub(channel)
