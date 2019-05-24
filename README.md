# Escalonador de Execução Postergada
    
## Introdução
 
- Projeto de Sistemas Operacionais 1/2019
- Professora: Dra. Alba Cristina Magalhães Alves de Melo
- Aluno: Bernardo Costa Nascimento; 140080279
- Aluno: Cristano

## Instruções Básicas

O presente trabalho é o desenvolvimento de um escalonador de execução postergada.
Para compilar o projeto, basta executar o seguinte comando:

> sh compile.sh

A compilação irá gerar 4 executáveis com os seguintes nomes:

- escalonador
- execucao_postergada
- shutdown
- hello

Para utilizar o sistema, basta executar o 'escalonador'. O programa deve ser iniciado
com um dos três argumentos -h, -t ou -f, como exemplificador abaixo:

> ./escalonador -h&

O comando acima irá iniciar a execução do escalonador em modo background (&). Feito isso
o usuário deve utilizar o programa execucao_postergada para passar algum programa para
escalonamento.

Para executar o programa execucao_postergada, o usuário deve passar 2 argumentos: nome do 
programa e delay. Segue um exemplo:

> ./execucao_postergada hello 4

O comando acima irá selecionar o programa hello para ser executado em 4 segundos. O usuário
poderá executar quantos programas desejar dessa maneira.

Quando o usuário quiser concluir a execução do sistema, basta executar o programa shutdown.

> ./shutdown

Após clonar este repositório, o usuário pode acessar a documentação do sistema por meio do 
site doxygen. Para gerar o site, baixe e execute o doxygen com o arquivo "escalonadorEP".
O programa irá gerar a pasta "docs" no diretório raíz do sistema. A documentação pode ser acessada
abrindo o arquivo docs/html/index.html.