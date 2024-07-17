
# run like a normal python script and observe results

import unittest
from parse_name import _parse_name

class TestParseName(unittest.TestCase):

    def test_addition(self):
        correct = {'asdf': 3}
        actual = _parse_name({}, "asdf", 3)

        self.assertEqual(correct, actual)

if __name__ == '__main__':
    unittest.main()