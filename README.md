# http-server-client

Este repositório contém os arquivos fonte do trabalho prático de Redes de Computadores do curso de Ciência da Computação da Universidade Federal de São João del-Rei.

O objetivo do trabalho é implementar um cliente e um servidor HTTP.

## Funcionalidades

### Cliente

* Fazer requisições `GET` e salvar a resposta localmente;
* Detecção minuciosa de erros com log para `stderr`;
* Permitir o usuário escolher onde salvar a resposta (via a opção `-o`);
* Suporte a redirecionamento (códigos HTTP 3xx);
* Suporte a `Transfer-Encoding: chunked`.

### Servidor

* Servir arquivos de uma pasta local via HTTP/1.1;
* Detecção minuciosa de erros com log para `stderr`;
* Multi-threading para servir múltiplos clientes simultaneamente;
* Detecção do tipo de mídia (*MIME Type*) para enviar no `Content-Type`;
* Proteção simples contra ataques do tipo *path traversal*; 
* Listagem de diretórios para pastas sem `index.html`;
* Se o cliente pedir uma pasta sem `/` no final, redirecionar para o endereço correto.

## Estrutura dos diretórios

```
http-server-client/
> bin/ *
    > http-client   Executável do cliente
    > http-server   Executável do servidor
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