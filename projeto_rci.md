# Projeto RCI

Diretrizes de projeto.

### Cliente: rmb 

```
rmb [-i ip do servidor de identidades] [-p porta do servidor de
identidades]
```

default:

siip -> tejo.tecnico.ulisboa.pt

sipt -> 59000

#### Interface para o utilizador:
----------------------------

`show_servers` pede msgservers registados no idserver


Usa `GET_SERVERS ` para comunicar (pedir msgservers) com o id server e
recebe 

`SERVERS (name;ip;upt;tpt)`

\*com todos os servidores
registados listados.

Necessita de tratar os dados recebidos e guardar para posterior
uso.

`publish message`

Envia mensagem para um servidor de mensagens com o
conteúdo‘message’. Tamanho máximo 140 caracteres.

Usa `PUBLISH` messagepara enviar a mensagem para o servidor

`show_latest_messages n`

Pede n mensagens ao servidor de mensagens.

Usa `GET_MESSAGES n` para pedir as ultimas n mensagens ao servidor
de mensagens conectado.

Recebe via `MESSAGES (message)`

Precisa de tratar os dados minimamente de modo a apresentar a
resposta ao utilizador.

`exit` termina

### Conexões internet:
------------------

UDP: `idserver` e `msgserver`

Sem necessidade de fazer threading, basta apenas receber
informação quando elaépedida. Comunicações inesperadas devem
ser descartadas. Ou nem sequer atendidas.

Garantir o formato correto da receção, de modo a impedir erros
de comunicação devido a UDP.

O `msgserver` é escolhido aleatoriamente pela aplicação. De modo a
todos serem usados.

Servidor de Mensagens

Servidor de Identidades

Chamadas de sistema permitidas

