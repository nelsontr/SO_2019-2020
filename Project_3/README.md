﻿# Projeto SO
>VERIFICAR SE FICHEIRO ESTA ABERTO PARA QUE

## Open
```
Lista para cada sessao de inodes (Max:5)
```
Erros:
+ Verificar se existe o ficheiro (lockup)
+ Verificar se há posições na qual conseguimos escrever
+ ...

## Write
```
Escrever no inode o conteudo do buffer
```
Erros:
+ Verificar se existe o ficheiro (lockup)
+ Verificar se o ficheiro tá aberto na tabela ou se está na Tabela
+ ...

## Read
```
Ler no inode o conteudo do buffer
```
Erros:
+ Verificar se existe o ficheiro (lockup)
+ Verificar se o ficheiro tá aberto na tabela ou se está na Tabela
+ ...

## Close
```
Retirar elemento da lista
```
Erros:
+ Verificar se existe o ficheiro (lockup)
+ Verificar se o ficheiro tá aberto na tabela ou se está na Tabela
+ ...


# Perguntas
+ No servidor utilizar mutexes ou rwlocks?
+ 


----
# O que falta
+ Meter locks em tudo o que falta

+ Ver qual o free que me falta


+ pthread_signal

+ Ver multiplos clientes, clientes iguais da erro de sessao repetida...

