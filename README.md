# http-server-client

Este repositório contém os arquivos fonte do trabalho prático de Redes de Computadores do curso de Ciência da Computação da Universidade Federal de São João del-Rei.

O objetivo do trabalho é implementar um cliente e um servidor HTTP.

## Estrutura dos diretórios

```
http-server-client/
> bin/ *
    > http-client   Executável do cliente
    > http-server   Executável do cliente
  client/
    > *.c; *.h      Arquivos fonte do cliente
> obj/ *
    > http-client/
      > *.o         Arquivos objeto do cliente
    > http-server/
      > *.o         Arquivos objeto do servidor
> server/
    > *.c; *.h      Arquivos fonte do servidor
```
\* Pastas criadas no processo de compilação.

## Como compilar e executar

### Compilar

**Cliente:**
`make bin/http-client`

**Servidor:**
`make bin/http-server`

`make` compila ambos cliente e servidor.

### Executar

**Cliente:**
```
bin/http-client [opções] <url>
  <url> é uma URL no formato [http://]host[:porta][/caminho/para/o/arquivo]
  Opções:
    -r <redirs> Configura o número máximo de redirecionamentos (padrão: 10)
    -o <file>   Configura o arquivo de saída; se <file> for 'stdout', usar a saída padrão
```

**Servidor:**
```
bin/http-server [opções] <dir>
  <dir> é o diretório local a ser compartilhado via HTTP
  Opções:
    -d          Desativa a listagem de diretórios (o servidor responde com '403 Forbidden')
    -p          Configura o número da porta do servidor (padrão: 8080)
```

## Créditos

**Discente:** João Felipe Silva Pereira

**Docente:** Flávio Luiz Schiavoni