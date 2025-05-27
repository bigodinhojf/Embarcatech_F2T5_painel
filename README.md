<div align="center">
    <img src="https://moodle.embarcatech.cepedi.org.br/pluginfile.php/1/theme_moove/logo/1733422525/Group%20658.png" alt="Logo Embarcatech" height="100">
</div>

<br>

# Painel de Controle Interativo de Acesso para um Escritório

## Sumário

- [Descrição](#descrição)
- [Funcionalidades Implementadas](#funcionalidades-implementadas)
- [Ferramentas utilizadas](#ferramentas-utilizadas)
- [Objetivos](#objetivos)
- [Instruções de uso](#instruções-de-uso)
- [Vídeo de apresentação](#vídeo-de-apresentação)
- [Aluno e desenvolvedor do projeto](#aluno-e-desenvolvedor-do-projeto)
- [Licensa](#licença)

## Descrição

Este projeto implementa um painel de controle interativo de acesso para um escritório, utilizando o microcontrolador RP2040 e o sistema operacional em tempo real FreeRTOS. O sistema é composto por por Display OLED, LED RGB, Buzzer e botões, e foi desenvolvido com tarefas do FreeRTOS que utilizam de mutex, semáforo binário e semáforo de contagem. A entrada de usuários no escritório é simulada pelo botão A, que também faz o controle do número máximo de usuários que pode entrar, a saída de usuários é simulada pelo botão B, e o botão do joystick simula a saída de todos os usuários. Um sinal sonoro é emitido quando um usuário tenta entrar na sala cheia, um outro sinal sonoro é emitido quando é feito o reset da sala. O LED RGB e o display OLED mostram quantos usuários têm na sala atualmente, e alguns alertas visuais.

## Funcionalidades Implementadas

1. Entrada de usuário

   - Foi implementada utilizando semáforo de contagem.
   - O botão A adiciona um usuário (caso haja vagas disponíveis) e diminui 1 na quantidade de vagas.
   - Caso não haja vagas disponíveis é emitido um alerta sonoro (beep curto) e visual.
   - Foi feito o tratamento de debounce no botão.

2. Saída de usuário

   - Foi implementada utilizando o mesmo semáforo de contagem da funcionalidade anterior.
   - O botão B subtrai um usuário (caso haja um) e aumenta 1 na quantidade de vagas.
   - Foi feito o tratamento de debounce no botão.

3. Reset de usuários

   - Foi implementada utilizando semáforo binário.
   - O botão do joystick zera a quantidade de usuários e maximiza a quantidade de vagas.
   - Emite um beep duplo.
   - Foi desenvolvido a partir de interrupção e debounce.

4. LED RGB

   - O LED RGB exibe a cor azul quando não há usuários, exibe verde quando há entre 1 usuário e 2 vagas disponíveis, amarelo quando há apenas uma vaga disponível e vermelho quando não há mais vagas disponíveis.
  
5. Display OLED

   - Foi implementada com a proteção do mutex.
   - Exibe informações da quantidade de usuários e alertas visuais.
  
6. Buzzers

   - Emitem um beep curto quando não há mais vagas disponíveis e o botão A é pressionado.
   - Emitem um beep duplo quando o botão do joystick é pressionado.
  
## Ferramentas utilizadas

- **Ferramenta educacional BitDogLab (versão 6.3)**: Placa de desenvolvimento utilizada para programar o microcontrolador.
- **Microcontrolador Raspberry Pi Pico W**: Responsável pela execução das tarefas do FreeRTOS e pelo controle de periféricos.
- **Display OLED SSD1306**: Exibe as informações de quantidade de usuários e alertas visuais.
- **LED RGB**: Altera de cor de acordo com a quantidade de usuários.
- **Buzzers**: Emite sinal sonoro curto e duplo.
- **Botões**: Controlam a quantidade de usuários.
- **Visual Studio Code (VS Code)**: IDE utilizada para o desenvolvimento do código com integração ao Pico SDK.
- **Pico SDK**: Kit de desenvolvimento de software utilizado para programar o Raspberry Pi Pico W em linguagem C.
- **FreeRTOS**: Sistema operacional em tempo real, utilizado para gerenciar as multitarefas, o mutex e os semáforos desenvolvidos.

## Objetivos

1. Aplicar os conceitos aprendidos sobre mutex, semáforo binário e semáforo de contagem no FreeRTOS.
2. Implementar um painel de controle interativo.
3. Implementar um sistema acessível.

## Instruções de uso

1. **Clonar o Repositório**:

```bash
git clone https://github.com/bigodinhojf/Embarcatech_F2T5_painel.git
```

2. **Compilar e Carregar o Código**:
   No VS Code, configure o ambiente e compile o projeto com os comandos:

```bash	
cmake -G Ninja ..
ninja
```

3. **Interação com o Sistema**:
   - Conecte a placa ao computador.
   - Clique em run usando a extensão do raspberry pi pico.
   - Clique os botões A, B e do joystick para controlar a quantidade de usuários.
   - Observe os sinais visuais e sonoros.

## Vídeo de apresentação

O vídeo apresentando o projeto pode ser assistido [clicando aqui](https://youtu.be/yeCX5y1nTMM).

## Aluno e desenvolvedor do projeto

<a href="https://github.com/bigodinhojf">
        <img src="https://github.com/bigodinhojf.png" width="150px;" alt="João Felipe"/><br>
        <sub>
          <b>João Felipe</b>
        </sub>
</a>

## Licença

Este projeto está licenciado sob a licença MIT.
