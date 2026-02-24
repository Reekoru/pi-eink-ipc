from enum import Enum

class IPCStatus(Enum):
    OK = 0
    ERROR = 1
    JSON_ERR = 2
    ID_ERR = 3
    CB_ERR = 4

class IPCMessage(Enum):
    IPC_MSG_PING = 0
    IPC_MSG_GET_SETTINGS = 1
    IPC_MSG_GET_EXTENSIONS = 2
    IPC_MSG_INSTALL_EXTENSION = 3
    IPC_MSG_UNINSTALL_EXTENSION = 4
    IPC_MSG_OPEN_EXTENSION_REPO = 5