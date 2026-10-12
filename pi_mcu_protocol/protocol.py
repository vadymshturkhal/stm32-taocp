from enum import IntEnum

class MsgType(IntEnum):
    SYN     = 0
    SYNACK  = 1
    ACK     = 2
    RST     = 3
    DATA    = 4

PROTOCOL_VERSION = 1
SESSION_TIMEOUT  = 4.0  # seconds
SYNACK_WAIT      = 0.2  # seconds to wait for SYNACK before resending SYN
SYN_TRIES        = 4