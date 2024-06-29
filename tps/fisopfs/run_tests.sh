mkdir -p test_filesystem
./fisopfs test_filesystem fs.fisopfs
cd test_filesystem 
python3 ../tests/test_filesystem.py || true
cd ..
fusermount -u test_filesystem