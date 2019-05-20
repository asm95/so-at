### SO At

O conjunto de programas realiza a execução postegarda de um processo seguindo
uma topologia escolhida pelo usuário.

A execução será realizada por até 15 nós criados pelo escalonador.
Cada nó executa o mesmo programa.
Quando todos os nós finalizarem a execução, o escalonador estará pronto para
receber outro programa.

A topologia define quais conexões os nós farão entre si.
Essas conexões permitem esses encaminharem mensagens por um caminho válido.

Um caminho válido consiste em uma rota de um nó fonte para um destino respeitando
as conexões estabelecidadas. Por exemplo, se existe um conexão entre (1,2) e (2,3),
e 1 deseja enviar uma mensagem para 3, então deve passar obrigatóriamente por 2.

### Instalação a partir do fonte (Linux)

Realize a clonagem do repositório utilizando a ferramenta git:

```sh
git clone https://github.com/asm95/so-at.git 'so-at'
cd 'so-at'
export BASE_DIR=$(pwd)
```

Utilize o GNU Make para realizar a compilação de todos os programas:

```sh
cd $BASE_DIR
make
```
No final, os programas `es`, `at`, e `hello` serão gerados.
Podes gerar os programas separadamente utilizando o `make <nome_do_programa>`,
por exemplo `make es` para compilar o escalonador.


### Utilização

<span>1.</span> O primeiro passo é iniciar o escalonador:

```sh
# substitua a variável pela topologia desejada:
# -f = fat tree; -t = torus; -h = hipercubo
export APP_TOPOLOGY=''
./es $APP_TOPOLOGY
```

Por exemplo, para o torus fazemos:

```sh
export APP_TOPOLOGY='-t'
./es $APP_TOPOLOGY

# ou simplesmente
./es -t
```

<span>2.</span> Utilizamos o comando at para requisitar o escalonador
executar novos programas:

```sh
# o delay irá fazer com que o programa seja postegardo por x segundos
# isto é, não será executado imediatamente
export APP_DELAY=''
# o nome do programa a ser executado
# caso somente o nome do programa for fornecido (ex: 'hello')
# o escalonador irá procura-lo na pasta onde ele foi iniciado
export APP_PROG=''
./at $APP_DELAY $APP_PROG
```

Por exemplo, para executar o programa de exemplo 'hello' com delay de
8 segundos:

```sh
export APP_DELAY='5'
export APP_PROG='hello'
./at $APP_DELAY $APP_PROG

# ou simplesmente:
./at 5 hello
```

### Contato

Cristiano Cardoso
&lt; <a href="mailto:asm95@outlook.com">asm95@outlook.com</a> &gt;

Bernado Costa Nascimento
&lt; <a href="mailto:bernardoc1104@gmail.com">bernardoc1104@gmail.com</a> &gt;