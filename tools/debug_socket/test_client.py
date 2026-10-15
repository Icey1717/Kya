import json
import socket
import threading
import unittest

from client import request


class DebugSocketClientTest(unittest.TestCase):
    def exchange(self, response, command="status"):
        listener = socket.socket()
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        listener.settimeout(3)
        port = listener.getsockname()[1]
        received = []

        def serve():
            with listener:
                connection, _ = listener.accept()
                with connection:
                    connection.settimeout(3)
                    line = connection.makefile("rb").readline()
                    received.append(json.loads(line))
                    # Split both the response header and payload across writes.
                    for offset in range(0, len(response), 3):
                        connection.sendall(response[offset:offset + 3])

        thread = threading.Thread(target=serve, daemon=True)
        thread.start()
        try:
            return request(command, port=port, timeout=3)
        finally:
            thread.join(timeout=4)
            self.assertFalse(thread.is_alive())
            self.assertEqual(received, [{"command": command, "id": 1}])

    def test_fragmented_image_payload_is_read_exactly(self):
        payload = b"\x89PNG\r\n\x1a\n\x00\xff\n"
        header = {"ok": True, "bytes": len(payload), "encoding": "png"}
        actual, image = self.exchange(json.dumps(header).encode() + b"\n" + payload,
                                      command="screenshot")
        self.assertEqual(actual, header)
        self.assertEqual(image, payload)

    def test_truncated_payload_is_rejected(self):
        with self.assertRaisesRegex(RuntimeError, "Truncated"):
            self.exchange(b'{"ok":true,"bytes":10}\nabc')

    def test_server_error_is_reported(self):
        with self.assertRaisesRegex(RuntimeError, "unavailable"):
            self.exchange(b'{"ok":false,"error":"unavailable","bytes":0}\n')


if __name__ == "__main__":
    unittest.main()
