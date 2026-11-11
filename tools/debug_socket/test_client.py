import json
import socket
import threading
import unittest
from unittest.mock import patch

from client import request, screenshot_without_actors


class DebugSocketClientTest(unittest.TestCase):
    def test_actor_free_capture_restores_previous_visibility(self):
        for was_hidden in (False, True):
            with self.subTest(was_hidden=was_hidden), patch('client.request') as send:
                send.side_effect = [({'actors_hidden': was_hidden}, b''),
                                    ({'frame': 10}, b''), ({'frame': 13}, b''),
                                    ({'actors_hidden': True}, b'png'), ({}, b'')]
                self.assertEqual(screenshot_without_actors(18438, 3)[1], b'png')
                self.assertEqual(send.call_args_list[-1].kwargs, {'visible': not was_hidden})
                self.assertEqual(send.call_args_list[3].kwargs, {'target': 'game'})

    def test_actor_free_capture_restores_visibility_on_readback_failure(self):
        with patch('client.request') as send:
            send.side_effect = [({'actors_hidden': False}, b''), ({'frame': 10}, b''),
                                ({'frame': 13}, b''), RuntimeError('readback failed'), ({}, b'')]
            with self.assertRaisesRegex(RuntimeError, 'readback failed'):
                screenshot_without_actors(18438, 3)
            self.assertEqual(send.call_args_list[-1].kwargs, {'visible': True})

    def exchange(self, response, command="status", target=None, **fields):
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
            return request(command, port=port, timeout=3, target=target, **fields)
        finally:
            thread.join(timeout=4)
            self.assertFalse(thread.is_alive())
            expected = {"command": command, "id": 1, **fields}
            if target is not None:
                expected["target"] = target
            self.assertEqual(received, [expected])

    def test_mesh_screenshot_target(self):
        header, payload = self.exchange(b'{"ok":true,"bytes":3,"target":"mesh"}\nabc',
                                        command="screenshot", target="mesh")
        self.assertEqual(header["target"], "mesh")
        self.assertEqual(payload, b"abc")

    def test_actor_override_fields(self):
        header, _ = self.exchange(b'{"ok":true,"actor_override":"texture_only"}\n',
                                  command="actor_override", mode="texture_only", actor=17)
        self.assertEqual(header["actor_override"], "texture_only")

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
