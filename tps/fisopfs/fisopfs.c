#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "admin_filesystem.h"

admin_filesystem_t *admin;

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	return admin_readdir(admin, path, buffer, filler);
}

static int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_open - path: %s\n", path);

	if ((admin_open(admin, path, fi, false)) == EXIT_FAILURE) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);

	fi->flags |= O_CREAT | O_WRONLY | O_TRUNC;

	return fisopfs_open(path, fi);
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	int size_read = admin_read(admin, path, buffer, size, offset, fi);
	if (size_read == -1) {
		return -ENOENT;
	}

	return size_read;
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	if (admin_write(admin, path, buffer, size, offset, fi) == -1) {
		return -ENOENT;
	}

	return size;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	if (admin_mkdir(admin, path, NEW_DIR_MODE) == EXIT_FAILURE) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	if (admin_unlink(admin, path, true) < 0) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	if (admin_rmdir(admin, path) < 0) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (admin_getattr(admin, path, st) < 0) {
		return -ENOENT;
	}

	return EXIT_SUCCESS;
}

static int
fisopfs_stat(const char *path, struct stat *stbuf)
{
	printf("[debug] fisopfs_stat - path: %s\n", path);

	if (admin_stat(admin, path, stbuf) < 0) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	if (admin_utimens(admin, path, tv) < 0) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush - path: %s\n", path);

	int error;
	if ((error = admin_flush(admin)) != 0) {
		return error;
	}

	return 0;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s, size: %lu\n", path, size);

	if (admin_truncate(admin, path, size) < 0) {
		return -ENOENT;
	}

	return 0;
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy\n");
	admin_destroy(admin, true);
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.open = fisopfs_open,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.mkdir = fisopfs_mkdir,
	.unlink = fisopfs_unlink,
	.rmdir = fisopfs_rmdir,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
	.truncate = fisopfs_truncate,
	.flush = fisopfs_flush,
	.destroy = fisopfs_destroy,
};

int
main(int argc, char *argv[])
{
	char *fs_backup_file_path = argv[argc - 1];

	if (admin_init(&admin, fs_backup_file_path)) {
		perror("Error al iniciar el filesystem");
		return EXIT_FAILURE;
	}

	printf("Filesystem iniciado correctamente\n");
	printf("Iniciando FUSE\n");
	int exit = fuse_main(argc - 1, argv, &operations, NULL);
	printf("FUSE main terminó con código: %d\n", exit);
	return exit;
}
