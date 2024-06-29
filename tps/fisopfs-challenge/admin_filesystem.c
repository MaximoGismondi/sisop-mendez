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
	time_t last_change_timestamp;
	time_t last_edit_timestamp;
	time_t last_access_timestamp;
	size_t size;
	size_t children_count;
	mode_t mode;
} packed_metadata_t;

// Flag devuelta por admin_load_from_file cuando el archivo pasado para guardar
// el filesystem no existe o está vacío.
#define EMPTY_BACKUP_FILE 2


file_t *find_file(file_t *current_file,
                  const char *path,
                  bool find_parent,
                  int *recursion_level);

file_t *find_file_in_directory(file_t *directory, char *file_name);

static int load_file_metadata_from_backup(file_t *current_file, FILE *backup_file);

static int load_file_data_from_backup(file_t *current_file, FILE *backup_file);

static int save_file_metadata_to_backup(file_t *current_file, FILE *backup_file);

static int save_file_data_to_backup(file_t *current_file, FILE *backup_file);

static void get_file_name(const char *path, char *file_name);

static void delete_file_from_directory(file_t *directory, file_t *file_to_delete);

file_t *create_file(char *file_name, mode_t mode);

file_t *create_file_in_dir(file_t *directory, char *file_name, mode_t mode);

int fill_stats_from_file(file_t *file, struct stat *stbuf);

void destroy_file(file_t *file);

static void parse_path(const char *path, char *token1, char *token2);

// Función auxiliar de admin_load_from_backup que carga los metadatos de un
// archivo/directorio desde el archivo de backup.
static int
load_file_metadata_from_backup(file_t *current_file, FILE *backup_file)
{
	packed_metadata_t metadata;
	if (fread(&metadata, sizeof(packed_metadata_t), 1, backup_file) != 1) {
		fclose(backup_file);
		perror("Error al cargar los metadatos de un archivo/directorio "
		       "del "
		       "archivo de backup del filesystem\n");
		return EXIT_FAILURE;
	}
	strncpy(current_file->metadata.name, metadata.name, MAX_FILE_NAME_SIZE);
	current_file->metadata.id = metadata.id;
	current_file->metadata.last_change_timestamp =
	        metadata.last_change_timestamp;
	current_file->metadata.last_edit_timestamp = metadata.last_edit_timestamp;
	current_file->metadata.last_access_timestamp =
	        metadata.last_access_timestamp;
	current_file->metadata.size = metadata.size;
	current_file->metadata.children_count = metadata.children_count;
	current_file->metadata.mode = metadata.mode;
	return EXIT_SUCCESS;
}

// Función auxiliar de admin_load_from_backup que carga los datos de un
// archivo desde el archivo de backup.
static int
load_file_data_from_backup(file_t *current_file, FILE *backup_file)
{
	if (fread(current_file->data,
	          sizeof(char),
	          current_file->metadata.size,
	          backup_file) != current_file->metadata.size) {
		fclose(backup_file);
		perror("Error al cargar los datos de un archivo del "
		       "archivo de backup del filesystem\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static file_t *
load_file_from_backup(FILE *backup_file)
{
	file_t *file = malloc(sizeof(file_t));
	if (!file) {
		fclose(backup_file);
		perror("Error al crear un archivo/directorio a partir "
		       "del archivo de backup del filesystem\n");
		return NULL;
	}
	file->parent = NULL;
	file->first_child = NULL;
	file->next_file = NULL;

	if (load_file_metadata_from_backup(file, backup_file) == EXIT_FAILURE) {
		return NULL;
	}

	if (file->metadata.mode != DIR_MODE) {
		if (load_file_data_from_backup(file, backup_file) == EXIT_FAILURE) {
			return NULL;
		}

		return file;
	}

	for (int i = 0; i < file->metadata.children_count; i++) {
		file_t *child_file = load_file_from_backup(backup_file);
		if (!child_file) {
			return NULL;
		}

		child_file->parent = file;

		if (!file->first_child) {
			file->first_child = child_file;
		} else {
			child_file->next_file = file->first_child;
			file->first_child = child_file;
		}
	}

	return file;
}

// Carga el filesystem desde el archivo de backup.
static int
admin_load_from_backup(admin_filesystem_t *admin)
{
	FILE *backup_file = fopen(admin->fs_backup_file_path, "ab");
	if (!backup_file) {
		perror("Error al crear el archivo de backup del filesystem\n");
		return EXIT_FAILURE;
	}

	if (fclose(backup_file)) {
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

	admin->root_file = load_file_from_backup(backup_file);
	if (!admin->root_file) {
		return EXIT_FAILURE;
	}

	fclose(backup_file);
	return EXIT_SUCCESS;
}

// Función auxiliar de admin_save_to_backup que guarda los metadatos de un
// archivo/directorio en el archivo de backup.
static int
save_file_metadata_to_backup(file_t *current_file, FILE *backup_file)
{
	packed_metadata_t metadata;
	strncpy(metadata.name, current_file->metadata.name, MAX_FILE_NAME_SIZE);
	metadata.id = current_file->metadata.id;
	metadata.last_change_timestamp =
	        current_file->metadata.last_change_timestamp;
	metadata.last_edit_timestamp = current_file->metadata.last_edit_timestamp;
	metadata.last_access_timestamp =
	        current_file->metadata.last_access_timestamp;
	metadata.size = current_file->metadata.size;
	metadata.children_count = current_file->metadata.children_count;
	metadata.mode = current_file->metadata.mode;

	if (fwrite(&metadata, sizeof(packed_metadata_t), 1, backup_file) != 1) {
		fclose(backup_file);
		perror("Error al guardar la metadata de un archivo/directorio "
		       "en el archivo de backup "
		       "del filesystem\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


// Función auxiliar de admin_save_to_backup que guarda los datos de un
// archivo en el archivo de backup.
static int
save_file_data_to_backup(file_t *current_file, FILE *backup_file)
{
	if (fwrite(current_file->data,
	           sizeof(char),
	           current_file->metadata.size,
	           backup_file) != current_file->metadata.size) {
		fclose(backup_file);
		perror("Error al guardar los datos de un archivo en el archivo "
		       "de backup "
		       "del filesystem\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int
save_file_to_backup(file_t *file, FILE *backup_file)
{
	if (save_file_metadata_to_backup(file, backup_file) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	if (file->metadata.mode != DIR_MODE) {
		if (save_file_data_to_backup(file, backup_file) == EXIT_FAILURE) {
			return EXIT_FAILURE;
		}
	}

	file_t *current_file = file->first_child;
	while (current_file) {
		if (save_file_to_backup(current_file, backup_file) ==
		    EXIT_FAILURE) {
			return EXIT_FAILURE;
		}
		current_file = current_file->next_file;
	}

	return EXIT_SUCCESS;
}

// Guarda el filesystem en el archivo de backup.
static int
admin_save_to_backup(admin_filesystem_t *admin)
{
	FILE *backup_file = fopen(admin->fs_backup_file_path, "wb");

	if (!backup_file) {
		perror("Error al abrir el archivo de backup del filesystem\n");
		return -EXIT_FAILURE;
	}

	if (save_file_to_backup(admin->root_file, backup_file) == EXIT_FAILURE) {
		return -EXIT_FAILURE;
	}

	fclose(backup_file);
	return EXIT_SUCCESS;
}

int
admin_init(admin_filesystem_t *admin, char *fs_backup_file_path)
{
	admin->fs_backup_file_path =
	        fs_backup_file_path ? fs_backup_file_path : DEFAULT_BACKUP_PATH;

	int load_result = admin_load_from_backup(admin);
	if (load_result != EMPTY_BACKUP_FILE) {
		return load_result;
	}
	admin->root_file = create_file(ROOT_PATH, DIR_MODE);

	return EXIT_SUCCESS;
}

int
admin_open(admin_filesystem_t *admin, const char *path, struct fuse_file_info *fi)
{
	int recursion_level = 0;
	file_t *file = find_file(admin->root_file, path, false, &recursion_level);

	if (!file) {
		if (!(fi->flags & O_CREAT)) {
			return -EXIT_FAILURE;
		}

		int error = admin_create_file(admin, path, FILE_MODE);
		if (error != EXIT_SUCCESS) {
			return error;
		}

		recursion_level = 0;
		file = find_file(admin->root_file, path, false, &recursion_level);
	} else if (file->metadata.mode == DIR_MODE) {
		return -EISDIR;
	} else if (fi->flags & O_TRUNC) {
		file->metadata.size = 0;
	}

	file->metadata.last_access_timestamp = time(NULL);

	while (file->metadata.mode == SOFT_LINK_MODE) {
		recursion_level = 0;
		file_t *parent_directory =
		        find_file(admin->root_file, path, true, &recursion_level);

		file = find_file(
		        parent_directory, file->data, false, &recursion_level);

		if (!file) {
			return -ENOENT;
		}
	}

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
	int recursion_level = 0;
	file_t *file = find_file(admin->root_file, path, false, &recursion_level);

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
admin_create_file(admin_filesystem_t *admin, const char *path, mode_t mode)
{
	int recursion_level = 0;
	file_t *target_directory =
	        find_file(admin->root_file, path, true, &recursion_level);

	if (!target_directory) {
		return -EXIT_FAILURE;
	}

	// Se quiere crear un directorio en un nivel de recursión mayor o igual que
	// el máximo permitido. No tiene sentido crear un directorio en el nivel
	// máximo permitido, ya que no se podrían crear archivos dentro de él.
	if (mode == DIR_MODE && recursion_level > MAX_RECURSION_LEVEL - 2) {
		return -EPERM;
	}

	// Se quiere crear un archivo en un nivel de recursión mayor que el
	// máximo permitido.
	if (mode == FILE_MODE && recursion_level > MAX_RECURSION_LEVEL - 1) {
		return -EPERM;
	}

	char file_name[MAX_FILE_NAME_SIZE];
	get_file_name(path, file_name);

	if (find_file_in_directory(target_directory, file_name)) {
		return -EXIT_FAILURE;
	}

	create_file_in_dir(target_directory, file_name, mode);

	return EXIT_SUCCESS;
}


// Crea un nuevo archivo con el nombre file_name y el modo mode en el directorio
// directory.
file_t *
create_file_in_dir(file_t *directory, char *file_name, mode_t mode)
{
	file_t *new_file = create_file(file_name, mode);

	if (!new_file) {
		return NULL;
	}

	new_file->parent = directory;
	new_file->next_file = directory->first_child;
	directory->first_child = new_file;
	directory->metadata.children_count++;
	memset(new_file->data, 0, MAX_FILE_SIZE);

	return new_file;
}

// Crea un nuevo archivo/directorio con el nombre file_name y el modo mode.
// Devuelve un puntero al archivo creado si se creó correctamente, NULL en caso
// contrario.
file_t *
create_file(char *file_name, mode_t mode)
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
	new_file->metadata.last_change_timestamp = time(NULL);
	new_file->metadata.last_edit_timestamp = time(NULL);
	new_file->metadata.last_access_timestamp = time(NULL);
	new_file->metadata.size = 0;
	new_file->metadata.children_count = 0;
	new_file->metadata.mode = mode;

	new_file->first_child = NULL;
	new_file->next_file = NULL;

	return new_file;
}

int
admin_utimens(admin_filesystem_t *admin,
              const char *path,
              const struct timespec tv[2])
{
	int recursion_level = 0;
	file_t *file = find_file(admin->root_file, path, false, &recursion_level);
	if (!file) {
		return -ENOENT;
	}

	file->metadata.last_access_timestamp = tv[0].tv_sec;
	file->metadata.last_edit_timestamp = tv[1].tv_sec;

	return EXIT_SUCCESS;
}

int
admin_delete_file(admin_filesystem_t *admin, const char *path, bool is_directory)
{
	if (strcmp(path, ROOT_PATH) == 0) {
		return -EPERM;
	}

	int recursion_level = 0;
	file_t *file_to_delete =
	        find_file(admin->root_file, path, false, &recursion_level);

	if (!file_to_delete) {
		return -ENOENT;
	}

	if (is_directory) {
		if (file_to_delete->metadata.mode != DIR_MODE) {
			return -ENOTDIR;
		}

		if (file_to_delete->first_child) {
			return -ENOTEMPTY;
		}
	} else {
		if (file_to_delete->metadata.mode == DIR_MODE) {
			return -EISDIR;
		}
	}

	recursion_level = 0;
	file_t *parent =
	        find_file(admin->root_file, path, true, &recursion_level);
	delete_file_from_directory(parent, file_to_delete);
	return EXIT_SUCCESS;
}

static void
delete_file_from_directory(file_t *directory, file_t *file_to_delete)
{
	if (directory->first_child == file_to_delete) {
		directory->first_child = file_to_delete->next_file;
	} else {
		file_t *current_file = directory->first_child;
		while (current_file->next_file != file_to_delete) {
			current_file = current_file->next_file;
		}
		current_file->next_file = file_to_delete->next_file;
	}
	directory->metadata.children_count--;
	free(file_to_delete);
}

int
admin_getattr(admin_filesystem_t *admin, const char *path, struct stat *stbuf)
{
	int recursion_level = 0;
	file_t *file = find_file(admin->root_file, path, false, &recursion_level);

	if (!file) {
		return -ENOENT;
	}

	return fill_stats_from_file(file, stbuf);
}

int
admin_stat(admin_filesystem_t *admin, const char *path, struct stat *stbuf)
{
	int recursion_level = 0;
	file_t *file = find_file(admin->root_file, path, false, &recursion_level);

	return fill_stats_from_file(file, stbuf);
}

// Completa el struct stat stbuf con los datos del archivo file.
int
fill_stats_from_file(file_t *file, struct stat *stbuf)
{
	if (!file) {
		return -ENOENT;
	}

	memset(stbuf, 0, sizeof(struct stat));

	stbuf->st_nlink = 1;
	if (file->metadata.mode == DIR_MODE) {
		stbuf->st_nlink++;
		file_t *current_file = file->first_child;
		while (current_file) {
			if (current_file->metadata.mode == DIR_MODE) {
				stbuf->st_nlink++;
			}
			current_file = current_file->next_file;
		}
	}

	stbuf->st_dev = 0;
	stbuf->st_rdev = 0;
	stbuf->st_mode = file->metadata.mode;
	stbuf->st_size = file->metadata.size;
	stbuf->st_ctime = file->metadata.last_change_timestamp;
	stbuf->st_atime = file->metadata.last_access_timestamp;
	stbuf->st_mtime = file->metadata.last_edit_timestamp;
	stbuf->st_blocks = (stbuf->st_size + 511) / 512;

	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_blksize = 4096;
	stbuf->st_ino = 3;

	return EXIT_SUCCESS;
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
	int recursion_level = 0;
	file_t *dir = find_file(admin->root_file, path, false, &recursion_level);

	if (!dir) {
		return -ENOENT;
	}

	if (dir->metadata.mode != DIR_MODE) {
		return -ENOTDIR;
	}

	filler(buffer, CURRENT_DIR, NULL, 0);
	filler(buffer, PARENT_DIR, NULL, 0);
	file_t *current_file = dir->first_child;
	while (current_file) {
		filler(buffer, current_file->metadata.name, NULL, 0);
		current_file->metadata.last_access_timestamp = time(NULL);
		current_file = current_file->next_file;
	}
	dir->metadata.last_access_timestamp = time(NULL);

	return EXIT_SUCCESS;
}

int
admin_flush(admin_filesystem_t *admin)
{
	return admin_save_to_backup(admin);
}

void
admin_destroy(admin_filesystem_t *admin)
{
	if (admin_save_to_backup(admin) != EXIT_SUCCESS) {
		perror("Error al guardar el filesystem en el archivo de "
		       "backup\n");
	}

	destroy_file(admin->root_file);
}

void
destroy_file(file_t *file)
{
	if (file->metadata.mode == DIR_MODE) {
		file_t *current_child = file->first_child;
		while (current_child) {
			file_t *next_child = current_child->next_file;
			destroy_file(current_child);
			current_child = next_child;
		}
	}
	free(file);
}

// Busca un archivo desde un archivo raíz (current_file) a partir de un path.
// Si find_parent es true, devuelve el directorio padre del archivo buscado.
// Devuelve un archivo si lo encuentra, NULL en caso contrario.
// La función establece el valor de recursión del archivo/directorio encontrado
// en recursion_level. Si no fue encontrado, establece recursion_level en -1.
file_t *
find_file(file_t *current_file, const char *path, bool find_parent, int *recursion_level)
{
	char token1[MAX_FILE_NAME_SIZE];
	char token2[MAX_PATH_SIZE];
	parse_path(path, token1, token2);

	if (strlen(token1) == 0 && find_parent) {
		*recursion_level = -1;
		return NULL;
	}

	if (strlen(token1) == 0 || (find_parent && strlen(token2) == 0)) {
		return current_file;
	}

	if (current_file->metadata.mode != DIR_MODE) {
		*recursion_level = -1;
		return NULL;
	}

	file_t *file = find_file_in_directory(current_file, token1);

	if (!file) {
		*recursion_level = -1;
		return NULL;
	}

	(*recursion_level)++;
	return find_file(file, token2, find_parent, recursion_level);
}


// Busca un archivo en un directorio a partir de su nombre. Devuelve un puntero
// al archivo si lo encuentra, NULL en caso contrario.
file_t *
find_file_in_directory(file_t *directory, char *file_name)
{
	if (strcmp(file_name, CURRENT_DIR) == 0) {
		return directory;
	} else if (strcmp(file_name, PARENT_DIR) == 0) {
		return directory->parent;
	}

	file_t *current_file = directory->first_child;

	while (current_file) {
		if (strcmp(current_file->metadata.name, file_name) == 0) {
			return current_file;
		}
		current_file = current_file->next_file;
	}

	return NULL;
}

int
admin_symlink(admin_filesystem_t *admin, const char *target, const char *link_path)
{
	int recursion_level = 0;
	file_t *target_directory =
	        find_file(admin->root_file, link_path, true, &recursion_level);

	if (!target_directory) {
		return -ENOENT;
	}

	// Se intenta crear un soft link en un nivel de recursión mayor al
	// máximo permitido.
	if (recursion_level > MAX_RECURSION_LEVEL - 1) {
		return -EPERM;
	}

	char file_name[MAX_FILE_NAME_SIZE];
	get_file_name(link_path, file_name);

	if (find_file_in_directory(target_directory, file_name)) {
		return -EEXIST;
	}

	file_t *new_file =
	        create_file_in_dir(target_directory, file_name, SOFT_LINK_MODE);

	if (!new_file) {
		return -EXIT_FAILURE;
	}

	new_file->metadata.size = strlen(target);
	strncpy(new_file->data, target, new_file->metadata.size);

	return 0;
}

int
admin_readlink(admin_filesystem_t *admin, const char *path, char *buffer, size_t size)
{
	int recursion_level = 0;
	file_t *file = find_file(admin->root_file, path, false, &recursion_level);

	if (!file) {
		return -ENOENT;
	}

	if (file->metadata.mode != SOFT_LINK_MODE) {
		return -EINVAL;
	}

	if (size < file->metadata.size) {
		return -EINVAL;
	}

	strncpy(buffer, file->data, file->metadata.size);

	return 0;
}

// Parsea el path pasado y guarda las dos partes en token1 y token2.
static void
parse_path(const char *path, char *token1, char *token2)
{
	memset(token1, 0, MAX_FILE_NAME_SIZE);
	memset(token2, 0, MAX_PATH_SIZE);

	if (strcmp(path, ROOT_PATH) == 0) {
		return;
	}

	int start_position = path[0] == DELIMITER ? 1 : 0;
	const char *delimiter_position = strchr(path + start_position, DELIMITER);

	if (delimiter_position) {
		strncpy(token1,
		        path + start_position,
		        delimiter_position - (path + start_position));
		strncpy(token2, delimiter_position + 1, MAX_PATH_SIZE);
	} else {
		strncpy(token1, path + start_position, MAX_FILE_NAME_SIZE);
	}
}

// Dado un path, obtiene el nombre del archivo/directorio y lo guarda en
// file_name.
static void
get_file_name(const char *path, char *file_name)
{
	char *last_delimiter = strrchr(path, DELIMITER);
	if (last_delimiter) {
		strncpy(file_name, last_delimiter + 1, MAX_FILE_NAME_SIZE);
	} else {
		strncpy(file_name, path, MAX_FILE_NAME_SIZE);
	}
}