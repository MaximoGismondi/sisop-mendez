#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "admin_filesystem.h"
#include <errno.h>

typedef struct __attribute__((__packed__)) {
	char name[MAX_FILE_NAME_SIZE];
	size_t id;
	time_t creation_timestamp;
	time_t last_edit_timestamp;
	time_t last_access_timestamp;
	size_t size;
	bool is_directory;
	size_t children_count;
	mode_t mode;
} packed_metadata_t;

// Flag devuelta por admin_load_from_file cuando el archivo pasado para guardar
// el filesystem no existe o está vacío.
#define EMPTY_BACKUP_FILE 2


file_t *admin_find_file(admin_filesystem_t *admin, const char *path);

file_t *find_file_in_directory(file_t *directory, char *file_name);

static int admin_load_file_metadata_from_backup_file(admin_filesystem_t *admin,
                                                     file_t *current_file,
                                                     FILE *backup_file);

static int admin_load_file_data_from_backup_file(admin_filesystem_t *admin,
                                                 file_t *current_file,
                                                 FILE *backup_file);

static int admin_save_file_metadata_to_backup_file(admin_filesystem_t *admin,
                                                   file_t *current_file,
                                                   FILE *backup_file);

static int admin_save_file_data_to_backup_file(admin_filesystem_t *admin,
                                               file_t *current_file,
                                               FILE *backup_file);

file_t *create_file(char *file_name, mode_t mode, bool is_directory);

void create_file_in_dir(file_t *directory,
                        char *file_name,
                        mode_t mode,
                        bool is_directory);

int fill_stats_from_file(file_t *file, struct stat *stbuf);

static void parse_path(const char *path, char *token1, char *token2);

// Carga el filesystem desde el archivo de backup.
static int
admin_load_from_backup_file(admin_filesystem_t *admin)
{
	FILE *backup_file = fopen(admin->fs_backup_file_path, "ab");
	if (!backup_file) {
		perror("Error al crear el archivo de backup del filesystem\n");
		return EXIT_FAILURE;
	}

	int file_error_code = fclose(backup_file);

	if (file_error_code) {
		perror("Error al abrir el archivo de backup del filesystem\n");
		return EXIT_FAILURE;
	}

	backup_file = fopen(admin->fs_backup_file_path, "rb");

	if (!backup_file) {
		perror("Error al abrir el archivo de backup del filesystem\n");
		return EXIT_FAILURE;
	}

	// Chequear que el archivo backup_file no esté vacío
	fseek(backup_file, 0, SEEK_END);
	long file_size = ftell(backup_file);
	if (file_size == 0) {
		fclose(backup_file);
		return EMPTY_BACKUP_FILE;
	}
	rewind(backup_file);

	admin->root_file = malloc(sizeof(file_t));
	if (!admin->root_file) {
		fclose(backup_file);
		perror("Error al crear el directorio raíz del filesystem\n");
		return EXIT_FAILURE;
	}

	file_t *root_file = admin->root_file;
	if (admin_load_file_metadata_from_backup_file(
	            admin, root_file, backup_file) == EXIT_FAILURE) {
		perror("Error al cargar un directorio del archivo de backup "
		       "del filesystem\n");
		return EXIT_FAILURE;
	}
	root_file->first_child = NULL;
	root_file->next_file = NULL;

	for (int i = 0; i < root_file->metadata.children_count; i++) {
		file_t *child_file = malloc(sizeof(file_t));
		if (child_file == NULL) {
			fclose(backup_file);
			perror("Error al crear un archivo/directorio a partir "
			       "del archivo de backup del filesystem\n");
			return EXIT_FAILURE;
		}
		child_file->first_child = NULL;
		child_file->next_file = NULL;

		if (admin_load_file_metadata_from_backup_file(
		            admin, child_file, backup_file) == EXIT_FAILURE) {
			perror("Error al cargar un archivo/directorio del "
			       "archivo de backup del filesystem\n");
			return EXIT_FAILURE;
		}

		if (!child_file->metadata.is_directory) {
			if (admin_load_file_data_from_backup_file(
			            admin, child_file, backup_file) ==
			    EXIT_FAILURE) {
				perror("Error al cargar los datos de un "
				       "archivo del archivo de backup del "
				       "filesystem\n");
				return EXIT_FAILURE;
			}
		} else {
			for (int j = 0; j < child_file->metadata.children_count;
			     j++) {
				file_t *grandchild_file = malloc(sizeof(file_t));
				if (grandchild_file == NULL) {
					fclose(backup_file);
					perror("Error al crear un archivo a "
					       "partir del archivo de backup "
					       "del filesystem\n");
					return EXIT_FAILURE;
				}
				grandchild_file->first_child = NULL;
				grandchild_file->next_file = NULL;

				if (admin_load_file_metadata_from_backup_file(
				            admin, grandchild_file, backup_file) ==
				    EXIT_FAILURE) {
					perror("Error al cargar un archivo del "
					       "archivo de backup del "
					       "filesystem\n");
					return EXIT_FAILURE;
				}

				if (admin_load_file_data_from_backup_file(
				            admin, grandchild_file, backup_file) ==
				    EXIT_FAILURE) {
					perror("Error al cargar los datos de "
					       "un archivo del archivo de "
					       "backup del filesystem\n");
					return EXIT_FAILURE;
				}

				if (child_file->first_child == NULL) {
					child_file->first_child = grandchild_file;
				} else {
					grandchild_file->next_file =
					        child_file->first_child;
					child_file->first_child = grandchild_file;
				}
			}
		}

		if (root_file->first_child == NULL) {
			root_file->first_child = child_file;
		} else {
			child_file->next_file = root_file->first_child;
			root_file->first_child = child_file;
		}
	}

	fclose(backup_file);
	return EXIT_SUCCESS;
}

// Guarda el filesystem en el archivo de backup.
static int
admin_save_to_backup_file(admin_filesystem_t *admin)
{
	FILE *backup_file = fopen(admin->fs_backup_file_path, "wb");

	if (backup_file == NULL) {
		perror("Error al abrir el archivo de backup del filesystem\n");
		return EXIT_FAILURE;
	}

	file_t *root_file = admin->root_file;
	if (admin_save_file_metadata_to_backup_file(admin, root_file, backup_file)) {
		perror("Error al guardar un directorio en el archivo de backup "
		       "del filesystem\n");
		return EXIT_FAILURE;
	}

	file_t *current_file = root_file->first_child;
	while (current_file) {
		admin_save_file_metadata_to_backup_file(admin,
		                                        current_file,
		                                        backup_file);
		if (!current_file->metadata.is_directory) {
			admin_save_file_data_to_backup_file(admin,
			                                    current_file,
			                                    backup_file);
		} else {
			file_t *current_child_file = current_file->first_child;
			while (current_child_file) {
				admin_save_file_metadata_to_backup_file(
				        admin, current_child_file, backup_file);
				admin_save_file_data_to_backup_file(
				        admin, current_child_file, backup_file);
				current_child_file = current_child_file->next_file;
			}
		}
		current_file = current_file->next_file;
	}

	fclose(backup_file);
	return EXIT_SUCCESS;
}

// Función auxiliar de admin_load_from_backup_file que carga los metadatos de un
// archivo/directorio desde el archivo de backup.
static int
admin_load_file_metadata_from_backup_file(admin_filesystem_t *admin,
                                          file_t *current_file,
                                          FILE *backup_file)
{
	packed_metadata_t metadata;
	if (fread(&metadata, sizeof(packed_metadata_t), 1, backup_file) != 1) {
		fclose(backup_file);
		return EXIT_FAILURE;
	}
	strncpy(current_file->metadata.name, metadata.name, MAX_FILE_NAME_SIZE);
	current_file->metadata.id = metadata.id;
	current_file->metadata.creation_timestamp = metadata.creation_timestamp;
	current_file->metadata.last_edit_timestamp = metadata.last_edit_timestamp;
	current_file->metadata.last_access_timestamp =
	        metadata.last_access_timestamp;
	current_file->metadata.size = metadata.size;
	current_file->metadata.is_directory = metadata.is_directory;
	current_file->metadata.children_count = metadata.children_count;
	current_file->metadata.mode = metadata.mode;
	return EXIT_SUCCESS;
}

// Función auxiliar de admin_load_from_backup_file que carga los datos de un
// archivo desde el archivo de backup.
static int
admin_load_file_data_from_backup_file(admin_filesystem_t *admin,
                                      file_t *current_file,
                                      FILE *backup_file)
{
	if (fread(current_file->data,
	          sizeof(char),
	          current_file->metadata.size,
	          backup_file) != current_file->metadata.size) {
		fclose(backup_file);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

// Función auxiliar de admin_save_to_backup_file que guarda los metadatos de un
// archivo/directorio en el archivo de backup.
static int
admin_save_file_metadata_to_backup_file(admin_filesystem_t *admin,
                                        file_t *current_file,
                                        FILE *backup_file)
{
	packed_metadata_t metadata;
	strncpy(metadata.name, current_file->metadata.name, MAX_FILE_NAME_SIZE);
	metadata.id = current_file->metadata.id;
	metadata.creation_timestamp = current_file->metadata.creation_timestamp;
	metadata.last_edit_timestamp = current_file->metadata.last_edit_timestamp;
	metadata.last_access_timestamp =
	        current_file->metadata.last_access_timestamp;
	metadata.size = current_file->metadata.size;
	metadata.is_directory = current_file->metadata.is_directory;
	metadata.children_count = current_file->metadata.children_count;
	metadata.mode = current_file->metadata.mode;

	if (fwrite(&metadata, sizeof(packed_metadata_t), 1, backup_file) != 1) {
		fclose(backup_file);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

// Función auxiliar de admin_save_to_backup_file que guarda los datos de un
// archivo en el archivo de backup.
static int
admin_save_file_data_to_backup_file(admin_filesystem_t *admin,
                                    file_t *current_file,
                                    FILE *backup_file)
{
	if (fwrite(current_file->data,
	           sizeof(char),
	           current_file->metadata.size,
	           backup_file) != current_file->metadata.size) {
		fclose(backup_file);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int
admin_init(admin_filesystem_t **admin, char *fs_backup_file_path)
{
	*admin = (admin_filesystem_t *) malloc(sizeof(admin_filesystem_t));
	if (!*admin) {
		return EXIT_FAILURE;
	}

	(*admin)->fs_backup_file_path =
	        fs_backup_file_path ? fs_backup_file_path : DEFAULT_BACKUP_PATH;

	int load_result = admin_load_from_backup_file(*admin);
	if (load_result == EXIT_FAILURE || load_result == EXIT_SUCCESS) {
		return load_result;
	}
	(*admin)->root_file = create_file(ROOT_PATH, NEW_DIR_MODE, true);

	return EXIT_SUCCESS;
}

int
admin_open(admin_filesystem_t *admin,
           const char *path,
           struct fuse_file_info *fi,
           bool opening_file_is_directory)
{
	file_t *file = admin_find_file(admin, path);

	if (!file) {
		mode_t mode;
		if (opening_file_is_directory)
			mode = NEW_DIR_MODE;
		else
			mode = NEW_FILE_MODE;
		if (fi->flags & O_CREAT &&
		    admin_create_file(admin, path, mode, opening_file_is_directory) ==
		            EXIT_SUCCESS) {
			file = admin_find_file(admin, path);
		} else {
			return EXIT_FAILURE;
		}
	} else if (file->metadata.is_directory != opening_file_is_directory) {
		return EXIT_FAILURE;
	} else if (fi->flags & O_TRUNC) {
		file->metadata.size = 0;
	}

	file->metadata.last_access_timestamp = time(NULL);

	fi->fh = (uint64_t) file;
	return EXIT_SUCCESS;
}


int
admin_read(admin_filesystem_t *admin,
           const char *path,
           char *buffer,
           size_t size,
           off_t offset,
           struct fuse_file_info *fi)
{
	file_t *file = (file_t *) fi->fh;

	if (!file) {
		file = admin_find_file(admin, path);

		if (!file)
			return -ENOENT;
	}


	if (offset >= file->metadata.size) {
		return 0;
	}

	size_t remaining_size = file->metadata.size - offset;
	size = size > remaining_size ? remaining_size : size;

	memcpy(buffer, file->data + offset, size);

	file->metadata.last_access_timestamp = time(NULL);

	return size;
}

int
admin_write(admin_filesystem_t *admin,
            const char *path,
            const char *buffer,
            size_t size,
            off_t offset,
            struct fuse_file_info *fi)
{
	file_t *file = (file_t *) fi->fh;

	if (!file) {
		file = admin_find_file(admin, path);

		if (!file)
			return -ENOENT;
	}

	if (offset + size > MAX_FILE_SIZE) {
		return -EFBIG;
	}

	memcpy(file->data + offset, buffer, size);

	file->metadata.last_access_timestamp = time(NULL);
	file->metadata.last_edit_timestamp = time(NULL);
	file->metadata.size = offset + size;

	return size;
}

int
admin_truncate(admin_filesystem_t *admin, const char *path, off_t size)
{
	file_t *file = admin_find_file(admin, path);

	if (!file) {
		return -ENOENT;
	}

	if (size > MAX_FILE_SIZE) {
		return -EFBIG;
	}

	if (size > file->metadata.size) {
		memset(file->data + file->metadata.size,
		       0,
		       size - file->metadata.size);
	}

	file->metadata.size = size;

	return 0;

	file->metadata.last_access_timestamp = time(NULL);
	file->metadata.last_edit_timestamp = time(NULL);
}


int
admin_create_file(admin_filesystem_t *admin,
                  const char *path,
                  mode_t mode,
                  bool is_directory)
{
	if (admin_find_file(admin, path)) {
		return EXIT_FAILURE;
	}

	char token1[MAX_FILE_NAME_SIZE];
	char token2[MAX_FILE_NAME_SIZE];

	parse_path(path, token1, token2);

	if (strlen(token1) == 0) {
		return EXIT_FAILURE;
	}

	char *file_name = token1;
	file_t *target_directory = admin->root_file;

	if (strlen(token2) > 0) {
		target_directory =
		        find_file_in_directory(admin->root_file, token1);
		if (!target_directory) {
			return EXIT_FAILURE;
		}
		file_name = token2;
	}

	create_file_in_dir(target_directory, file_name, mode, is_directory);

	return 0;
}


// Crea un nuevo archivo con el nombre file_name y el modo mode en el directorio
// directory.
void
create_file_in_dir(file_t *directory, char *file_name, mode_t mode, bool is_directory)
{
	file_t *new_file = create_file(file_name, mode, is_directory);
	file_t *dir_first_child = directory->first_child;

	if (directory->first_child) {
		new_file->next_file = dir_first_child;
	}

	directory->first_child = new_file;

	directory->metadata.children_count++;

	return;
}

// Crea un nuevo archivo/directorio con el nombre file_name y el modo mode.
// Devuelve un puntero al archivo creado si se creó correctamente, NULL en caso
// contrario.
file_t *
create_file(char *file_name, mode_t mode, bool is_directory)
{
	size_t file_name_lenght = strlen(file_name);
	if (file_name_lenght == 0 || file_name_lenght > MAX_FILE_NAME_SIZE) {
		return NULL;
	}
	file_t *new_file = malloc(sizeof(file_t));

	if (!new_file) {
		return NULL;
	}

	strncpy(new_file->metadata.name, file_name, MAX_FILE_NAME_SIZE - 1);
	new_file->metadata.id = 0;
	new_file->metadata.creation_timestamp = time(NULL);
	new_file->metadata.last_edit_timestamp = time(NULL);
	new_file->metadata.last_access_timestamp = time(NULL);
	new_file->metadata.size = 0;
	new_file->metadata.children_count = 0;
	new_file->metadata.mode = mode;
	new_file->metadata.is_directory = is_directory;

	new_file->first_child = NULL;
	new_file->next_file = NULL;

	return new_file;
}

int
admin_mkdir(admin_filesystem_t *admin, const char *path, mode_t mode)
{
	return admin_create_file(admin, path, mode, true);
}

int
admin_utimens(admin_filesystem_t *admin,
              const char *path,
              const struct timespec tv[2])
{
	file_t *file = admin_find_file(admin, path);
	if (!file) {
		return -ENOENT;  // File not found
	}

	file->metadata.last_access_timestamp = tv[0].tv_sec;
	file->metadata.last_edit_timestamp = tv[1].tv_sec;

	return 0;
}

int
admin_unlink(admin_filesystem_t *admin, const char *path, bool delete_file)
{
	char parent_path[MAX_FILE_NAME_SIZE];
	char file_name[MAX_FILE_NAME_SIZE];
	parse_path(path, parent_path, file_name);

	// Se intenta borrar root
	if (strlen(parent_path) == 0) {
		return -EPERM;
	}

	file_t *parent = admin_find_file(admin, parent_path);

	// Si no tiene padre no existe el archivo
	if (!parent) {
		return -ENOENT;
	}


	if (parent->metadata.is_directory) {
		if (strlen(file_name) == 0) {
			return -EISDIR;
		}

		file_t *file_to_delete =
		        find_file_in_directory(parent, file_name);

		if (!file_to_delete) {
			return -ENOENT;
		}

		if (file_to_delete->metadata.is_directory) {
			return -EISDIR;
		}

		if (parent->first_child == file_to_delete) {
			parent->first_child = file_to_delete->next_file;
		} else {
			file_t *current_file = parent->first_child;
			while (current_file->next_file != file_to_delete) {
				current_file = current_file->next_file;
			}
			current_file->next_file = file_to_delete->next_file;
		}

		parent->metadata.children_count--;

		if (delete_file) {
			free(file_to_delete);
		}

	} else {  // El archivo está en el root
		// caso archivo dentro de archivo no permitido
		if (strlen(file_name) > 0) {
			return -EPERM;
		}

		file_t *root = admin->root_file;

		if (root->first_child == parent) {
			root->first_child = parent->next_file;
		} else {
			file_t *current_file = root->first_child;
			while (current_file->next_file != parent) {
				current_file = current_file->next_file;
			}
			current_file->next_file = parent->next_file;
		}

		root->metadata.children_count--;

		if (delete_file) {
			free(parent);
		}
	}

	return 0;
}


int
admin_rmdir(admin_filesystem_t *admin, const char *path)
{
	file_t *parent_directory = admin->root_file;

	char directory_name[MAX_FILE_NAME_SIZE];
	char invalid_token[MAX_FILE_NAME_SIZE];
	parse_path(path, directory_name, invalid_token);

	if (strlen(invalid_token) > 0) {
		return -EPERM;
	}

	file_t *directory_to_delete =
	        find_file_in_directory(parent_directory, directory_name);
	if (!directory_to_delete) {
		return -ENOENT;
	}

	if (!directory_to_delete->metadata.is_directory) {
		return -ENOTDIR;
	}

	if (directory_to_delete->first_child) {
		return -ENOTEMPTY;
	}

	if (parent_directory->first_child == directory_to_delete) {
		parent_directory->first_child = directory_to_delete->next_file;
	} else {
		file_t *current_file = parent_directory->first_child;
		while (current_file->next_file != directory_to_delete) {
			current_file = current_file->next_file;
		}
		current_file->next_file = directory_to_delete->next_file;
	}

	parent_directory->metadata.children_count--;

	free(directory_to_delete);

	return 0;
}

int
admin_getattr(admin_filesystem_t *admin, const char *path, struct stat *stbuf)
{
	file_t *file = admin_find_file(admin, path);

	if (!file) {
		return -ENOENT;
	}

	return fill_stats_from_file(file, stbuf);
}

int
admin_stat(admin_filesystem_t *admin, const char *path, struct stat *stbuf)
{
	file_t *file = admin_find_file(admin, path);

	return fill_stats_from_file(file, stbuf);
}

// Completa el struct stat stbuf con los datos del archivo file.
int
fill_stats_from_file(file_t *file, struct stat *stbuf)
{
	if (!file) {
		return -ENOENT;
	}
	/*
	_dev_t         st_dev;		X
	_ino_t         st_ino;		X
	unsigned short st_mode;		X
	short          st_nlink; 	X
	short          st_uid;		X
	short          st_gid;		X
	_dev_t         st_rdev;		X
	_off_t         st_size;		X
	time_t         st_atime;	X
	time_t         st_mtime;	X
	time_t         st_ctime;	X
	*/

	memset(stbuf, 0, sizeof(struct stat));

	stbuf->st_nlink = 1;
	if (file->metadata.is_directory) {
		stbuf->st_nlink++;
		file_t *current_file = file->first_child;
		while (current_file) {
			if (current_file->metadata.is_directory) {
				stbuf->st_nlink++;
			}
			current_file = current_file->next_file;
		}
	}

	stbuf->st_dev = 0;
	stbuf->st_rdev = 0;
	stbuf->st_mode = file->metadata.mode;
	stbuf->st_size = file->metadata.size;
	stbuf->st_atime = file->metadata.last_access_timestamp;
	stbuf->st_mtime = file->metadata.last_edit_timestamp;
	stbuf->st_ctime = file->metadata.creation_timestamp;
	stbuf->st_blocks = (stbuf->st_size + 511) / 512;

	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_blksize = 4096;
	stbuf->st_ino = 3;

	return 0;
}

struct stat *
create_stats_from_file(file_t *file)
{
	struct stat *st = malloc(sizeof(struct stat));
	if (!st) {
		return NULL;
	}

	fill_stats_from_file(file, st);

	return st;
}

int
admin_readdir(admin_filesystem_t *admin,
              const char *path,
              void *buffer,
              fuse_fill_dir_t filler)
{
	file_t *dir = admin_find_file(admin, path);

	if (!dir || !dir->metadata.is_directory) {
		return -ENOENT;
	}

	filler(buffer, ".", NULL, 0);
	file_t *current_file = dir->first_child;
	while (current_file) {
		filler(buffer, current_file->metadata.name, NULL, 0);
		current_file->metadata.last_access_timestamp = time(NULL);
		current_file = current_file->next_file;
	}
	dir->metadata.last_access_timestamp = time(NULL);

	return 0;
}

int
admin_flush(admin_filesystem_t *admin)
{
	return admin_save_to_backup_file(admin);
}

void
admin_destroy(admin_filesystem_t *admin, bool save_to_backup)
{
	if (save_to_backup) {
		admin_save_to_backup_file(admin);
	}

	file_t *current_file = admin->root_file->first_child;
	while (current_file) {
		file_t *next_file = current_file->next_file;

		if (current_file->metadata.is_directory) {
			file_t *current_child = current_file->first_child;
			while (current_child) {
				file_t *next_child = current_child->next_file;
				free(current_child);
				current_child = next_child;
			}
		}

		free(current_file);
		current_file = next_file;
	}
	free(admin->root_file);
	free(admin);
}


// Busca un archivo en el filesystem a partir de su path. Devuelve un puntero al
// archivo si lo encuentra, NULL en caso contrario.
file_t *
admin_find_file(admin_filesystem_t *admin, const char *path)
{
	char token1[MAX_FILE_NAME_SIZE];
	char token2[MAX_FILE_NAME_SIZE];
	parse_path(path, token1, token2);

	if (strlen(token1) == 0) {
		return admin->root_file;
	}

	if (strlen(token2) == 0) {
		return find_file_in_directory(admin->root_file, token1);
	}

	file_t *directory = find_file_in_directory(admin->root_file, token1);

	if (!directory) {
		return NULL;
	}

	return find_file_in_directory(directory, token2);
}


// Busca un archivo en un directorio a partir de su nombre. Devuelve un puntero
// al archivo si lo encuentra, NULL en caso contrario.
file_t *
find_file_in_directory(file_t *directory, char *file_name)
{
	file_t *current_file = directory->first_child;

	while (current_file) {
		if (strcmp(current_file->metadata.name, file_name) == 0) {
			return current_file;
		}
		current_file = current_file->next_file;
	}

	return NULL;
}


// Parsea el path pasado y guarda las dos partes en token1 y token2.
static void
parse_path(const char *path, char *token1, char *token2)
{
	memset(token1, 0, MAX_FILE_NAME_SIZE);
	memset(token2, 0, MAX_FILE_NAME_SIZE);

	if (strcmp(path, ROOT_PATH) == 0) {
		return;
	}

	bool first_token_found = false;
	bool starts_with_delimiter = path[0] == DELIMITER;
	for (int i = 1 ? path[0] == DELIMITER : 0; i < strlen(path); i++) {
		int token_index = i - (starts_with_delimiter ? 1 : 0);
		if (path[i] == DELIMITER) {
			first_token_found = true;
		} else if (!first_token_found) {
			token1[token_index] = path[i];
		} else {
			token2[token_index - strlen(token1) - 1] = path[i];
		}
	}

	// La ruta termina con un "/"
	if (token2[strlen(token2) - 1] == DELIMITER) {
		token2[strlen(token2) - 1] = 0;
	}
}