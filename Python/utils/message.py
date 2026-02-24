def create_message(status: int, content: str) -> bytes:
    return bytes([status]) + f"{content}\0".encode()