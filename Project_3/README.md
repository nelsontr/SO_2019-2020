# Projeto SO
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
+ Colocar Mutexes/Rwlocks (?) Onde são?
+ No servidor utilizar mutexes ou rwlocks?
+ Como funciona o inodes, é preciso colocar na estrutura ou ele já faz uma tabela no inodes.c?
+ 