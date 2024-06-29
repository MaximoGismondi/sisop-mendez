import time
import unittest
import os

TEST_FILE = "testfile"
TEST_DIR = "testdir"
TEST_CONTENT = "Hello, world!"

FILE_SYSTEM_ROOT_PATH = "/"  # change to the path of the root of the filesystem
BIG_FILE_SIZE = 1024


class TestFileSystem(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestFileSystem, self).__init__(*args, **kwargs)
        # clean up
        if os.path.exists(TEST_FILE):
            os.remove(TEST_FILE)
        if os.path.exists(TEST_DIR):
            os.rmdir(TEST_DIR)

    def test_create_file(self):
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(TEST_FILE))
        self.assertTrue(os.path.isfile(TEST_FILE))

        with open(TEST_FILE, "r") as f:
            content = f.read()
        self.assertEqual(content, TEST_CONTENT)
        os.remove(TEST_FILE)
        time.sleep(1)

    def test_delete_file(self):
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(TEST_FILE))
        os.remove(TEST_FILE)
        self.assertFalse(os.path.exists(TEST_FILE))
        time.sleep(1)

    def test_create_directory(self):
        os.mkdir(TEST_DIR)
        self.assertTrue(os.path.exists(TEST_DIR))
        self.assertTrue(os.path.isdir(TEST_DIR))
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_delete_directory(self):
        os.mkdir(TEST_DIR)
        self.assertTrue(os.path.exists(TEST_DIR))
        os.rmdir(TEST_DIR)
        self.assertFalse(os.path.exists(TEST_DIR))
        time.sleep(1)

    def test_create_file_in_directory(self):
        os.mkdir(TEST_DIR)
        with open(os.path.join(TEST_DIR, TEST_FILE), "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(os.path.join(TEST_DIR, TEST_FILE)))
        self.assertTrue(os.path.isfile(os.path.join(TEST_DIR, TEST_FILE)))
        os.remove(os.path.join(TEST_DIR, TEST_FILE))
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_delete_file_in_directory(self):
        os.mkdir(TEST_DIR)
        with open(os.path.join(TEST_DIR, TEST_FILE), "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(os.path.join(TEST_DIR, TEST_FILE)))
        os.remove(os.path.join(TEST_DIR, TEST_FILE))
        self.assertFalse(os.path.exists(os.path.join(TEST_DIR, TEST_FILE)))
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_list_directory(self):
        os.mkdir(TEST_DIR)
        with open(os.path.join(TEST_DIR, TEST_FILE), "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(TEST_DIR))
        self.assertTrue(os.path.exists(os.path.join(TEST_DIR, TEST_FILE)))
        files = os.listdir(TEST_DIR)
        self.assertTrue(TEST_FILE in files)
        os.remove(os.path.join(TEST_DIR, TEST_FILE))
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_read_write_file(self):
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        with open(TEST_FILE, "r") as f:
            content = f.read()
        self.assertEqual(content, TEST_CONTENT)
        os.remove(TEST_FILE)
        time.sleep(1)

    def test_stat_file(self):
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        stat = os.stat(TEST_FILE)
        self.assertEqual(stat.st_size, len(TEST_CONTENT))
        self.assertEqual(stat.st_nlink, 1)
        os.remove(TEST_FILE)
        time.sleep(1)

    def test_stat_directory(self):
        os.mkdir(TEST_DIR)
        stat = os.stat(TEST_DIR)
        # one for the directory itself and one for the parent directory
        self.assertEqual(stat.st_nlink, 2)
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_flush_file(self):
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
            stat = os.stat(TEST_FILE)
            self.assertEqual(stat.st_size, 0)
            f.flush()
            stat = os.stat(TEST_FILE)
            self.assertEqual(stat.st_size, len(TEST_CONTENT))
        os.remove(TEST_FILE)
        time.sleep(1)

    def test_utimens(self):
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        stat = os.stat(TEST_FILE)
        times = (int(time.time()), int(time.time()))
        self.assertEqual(stat.st_atime, times[0])
        self.assertEqual(stat.st_mtime, times[1])
        os.utime(TEST_FILE, times)
        stat = os.stat(TEST_FILE)
        self.assertEqual(stat.st_atime, times[0])
        self.assertEqual(stat.st_mtime, times[1])
        os.remove(TEST_FILE)
        time.sleep(1)

    def test_handle_nonexistent_file(self):
        with self.assertRaises(FileNotFoundError):
            with open(TEST_FILE, "r") as f:
                f.read()
        time.sleep(1)

    def test_handle_nonexistent_directory(self):
        with self.assertRaises(FileNotFoundError):
            os.listdir(TEST_DIR)
        time.sleep(1)

    def test_handle_nonexistent_file_rename(self):
        with self.assertRaises(FileNotFoundError):
            os.rename(TEST_FILE, "newfile")
        time.sleep(1)

    def test_handle_nonexistent_directory_rename(self):
        with self.assertRaises(FileNotFoundError):
            os.rename(TEST_DIR, "newdir")
        time.sleep(1)

    def test_handle_nonexistent_file_remove(self):
        with self.assertRaises(FileNotFoundError):
            os.remove(TEST_FILE)
        time.sleep(1)

    def test_handle_nonexistent_directory_remove(self):
        with self.assertRaises(FileNotFoundError):
            os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_large_file(self):
        with open(TEST_FILE, "w") as f:
            f.write("a" * BIG_FILE_SIZE)
        with open(TEST_FILE, "r") as f:
            content = f.read()
        self.assertEqual(len(content), BIG_FILE_SIZE)
        time.sleep(1)


if __name__ == "__main__":
    unittest.main()
