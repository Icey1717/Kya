"""Tier selection must be a disjoint, exhaustive partition of all regressions."""
import unittest
from regressions import flatten, load, RETAINED_MODULES, ANIMATION_MODULES


class RegressionTierTests(unittest.TestCase):
    def test_tiers_are_disjoint_and_exhaustive(self):
        identities = {tier: [test.id() for test in flatten(load(tier))] for tier in ('all', 'fast', 'retained', 'animation')}
        self.assertEqual(len(set(identities['all'])), len(identities['all']))
        self.assertFalse(set(identities['fast']) & set(identities['retained']))
        self.assertFalse(set(identities['animation']) & (set(identities['fast']) | set(identities['retained'])))
        self.assertEqual(set(identities['all']), set(identities['fast']) | set(identities['retained']) | set(identities['animation']))
        self.assertEqual({name.split('.')[0] for name in identities['retained']}, RETAINED_MODULES)
        self.assertEqual({name.split('.')[0] for name in identities['animation']}, ANIMATION_MODULES)

    def test_unknown_tier_is_rejected(self):
        with self.assertRaises(ValueError):
            load('partial')


if __name__ == '__main__':
    unittest.main()
