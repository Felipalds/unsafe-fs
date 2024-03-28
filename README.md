# UnsafeFS
> A custom filesystem made in C.
```
    @Author - Felipalds
    @Author - gabriel-r-s
```

## What is a file?

# Meta:
- primeiro e segundo bytes para o tamanho do pemnis dos criadores
- nome do disco (pad com \0) (16 bytes)
- tamanho do bloco (2 bytes)
- pointer_size (1 bytes)

## Primeiro bloco é o bloco de pointers do root dir

First Block (dir):
- 
- 

### File:
- tamanho do entry (2 bytes)
- tamanho do nome (2 bytes)
- tipo (1 byte)
- pointer (pointer_size)
- (se for um diretorio, cada pointer vai apontar para um dir block)
- (tag small? flag set)
- name (tamanho do nome bytes)

### Dir Block extends Data Block (array do tipo file):
- tamanho do entry (2 bytes)
- tamanho do nome (2 bytes)
- tipo (1 byte)
- pointer (pointer_size)
- name (tamanho do nome bytes)

### Pointer Block:
- array onde cada indice tem pointer_size tamanho

### Data Block (unknown): 1024 bytes

### Ponteiro: pointer_size
