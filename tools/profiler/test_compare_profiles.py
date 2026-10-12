import unittest

from compare_profiles import compare, summarize


class ProfileComparisonTests(unittest.TestCase):
    def test_shifted_lines_preserve_distinct_same_name_zones(self):
        before = summarize({("Level_Manage", "scene.cpp", 887): [2000000, 4000000],
                            ("Level_Manage", "scene.cpp", 892): [100000, 300000]})
        after = summarize({("Level_Manage", "scene.cpp", 897): [1000000, 3000000],
                           ("Level_Manage", "scene.cpp", 902): [50000, 150000]})
        rows = compare(before, after)
        self.assertEqual(len(rows), 2)
        self.assertEqual((rows[0]["before_line"], rows[0]["after_line"]), (887, 897))
        self.assertEqual(rows[0]["before_median_ms"], 3)
        self.assertEqual(rows[0]["after_median_ms"], 2)
        self.assertAlmostEqual(rows[0]["change_percent"], -100 / 3)
        self.assertEqual(rows[1]["change_percent"], -50)

    def test_added_removed_and_zero_baseline_are_not_invented_percentages(self):
        before = summarize({("removed", "a.cpp", 1): [1000], ("zero", "a.cpp", 2): [0]})
        after = summarize({("added", "a.cpp", 3): [2000], ("zero", "a.cpp", 2): [1000]})
        rows = {row["name"]: row for row in compare(before, after)}
        self.assertEqual(rows["removed"]["status"], "removed")
        self.assertEqual(rows["added"]["status"], "added")
        self.assertEqual(rows["zero"]["delta_ms"], 0.001)
        for row in rows.values():
            self.assertIsNone(row["change_percent"])


if __name__ == "__main__":
    unittest.main()
