"""Select fast checks or expensive retained fighter replays without omissions."""
import argparse
from pathlib import Path
import unittest


RETAINED_MODULES = frozenset(('test_fighter_animation', 'test_fighter_runtime', 'test_fighter_state_entry'))
ANIMATION_MODULES = frozenset(('test_animation_morphing',))


def flatten(suite):
    for test in suite:
        if isinstance(test, unittest.TestSuite):
            yield from flatten(test)
        else:
            yield test


def load(tier='all'):
    if tier not in ('all', 'fast', 'retained', 'animation'):
        raise ValueError('Unknown regression tier')
    suite = unittest.defaultTestLoader.discover(str(Path(__file__).parent), 'test_*.py')
    selected = []
    for test in flatten(suite):
        identity = test.id()
        # Failed imports must fail their own tier, rather than disappearing.
        module = identity.rsplit('.', 1)[-1] if identity.startswith('unittest.loader._FailedTest.') else identity.split('.')[0]
        selected_tier = 'retained' if module in RETAINED_MODULES else 'animation' if module in ANIMATION_MODULES else 'fast'
        if tier == 'all' or tier == selected_tier:
            selected.append(test)
    return unittest.TestSuite(selected)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tier', choices=('all', 'fast', 'retained', 'animation'), default='all')
    parser.add_argument('--list', action='store_true', help='List selected test IDs without running them')
    args = parser.parse_args()
    suite = load(args.tier)
    if args.list:
        for test in flatten(suite):
            print(test.id())
        return 0
    return 0 if unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful() else 1


if __name__ == '__main__':
    raise SystemExit(main())
