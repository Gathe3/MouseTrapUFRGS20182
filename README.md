# MouseTrapUFRGS20182
Jogo que desenvolvemos para o trabalho final da disciplina de Algoritmos e Programação (INF01202) de 2018/2 utilizando a biblioteca Allegro para C no CodeBlocks.
Turma B (professores: Edison e Joel).

Obs.: tutoriais de Allegro (de onde começamos a aprender): www.aprendendoallegro.tk

Descrição do jogo:
O usuário controla um rato que deve percorrer pelo mapa coletando queijos e fugindo dos gatos, que andam aleatoriamente e, caso encontrem o rato, o devoram (perde uma vida). No caminho, pode coletar ossos. Um osso transforma o rato momentaneamente em um cachorro que, além de andar mais rápido, pode devorar os gatos. Há, também, portas no mapa. Apertando uma tecla, elas mudam de posição, bloqueando outros caminhos e liberando aqueles que eram bloqueados pelas portas anteriormente. O objetivo do jogo é coletar todos os queijos do mapa, passando, assim, entre os níveis.
Além disso, os níveis são carregados a partir de um arquivo .txt, sendo que todo nível deve conter 11 linhas e 27 colunas de caracteres, 8 portas, 4 ossos e 4 gatos. Um X no arquivo representa uma parede; um T, uma porta; um Q, um queijo; um G, a posição inicial de algum gato; um M, a posição inicial do rato; e um O, um osso. Podem ser adicionados até 99 níveis. Há também funções de salvar níveis e high scores no jogo.

Conteúdo do jogo:
Interface gráfica tendendo ao estilo 8-bit;
Menu inicial;
Menu no jogo;
Saves;
High Scores;
4 cenários diferentes, que alternam a cada nível (repetem-se de 4 em 4 níveis);
Músicas de fundo;
Efeitos sonoros;
Movimento animado dos personagens (com a utilização de sprites);
Animações de game over e de jogo concluído;
Barra indicando o tempo restante no modo cachorro (+5 segundos para cada osso pego);
Indicações sobre quais gatos estão vivos (após o modo cachorro acabar, renascem);
Ganho de uma vida a cada 3000 pontos ou quando são devorados todos os gatos de uma vez só (antes de acabar o modo cachorro);
etc.

Mecânica fundamental no jogo: matriz x tela
Aproveitamos os trabalhos feitos nas aulas práticas com temática do jogo, que envolviam movimentação de posições em matrizes e não uma movimentação contínua na tela, e elaboramos um algoritmo que relaciona a posição na tela de cada personagem com a sua posição na matriz, ao invés de usar colisões e hitboxes puras. Assim, tudo que acontece na tela é refletido na matriz do mapa e em estruturas com as posições atuais (linha e coluna) dos gatos e do rato nessa matriz.

Estrutura básica:
main() -> menu() -> tratar retorno de menu()
função jogo(): entra no jogo no nível, com as vidas e os pontos determinados por um parâmetro (utilizada tanto para um novo jogo como para um jogo salvo)

Bugs conhecidos:
Se um arquivo high_scores.bin ainda não existir, salvar a pontuação (somente) após zerar o jogo às vezes resulta em crashes;
Pela maneira como foi feita a detecção das posições do gato na tela, mudar uma porta logo que um gato vai passar pelo local para onde a porta irá pode resultar no gato atravessando paredes;
A música de fundo que toca ao zerar o jogo parece estar distorcida;
Às vezes, dependendo da posição do rato ou dos gatos na tela, salvar um jogo e carregá-lo depois resulta em algumas paredes sumindo do mapa e sendo trocadas por espaços vazios.

Observação:
Muitos assets foram retirados da internet ou (principalmente os áudios) de jogos conhecidos (Super Mario Bros, Mario Kart Wii, Sonic 3, Yoshi's Island, Stardew Valley, Minecraft, Lego Star Wars, Donkey Kong, Club Penguin, etc.). Tudo isso é propriedade dos seus criadores e não tem finalidade comercial nesse projeto. A única finalidade em expor o trabalho aqui é proporcionar material para conhecimento a quem interessar.
