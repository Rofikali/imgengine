import unittest

from check_compose_loopback_bindings import validate_config


def valid_config():
    return {
        "services": {
            "api": {"ports": [{"target": 8000, "published": "8000", "host_ip": "127.0.0.1"}]},
            "rust-api": {"ports": [{"target": 8000, "published": "8001", "host_ip": "127.0.0.1"}]},
            "web": {"ports": [{"target": 3000, "published": "3000", "host_ip": "127.0.0.1"}]},
            "minio": {"ports": [
                {"target": 9000, "published": "9000", "host_ip": "127.0.0.1"},
                {"target": 9001, "published": "9001", "host_ip": "127.0.0.1"},
            ]},
            "grafana": {"ports": [{"target": 3000, "published": "3001", "host_ip": "127.0.0.1"}]},
        }
    }


class LoopbackBindingsTest(unittest.TestCase):
    def test_accepts_required_and_optional_loopback_ports(self):
        self.assertEqual(validate_config(valid_config()), [])

    def test_rejects_public_web_binding(self):
        config = valid_config()
        config["services"]["web"]["ports"][0]["host_ip"] = "0.0.0.0"
        self.assertTrue(any("web" in error and "0.0.0.0" in error
                            for error in validate_config(config)))

    def test_rejects_public_optional_profile_binding(self):
        config = valid_config()
        config["services"]["minio"]["ports"][1]["host_ip"] = None
        self.assertTrue(any("minio" in error for error in validate_config(config)))

    def test_rejects_missing_or_changed_required_binding(self):
        config = valid_config()
        config["services"]["rust-api"]["ports"][0]["published"] = "8002"
        self.assertTrue(any("rust-api" in error for error in validate_config(config)))

    def test_rejects_missing_required_service(self):
        config = valid_config()
        del config["services"]["api"]
        self.assertTrue(any("api" in error for error in validate_config(config)))

    def test_rejects_invalid_config_shape(self):
        self.assertTrue(validate_config([]))
        self.assertTrue(validate_config({"services": []}))


if __name__ == "__main__":
    unittest.main()
