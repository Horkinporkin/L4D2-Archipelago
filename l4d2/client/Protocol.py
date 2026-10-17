from __future__ import annotations

import asyncio
import json
from dataclasses import dataclass, field
from typing import Any

PROTOCOL_VERSION = 1
DEFAULT_BRIDGE_HOST = "127.0.0.1"
DEFAULT_BRIDGE_PORT = 12138


class BridgeError(Exception):
    """Something went wrong talking to the Left 4 Dead 2 bridge."""


class BridgeProtocolError(BridgeError):
    """The bridge sent something that is not a bridge message."""


class BridgeVersionError(BridgeProtocolError):
    """The bridge is a different version of the protocol than this client."""

    def __init__(self, version: Any) -> None:
        super().__init__(
            f"the game uses bridge protocol version {version!r}, this client uses version "
            f"{PROTOCOL_VERSION}. Update the Left 4 Dead 2 APWorld and the SourceMod plugin together."
        )
        self.version = version


@dataclass
class BridgeMessage:
    type: str
    payload: dict[str, Any] = field(default_factory=dict)

    def encode(self) -> bytes:
        message = {"version": PROTOCOL_VERSION, "type": self.type, "payload": self.payload}
        return (json.dumps(message, ensure_ascii=False, separators=(",", ":")) + "\n").encode("utf-8")

    @classmethod
    def decode(cls, line: bytes) -> BridgeMessage:
        try:
            raw = json.loads(line.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError) as error:
            raise BridgeProtocolError(f"bridge message is not valid JSON: {line!r}") from error

        if not isinstance(raw, dict):
            raise BridgeProtocolError("bridge message must be a JSON object.")

        version = raw.get("version")
        if not isinstance(version, int):
            raise BridgeProtocolError("bridge message has no protocol version.")
        if version != PROTOCOL_VERSION:
            raise BridgeVersionError(version)

        message_type = raw.get("type")
        if not isinstance(message_type, str) or not message_type:
            raise BridgeProtocolError("bridge message has no type.")

        payload = raw.get("payload", {})
        if not isinstance(payload, dict):
            raise BridgeProtocolError(f"bridge message {message_type!r} has a non-object payload.")

        return cls(message_type, payload)


class BridgeConnection:
    def __init__(self, host: str = DEFAULT_BRIDGE_HOST, port: int = DEFAULT_BRIDGE_PORT) -> None:
        self.host = host
        self.port = port
        self._reader: asyncio.StreamReader | None = None
        self._writer: asyncio.StreamWriter | None = None
        self._write_lock = asyncio.Lock()

    @property
    def connected(self) -> bool:
        return self._writer is not None and not self._writer.is_closing()

    async def connect(self) -> None:
        self._reader, self._writer = await asyncio.open_connection(self.host, self.port)

    async def close(self) -> None:
        self._reader = None
        writer, self._writer = self._writer, None
        if writer is None:
            return
        writer.close()
        try:
            await writer.wait_closed()
        except ConnectionError:
            pass

    async def send(self, message: BridgeMessage) -> None:
        writer = self._writer
        if writer is None or writer.is_closing():
            raise ConnectionError("the Left 4 Dead 2 bridge is not connected.")
        async with self._write_lock:
            writer.write(message.encode())
            await writer.drain()

    async def receive(self) -> BridgeMessage:
        reader = self._reader
        if reader is None or not self.connected:
            raise ConnectionError("the Left 4 Dead 2 bridge is not connected.")
        line = await reader.readline()
        if not line:
            raise ConnectionError("the Left 4 Dead 2 bridge closed the connection.")
        return BridgeMessage.decode(line)
