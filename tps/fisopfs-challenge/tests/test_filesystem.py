import time
import unittest
import os
import shutil

TEST_FILE = "testfile"
TEST_DIR = "testdir"
TEST_CONTENT = "Hello, world!"

CHALLENGE_TEST_DIR = "0"
CHALLENGE_TEST_SYMLINK = "testsymlink"
CHALLENGE_TEST_SYMLINK_DIR = "testsymlinkdir"
CHALLENGE_TEST_SYMLINK_FILE = "testsymlinkfile"

FILE_SYSTEM_ROOT_PATH = "/"  # change to the path of the root of the filesystem
BIG_FILE_SIZE = 1024

# Nivel máximo de recursión permitido para anidar directorios
# (asegurarse que sea el mismo que el definido en el filesystem)
MAX_RECURSION_LEVEL = 16


class TestFileSystem(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestFileSystem, self).__init__(*args, **kwargs)
        # clean up
        init_test()
    
    def test_create_file(self):
        init_test()
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
        init_test()
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(TEST_FILE))
        os.remove(TEST_FILE)
        self.assertFalse(os.path.exists(TEST_FILE))
        time.sleep(1)

    def test_create_directory(self):
        init_test()
        os.mkdir(TEST_DIR)
        self.assertTrue(os.path.exists(TEST_DIR))
        self.assertTrue(os.path.isdir(TEST_DIR))
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_delete_directory(self):
        init_test()
        os.mkdir(TEST_DIR)
        self.assertTrue(os.path.exists(TEST_DIR))
        os.rmdir(TEST_DIR)
        self.assertFalse(os.path.exists(TEST_DIR))
        time.sleep(1)

    def test_create_file_in_directory(self):
        init_test()
        os.mkdir(TEST_DIR)
        with open(os.path.join(TEST_DIR, TEST_FILE), "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(os.path.join(TEST_DIR, TEST_FILE)))
        self.assertTrue(os.path.isfile(os.path.join(TEST_DIR, TEST_FILE)))
        os.remove(os.path.join(TEST_DIR, TEST_FILE))
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_delete_file_in_directory(self):
        init_test()
        os.mkdir(TEST_DIR)
        with open(os.path.join(TEST_DIR, TEST_FILE), "w") as f:
            f.write(TEST_CONTENT)
        self.assertTrue(os.path.exists(os.path.join(TEST_DIR, TEST_FILE)))
        os.remove(os.path.join(TEST_DIR, TEST_FILE))
        self.assertFalse(os.path.exists(os.path.join(TEST_DIR, TEST_FILE)))
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_list_directory(self):
        init_test()
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
        init_test()
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        with open(TEST_FILE, "r") as f:
            content = f.read()
        self.assertEqual(content, TEST_CONTENT)
        os.remove(TEST_FILE)
        time.sleep(1)

    def test_stat_file(self):
        init_test()
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        stat = os.stat(TEST_FILE)
        self.assertEqual(stat.st_size, len(TEST_CONTENT))
        self.assertEqual(stat.st_nlink, 1)
        os.remove(TEST_FILE)
        time.sleep(1)

    def test_stat_directory(self):
        init_test()
        os.mkdir(TEST_DIR)
        stat = os.stat(TEST_DIR)
        # one for the directory itself and one for the parent directory
        self.assertEqual(stat.st_nlink, 2)
        os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_flush_file(self):
        init_test()
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
        init_test()
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
        init_test()
        with self.assertRaises(FileNotFoundError):
            with open(TEST_FILE, "r") as f:
                f.read()
        time.sleep(1)

    def test_handle_nonexistent_directory(self):
        init_test()
        with self.assertRaises(FileNotFoundError):
            os.listdir(TEST_DIR)
        time.sleep(1)

    def test_handle_nonexistent_file_rename(self):
        init_test()
        with self.assertRaises(FileNotFoundError):
            os.rename(TEST_FILE, "newfile")
        time.sleep(1)

    def test_handle_nonexistent_directory_rename(self):
        init_test()
        with self.assertRaises(FileNotFoundError):
            os.rename(TEST_DIR, "newdir")
        time.sleep(1)

    def test_handle_nonexistent_file_remove(self):
        init_test()
        with self.assertRaises(FileNotFoundError):
            os.remove(TEST_FILE)
        time.sleep(1)

    def test_handle_nonexistent_directory_remove(self):
        init_test()
        with self.assertRaises(FileNotFoundError):
            os.rmdir(TEST_DIR)
        time.sleep(1)

    def test_large_file(self):
        init_test()
        with open(TEST_FILE, "w") as f:
            f.write("a" * BIG_FILE_SIZE)
        with open(TEST_FILE, "r") as f:
            content = f.read()
        self.assertEqual(len(content), BIG_FILE_SIZE)
        time.sleep(1)


    # Tests del challenge

    # Testea que se puedan anidar directorios hasta el máximo nivel de recursión
    def test_multiple_nested_directories(self):
        init_test()
        os.makedirs(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)]), exist_ok=True)
        for i in range(MAX_RECURSION_LEVEL - 1):
            self.assertTrue(os.path.exists(os.path.join(*[f"{i}" for i in range(i + 1)])))
        shutil.rmtree(CHALLENGE_TEST_DIR)
        time.sleep(1)

    # Testea que se puedan anidar directorios hasta el máximo nivel de recursión
    # y que se puedan crear archivos en cada uno de ellos
    def test_multiple_nested_directories_and_files(self):
        init_test()
        os.makedirs(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)]), exist_ok=True)
        for i in range(MAX_RECURSION_LEVEL - 1):
            with open(os.path.join(*[f"{i}" for i in range(i + 1)], TEST_FILE), "w") as f:
                f.write(TEST_CONTENT)
        
        for i in range(MAX_RECURSION_LEVEL - 1):
            self.assertTrue(os.path.exists(os.path.join(*[f"{i}" for i in range(i + 1)])))
            self.assertTrue(os.path.exists(os.path.join(*[f"{i}" for i in range(i + 1)], TEST_FILE)))
            with open(os.path.join(*[f"{i}" for i in range(i + 1)], TEST_FILE), "r") as f:
                content = f.read()
            self.assertEqual(content, TEST_CONTENT)
        shutil.rmtree(CHALLENGE_TEST_DIR)
        time.sleep(1)

    # Testea que se produzca un PermissionError al intentar anidar directorios
    # más allá del nivel de recursión permitido
    def test_handle_recursion_level_exceeded(self):
        init_test()
        os.makedirs(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)]), exist_ok=True)
        with self.assertRaises(PermissionError):
            os.mkdir(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL)]))
        shutil.rmtree(CHALLENGE_TEST_DIR)
        time.sleep(1)

    # Testea que se puedan crear symbolic links a archivos
    def test_create_symbolic_link_to_file(self):
        init_test()
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        os.symlink(TEST_FILE, CHALLENGE_TEST_SYMLINK)
        self.assertTrue(os.path.exists(CHALLENGE_TEST_SYMLINK))
        self.assertTrue(os.path.islink(CHALLENGE_TEST_SYMLINK))
        os.remove(TEST_FILE)
        os.unlink(CHALLENGE_TEST_SYMLINK)
        time.sleep(1)
    
    # Testea que se puedan crear symbolic links a directorios
    def test_create_symbolic_link_to_directory(self):
        init_test()
        os.mkdir(TEST_DIR)
        os.symlink(TEST_DIR, CHALLENGE_TEST_SYMLINK)
        self.assertTrue(os.path.exists(CHALLENGE_TEST_SYMLINK))
        self.assertTrue(os.path.islink(CHALLENGE_TEST_SYMLINK))
        os.rmdir(TEST_DIR)
        os.unlink(CHALLENGE_TEST_SYMLINK)
        time.sleep(1)
    
    # Testea que se pueda acceder al contendio de un archivo a través de un
    # symbolic link
    def test_access_symbolic_links(self):
        init_test()
        with open(TEST_FILE, "w") as f:
            f.write(TEST_CONTENT)
        os.symlink(TEST_FILE, CHALLENGE_TEST_SYMLINK_FILE)
        with open(CHALLENGE_TEST_SYMLINK_FILE, "r") as f:
            content = f.read()
        self.assertEqual(content, TEST_CONTENT)
        os.remove(TEST_FILE)
        os.unlink(CHALLENGE_TEST_SYMLINK_FILE)
        time.sleep(1)
    
    # Testea que se produzca un FileNotFoundError al intentar acceder a un symbolic link
    # que apunta a un archivo inexistente
    def test_handle_nonexistent_file_symbolic_link(self):
        init_test()
        os.symlink(TEST_FILE, CHALLENGE_TEST_SYMLINK)
        with self.assertRaises(FileNotFoundError):
            with open(CHALLENGE_TEST_SYMLINK, "r") as f:
                f.read()
        os.unlink(CHALLENGE_TEST_SYMLINK)
        time.sleep(1)
        
    # Testea que se produzca un FileNotFoundError al intentar acceder a un symbolic link
    # que apunta a un directorio inexistente
    def test_handle_nonexistent_directory_symbolic_link(self):
        init_test()
        os.symlink(TEST_DIR, CHALLENGE_TEST_SYMLINK)
        with self.assertRaises(FileNotFoundError):
            os.listdir(CHALLENGE_TEST_SYMLINK)
        os.unlink(CHALLENGE_TEST_SYMLINK)
        time.sleep(1)
    
    # Testea que se pueda escribir en un archivo a través de un symbolic link
    # y que el contenido sea el esperado en el archivo original
    def test_write_to_nonexistent_file_through_symbolic_link(self):
        init_test()
        os.symlink(TEST_FILE, CHALLENGE_TEST_SYMLINK)
        with open(CHALLENGE_TEST_SYMLINK, "w") as f:
            f.write(TEST_CONTENT)

        self.assertTrue(os.path.exists(TEST_FILE))
        with open(TEST_FILE, "r") as f:
            content = f.read()
        self.assertEqual(content, TEST_CONTENT)
        with open(CHALLENGE_TEST_SYMLINK, "r") as f:
            content = f.read()
        self.assertEqual(content, TEST_CONTENT)
        os.remove(TEST_FILE)
        os.unlink(CHALLENGE_TEST_SYMLINK)
        time.sleep(1)

    # Testea que se pueda crear y utilizar correctamente un symbolic link a un
    # directorio, ubicando el symbolic link en un directorio anidado en el
    # máximo nivel de recursión
    def test_create_symbolic_link_at_recursion_limit(self):
        init_test()
        os.mkdir(TEST_DIR)
        with open(os.path.join(TEST_DIR, TEST_FILE), "w") as f:
            f.write(TEST_CONTENT)
        os.makedirs(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)]),
                    exist_ok=True)
        os.symlink(os.path.join(*[".." for _ in range(MAX_RECURSION_LEVEL - 1)], TEST_DIR),
                   os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)],
                                CHALLENGE_TEST_SYMLINK_DIR))

        self.assertTrue(os.path.exists(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)])))
        self.assertTrue(os.path.exists(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)],
                                                    CHALLENGE_TEST_SYMLINK_DIR)))
        self.assertTrue(os.path.exists(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)],
                                                    CHALLENGE_TEST_SYMLINK_DIR, TEST_FILE)))
        with open(os.path.join(*[f"{i}" for i in range(MAX_RECURSION_LEVEL - 1)],
                               CHALLENGE_TEST_SYMLINK_DIR, TEST_FILE), "r") as f:
            content = f.read()
        self.assertEqual(content, TEST_CONTENT)
        shutil.rmtree(TEST_DIR)
        shutil.rmtree(CHALLENGE_TEST_DIR)
        time.sleep(1)
    
def init_test():
    if os.path.exists(CHALLENGE_TEST_DIR):
        shutil.rmtree(CHALLENGE_TEST_DIR)
    if (os.path.exists(CHALLENGE_TEST_SYMLINK)):
        os.unlink(CHALLENGE_TEST_SYMLINK)
    if (os.path.exists(CHALLENGE_TEST_SYMLINK_DIR)):
        os.unlink(CHALLENGE_TEST_SYMLINK_DIR)
    if (os.path.exists(CHALLENGE_TEST_SYMLINK_FILE)):
        os.unlink(CHALLENGE_TEST_SYMLINK_FILE)
    if os.path.exists(TEST_FILE):
        os.remove(TEST_FILE)
    if os.path.exists(TEST_DIR):
        shutil.rmtree(TEST_DIR)

if __name__ == "__main__":
    unittest.main()
