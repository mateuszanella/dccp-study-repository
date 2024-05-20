# Repositório de estudos do protocolo DCCP

O presente repositório é o trabalho final da disciplina de Redes e Telecomunicações, ministrada pelo professor Me. Marcos André Lucas, no curso de Ciência da Computação da Universidade Regional Integrada do Alto Uruguai e das Missões Campus Erechim.

Integrantes: Felipe T. Malacarne e Mateus R. Zanella.

## Introdução

O objetivo desse trabalho é realizar estudos sobre o protocolo DCCP (Datagram Congestion Control Protocol), fornecendo um repostiório de estudos que deve ser prático e conciso. Por fim, a atividade requer a realização de uma apresentação prática sobre o protocolo estudado. Nesse caso, a atividade prática será uma simulação de um servidor e um cliente utilizando o protocolo DCCP.

## Referências e fontes

A principal fonte utilizada neste trabalho foi a referência do protocolo no [IETF](https://datatracker.ietf.org/doc/rfc4340/). Porém, outros RFCs também fomram estudados, como o [RFC 4336](https://datatracker.ietf.org/doc/rfc4336), o [RFC 4341](https://datatracker.ietf.org/doc/rfc4341) e o  [RFC 5595](https://www.rfc-editor.org/rfc/rfc5595#section-1.1). Outras fontes que também foram utilizadas, podem ser encontradas na seção de refrências do Estudo Bibliográfico. 

## Protocolo DCCP

O Datagram Congestion Control Protocol (DCCP) é um protocolo de transporte desenvolvido para fornecer conexões bidirecionais e não confiáveis, com controle de congestionamento integrado, incluindo notificações de congestionamento explícito e mecanismos de reconhecimento de perda de pacotes. Destina-se a aplicações que exigem um equilíbrio entre latência e entrega confiável, como o streaming de mídia, oferecendo vantagens sobre o UDP, que não inclui controle de congestionamento nativo. Embora não seja amplamente adotado, o DCCP é suportado no sistema operacional Linux e usado em algumas redes experimentais ou de pesquisa, com utilitários como o "dccp" no filesystem dCache para funcionalidades de cópia de arquivos.

Desenvolvido em 2001 e padronizado em 2006, é uma alternativa aos protocolos de transporte comuns como TCP, UDP e SCTP, que dependem de portas publicadas conhecidas para a conexão. DCCP aborda os desafios associados ao espaço limitado de portas e problemas de segurança gerados por firewalls e proxies, introduzindo um Código de Serviço de 32 bits único para cada aplicação, incluído apenas nos pacotes de requisição. Essa abordagem elimina conflitos de portas e facilita a identificação única das aplicações sem sobrecarga adicional, embora restrinja a hospedagem de múltiplos servidores para o mesmo serviço em uma única máquina.

## Exemplo de uso

### Progresso de uma Conexão DCCP Típica

| Passo | Cliente                               | Servidor                              |
| ----- | ------------------------------------- | ------------------------------------- |
| 0.    | [CLOSED]                              | [LISTEN]                              |
| 1.    | DCCP-Request -->                      |                                       |
| 2.    |                                       | <-- DCCP-Response                     |
| 3.    | DCCP-Ack -->                          |                                       |
| 4.    | DCCP-Data, DCCP-Ack, DCCP-DataAck --> | <-- DCCP-Data, DCCP-Ack, DCCP-DataAck |
| 5.    |                                       | <-- DCCP-CloseReq                     |
| 6.    | DCCP-Close -->                        |                                       |
| 7.    |                                       | <-- DCCP-Reset (Código 1, "Fechado")  |
| 8.    | [TIMEWAIT]                            |                                       |

1. O cliente envia ao servidor um pacote DCCP-Request especificando as portas do cliente e do servidor, o serviço solicitado e quaisquer recursos negociados, incluindo o CCID que o cliente gostaria que o servidor usasse. O cliente pode opcionalmente incluir um pedido de aplicação no pacote DCCP-Request. O servidor pode ignorar esse pedido de aplicação.

2. O servidor envia ao cliente um pacote DCCP-Response indicando que está disposto a se comunicar com o cliente. Esta resposta indica quaisquer recursos e opções que o servidor concorda, inicia outras negociações de recursos conforme desejado e opcionalmente inclui Cookies Iniciais que encapsulam todas essas informações e que devem ser retornados pelo cliente para a conexão ser completada.

3. O cliente envia ao servidor um pacote DCCP-Ack que reconhece o pacote DCCP-Response. Isso reconhece o número de sequência inicial do servidor e retorna quaisquer Cookies Iniciais no DCCP-Response. Também pode continuar a negociação de recursos. O cliente pode incluir um pedido de nível de aplicação neste ack, produzindo um pacote DCCP-DataAck.

4. O servidor e o cliente então trocam pacotes DCCP-Data, pacotes DCCP-Ack reconhecendo esses dados e, opcionalmente, pacotes DCCP-DataAck contendo dados com reconhecimentos embutidos. Se o cliente não tiver dados para enviar, o servidor enviará pacotes DCCP-Data e DCCP-DataAck, enquanto o cliente enviará exclusivamente DCCP-Acks. (No entanto, o cliente não pode enviar pacotes DCCP-Data antes de receber pelo menos um pacote não DCCP-Response do servidor.)

5. O servidor envia um pacote DCCP-CloseReq solicitando um fechamento.

6. O cliente envia um pacote DCCP-Close reconhecendo o fechamento.

7. O servidor envia um pacote DCCP-Reset com Código de Reset 1, "Fechado", e limpa seu estado de conexão. DCCP-Resets fazem parte da terminação normal da conexão; consulte a Seção 5.6.

8. O cliente recebe o pacote DCCP-Reset e mantém o estado por dois tempos de vida do segmento máximo, ou 2MSL, para permitir que quaisquer pacotes restantes sejam limpos da rede.

Uma sequência alternativa de fechamento de conexão é iniciada pelo cliente:

5b. O cliente envia um pacote DCCP-Close fechando a conexão.

6b. O servidor envia um pacote DCCP-Reset com Código de Reset 1, "Fechado", e limpa seu estado de conexão.

7b. O cliente recebe o pacote DCCP-Reset e mantém o estado por 2MSL para permitir que quaisquer pacotes restantes sejam limpos da rede.


## Formato dos pacotes

> Todas as informações dessa seção são retiradas diretamente do ([IETF, RFC 4340](https://datatracker.ietf.org/doc/rfc4340/)).

O cabeçalho do DCCP pode ter de 12 a 1020 bytes de comprimento. A parte inicial do cabeçalho tem a mesma semântica para todos os tipos de pacotes atualmente definidos. Em seguida, vêm quaisquer campos de comprimento fixo adicionais necessários pelo tipo de pacote e, depois disso, uma lista de opções de comprimento variável. A área de dados da aplicação segue o cabeçalho. Em alguns tipos de pacotes, essa área contém dados para a aplicação; em outros tipos de pacotes, seu conteúdo é ignorado.

### Tipos de pacotes

O DCCP define dez tipos de pacotes (mais 6 reservados), cada um com um propósito específico. Os tipos de pacotes são:
0. DCCP-Request
1. DCCP-Response
2. DCCP-Data
3. DCCP-Ack
4. DCCP-DataAck
5. DCCP-CloseReq
6. DCCP-Close
7. DCCP-Reset
8. DCCP-Sync
9. DCCP-SyncAck
10. Reservado
11. Reservado
12. Reservado
13. Reservado
14. Reservado
15. Reservado

Nessa seção, iremos abordar o apenas formato dos pacotes principais, no caso, o de requisição e de resposta. Além disso, iremos abordar o cabeçalho genérico do DCCP. Para mais informações sobre os outros tipos de pacotes, consulte o arquivo `tipos-de-pacotes.md`, lá entra-se em detalhes sobre o fomato de cada pacote e suas particularidades.

#### Cabeçalho Genérico

O cabeçalho genérico do DCCP é a parte inicial de todos os pacotes DCCP. Ele contém informações comuns a todos os tipos de pacotes, como portas de origem e destino, números de sequência e tipo de pacote. O cabeçalho genérico é seguido por campos adicionais e opções, dependendo do tipo de pacote.

| Parte do Cabeçalho    | Descrição                                                                              |
| --------------------- | -------------------------------------------------------------------------------------- |
| Generic Header        | Parte inicial com semântica comum para todos os tipos de pacotes definidos atualmente. |
| Additional Fields     | Campos adicionais de comprimento fixo, variando conforme o tipo de pacote.             |
| Options (optional)    | Lista de opções de comprimento variável, opcional.                                     |
| Application Data Area | Área de dados da aplicação, seguindo o cabeçalho.                                      |

O cabeçalho genérico do DCCP assume diferentes formas dependendo do valor de X, o bit de Números de Sequência Estendidos. Se X for um, o campo Número de Sequência tem 48 bits de comprimento, e o cabeçalho genérico tem 16 bytes, conforme a seguir.

| Campo           | Descrição                                         |
| --------------- | ------------------------------------------------- |
| Source Port     | Porta de Origem                                   |
| Dest Port       | Porta de Destino                                  |
| Data Offset     | Deslocamento de Dados                             |
| CCVal           | Valor do Controle de Congestionamento             |
| CsCov           | Cobertura do Checksum                             |
| Checksum        | Soma de Verificação                               |
| Res             | Reservado                                         |
| Type            | Tipo                                              |
| X               | Bit de Números de Sequência Estendidos            |
| Sequence Number | Número de Sequência (48 bits, parte alta e baixa) |

No caso de X ser zero, o campo Número de Sequência tem 24 bits de comprimento, e o cabeçalho genérico tem 12 bytes, conforme a seguir.

| Campo           | Descrição                                  |
| --------------- | ------------------------------------------ |
| Source Port     | Porta de Origem                            |
| Dest Port       | Porta de Destino                           |
| Data Offset     | Deslocamento de Dados                      |
| CCVal           | Valor do Controle de Congestionamento      |
| CsCov           | Cobertura do Checksum                      |
| Checksum        | Soma de Verificação                        |
| Res             | Reservado                                  |
| Type            | Tipo                                       |
| X               | Bit de Números de Sequência Estendidos     |
| Sequence Number | Número de Sequência (24 bits, parte baixa) |


Os campos do cabeçalho genérico são:

- **Source and Destination Ports (Portas de Origem e Destino)**: 16 bits cada. Identificam a conexão, semelhante aos campos correspondentes em TCP e UDP. A Porta de Origem representa a porta relevante no endpoint que enviou este pacote, e a Porta de Destino representa a porta relevante no outro endpoint. Ao iniciar uma conexão, o cliente DEVE escolher sua Porta de Origem aleatoriamente para reduzir a probabilidade de ataque.

- **Data Offset (Deslocamento de Dados)**: 8 bits. O deslocamento do início do cabeçalho DCCP até o início da área de dados da aplicação, em palavras de 32 bits. O receptor DEVE ignorar pacotes cujo Deslocamento de Dados seja menor que o tamanho mínimo do cabeçalho para o Tipo dado ou maior que o próprio pacote DCCP.

- **CCVal**: 4 bits. Usado pelo HC-Sender CCID. Por exemplo, o emissor do CCID A-to-B, que está ativo no DCCP A, PODE enviar 4 bits de informação por pacote ao seu receptor, codificando essa informação em CCVal. O emissor DEVE definir CCVal como zero a menos que seu CCID HC-Sender especifique o contrário, e o receptor DEVE ignorar o campo CCVal a menos que seu CCID HC-Receiver especifique o contrário.

- **Checksum Coverage (CsCov) (Cobertura do Checksum)**: 4 bits. Determina as partes do pacote cobertas pelo campo de Soma de Verificação. Isso sempre inclui o cabeçalho e as opções do DCCP, mas parte ou todo o dados da aplicação podem ser excluídos. Isso pode melhorar o desempenho em links ruidosos para aplicações que toleram corrupção.

- **Checksum (Soma de Verificação)**: 16 bits. A soma de verificação do cabeçalho do pacote DCCP (incluindo opções), um pseudo-cabeçalho da camada de rede e, dependendo da Cobertura do Checksum, todo, parte ou nenhum dos dados da aplicação.

- **Reserved (Reservado)**: 3 bits. Os emissores DEVEM definir este campo como zero em pacotes gerados, e os receptores DEVEM ignorar seu valor.

- **Type (Tipo)**: 4 bits. Especifica o tipo do pacote. Os valores definidos são: DCCP-Request, DCCP-Response, DCCP-Data, DCCP-Ack, DCCP-DataAck, DCCP-CloseReq, DCCP-Close, DCCP-Reset, DCCP-Sync, DCCP-SyncAck. Pacotes com tipo reservado DEVEM ser ignorados pelos receptores.

- **Extended Sequence Numbers (Números de Sequência Estendidos) (X)**: 1 bit. Define se o cabeçalho genérico estendido é usado com Números de Sequência e Acknowledgement de 48 bits. Os pacotes DCCP-Request, DCCP-Response, DCCP-CloseReq, DCCP-Close, DCCP-Reset, DCCP-Sync e DCCP-SyncAck DEVEM definir X como um; os pacotes com X igual a zero DEVEM ser ignorados.

- **Sequence Number (Número de Sequência)**: 48 ou 24 bits. Identifica o pacote de forma única na sequência de todos os pacotes enviados pela fonte nesta conexão. O Número de Sequência aumenta em um a cada pacote enviado, incluindo pacotes como DCCP-Ack que não carregam dados da aplicação.

Todos os tipos de pacotes atualmente definidos, exceto DCCP-Request e DCCP-Data, carregam um Subcabeçalho de Número de Acknowledgement nos quatro ou oito bytes imediatamente após o cabeçalho genérico.

#### Pacote de Requisição

Um cliente inicia uma conexão DCCP enviando um pacote DCCP-Request. Esses pacotes PODEM conter dados da aplicação e DEVEM usar números de sequência de 48 bits (X=1).

| Byte Offset | Campos                                       | Descrição                                                                                           |
| ----------- | -------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| 0-15        | Cabeçalho Genérico DCCP com X=1 (16 bytes)   | Cabeçalho genérico DCCP com X=1, indicando 48 bits de números de sequência e Tipo=0 (DCCP-Request). |
| 16-19       | Service Code (Código de Serviço)             | Descreve o serviço de nível de aplicação ao qual o cliente deseja se conectar.                      |
| 20-31       | Options and Padding (Opções e Preenchimento) | Espaço para opções adicionais e preenchimento.                                                      |
| 32+         | Application Data (Dados da Aplicação)        | Dados da aplicação, se houver.                                                                      |

- **Código de Serviço**: 32 bits. Descreve o serviço de nível de aplicação ao qual a aplicação cliente deseja se conectar. Os Códigos de Serviço têm o objetivo de fornecer informações sobre qual protocolo de aplicação uma conexão pretende usar, auxiliando assim middleboxes e reduzindo a dependência de portas globalmente conhecidas.

#### Pacote de Resposta

O servidor responde a pacotes DCCP-Request válidos com pacotes DCCP-Response. Esta é a segunda fase do handshake de três vias. Os pacotes DCCP-Response PODEM conter dados da aplicação e DEVEM usar números de sequência de 48 bits (X=1).

| Byte Offset | Campos                                       | Descrição                                                                                                                   |
| ----------- | -------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| 0-15        | Cabeçalho Genérico DCCP com X=1 (16 bytes)   | Cabeçalho genérico DCCP com X=1, indicando 48 bits de números de sequência e Tipo=1 (DCCP-Response).                        |
| 16-23       | Acknowledgement Number Subheader (8 bytes)   | Subcabeçalho de Número de Acknowledgement, contendo o Número de Acknowledgement de 48 ou 24 bits, dependendo do valor de X. |
| 24-27       | Service Code (Código de Serviço)             | Descreve o serviço de nível de aplicação ao qual o cliente deseja se conectar.                                              |
| 28-39       | Options and Padding (Opções e Preenchimento) | Espaço para opções adicionais e preenchimento.                                                                              |
| 40+         | Application Data (Dados da Aplicação)        | Dados da aplicação, se houver.                                                                                              |

- **Número de Acknowledgement**: 48 bits. Contém o GSR (Greatest Sequence Number Received). Como os DCCP-Responses são enviados apenas durante a inicialização da conexão, isso sempre será igual ao Número de Sequência em um DCCP-Request recebido.

- **Código de Serviço**: 32 bits. DEVE ser igual ao Código de Serviço no DCCP-Request correspondente.
