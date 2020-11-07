# Hilzer's Barbershop problem

O relatório completo com a descrição e solução pode ser encontrado no arquivo [Relatorio.pdf](Relatorio.pdf)

## Descrição

Este problema envolve o gerenciamento de uma barbearia que possui três barbeiros e três cadeiras para cortar o cabelo dos clientes. Nessa barbearia, há uma área de espera com um sofá de quatro lugares e uma parte para esperar de pé. A lotação é de 20 clientes, e os clientes adicionais são barrados na entrada.

Quando um cliente entra na loja, ele pode sentar no sofá, caso haja vaga, ou esperar de pé, mas a ordem de fila sempre é respeitada. Se um barbeiro está livre, é atendido o cliente que está no sofá há mais tempo, e a pessoa de pé há mais tempo, caso haja, pode sentar no sofá.

Ao finalizar um corte, um dos barbeiros deve cobrar o cliente, mas há apenas um caixa, então apenas um cliente pode pagar por vez. Caso não haja mais clientes para um barbeiro atender, ele pode dormir em sua cadeira, esperando por novos clientes.

**O objetivo então é solucionar esse problema com o uso de threads de forma a garantir que não haja deadlocks ou race conditions.**

## Soluções

Foram propostas duas soluções para o problema. O código da Solução 1 encontra-se [aqui](Solucao_1/), enquanto a Solucao 2 pode ser vista [aqui](Solucao_2/). A descrição completa pode ser lida no [relatório final](Relatorio.pdf).

## Participantes

O trabalho contou com a participação de:

- Eric Sales
- Fernanda Malheiros
- João Hirasawa
- Pabolo Vinícius
- Víctor Colombo
