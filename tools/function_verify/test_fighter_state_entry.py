"""Retained complete production bodies, including real fighter entry and exit."""
from pathlib import Path
import unittest
from fighter_transitions import base_transition, load_fixture


class FighterStateEntryTests(unittest.TestCase):
    def test_live_run_to_idle_with_null_animation_resources(self):
        manifest, originals, _ = load_fixture(Path(__file__).parent / 'fixtures/fighter-state-entry.json.gz')
        result = base_transition(manifest, originals, old_states=(9,), new_states=(6,),
                                 live_behaviour=True, animation='configured', idle_loops='zero')
        self.assertEqual(result['result'], 'equivalent')
        self.assertGreater(result['paired_outcomes'], 0)
        self.assertEqual(result['reference_models'], [])
        self.assertEqual(result['numeric_policies'], {})
        self.assertFalse(result['full_fighter_contract'])


if __name__ == '__main__':
    unittest.main()
