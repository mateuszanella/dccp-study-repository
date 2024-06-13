# Repositório de estudos do protocolo DCCP

O presente repositório é o trabalho final da disciplina de Redes e Telecomunicações, ministrada pelo professor Me. Marcos André Lucas, no curso de Ciência da Computação da Universidade Regional Integrada do Alto Uruguai e das Missões Campus Erechim.

Integrantes: Felipe T. Malacarne e Mateus R. Zanella.

## Objetivo do projeto

O objetivo desse trabalho é realizar estudos sobre o protocolo DCCP (Datagram Congestion Control Protocol), fornecendo um repostiório de estudos que deve ser prático e robusto, oferecendo uma linha sequencial para aprender e endender o assunto estudado. Por fim, a atividade requer a realização de uma demonstração prática sobre o protocolo analisado. Nesse caso, a prática escolhida será uma dinâmica de um servidor e um cliente simulando o protocolo DCCP.

> A ideia inicial na verdade, era realmente utilizar o próprio protocolo DCCP, porém, devido ao seu suporte limitado, optamos por apenas simular o protocolo em uma aplicação prática.

<!-- Para ser sincero, talvez seria possível executar o protocolo em uma VM, porém não fui capaz.-->

A ideia de criar um repositório se deu ao fato de que o DCCP não possui muita documentação além de suas referências no [IETF](https://datatracker.ietf.org/doc/rfc4340/), o que dificulta o aprendizado do protocolo devido à densidade e à complexidade do conteúdo. Desse modo, resolvemos criar um ambiente de estudos que seja mais acessível e leve para o aprendizado.

## Roadmap

A abordagem escolhida para o estudo do protocolo DCCP foi a de criar um roadmap. Um roadmap é uma ferramenta de planejamento estratégico que descreve as metas e objetivos de um projeto ou iniciativa, juntamente com os principais marcos, atividades e prazos necessários para alcançá-los.

O roadmap do projeto é composto por 8 etapas:

**1. [Introdução ao protocolo DCCP](Roadmap/1.%20Introdução.md)**

Características gerais do protocolo DCCP, explicando o que é, para que serve e como funciona.

**2. [História e casos de uso](Roadmap/2.%20Histórico.md)**

História do protocolo DCCP, bem como seus casos de uso.

**3. [Funcionamento do protocolo](Roadmap/3.%20Overview.md)**

Conceitos do protocolo em mais detalhes, como o funcionamento do handshake, a forma como o protocolo lida com a congestão e a forma como lida com a perda de pacotes

**4. [Tipos e formatos de pacotes](Roadmap/4.%20Pacotes.md)**

Tipos de pacotes e seus formatos.

**5. [Negociação de parâmetros](Roadmap/5.%20Negociação%20de%20Parâmetros.md)**

Como é feita a negociação de parâmetros entre cliente e servidor.

**6. [Processamento de eventos e checksums](Roadmap/6.%20Eventos%20e%20Checksum.md)**

Processamento de eventos e checksums para garantir a integridade dos pacotes.

**7. [Sistema de congestão](Roadmap/7.%20Congestão.md)**

Como o protocolo lida com a congestão.

**8. [Implementação prática](Roadmap/8.%20Implementação.md)**

Como executar a aplicação prática da atividade.

---

## Começe agora a estudar o roadmap em [Introdução ao protocolo DCCP](Roadmap/1.%20Introdução.md).

---

## Referências e fontes

A principal fonte utilizada neste trabalho foi a referência do protocolo no [IETF](https://datatracker.ietf.org/doc/rfc4340/). Porém, outros RFCs também foram estudados, como o [RFC 4336](https://datatracker.ietf.org/doc/rfc4336), o [RFC 4341](https://datatracker.ietf.org/doc/rfc4341) e o [RFC 5595](https://www.rfc-editor.org/rfc/rfc5595#section-1.1). Outras fontes que também foram utilizadas, podem ser encontradas na seção de refrências do Estudo Bibliográfico.

## Links úteis

- [RFC 4340](https://datatracker.ietf.org/doc/rfc4340/), IETF.
- [RFC 4336](https://datatracker.ietf.org/doc/rfc4336), IETF.
- [RFC 4341](https://datatracker.ietf.org/doc/rfc4341), IETF.
- [RFC 5595](https://www.rfc-editor.org/rfc/rfc5595#section-1.1), IETF.
- [DCCP, UFRJ](https://www.gta.ufrj.br/ensino/eel879/vf/dccp), por Aramys Mattos e Marcos Benigno.
