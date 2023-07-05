from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Optional as _Optional

DESCRIPTOR: _descriptor.FileDescriptor

class ClientMessage(_message.Message):
    __slots__ = ["ProcessId"]
    PROCESSID_FIELD_NUMBER: _ClassVar[int]
    ProcessId: int
    def __init__(self, ProcessId: _Optional[int] = ...) -> None: ...

class GrantedMessage(_message.Message):
    __slots__ = ["TargetProcessId", "granted"]
    TARGETPROCESSID_FIELD_NUMBER: _ClassVar[int]
    GRANTED_FIELD_NUMBER: _ClassVar[int]
    TargetProcessId: int
    granted: bool
    def __init__(self, TargetProcessId: _Optional[int] = ..., granted: bool = ...) -> None: ...

class ReleaseResponse(_message.Message):
    __slots__ = ["TargetProcessId", "status"]
    TARGETPROCESSID_FIELD_NUMBER: _ClassVar[int]
    STATUS_FIELD_NUMBER: _ClassVar[int]
    TargetProcessId: int
    status: bool
    def __init__(self, TargetProcessId: _Optional[int] = ..., status: bool = ...) -> None: ...
