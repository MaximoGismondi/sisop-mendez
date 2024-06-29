#ifndef ADMIN_FILESYSTEM_H
#define ADMIN_FILESYSTEM_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_FILE_NAME_SIZE 50
#define MAX_FILE_SIZE 1024
#define MAX_PATH_SIZE 1024

// Máximo nivel de recursión permitido para la creación de directorios y archivos
#define MAX_RECURSION_LEVEL 16

#define ROOT_PATH "/"
#define DELIMITER '/'

#define DEFAULT_BACKUP_PATH "./filesystem.fisopfs"

#define CURRENT_DIR "."
#define PARENT_DIR ".."

#define DIR_MODE (__S_IFDIR | 0755)
#define FILE_MODE (__S_IFREG | 0644)
#define SOFT_LINK_MODE (__S_IFLNK | 0777)

typedef struct {
	char name[MAX_FILE_NAME_SIZE];
	size_t id;
	time_t last_change_timestamp;
	time_t last_edit_timestamp;
	time_t last_access_timestamp;
	size_t size;
	size_t children_count;
	mode_t mode;
} metadata_t;

typedef struct file_t file_t;

typedef struct file_t {
	metadata_t metadata;
	file_t *parent;
	file_t *first_child;
	file_t *next_file;
	char data[MAX_FILE_SIZE];
} file_t;

typedef struct {
	char *fs_backup_file_path;
	file_t *root_file;
} admin_filesystem_t;


// Constructor del filesystem. Devuelve EXIT_SUCCESS si se inicializó
// correctamente, EXIT_FAILURE en caso contrario.
int admin_init(admin_filesystem_t *admin, char *fs_backup_file_path);

// Lee un archivo del filesystem, especificado por su path y por un puntero en
// fi. Se leen `size` bytes desde la posición offset en el archivo. Devuelve
// EXIT_SUCCESS si se leyó correctamente, código de error en caso contrario.
int admin_read(admin_filesystem_t *admin,
               const char *path,
               char *buffer,
               size_t size,
               off_t offset,
               struct fuse_file_info *fi);

// Escribe en un archivo del filesystem, especificado por su path y por un
// puntero en fi. Se escriben `size` bytes desde la posición offset en el
// archivo. Devuelve EXIT_SUCCESS si se escribió correctamente, código de error
// en caso contrario.
int admin_write(admin_filesystem_t *admin,
                const char *path,
                const char *buffer,
                size_t size,
                off_t offset,
                struct fuse_file_info *fi);

// Trunca un archivo del filesystem, especificado por su path. Se trunca el
// archivo a `size` bytes. Devuelve EXIT_SUCCESS si se truncó correctamente,
// código de error en caso contrario.
int admin_truncate(admin_filesystem_t *admin, const char *path, off_t size);

// Abre un archivo del filesystem, especificado por su path. Devuelve
// EXIT_SUCCESS si se abrió correctamente, código de error en caso contrario.
int admin_open(admin_filesystem_t *admin,
               const char *path,
               struct fuse_file_info *fi);

// Crea un archivo/directorio nuevo en el filesystem, especificado por su path.
// Devuelve EXIT_SUCCESS si se creó correctamente, código de error en caso
// contrario.
int admin_create_file(admin_filesystem_t *admin, const char *path, mode_t mode);

// Cambia la fecha de acceso y modificación de un archivo del filesystem por las
// especificadas en ts. Devuelve EXIT_SUCCESS si se cambió correctamente,
// código de error en caso contrario.
int admin_utimens(admin_filesystem_t *admin,
                  const char *path,
                  const struct timespec ts[2]);

// Elimina un archivo/directorio del filesystem, especificado por su path.
// Devuelve EXIT_SUCCESS si se eliminó correctamente, código de error en caso contrario.
int admin_delete_file(admin_filesystem_t *admin,
                      const char *path,
                      bool is_directory);

// Obtiene los atributos de un archivo del filesystem, especificado por su path,
// de forma similar a stat, y los coloca en el buffer `stbuf`. Devuelve
// EXIT_SUCCESS si se obtuvieron correctamente, código de error en caso
// contrario.
int admin_getattr(admin_filesystem_t *admin, const char *path, struct stat *stbuf);

// Obtiene los atributos de un archivo del filesystem, especificado por su path.
// Devuelve EXIT_SUCCESS si se obtuvieron correctamente, código de error en caso
// contrario.
int admin_stat(admin_filesystem_t *admin, const char *path, struct stat *stbuf);

// Lee el contenido de un directorio del filesystem, especificado por su path.
// Devuelve EXIT_SUCCESS si se leyó correctamente, código de error en caso
// contrario.
int admin_readdir(admin_filesystem_t *admin,
                  const char *path,
                  void *buffer,
                  fuse_fill_dir_t filler);

// Lee el contenido de un enlace simbólico del filesystem, especificado por su
// path. Devuelve EXIT_SUCCESS si se leyó correctamente, código de error en caso
// contrario.
int admin_readlink(admin_filesystem_t *admin,
                   const char *path,
                   char *buffer,
                   size_t size);

// Crea un enlace simbólico en el filesystem al archivo/directiorio especificado
// por el path `target`. El enlace simbólico se crea en el path `link_path`.
// Devuelve EXIT_SUCCESS si se creó correctamente, código de error en caso
// contrario.
int admin_symlink(admin_filesystem_t *admin,
                  const char *target,
                  const char *link_path);

// Guarda todo el contenido del filesystem en el archivo de backup. Devuelve
// EXIT_SUCCESS si se guardó correctamente, código de error en caso contrario.
int admin_flush(admin_filesystem_t *admin);

// Destruye el filesystem, liberando toda la memoria utilizada.
void admin_destroy(admin_filesystem_t *admin);


#endif  // ADMIN_FILESYSTEM_H