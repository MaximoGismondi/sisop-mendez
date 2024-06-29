# fisopfs

Para compilar el filesystem, ejecutar:

```
make
```

Para eliminar los archivos compilados, ejecutar:

```
make clean
```

Para montar y correr el filesystem, se debe ejecutar: 

```
./fisopfs [flags opcionales de FUSE] <directorio de mount point> <archivo de backup del filesystem>
```

Para desmontar el filesystem, ejecutar:

```
sudo umount <directorio de mount point>
```

Para correr los tests de filesystem, ejecutar:

```
make test
```
(Se monta y desmonta el filesystem automáticamente)
