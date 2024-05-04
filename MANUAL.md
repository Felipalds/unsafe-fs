# Manual de uso para unsafe-fs

Gabriel Rodrigues dos Santos <br>
Luiz Felipe Fonseca Rosa

### Formatador 

        $ ./main --create /path/to/image DISK_NAME BLOCK_SIZE DISK_SIZE

        /path/to/image: caminho do arquivo onde armazenar a nova imagem. Não deve sobreescrever nenhum arquivo existente.
        DISK_NAME: nome a ser designado ao disco. Se maior que 16 bytes, será truncado.
        BLOCK_SIZE: determina o tamanho do blocos no disco. Deve ser suficiente para conter um MetaBlock.
        DISK_SIZE: determina o tamanho do disco em blocos. Deve ser ao menos 2.

### Importar arquivo para imagem

        $ ./main --import /path/to/image /path/to/file FILE_NAME

        /path/to/image: caminho do arquivo contendo a imagem.
        /path/to/file: caminho do arquivo a ser importado.
        FILE_NAME: nome do arquivo que será criado na imagem.

### Exportar arquivo da imagem

        $ ./main --export /path/to/image /path/to/file FILE_NAME

        /path/to/image: caminho do arquivo contendo a imagem.
        /path/to/file: caminho do arquivo a ser criado. Não deve sobreescrever nenhum arquivo existente.
        FILE_NAME: nome do arquivo na imagem a ser exportado.

### Remover arquivo da imagem

        $ ./main --delete /path/to/image FILE_NAME

        /path/to/image: caminho do arquivo contendo a imagem.
        FILE_NAME: nome do arquivo na imagem a ser deletado.
    
### Listar arquivos da imagem

        $ ./main --list /path/to/image

        /path/to/image: caminho do arquivo contendo a imagem.

