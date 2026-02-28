from typing import Any, Optional

from attr import dataclass


def create_message(status: int, content: str) -> bytes:
    return bytes([status]) + f"{content}\0".encode()

@dataclass
class InputMessage:
    pagination: Optional[dict[str, Any]]
    params: Optional[dict[str, Any]]