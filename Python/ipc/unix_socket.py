import json
import os
import socket
import threading
from types import TracebackType
from typing import Any, Callable, Optional, Type, cast
from socketserver import StreamRequestHandler, ThreadingMixIn, UnixStreamServer
from utils.message import InputMessage
from utils.debug import get_logger, setup_logging

from ipc.ipc_types import IPCMessage, IPCStatus

callbacks: dict[IPCMessage, Callable[[Any], bytes]] = {}


class UnixSocketHandler(StreamRequestHandler):
    def handle(self):
        while True:
            data = self.rfile.readline()
            if not data:
                break
            message = data.decode(errors="replace").strip()
            response = self._handle_message(message)
            if isinstance(response, IPCStatus):
                self.wfile.write(bytes([response.value]))
            else:
                self.wfile.write(response)
            self.wfile.flush()

    def _handle_message(self, message: str) -> bytes | IPCStatus:
        try:
            payload = json.loads(message)
        except json.JSONDecodeError:
            return IPCStatus.JSON_ERR

        if not isinstance(payload, dict):
            return IPCStatus.ERROR

        payload_dict = cast(dict[str, Any], payload)
        command_id = payload_dict.get("id")
        params: dict[str, Any] = payload_dict.get("params", {})
        pagination = params.get("pagination", None)
        params.pop("pagination", None)

        if not isinstance(command_id, int):
            return IPCStatus.ID_ERR

        try:
            command = IPCMessage(command_id)
        except ValueError:
            return IPCStatus.ID_ERR

        callback = callbacks.get(command)
        if callback is None:
            return IPCStatus.ID_ERR

        try:
            args = InputMessage(pagination=pagination, params=params)
            result = callback(args)
        except Exception:
            return IPCStatus.CB_ERR

        return result


class ThreadedUnixSocketServer(ThreadingMixIn, UnixStreamServer):
    pass


class UnixSocketServer:
    def __init__(self, socket_path: str = "/tmp/suwayomi.sock") -> None:
        self.socket_path = socket_path
        self._server: ThreadedUnixSocketServer | None = None
        self._thread: threading.Thread | None = None
        self._client_sock: socket.socket | None = None

    def add_response_callback(
        self,
        key: IPCMessage,
        callback: Callable[[InputMessage], bytes],
    ) -> None:
        callbacks[key] = callback

    def start(self) -> None:
        if self._server is not None:
            return

        if os.path.exists(self.socket_path):
            try:
                os.remove(self.socket_path)
            except OSError as exc:
                raise RuntimeError(
                    f"Failed to remove existing socket at {self.socket_path}"
                ) from exc

        self._server = ThreadedUnixSocketServer(self.socket_path, UnixSocketHandler)
        self._thread = threading.Thread(
            target=self._server.serve_forever,
            name="unix-socket-server",
            daemon=True,
        )
        self._thread.start()

    def stop(self) -> None:
        if self._server is None:
            return

        self._server.shutdown()
        self._server.server_close()
        self._server = None

        if os.path.exists(self.socket_path):
            try:
                os.remove(self.socket_path)
            except OSError:
                pass

        if self._thread is not None:
            self._thread.join(timeout=2)
            self._thread = None

    def __enter__(self) -> "UnixSocketServer":
        self.start()
        return self

    def __exit__(
        self,
        _exc_type: Optional[Type[BaseException]],
        _exc: Optional[BaseException],
        _tb: Optional[TracebackType],
    ) -> None:
        self.stop()


if __name__ == "__main__":
    setup_logging()
    logger = get_logger(__name__)
    server = UnixSocketServer()
    server.start()
    logger.info("Unix socket server is running...")
    try:
        while True:
            threading.Event().wait(1)
    except KeyboardInterrupt:
        server.stop()
