# Modem dial-up

## Introdução

Nesta prática, vamos implementar um modem dial-up compatível com o padrão [V.21](https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-V.21-198811-I!!PDF-E&type=items).


## Dependências

Se quiser compilar localmente, você precisa essencialmente das bibliotecas gtest e rtaudio.

```bash
sudo pacman -S cmake rtaudio gtest python-matplotlib  # no Arch
sudo apt-get install cmake librtaudio-dev libgtest-dev python3-matplotlib  # no Ubuntu
./run-grader
```

O código foi testado com o rtaudio versão 5.2.0. A API do rtaudio é um pouco instável, então talvez não compile com outras versões.

Como IDE, recomendo usar o VSCode com CMake Tools ou o neovim com clangd.


## Passo-a-passo da implementação

### UART

Primeiro, você deve trabalhar nos arquivos `uart.cpp` e `uart.hpp` para implementar um receptor de UART. Uma boa referência para entender o que deve ser feito é a [*application note* da Maxim](https://www.analog.com/en/technical-articles/determining-clock-accuracy-requirements-for-uart-communications.html). A diferença é que lá se discute um receptor com clock 16 vezes superior ao *baud rate*, ao passo que aqui temos um clock ainda mais rápido — 160 vezes superior ao *baud rate* — já que vamos usar a taxa de amostragem nativa da maioria das interfaces de áudio (48000 Hz).

O método `UART_RX::put_samples` recebe um `buffer` contendo um sinal binário (cada elemento do `buffer` é 0 ou 1) amostrado à mesma taxa da interface de áudio. Esse `buffer` pode ter qualquer tamanho `n` e você deve lidar com esse fato. A forma mais fácil é fazer um loop iterando por cada amostra individual de `buffer` e nunca tentar olhar para as amostras futuras, nem olhar para amostras passadas de forma direta. Ou seja, se você precisar consultar amostras passadas, armazene-as de alguma forma como atributo da classe, mas nunca faça coisas como `buffer[i-1]`. Seguindo essas regras básicas, o seu código vai funcionar como uma máquina de estados fácil de entender e você não vai precisar ficar tratando casos especiais.

Sempre que você terminar de receber um byte completo, chame a função `get_byte` passando esse byte como argumento.

**Sugestão**: Na *application note*, discute-se um receptor que espera uma amostra com nível lógico baixo no início do *start bit*, e três amostras com nível lógico baixo no meio do *start bit*. Como nosso clock é dez vezes maior, o análogo seria esperar 30 amostras com nível lógico baixo no meio do *start bit*. Para tolerar um pouco de erro, eu sugiro verificar se pelo menos 25 amostras dentre as últimas 30 recebidas têm nível lógico baixo. Se, além disso, a amostra que estaria no início do *start bit* tiver nível lógico baixo, considere que você achou o *start bit*. Quando isso acontecer, comece a contar múltiplos de 160 amostras a partir do meio do *start bit*, acrescentando o valor de cada amostra situada nesses múltiplos ao byte que está sendo recebido. Ao chegar no *stop bit*, passe o byte recebido para `get_byte` e volte sua lógica para o estado ocioso, no qual você deve procurar pelo próximo *start bit*.

Há quatro testes para a UART — trivial, unsync, noisy e noisy\_unsync. O teste trivial não introduz ruído nem diferença entre os clocks do receptor e transmissor. Você pode tentar passar nesse teste primeiro para se ambientar, o que deve ser possível mesmo se você começar a contagem no início do *start bit*. Mas se você começar a contagem no meio do *start bit* e estiver fazendo tudo certo, você deve passar em todos os quatro testes de uma vez só.


### V21

Uma vez funcionando a UART, você deve trabalhar nos arquivos `v21.cpp` e `v21.hpp` para implementar um demodulador FSK.

O método `V21_RX::demodulate` recebe um array `in_analog_samples` contendo um sinal "analógico" (entre aspas porque esse sinal é quantizado) proveniente da interface de áudio. Esse sinal pode ter qualquer tamanho `n` e você deve lidar com isso, similar ao passo anterior. A dica é a mesma de antes: tratar uma amostra por vez e nunca olhar para o futuro, nem olhar para o passado diretamente no array de entrada.

Sempre que você tiver demodulado uma quantidade conveniente de amostras binárias, chame `get_digital_samples` passando um buffer com essas amostras binárias e o tamanho desse buffer. No final das contas, esse buffer vai acabar sendo passado para a UART.

Minha sugestão é implementar um demodulador que não se importa com a sincronia entre relógios, ou seja, que deixa isso a cargo da UART. Para isso, você pode chamar `get_digital_samples` uma única vez ao final do método `V21_RX::demodulate`, passando um buffer do mesmo tamanho de `in_analog_samples`. Essa parte já vem inclusive pronta no esqueleto de código fornecido a vocês.

Faça dois filtros passa-bandas, um para o tom de marca, outro para o tom de espaço. Subtraia a saída de um dos filtros passa-bandas da saída do outro, e filtre essa diferença com um passa-baixas. Quando a diferença filtrada indicar uma energia maior na frequência de espaço, insira um 0 no buffer de saída. Caso contrário, insira um 1.

Implemente uma estratégia para detectar a ausência de uma portadora (ou seja, quando não existe nem tom de marca nem tom de espaço). Enquanto não houver portadora, insira sempre 1 no buffer de saída.

A seção "Demodulando um sinal FSK" [deste notebook Jupyter](https://colab.research.google.com/drive/1tjileevEYGz6IMGCzqgFq9Sy4GvYtBuL?usp=sharing) ensina a fazer tudo isso em Python de forma *offline*. Traduzir esse código para C++ não é muito difícil. Mas atente-se para o fato de que você deve transformá-lo em um código que opere de forma *online*, ou seja, tratando uma amostra de entrada por vez e guardando todo estado que for necessário como atributo da classe.

Os testes de V21 traçam o gráfico de BER vs Eb/N0 e comparam alguns pontos desse gráfico com valores de referência. Se você implementar tudo certo, deve passar nos dois testes de uma vez só. O teste unsync corresponde a relógios ligeiramente diferentes no transmissor e no receptor, mas a própria UART deve conseguir lidar com isso.


### Recuperação do clock (opcional)

Um desafio opcional é implementar o descrito na seção "Recuperando o sinal de relógio com um filtro" do [notebook Jupyter](https://colab.research.google.com/drive/1tjileevEYGz6IMGCzqgFq9Sy4GvYtBuL?usp=sharing). Nesse caso, em vez de chamar `get_digital_samples` uma única vez no final do método `V21_RX::demodulate`, você poderia chamar `get_digital_samples` passando um buffer de 160 amostras idênticas assim que demodulasse cada símbolo do sinal. Assim, o V21 mandaria o sinal já com a temporização correta para a UART

Por favor, note que a abordagem de deixar a sincronização a cargo da UART já funciona significativamente bem. **Os testes não vão avaliar se você implementou ou não este desafio.** Encare-o somente se tiver curiosidade e se tiver tempo de sobra.


### Testes de bancada

De 10 pontos da nota, 8 são atribuídos automaticamente pelo robô de correção.

Para conseguir os 2 pontos restantes, seu grupo deve submeter a implementação a um teste de bancada, interoperando com [um modem dial-up disponível comercialmente](https://pt.aliexpress.com/item/2032456154.html).

Para isso, é necessário saber como **usar** o modem, e não só como executar os testes automatizados.

Como cada ponta do enlace usa um par de frequências diferente para transmitir, a convenção é escolher esse par de frequências de acordo com o papel da ponta na chamada telefônica. Se foi quem discou (em inglês, *caller*), utiliza-se um par de frequências; se foi quem recebeu a chamada (em inglês, *answerer*), utiliza-se o outro. Por isso, será necessário passar essa informação ao modem.

#### Linux

Em um terminal, execute:

```bash
export PIPEWIRE_NOJACK=1
./Modem call   # na outra ponta, ./Modem ans
```

Em outro terminal, execute o picocom passando o dispositivo informado na saída do Modem:

```bash
picocom -b 115200 --echo /dev/pts/N
```

Com o picocom, você pode trocar mensagens de texto diretamente com a outra ponta.

Em vez de usar o picocom, você também pode usar o slattach para subir uma interface SLIP:

```bash
sudo slattach -v -p slip /dev/pts/N

# em outro terminal:
sudo ifconfig sl0 192.168.123.x pointopoint 192.168.123.y
```

ou até mesmo usar seu modem em conjunto com as suas [práticas da disciplina de Redes](https://github.com/thotypous/redes-s1)!


#### Windows

Antes de usar o modem no Windows, você precisa instalar o [com0com](https://sourceforge.net/projects/com0com/files/latest/download). Depois de instalar, reinicie o computador e faça uma atualização do driver pelo Windows Update. Você provavelmente terá de habilitar essa atualização manualmente na lista de atualizações opcionais de drivers. **Isso é muito importante**, pois a assinatura do driver que vem no instalador do com0com expirou, então o driver não funciona a menos que você o atualize pelo Windows Update.

Se você tiver dificuldades para montar o ambiente de compilação no Windows, basta pegar o binário pronto nos artefatos gerados pelo GitHub Actions no seu repositório. Lá você vai encontrar um arquivo `Modem.exe` compilado a partir do código que você enviou para o GitHub.

Como o modem precisa de algumas bibliotecas, descompacte as [DLLs das bibliotecas](https://drive.google.com/file/d/1pl44LOPE9bJrzsEediIDPpgSQ73rVrng/view?usp=sharing) no mesmo diretório que você colocar o `Modem.exe`.

Agora precisamos esclarecer o funcionamento do com0com. Ao contrário do Linux, em que o subsistema pty permite alocar portos seriais virtuais dinamicamente, com o com0com o nosso modem precisa conectar a um porto serial virtual previamente configurado.

Durante a instalação do com0com, se você não tiver alterado nenhuma opção, ele terá criado um par de portos seriais virtuais COM3/COM4. O padrão do modem é conectar em COM3, de forma que ele ficará acessível para você na COM4. Se você precisar mudar isso, pode passar a opção `-s PORTO` para o modem.

Abra um Prompt do MS-DOS ou um terminal do PowerShell e execute `Modem call` ou `Modem ans`.

Utilize o Putty para conectar-se à COM4 se quiser trocar mensagens de texto diretamente com a outra ponta.

Utilize o discador do Windows se quiser subir uma interface de rede. Infelizmente, o Windows 7 parece ter sido a última versão do Windows a suportar SLIP. Mas você pode tentar usar PPP.


#### Modem comercialmente disponível

Os modems comercialmente disponíveis também apresentam-se ao usuário como dispositivos seriais mas, como eles suportam uma diversidade de protocolos e modos de operação, é necessário operá-los usando comandos AT (também conhecidos como comandos Hayes).

Conecte-se ao dispositivo do modem utilizando o picocom (no Linux), Putty (no Windows) ou outro software que funcione como um terminal serial. No Linux, modems USB costumam ser detectados como `/dev/ttyUSB0` ou `/dev/ttyACM0`.

Se você estiver usando o simulador de linha telefônica (em vez do PABX), use o seguinte comando para que o modem ignore a ausência de um tom de discagem:

```
ATX3
```

Use o seguinte comando para colocar o modem em modo V.21:

```
AT+MS=V21,0
```

Para estabelecer a conexão, caso você esteja na ponta que efetua a chamada, use o comando:

```
ATDT1
```

Acima, `1` é um número de telefone, que será chamado usando tons [DTMF](https://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling). Perceba que o modem que nós implementamos não possui essa funcionalidade de chamar um número — ele assume que a chamada telefônica já está estabelecida no momento que ele começa a operar.

Caso você esteja na ponta que recebe a chamada, use o comando:

```
ATA
```

A partir daí, você pode trocar mensagens de texto diretamente com a outra ponta.

Se estiver no picocom e quiser subir uma interface SLIP, use as teclas Ctrl+A seguidas de Ctrl+Q para sair do picocom sem desligar a chamada (sem "colocar o telefone no gancho"). Em seguida, utilize o comando `slattach`, passando o dispositivo serial do modem.
