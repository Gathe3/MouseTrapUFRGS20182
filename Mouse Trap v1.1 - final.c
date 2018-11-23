#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LARGURA 1280
#define ALTURA 720
#define FPS 60.0 // Precisa de 1 casa decimal para entrar em al_start_timer()!!!!
#define TAMTEXTO 35 // Tamanho do texto do menu
#define VEL_CORES_MENU 0.90 // Tempo, em segundos, com o qual mudam as cores dos gradientes
#define TEMPO_RATO_MENU 10 // Tempo, em segundos, entre ratos passando no fundo do menu
#define VEL_RATO 4 // Quantos pixels o rato anda por frame (no menu)
#define N_FRAMES_SPRITE_RATO 9 // Quantos frames para trocar a imagem do sprite
#define ESCALA_RATO_JOGO 2
#define VEL_RATO_JOGO 3
#define TILESIZE 47
#define LINHAS 11
#define COLUNAS 28 //27 + '\n'
#define PORTAS 8
#define HITBOX_X_DIR 56
#define HITBOX_X_ESQ 9
#define HITBOX_Y_CIMA 0
#define HITBOX_Y_BAIXO 45
#define VEL_GATO 1 // Tem que ser 1 para os gatos nao fugirem do mapa :( Solucao possivel para mudar a velocidade dos gatos: fazer um atualizar diferente so para eles
#define VEL_GATO_OSSO 1 // IDEM
#define NUM_SAMPLES 5
#define VOLUME_MUSICA_JOGO 0.7 // Valor float, onde 1.0 eh o volume da musica no arquivo
#define VOLUME_MUSICA_MENU 0.6
#define ESCALA_CACHORRO_JOGO 2.33
#define VELOCIDADE_MUSICA_CACHORRO 1.75 // Valor float, onde 1.0 eh a velocidade da musica no arquivo
#define DURACAO_MODO_CACHORRO 5 // Duração em segundos
#define VEL_CACHORRO 4
#define VOLUME_EFEITOS_MENU 0.8
#define VOLUME_SFX_JOGO 0.8
#define DURACAO_AVISO_VIDA 3 // Em segundos
#define TEMPO_TELA_VIDAS_NIVEL 3
#define TEMPO_JOGO_SALVO 2
#define N_FRAMES_SPRITE_GAMEOVER 30 // Do cao que dorme
#define TEMPO_GAME_OVER 10 // Em segundos
#define N_FRAMES_FIREWORK 15
#define DURACAO_JOGO_ZERADO 15


typedef struct pos_rato
{
    int x;
    int y;
} RATO;

typedef struct str_gato
{
    int x;
    int y;
    int direcao; // 1 = direita, 2 = esquerda, 3 = cima, 4 = baixo
} GATO;

typedef struct pos_porta
{
    int x;
    int y;
} PORTA;

typedef struct str_sprite
{
    int num_col, num_lin;
    int ind_col, ind_lin;
    ALLEGRO_BITMAP *folha;

} SPRITE;

typedef struct estado_save_jogo
{
    int nivel;
    int pontos;
    int vidas;
    int flag; /* Usado para a função carregar jogo: se alguém quiser voltar ao menu (apertando ESC), a flag é 3 (pois a função retorna um SAVE);
    se o jogo deve fechar (se falhou ao inicializar ou apertou para fechar a janela), a flag é 0;
    se o arquivo é um save de fato, a flag é 1; se o arquivo eh um novo jogo, a flag eh 2 */
    char mapa[LINHAS][COLUNAS];
    RATO rato_pos_matriz; // Representa a posicao do rato na matriz quanto ao indice (x de 0 a 26, y de 0 a 10)
    GATO gatos_matriz[4]; //  Posicao dos gatos na matriz; 0 = verm, 1 = amar, 2 = cinza, 3 = branco
} SAVE;

typedef struct high_score
{
    char nome[4]; // 3 caracteres na string do nome
    int pontos;
} SCORE;

typedef struct str_cenario
{
    ALLEGRO_BITMAP *chao;
    ALLEGRO_BITMAP *parede;
    ALLEGRO_BITMAP *porta;
    ALLEGRO_COLOR cor_fundo;
    ALLEGRO_AUDIO_STREAM *musica;
} CENARIO;

void msg_erro(char *texto, ALLEGRO_DISPLAY *display);
int init_addons(ALLEGRO_DISPLAY *display);
void desalocar(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer);
void textomenu(ALLEGRO_FONT *fontemenu, int sel);
char menu(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer);
int jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, SAVE save);
int checaSaves(SAVE *save1, SAVE *save2);
int salvaSave(SAVE save, char *diretorio_do_arquivo);
SAVE carregar_jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer);
int mostra_high_scores(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer);
int leHighScores(SCORE *maisaltas);
int salvarScore(SCORE pontuacao); // Recebe o score. Se for um dos 5 maiores, salva.
int desenhar_mapa(char mapa[LINHAS][COLUNAS], CENARIO cenario);
int carregar_mapa(char *nome_arquivo, char mapa[LINHAS][COLUNAS], RATO *pos_rato, GATO pos_gato[4]);
void arquivo_nivel(int nivel, char *nome_arquivo);
int podeMoverRato(char mapa[LINHAS][COLUNAS], RATO pos_rato, char direcao);
int mudaDirecaoGato(char mapa[LINHAS][COLUNAS], int col_gato, int lin_gato, int direcao_gato);
void textomenu_pause(ALLEGRO_FONT *fontemenu, int sel);
int tela_nivel_vidas(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, ALLEGRO_COLOR fundo, int nivel, int vidas);
int tela_salvar_jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, SAVE estado_do_jogo);
int tela_game_over(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, int pontuacao);
int tela_salvar_score(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, int pontuacao);
int tela_zerar_jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, int pontos, int vidas);

void msg_erro(char *texto, ALLEGRO_DISPLAY *display)
{
    if(display) // Se display existir (não for NULL)
    {
        al_show_native_message_box(display, "Erro", "Ocorreu um erro:", texto, NULL, ALLEGRO_MESSAGEBOX_ERROR);
    }
    else
        al_show_native_message_box(NULL, "Erro", "Ocorreu um erro:", texto, NULL, ALLEGRO_MESSAGEBOX_ERROR);
}

int init_addons(ALLEGRO_DISPLAY *display)
{
    int retorno = 1;

    al_init_font_addon();
    if(!al_init_ttf_addon())
    {
        msg_erro("Falha ao inicializar add-on allegro_ttf.", display);
        retorno = 0;
    }

    if(!al_init_image_addon())
    {
        msg_erro("Falha ao inicializar add-on allegro_image.", display);
        retorno = 0;
    }

    if(!al_install_mouse())
    {
        msg_erro("Falha ao inicializar compatibilidade com o mouse.", display);
        retorno = 0;
    }
    if(!al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT))
    {
        msg_erro("Falha ao atribuir ponteiro do mouse.", display);
        retorno = 0;
    }
    al_hide_mouse_cursor(display);

    if(!al_install_keyboard())
    {
        msg_erro("Falha ao inicializar compatibilidade com o teclado.", display);
        retorno = 0;
    }

    if(!al_install_audio())
    {
        msg_erro("Falha ao inicializar add-on de audio.", display);
        retorno = 0;
    }

    if(!al_init_acodec_addon())
    {
        msg_erro("Falha ao inicializar suporte aos arquivos de audio.", display);
        retorno = 0;
    }

    return retorno;
}

void desalocar(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer)
{
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);

    al_uninstall_keyboard();
    al_uninstall_mouse();
}

void textomenu(ALLEGRO_FONT *fontemenu, int sel) // Desenha os elementos do menu, menos aquele selecionado.
/*Parametros: a fonte utilizada no menu. Ela eh um pointer que recebe como 'endereco' o arquivo da fonte e as dimensoes
do texto. Esse conteudo que eh de interesse para essa funcao, portanto isso que deve ser passado. Alem disso, em 'sel' indica-se qual dos textos nao deve ser impresso novamente,
pois eh ele que ja esta mudando de cor (esta selecionado). */
{
    if(sel != 1)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_CENTER, "Novo Jogo");
    if(sel != 2)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 + 1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Carregar");
    if(sel != 3)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 + 2*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Controles");
    if(sel != 4)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 + 3*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "High Scores");
    if(sel != 5)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 + 4*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Creditos");
    if(sel != 6)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 + 5*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Sair");
}

char menu(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer)
/* Tem de passar o timer por referência, porque, caso contrário, ao sair de main ele pararia de contar.
*/
{
    ALLEGRO_BITMAP *fundomenu = NULL;
    ALLEGRO_BITMAP *logo = NULL;
    ALLEGRO_BITMAP *selmenu1 = NULL;
    ALLEGRO_BITMAP *selmenu2 = NULL;
    ALLEGRO_BITMAP *selmenu3 = NULL;
    ALLEGRO_BITMAP *selmenu4 = NULL;
    ALLEGRO_BITMAP *selmenu5 = NULL;
    ALLEGRO_BITMAP *selmenu6 = NULL;
    ALLEGRO_BITMAP *folha_sprites = NULL;
    ALLEGRO_BITMAP *canvas_preto = NULL;
    ALLEGRO_BITMAP *controles = NULL;
    ALLEGRO_BITMAP *creditos = NULL;
    ALLEGRO_BITMAP *borda_texto = NULL;
    ALLEGRO_FONT *fontetexto = NULL;
    ALLEGRO_FONT *fontetextomenor = NULL;
    ALLEGRO_SAMPLE *menu_selec = NULL, *menu_mover = NULL;
    ALLEGRO_EVENT evento;
    char retorno = 'n'; // 'n' de nada, pois ainda não foi selecionado nada no menu.
    int l, h, dim, x0logo, y0logo, tecla = -1, atualizar = 0;
    float r = 249, g = 247, b = 128; // Para a mudanca de cores no texto do menu
    int inc = 1; // IDEM
    int posicao = 1; // Dita a posicao do menu (1 = Novo Jogo, 2 = Carregar, ...)
    //Ratos do fundo:
    int tipo_rato = -1; // O primeiro eh do tipo 0 (anda em cima para a direita); iniciamos com -1 pois, antes de iterar sobre o tipo do rato, eh somado 1 a ele
    int cont_frames = -2*FPS; /* Na primeira iteracao, o cont_frames comeca antes de 0; no programa, se checara se o cont_frames esta negativo. O unico momento que ele eh negativo eh na
    passagem do primeiro rato. Isso foi feito para nao demorar tanto para passar o primeiro rato.
    Logica do cont_frames e do passa_rato:
    cont_frames ativa quando nao ha ratos na tela (passa_rato = 0) e para de contar quando ha ratos na tela (passa_rato = 1).
    A cada TEMPO_RATO_MENU segundos, ele permite a passagem de um rato, parando de contar ate o rato atravessar a tela. */
    int passa_rato = 0;
    int ind_col = 0, num_col = 3, ind_lin = 2, num_lin = 4; // Indice linha inicial eh 2 pois o primeiro sprite que vai aparecer no menu eh o da 3a linha da folha
    int rato_x = 0, rato_y = LARGURA/8, dir = 1;
    int escala_rato = 5; // Para poder mudar o tamanho do rato
    int cont_frames_sprite = 0;
    int rato2_y = ALTURA/4;
    int anim_novo_jogo = 0, ratoN_x = 0, ratoN_y = 0, ind_lin_N = 2, cont_N = 0;
    int canvas_y = -1*ALTURA, canvas_vel = 1, canvas_acel = 1.33;
    int em_controles = 0;
    int em_creditos = 0;

    fundomenu = al_load_bitmap("assets/img/menu_fundo.jpg");
    if (!fundomenu)
    {
        msg_erro("Falha carregando menu_fundo.jpg.", display);
        retorno = 'f'; // 'f' para falha; cai dentro do default no switch. Foi utilizada uma variável retorno aqui para, ao final de menu(), desalocar o que foi carregado.
    }
    else
    {
        logo = al_load_bitmap("assets/img/titulo.png");
        if (!logo)
        {
            msg_erro("Falha carregando titulo.png.", display);
            retorno = 'f'; // 'f' para falha; cai dentro do default no switch.
        }
        else
        {
            fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO, 0);
            if (!fontetexto)
            {
                msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                retorno = 'f';
            }
            else
            {
                fontetextomenor = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO/2, 0);
                if (!fontetextomenor)
                {
                    msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                    retorno = 'f';
                }
                else
                {
                    selmenu1 = al_load_bitmap("assets/img/menu_select_1.png");
                    if (!selmenu1)
                    {
                        msg_erro("Falha carregando menu_select_1.png.", display);
                        retorno = 'f'; // 'f' para falha; cai dentro do default no switch.
                    }
                    else
                    {
                        selmenu2 = al_load_bitmap("assets/img/menu_select_2.png");
                        if (!selmenu2)
                        {
                            msg_erro("Falha carregando menu_select_2.png.", display);
                            retorno = 'f'; // 'f' para falha; cai dentro do default no switch.
                        }
                        else
                        {
                            folha_sprites = al_load_bitmap("assets/img/rato_sprites.png");
                            if(!folha_sprites)
                            {
                                msg_erro("Falha carregando rato_sprites.png.", display);
                                retorno = 'f';
                            }
                            else
                            {
                                canvas_preto = al_create_bitmap(LARGURA, ALTURA);
                                if(!canvas_preto)
                                {
                                    msg_erro("Falha ao criar bitmap.", display);
                                    retorno = 'f';
                                }
                                else
                                {
                                    al_set_target_bitmap(canvas_preto); // Deixa como alvo do desenho o bitmap 'canvas'
                                    al_clear_to_color(al_map_rgb(0,0,0)); // Colore o bitmap
                                    al_set_target_bitmap(al_get_backbuffer(display)); // Volta a deixar como padrão o desenho na janela

                                    controles = al_load_bitmap("assets/img/controles.png");
                                    if(!controles)
                                    {
                                        msg_erro("Falha ao carregar controles.png.", display);
                                        retorno = 'f';
                                    }
                                    else
                                    {
                                        selmenu3 = al_load_bitmap("assets/img/menu_select_3.png");
                                        if(!selmenu3)
                                        {
                                            msg_erro("Falha ao carregar menu_select_3.png.", display);
                                            retorno = 'f';
                                        }
                                        else
                                        {
                                            borda_texto = al_load_bitmap("assets/img/borda_texto.png");
                                            if(!borda_texto)
                                            {
                                                msg_erro("Falha ao carregar borda_texto.png.", display);
                                                retorno = 'f';
                                            }
                                            else
                                            {
                                                selmenu4 = al_load_bitmap("assets/img/menu_select_4.png");
                                                if(!selmenu4)
                                                {
                                                    msg_erro("Falha ao carregar menu_select_4.png.", display);
                                                    retorno = 'f';
                                                }
                                                else
                                                {
                                                    selmenu5 = al_load_bitmap("assets/img/menu_select_5.png");
                                                    if(!selmenu5)
                                                    {
                                                        msg_erro("Falha ao carregar menu_select_5.png.", display);
                                                        retorno = 'f';
                                                    }
                                                    else
                                                    {
                                                        selmenu6 = al_load_bitmap("assets/img/menu_select_6.png");
                                                        if(!selmenu6)
                                                        {
                                                            msg_erro("Falha ao carregar menu_select_6.png.", display);
                                                            retorno = 'f';
                                                        }
                                                        else
                                                        {
                                                            creditos = al_load_bitmap("assets/img/creditos.png");
                                                            if(!creditos)
                                                            {
                                                                msg_erro("Falha ao carregar creditos.png.", display);
                                                                retorno = 'f';
                                                            }
                                                            else
                                                            {
                                                                menu_selec = al_load_sample("assets/audio/menu_lego_sw_selecionar.ogg");
                                                                if(!menu_selec)
                                                                {
                                                                    msg_erro("Falha ao carregar menu_lego_sw_selecionar.ogg.", display);
                                                                    retorno = 'f';
                                                                }
                                                                else
                                                                {
                                                                    menu_mover = al_load_sample("assets/audio/menu_lego_sw_mover.ogg");
                                                                    if(!menu_mover)
                                                                    {
                                                                        msg_erro("Falha ao carregar menu_lego_sw_mover.ogg.", display);
                                                                        retorno = 'f';
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    while(retorno == 'n')
    {
        while(!al_is_event_queue_empty(queue)) /* Enquanto a fila de eventos tiver eventos (retorno da função = 0),
        pegará cada evento da fila e testará seu tipo, tomando as providências adequadas */
        {
            al_wait_for_event(queue, &evento);

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                retorno = 'Q';
            }
            else if(evento.type == ALLEGRO_EVENT_KEY_DOWN) // Se o evento é pressionar uma tecla
            {
                switch(evento.keyboard.keycode)
                {
                    case ALLEGRO_KEY_UP :
                    case ALLEGRO_KEY_W :
                        tecla = 1;
                        break;

                    case ALLEGRO_KEY_DOWN :
                    case ALLEGRO_KEY_S :
                        tecla = 2;
                        break;

                    case ALLEGRO_KEY_ENTER :
                    case ALLEGRO_KEY_PAD_ENTER :
                        tecla = 0;
                        break;

                    case ALLEGRO_KEY_ESCAPE :
                        tecla = 3;
                        break;
                }
                if(anim_novo_jogo == 0 && em_controles == 0 && em_creditos == 0) // Quando estiver na animação de apertar em 'novo jogo', nos controles ou nos créditos, não poderá mexer os botões no menu
                {
                   if(tecla == 1 && posicao > 1) // Seta para cima; se a posicao for 1 e apertar a seta para cima, nao muda de posicao.
                    {
                        posicao--;
                        al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                    else if(tecla == 2 && posicao < 6) // Seta para baixo; se a posicao for 6 e apertar a seta para baixo, nao muda de posicao.
                    {
                        posicao++;
                        al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                    else if(tecla == 0)
                    {
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                        switch(posicao)
                        {
                            case 1 : // Apertou enter no Novo Jogo
                                retorno = 'N';
                                break;

                            case 2 : // Apertou enter no Carregar
                                retorno = 'C';
                                break;

                            case 3 : // Apertou enter no Controles
                                retorno = 'c';
                                break;

                            case 4 : // Apertou enter em High Scores
                                retorno = 'H';
                                break;

                            case 5 : // Apertou enter em Creditos
                                retorno = 'k';
                                break;

                            case 6 : // Apertou enter em Sair
                                retorno = 'Q';
                                break;
                        }
                    }
                }
                else if(em_controles == 1)
                {
                    if(tecla == 3 || tecla == 0)
                    {
                        em_controles = 0;
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                }
                else if(em_creditos == 1)
                {
                    if(tecla == 3 || tecla == 0)
                    {
                        em_creditos = 0;
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                }

                tecla = -1; // Reinicia a variável tecla para a próxima iteração
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                // O seguinte rodara a cada vez que o timer for disparadoal_start_timer(timer);
                atualizar = 1; // Sempre que o timer disparar, deve atualizar a tela (FPS)
                if(g >= 247 || g <= 61) // Garante que as cores fiquem oscilando (toma como base a verde, mas funciona para todas)
                    inc = inc*(-1); // Quando chega em 0 ou em 255, inverte o sentido (na primeira iteracao, ja troca)

                g = g + (186/(FPS*VEL_CORES_MENU))*inc; //Velocidade = quantos segundos demora para alternar entre os extremos das cores
                /*Explicacao matematica:
                (FPS => 60 frames/ticks por segundo, por padrao)
                g: 2 segundos => 60*2 ticks => 186 (nesse tempo, precisa mudar 186 no valor)
                assim, para fazer isso em 2 segundos, precisa de 1 incremento a cada 186/60*2 ticks. Portanto, g=g+(186/60*2)*inc, pois, a cada vez que
                o timer dispara (1 tick), faz esse incremento. Assim, em 2 segundos = 120 ticks, fez 186 de incremento.
                */
                r = r - (5/(FPS*VEL_CORES_MENU))*inc; // eh '-' porque o vermelho precisa aumentar para mudar as cores, enquanto o verde precisa diminuir: (249,247,128) -> (254,61,52) = amarelo -> vermelho
                b = b + (76/(FPS*VEL_CORES_MENU))*inc;
                // Novo problema: r, g e b sao numeros com virgula, que nao cabem no al_map_rgb. Solucao: round()

                if(passa_rato == 0)
                {
                    cont_frames++; // Soh conta frames quando o rato nao esta passando
                }
                else if(passa_rato == 1)
                {
                    rato_x += dir*VEL_RATO;
                }
                if(dir == 1 && rato_x > LARGURA) // Se estiver indo para a esquerda e atravessou toda a tela
                {
                    passa_rato = 0;
                }
                else if(dir == -1 && rato_x < -2*al_get_bitmap_width(folha_sprites)) // Se estiver indo para a direita e atravessou toda a tela (cuidar: sentido das coordenadas no Allegro)
                {
                    passa_rato = 0;
                }
                cont_frames_sprite++;
                // Quando for para lancar um rato:
                if(cont_frames >= TEMPO_RATO_MENU*FPS || cont_frames == -1) // cont_frames => ativa e desativa a passagem do rato; ele so sera -1 na passagem do primeiro rato - isso foi feito para ela nao demorar tanto a vir
                {
                    cont_frames = 0;
                    if(anim_novo_jogo == 0) // Com isso, só passará um >novo< rato no fundo se estiver fora da animação de novo jogo
                        passa_rato = 1;
                    else
                        passa_rato = 0;

                    if(tipo_rato < 3)
                        tipo_rato++; // Alterna entre os tipos 0, 1, 2 e 3 do rato
                    else if(tipo_rato == 3)
                        tipo_rato = 0; // Quando o tipo for 3, o proximo voltara ao tipo 0

                    switch(tipo_rato) // O que cada tipo determina
                    {
                        case 0 : // Em cima, para a direita
                            dir = 1;
                            rato_y = ALTURA/7;
                            escala_rato = 5;
                            rato_x = -2*al_get_bitmap_width(folha_sprites);
                            break;

                        case 1 : // No meio, para a esquerda
                            dir = -1;
                            rato_y = ALTURA/2;
                            escala_rato = 6;
                            rato_x = LARGURA;
                            break;

                        case 2 : // Embaixo, para a esquerda
                            dir = -1;
                            rato_y = ALTURA-ALTURA/3;
                            escala_rato = 4;
                            rato_x = LARGURA;

                        case 3 : // Dois, um em cima e outro embaixo, para a direita
                            dir = 1;
                            escala_rato = 5;
                            rato_y = ALTURA-ALTURA/4; // y do rato de baixo
                            rato_x = -2*al_get_bitmap_width(folha_sprites);
                    }

                    if(dir == 1) // Segundo a direcao que o rato anda, escolhe o sprite correspondente
                        ind_lin = 2;
                    else if(dir == -1)
                        ind_lin = 1;
                }

                if(cont_frames_sprite >= N_FRAMES_SPRITE_RATO) // A cada N_FRAMES_SPRITE_RATO, muda a coluna do sprite
                {
                    cont_frames_sprite = 0; // Reseta a variavel contadora
                    if(ind_col < num_col-1)
                        ind_col++;
                    else
                        ind_col = 0;
                }

                if(anim_novo_jogo == 1)
                {
                    cont_N++; // Conta as frames decorridas desde o início da animação
                }
            }
        }

        if(retorno == 'N') // Só roda 1 vez
        {
            retorno = 'n';
            anim_novo_jogo = 1;
            ratoN_x = -3*al_get_bitmap_width(folha_sprites);
            ratoN_y = ALTURA/6;

        }
        if(retorno == 'c') // Se apertou em controles
        {
            retorno = 'n';
            em_controles = 1;
        }
        if(retorno == 'k') // Se apertou em créditos
        {
            retorno = 'n';
            em_creditos = 1;
        }

        if(atualizar == 1) // Se já tiver tratado todos os eventos (sai do while) e tiver de atualizar a tela (segundo a taxa de FPS), atualiza a tela (desenha tudo na tela a cada frame)
        {
            //Fundo do menu:
            al_clear_to_color(al_map_rgb(66,51,31));
            dim = al_get_bitmap_height(fundomenu); // Como `fundomenu` eh quadrado, tanto faz qual dimensao se pega
            for(l=0; l<ceil((float)LARGURA/dim); l++)
            {
                for(h=0; h<ceil((float)ALTURA/dim); h++)
                {
                    al_draw_bitmap(fundomenu, dim*l, dim*h, 0);
                }
            }
            //Ratos no fundo:
            if(passa_rato == 1)
            {
                al_draw_scaled_bitmap(folha_sprites, (ind_col*al_get_bitmap_width(folha_sprites))/num_col, (ind_lin*al_get_bitmap_height(folha_sprites))/num_lin,
                                      al_get_bitmap_width(folha_sprites)/num_col, al_get_bitmap_height(folha_sprites)/num_lin, rato_x, rato_y,
                                      escala_rato*(al_get_bitmap_width(folha_sprites)/num_col), escala_rato*(al_get_bitmap_height(folha_sprites)/num_lin), 0);
                if(tipo_rato == 3)
                {
                    al_draw_scaled_bitmap(folha_sprites, (ind_col*al_get_bitmap_width(folha_sprites))/num_col, (ind_lin*al_get_bitmap_height(folha_sprites))/num_lin,
                                      al_get_bitmap_width(folha_sprites)/num_col, al_get_bitmap_height(folha_sprites)/num_lin, rato_x-50, rato2_y,
                                      escala_rato*(al_get_bitmap_width(folha_sprites)/num_col), escala_rato*(al_get_bitmap_height(folha_sprites)/num_lin), 0);
                }
            }

            x0logo = LARGURA/2 - al_get_bitmap_width(logo)/2;
            y0logo = ALTURA/4 - al_get_bitmap_height(logo)/2;
            al_draw_bitmap(logo, x0logo, y0logo+50, 0);
            al_draw_text(fontetextomenor, al_map_rgb(249, 247, 128), LARGURA - TAMTEXTO, ALTURA - TAMTEXTO, ALLEGRO_ALIGN_RIGHT, "Selecionar: Enter");
            al_draw_bitmap(borda_texto, LARGURA/2-al_get_bitmap_width(borda_texto)/2, ALTURA/2,0);

            switch(posicao)
            {
                case 1 : // Posicao esta no Novo Jogo
                    textomenu(fontetexto, 1);
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_CENTER, "Novo Jogo");
                    al_draw_bitmap(selmenu1, LARGURA/2 - al_get_bitmap_width(selmenu1)/2, ALTURA/2 - TAMTEXTO/6, 0);
                    break;

                case 2 :
                    textomenu(fontetexto, 2);
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/2, ALTURA/2 + 1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Carregar");
                    al_draw_bitmap(selmenu2, LARGURA/2 - al_get_bitmap_width(selmenu2)/2, ALTURA/2 + 1.5*TAMTEXTO - TAMTEXTO/6, 0);
                    break;

                case 3 :
                    textomenu(fontetexto, 3);
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/2, ALTURA/2 + 2*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Controles");
                    al_draw_bitmap(selmenu3, LARGURA/2 - al_get_bitmap_width(selmenu3)/2, ALTURA/2 + 2*1.5*TAMTEXTO - TAMTEXTO/6, 0);
                    break;

                case 4 :
                    textomenu(fontetexto, 4);
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/2, ALTURA/2 + 3*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "High Scores");
                    al_draw_bitmap(selmenu4, LARGURA/2 - al_get_bitmap_width(selmenu4)/2, ALTURA/2 + 3*1.5*TAMTEXTO - TAMTEXTO/6, 0);
                    break;

                case 5 :
                    textomenu(fontetexto, 5);
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/2, ALTURA/2 + 4*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Creditos");
                    al_draw_bitmap(selmenu5, LARGURA/2 - al_get_bitmap_width(selmenu5)/2, ALTURA/2 + 4*1.5*TAMTEXTO - TAMTEXTO/6, 0);
                    break;

                case 6 :
                    textomenu(fontetexto, 6);
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/2, ALTURA/2 + 5*1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Sair");
                    al_draw_bitmap(selmenu6, LARGURA/2 - al_get_bitmap_width(selmenu6)/2, ALTURA/2 + 5*1.5*TAMTEXTO - TAMTEXTO/6, 0);
                    break;
            }

            if(anim_novo_jogo == 1)
            {
                al_draw_scaled_bitmap(folha_sprites, (ind_col*al_get_bitmap_width(folha_sprites))/num_col, (ind_lin_N*al_get_bitmap_height(folha_sprites))/num_lin,
                                      al_get_bitmap_width(folha_sprites)/num_col, al_get_bitmap_height(folha_sprites)/num_lin, ratoN_x, ratoN_y,
                                      10*(al_get_bitmap_width(folha_sprites)/num_col), 10*(al_get_bitmap_height(folha_sprites)/num_lin), 0);

                if(ratoN_x < LARGURA/2)
                {
                    ratoN_x+=VEL_RATO;
                }
                else
                {
                    ind_lin_N = 3;
                    if(cont_N >= 5.5*FPS) // Após 5,5 segundos
                    {
                        al_draw_bitmap(canvas_preto, 0, canvas_y, 0);
                        if(canvas_y < 0)
                        {
                            canvas_vel+=canvas_acel; // A cada iteração, a velocidade do bloco aumenta de acordo com sua aceleração
                            canvas_y+=canvas_vel;
                        }
                        else if(canvas_y >= 0)
                        {
                            retorno = 'N'; // Após a animação, retornar N de Novo Jogo
                        }
                    }
                }
            }

            if(em_controles == 1)
            {
                al_draw_bitmap(controles, LARGURA/2-al_get_bitmap_width(controles)/2, ALTURA/2-al_get_bitmap_height(controles)/2, 0);
            }

            if(em_creditos == 1)
            {
                al_draw_bitmap(creditos, LARGURA/2-al_get_bitmap_width(creditos)/2, ALTURA/2-al_get_bitmap_height(creditos)/2, 0);
                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), 50+LARGURA/2-al_get_bitmap_width(creditos)/2, ALTURA/2, 0, "Autores:");
                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), 50+LARGURA/2-al_get_bitmap_width(creditos)/2, ALTURA/2+TAMTEXTO, 0, "Helena");
                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), 50+10+LARGURA/2-al_get_bitmap_width(creditos)/2, ALTURA/2+0.3*TAMTEXTO, 0, "              ..");
                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), 50+LARGURA/2-al_get_bitmap_width(creditos)/2, ALTURA/2+2*TAMTEXTO, 0, "Lucca");
            }

            al_flip_display();
            atualizar = 0;
            al_clear_to_color(al_map_rgb(0,0,0)); // Com isso, >a cada frame se faz uma imagem nova<, facilitando a organizacao
        }
    }
    al_destroy_font(fontetexto);
    al_destroy_font(fontetextomenor);
    al_destroy_bitmap(fundomenu);
    al_destroy_bitmap(logo);
    al_destroy_bitmap(selmenu1);
    al_destroy_bitmap(selmenu2);
    al_destroy_bitmap(selmenu3);
    al_destroy_bitmap(selmenu4);
    al_destroy_bitmap(selmenu5);
    al_destroy_bitmap(selmenu6);
    al_destroy_bitmap(folha_sprites);
    al_destroy_bitmap(canvas_preto);
    al_destroy_bitmap(controles);
    al_destroy_bitmap(creditos);
    al_destroy_bitmap(borda_texto);
    al_destroy_sample(menu_mover);
    al_destroy_sample(menu_selec);

    return retorno;
}

SAVE carregar_jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer)
{
    ALLEGRO_BITMAP *fundomenu = NULL;
    ALLEGRO_EVENT evento;
    ALLEGRO_FONT *fontetextomenor = NULL;
    ALLEGRO_FONT *fontetexto = NULL;
    ALLEGRO_FONT *fontetextosave = NULL;
    ALLEGRO_SAMPLE *menu_selec = NULL, *menu_mover = NULL;
    SAVE nulo = {0, 0, 0, 0};
    SAVE retorno = nulo; // Se não tiver sua flag alterada, irá indicar que houve falha na execução da função (flag = 0)
    SAVE save1;
    SAVE save2;
    int executar = 1;
    int posicao = 0; // Posição: 0 => esquerda, 1 => direita
    int tecla = -1;
    int atualizar = 0;
    int dim, l, h;
    float r = 249, g = 247, b = 128;
    int inc = 1;
    int tem_certeza = 0;
    int posicao_del = 1; // Pos. inicial = Nao
    float rdel = 255, gdel = 88, bdel = 120;
    int incdel = 1;

    fundomenu = al_load_bitmap("assets/img/menu_fundo.jpg");
    if (!fundomenu)
    {
        msg_erro("Falha carregando menu_fundo.jpg.", display);
        executar = 0;
    }
    else
    {
        fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO, 0);
        if (!fontetexto)
        {
            msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
            executar = 0;
        }
        else
        {
            fontetextomenor = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO/2, 0);
            if (!fontetextomenor)
            {
                msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                executar = 0;
            }
            else
            {
                fontetextosave = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO/1.5, 0);
                if (!fontetextosave)
                {
                    msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                    executar = 0;
                }
                else
                {
                    menu_selec = al_load_sample("assets/audio/menu_lego_sw_selecionar.ogg");
                    if(!menu_selec)
                    {
                        msg_erro("Falha ao carregar menu_lego_sw_selecionar.ogg.", display);
                        executar = 0;
                    }
                    else
                    {
                        menu_mover = al_load_sample("assets/audio/menu_lego_sw_mover.ogg");
                        if(!menu_mover)
                        {
                            msg_erro("Falha ao carregar menu_lego_sw_mover.ogg.", display);
                            executar = 0;
                        }
                    }
                }
            }
        }
    }

    checaSaves(&save1, &save2);

    al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // Toca o som aqui pois, quando aperta na funcao menu, ele nao chega a tocar

    while(executar == 1)
    {
        while(!al_is_event_queue_empty(queue)) // Checa todos os eventos
        {
            al_wait_for_event(queue, &evento);

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                executar = 0;
            }
            else if(evento.type == ALLEGRO_EVENT_KEY_DOWN) // Se o evento é pressionar uma tecla
            {
                switch(evento.keyboard.keycode)
                {
                    case ALLEGRO_KEY_ENTER :
                    case ALLEGRO_KEY_PAD_ENTER :
                        tecla = 0;
                        break;
                    case ALLEGRO_KEY_ESCAPE :
                        tecla = 3;
                        break;
                    case ALLEGRO_KEY_RIGHT :
                    case ALLEGRO_KEY_D :
                        tecla = 4;
                        break;
                    case ALLEGRO_KEY_LEFT :
                    case ALLEGRO_KEY_A :
                        tecla = 5;
                        break;
                    case ALLEGRO_KEY_DELETE :
                        tecla = 6;
                        break;
                }
                if(tecla == 3) // Se apertar ESC, sai do carregar jogo e retorna a variável SAVE com flag 3
                {
                    executar = 0;
                    retorno.flag = 3;
                }
                else if(tecla == 4 && posicao == 0 && tem_certeza == 0)
                {
                    posicao++;
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                else if(tecla == 5 && posicao == 1 && tem_certeza == 0)
                {
                    posicao--;
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                else if(tecla == 4 && posicao_del == 0 && tem_certeza == 1)
                {
                    posicao_del++;
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                else if(tecla == 5 && posicao_del == 1 && tem_certeza == 1)
                {
                    posicao_del--;
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                else if(tecla == 0)
                {
                    if(posicao == 0 && save1.flag == 1 && tem_certeza == 0) // Se estiver na posição da esquerda, apertar enter e haver jogo salvo naquela posição
                    {
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        retorno = save1;
                        executar = 0;
                    }
                    else if(posicao == 1 && save2.flag == 1 && tem_certeza == 0) // Se estiver na posição da direita, apertar enter e haver jogo salvo naquela posição
                    {
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        retorno = save2;
                        executar = 0;
                    }

                    if(posicao_del == 0 && tem_certeza == 1) // Se estiver no texto de `tem certeza?` e clicar no sim (posicao_del = 0), deleta o save do slot.
                    // Obs.: ele so entra na parte de deletar se houver save para deletar
                    {
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        if(posicao == 0) // Se apertou para deletar o save 1 (posicao 0)
                        // Obs.: Deletar um save = tornar o SAVE nulo (0,0,0,0)
                        {
                            save1 = nulo;
                            if(!salvaSave(save1, "saves/save1.bin"))
                            {
                                msg_erro("Erro ao deletar save1.", display);
                            }

                            posicao_del = 1;
                            tem_certeza = 0;
                        }
                        else if(posicao == 1) // Para deletar o save 2 (posicao 1)
                        {
                            save2 = nulo;
                            if(!salvaSave(save2, "saves/save2.bin"))
                            {
                                msg_erro("Erro ao deletar save2.", display);
                            }

                            posicao_del = 1;
                            tem_certeza = 0;
                        }
                    }
                    else if(posicao_del == 1 && tem_certeza == 1) // Se apertar no nao, sai do 'tem certeza?'
                    {
                        tem_certeza = 0;
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                }
                else if(tecla == 6) // Se apertar para deletar
                {
                    if(posicao == 0 && save1.flag == 1) // No slot 1; se houver jogo salvo nele
                    {
                        tem_certeza = 1;
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                    else if(posicao == 1 && save2.flag == 1) // No slot 2; se houver jogo salvo nele
                    {
                        tem_certeza = 1;
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                }
            tecla = -1; // Reseta a variável tecla
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                atualizar = 1;
                // Para o gradiente:
                if(g >= 247 || g <= 61)
                {
                   inc = inc*(-1);
                }
                g = g + (186/(FPS*VEL_CORES_MENU))*inc;
                r = r - (5/(FPS*VEL_CORES_MENU))*inc;
                b = b + (76/(FPS*VEL_CORES_MENU))*inc;
                // Para o gradiente do aviso de não poder deletar:
                if(tem_certeza == 1)
                { // Vai de 255,88,120 a 249,249,128
                    gdel = gdel + ((247-88)/(FPS*VEL_CORES_MENU))*incdel;
                    rdel = rdel + ((249-255)/(FPS*VEL_CORES_MENU))*incdel;
                    bdel = bdel + ((128-120)/(FPS*VEL_CORES_MENU))*incdel;

                    if(gdel >= 247 || gdel <= 88)
                    {
                        incdel = incdel*(-1);
                    }
                }
            }
        }

        if(atualizar)
        {
            al_clear_to_color(al_map_rgb(66,51,31));
            dim = al_get_bitmap_height(fundomenu); // Como `fundomenu` eh quadrado, tanto faz qual dimensao se pega
            for(l=0; l<ceil((float)LARGURA/dim); l++)
            {
                for(h=0; h<ceil((float)ALTURA/dim); h++)
                {
                    al_draw_bitmap(fundomenu, dim*l, dim*h, 0);
                }
            }

            if(posicao == 0)
            {
                if(tem_certeza == 0)
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 1");
                else
                    al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 1");

                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA-LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 2");
            }
            else if(posicao == 1)
            {
                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 1");

                if(tem_certeza == 0)
                    al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA-LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 2");
                else
                    al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA-LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 2");
            }
            al_draw_text(fontetextomenor, al_map_rgb(249, 247, 128), LARGURA - TAMTEXTO, ALTURA - TAMTEXTO, ALLEGRO_ALIGN_RIGHT, "Voltar: ESC   Selecionar: Enter");

            if(save1.flag == 1)
            {
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "nivel: %d", save1.nivel);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "vidas: %d", save1.vidas);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA/4, ALTURA/3+1.5*TAMTEXTO+2*TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "pontos: %d", save1.pontos);

                if(posicao == 0)
                    al_draw_text(fontetextomenor, al_map_rgb(255, 88, 120), LARGURA-10.75*TAMTEXTO, ALTURA-TAMTEXTO, ALLEGRO_ALIGN_RIGHT, "Deletar: DEL");
            }
            else if(save1.flag == 0)
            {
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "sem jogo");
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "salvo");
            }

            if(save2.flag == 1)
            {
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "nivel: %d", save2.nivel);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "vidas: %d", save2.vidas);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO+2*TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "pontos: %d", save2.pontos);

                if(posicao == 1)
                    al_draw_text(fontetextomenor, al_map_rgb(255, 88, 120), LARGURA-10.75*TAMTEXTO, ALTURA-TAMTEXTO, ALLEGRO_ALIGN_RIGHT, "Deletar: DEL");
            }
            else if(save2.flag == 0)
            {
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "sem jogo");
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "salvo");
            }

            if(tem_certeza == 1)
            {
                al_draw_text(fontetexto, al_map_rgb(255, 88, 120), LARGURA/2, ALTURA-ALTURA/3, ALLEGRO_ALIGN_CENTER, "Tem certeza?");
                if(posicao_del == 0)
                {
                    al_draw_text(fontetexto, al_map_rgb(round(rdel), round(gdel), round(bdel)), LARGURA/3, ALTURA-ALTURA/4, ALLEGRO_ALIGN_LEFT, "Sim");
                    al_draw_text(fontetexto, al_map_rgb(255, 88, 120), LARGURA-LARGURA/3, ALTURA-ALTURA/4, ALLEGRO_ALIGN_RIGHT, "Nao");
                }

                else if(posicao_del == 1)
                {
                    al_draw_text(fontetexto, al_map_rgb(255, 88, 120), LARGURA/3, ALTURA-ALTURA/4, ALLEGRO_ALIGN_LEFT, "Sim");
                    al_draw_text(fontetexto, al_map_rgb(round(rdel), round(gdel), round(bdel)), LARGURA-LARGURA/3, ALTURA-ALTURA/4, ALLEGRO_ALIGN_RIGHT, "Nao");
                }
            }

            al_flip_display();
            atualizar = 0;
            al_clear_to_color(al_map_rgb(0,0,0)); // Faz com que, a cada frame, redesenhe tudo
        }
    }

    al_destroy_bitmap(fundomenu);
    al_destroy_font(fontetexto);
    al_destroy_font(fontetextomenor);
    al_destroy_font(fontetextosave);
    al_destroy_sample(menu_selec);
    al_destroy_sample(menu_mover);

    return retorno;
}

int checaSaves(SAVE *save1, SAVE *save2)
{
    FILE *arq;

    arq = fopen("saves/save1.bin", "rb");
    if(!arq) // Se a leitura falha (ou seja, o arquivo não existe) a função retorna NULL
    {
        save1->flag = 0;
    }
    else // Se saves.bin existir
    {
        if(!fread(save1, sizeof(SAVE), 1, arq)) // Se fread falhar = não tem memória salva na posição no arquivo (não tem save no slot), retorna 0
        {
            save1->flag = 0;
        }
        // Caso não falhe, save1 irá receber a leitura
    }
    fclose(arq);

    arq = fopen("saves/save2.bin", "rb");
    if(!arq) // Se a leitura falha (ou seja, o arquivo não existe) a função retorna NULL
    {
        save2->flag = 0;
    }
    else // Se saves.bin existir
    {
        if(!fread(save2, sizeof(SAVE), 1, arq)) // Se fread falhar = não tem memória salva na posição no arquivo (não tem save no slot), retorna 0
        {
            save2->flag = 0;
        }
        // Caso não falhe, save1 irá receber a leitura
    }
    fclose(arq);

    return 1;
}

int salvaSave(SAVE save, char *diretorio_do_arquivo)
{
    FILE *aberto;
    int retorno = 0;
    int l, c;
    int g;
    /* Debug:
    printf("rato: (%d,%d)\n", save.rato_pos_matriz.x, save.rato_pos_matriz.y);
    printf("flag: %d\n", save.flag);
    printf("nivel: %d\n", save.nivel);
    printf("ptos: %d\n", save.pontos);
    printf("vidas: %d\n", save.vidas);

    for(l=0; l<LINHAS; l++)
    {
        for(c=0; c<COLUNAS-1; c++)
        {
            printf("%c", save.mapa[l][c]);
        }
        printf("\n");
    }*/

    save.flag = 1; // Para indicar que o arquivo sendo salvado eh um save e nao um novo jogo

    aberto = fopen(diretorio_do_arquivo, "wb");
    if(!aberto)
    {
        retorno = 0;
    }
    else
    {
        fwrite(&save, sizeof(SAVE), 1, aberto);
        retorno = 1;
    }
    fclose(aberto);

    return retorno;
}

int mostra_high_scores(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer)
{
    ALLEGRO_BITMAP *fundomenu = NULL;
    ALLEGRO_EVENT evento;
    ALLEGRO_FONT *fontetextomenor = NULL;
    ALLEGRO_FONT *fontetexto = NULL;
    ALLEGRO_SAMPLE *menu_selec = NULL;
    SCORE maisaltas[5];
    int executar = 1;
    int tecla = -1;
    int atualizar = 0;
    int dim, l, h, i;
    int retorno = 1;

    fundomenu = al_load_bitmap("assets/img/menu_fundo.jpg");
    if (!fundomenu)
    {
        msg_erro("Falha carregando menu_fundo.jpg.", display);
        executar = 0;
    }
    else
    {
        fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO, 0);
        if (!fontetexto)
        {
            msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
            executar = 0;
        }
        else
        {
            fontetextomenor = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO/2, 0);
            if (!fontetextomenor)
            {
                msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                executar = 0;
            }
            else
            {
                menu_selec = al_load_sample("assets/audio/menu_lego_sw_selecionar.ogg");
                if(!menu_selec)
                {
                    msg_erro("Falha ao carregar menu_lego_sw_selecionar.ogg.", display);
                    executar = 0;
                }
                else
                {
                    dim = al_get_bitmap_height(fundomenu); // Como `fundomenu` eh quadrado, tanto faz qual dimensao se pega
                }
            }
        }
    }

    if(!leHighScores(maisaltas))
    {
        msg_erro("Erro ao ler pontuacoes mais altas.", display);
    }

    al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

    while(executar == 1)
    {
        while(!al_is_event_queue_empty(queue)) // Checa todos os eventos
        {
            al_wait_for_event(queue, &evento);

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                executar = 0;
                retorno = 0;
            }
            else if(evento.type == ALLEGRO_EVENT_KEY_DOWN) // Se o evento é pressionar uma tecla
            {
                switch(evento.keyboard.keycode)
                {
                    case ALLEGRO_KEY_ESCAPE :
                        tecla = 1;
                        break;
                }
                if(tecla == 1)
                {
                    executar = 0;
                }
                tecla = -1;
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                atualizar = 1;
            }
        }

        if(atualizar == 1)
        {
            al_clear_to_color(al_map_rgb(66,51,31));
            for(l=0; l<ceil((float)LARGURA/dim); l++)
            {
                for(h=0; h<ceil((float)ALTURA/dim); h++)
                {
                    al_draw_bitmap(fundomenu, dim*l, dim*h, 0);
                }
            }

            al_draw_text(fontetextomenor, al_map_rgb(249, 247, 128), LARGURA - TAMTEXTO, ALTURA - TAMTEXTO, ALLEGRO_ALIGN_RIGHT, "Voltar: ESC");

            al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/10, ALLEGRO_ALIGN_CENTER, "High Scores");

            al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/5, ALTURA/4, ALLEGRO_ALIGN_LEFT, "1.");
            al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/5, ALTURA/4+2*TAMTEXTO, ALLEGRO_ALIGN_LEFT, "2.");
            al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/5, ALTURA/4+4*TAMTEXTO, ALLEGRO_ALIGN_LEFT, "3.");
            al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/5, ALTURA/4+6*TAMTEXTO, ALLEGRO_ALIGN_LEFT, "4.");
            al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/5, ALTURA/4+8*TAMTEXTO, ALLEGRO_ALIGN_LEFT, "5.");

            if(maisaltas[0].pontos != 0)
            {
                al_draw_textf(fontetexto, al_map_rgb(107, 54, 172), LARGURA/2, ALTURA/4, ALLEGRO_ALIGN_CENTER, "%6d pontos, por %s", maisaltas[0].pontos, maisaltas[0].nome);
            }
            if(maisaltas[1].pontos != 0)
            {
                al_draw_textf(fontetexto, al_map_rgb(182, 128, 249), LARGURA/2, ALTURA/4+2*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "%6d pontos, por %s", maisaltas[1].pontos, maisaltas[1].nome);
            }
            if(maisaltas[2].pontos != 0)
            {
                al_draw_textf(fontetexto, al_map_rgb(160, 112, 147), LARGURA/2, ALTURA/4+4*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "%6d pontos, por %s", maisaltas[2].pontos, maisaltas[2].nome);
            }
            if(maisaltas[3].pontos != 0)
            {
                al_draw_textf(fontetexto, al_map_rgb(204, 211, 128), LARGURA/2, ALTURA/4+6*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "%6d pontos, por %s", maisaltas[3].pontos, maisaltas[3].nome);
            }
            if(maisaltas[4].pontos != 0)
            {
                al_draw_textf(fontetexto, al_map_rgb(181, 172, 111), LARGURA/2, ALTURA/4+8*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "%6d pontos, por %s", maisaltas[4].pontos, maisaltas[4].nome);
            }

            al_flip_display();
            atualizar = 0;
            al_clear_to_color(al_map_rgb(0,0,0));
        }
    }

    al_destroy_bitmap(fundomenu);
    al_destroy_font(fontetexto);
    al_destroy_font(fontetextomenor);
    al_destroy_sample(menu_selec);

    return retorno; // Se apertou no X, retorna 0; se nao, retorna 1.
}

int salvarScore(SCORE pontuacao)
{
    /* Funcao que recebe uma pontuacao, checa se ela deve entrar nas high scores, a coloca se necessario e ordena o arquivo final, do maior para o menor.
    Caso nao haja pontuacoes para botar no arquivo, preenche as posicoes com scores nulos. */
    FILE *arquivo;
    SCORE auxiliar[5];
    SCORE nulo = {"", 0};
    SCORE tmp, menor;
    int retorno = 1, contador, i, j, indice_menor;

    arquivo = fopen("saves/high_scores.bin", "r+b"); // Tenta abrir para leitura e escrita (sem recriar o arquivo)
    if(!arquivo) // Se o arquivo nao existir
    {
        arquivo = fopen("saves/high_scores.bin", "wb"); // Cria o arquivo
        if(!arquivo)
        {
            retorno = 0;
        }

        for(i=0; i<5; i++) // Preenche o arquivo com pontuacoes nulas
        {
            fwrite(&nulo, sizeof(SCORE), 1, arquivo);
        }

        fclose(arquivo);
        fopen("saves/high_scores.bin", "r+b"); // Reabre o arquivo, para leitura e escrita
    }

    contador = 0;
    while(!feof(arquivo))
    {
        if(fread(&auxiliar[contador], sizeof(SCORE), 1, arquivo) == 1)
        {
            contador++;
        }
    }
    for(i=contador; i<5; i++) // Preenche as outras posicoes do vetor com um nulo
    {
        auxiliar[i] = nulo;
    }

    //Colocar o score entre os high scores, caso ele seja maior que algum valor. Colocar no lugar do menor valor
    menor = auxiliar[0];
    indice_menor = 0;
    for(i=0; i<5; i++) // Acha o menor
    {
        if(auxiliar[i].pontos < menor.pontos)
        {
            menor = auxiliar[i];
            indice_menor = i;
        }
    }
    if(pontuacao.pontos > menor.pontos) // Se o score recebido for maior que o menor high score, o coloca entre os high scores
    {
        auxiliar[indice_menor] = pontuacao;
    }

    //Ordenar:
    for (i=0; i<5; i++)
    {
        j=0;
        for (j=0; j<5; j++)
        {
            if (auxiliar[j].pontos < auxiliar[i].pontos)
            {
                tmp = auxiliar[i];
                auxiliar[i] = auxiliar[j];
                auxiliar[j] = tmp;
            }
        }
    }

    rewind(arquivo);

    contador = 0;
    while(contador<5)
    {
        if(fwrite(&auxiliar[contador], sizeof(SCORE), 1, arquivo) != 1)
        {
            retorno = 0;
        }
        else
        {
            contador++;
        }
    }

    fclose(arquivo);

    return retorno;
}

int leHighScores(SCORE *maisaltas)
{
    FILE *arquivo;
    int retorno = 1, i, contador;
    SCORE nulo = {"", 0};

    arquivo = fopen("saves/high_scores.bin", "rb");
    if(!arquivo) // Se o arquivo nao existir
    {
        arquivo = fopen("saves/high_scores.bin", "wb"); // Cria o arquivo
        if(!arquivo)
        {
            retorno = 0;
        }

        for(i=0; i<5; i++) // Preenche o arquivo com pontuacoes nulas
        {
            fwrite(&nulo, sizeof(SCORE), 1, arquivo);
        }

        fclose(arquivo);
        fopen("saves/high_scores.bin", "rb"); // Reabre o arquivo, para leitura
    }

    contador = 0;
    while(!feof(arquivo))
    {
        fread(maisaltas+contador, sizeof(SCORE), 1, arquivo);
        contador++;
    }

    return retorno;
}

int jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, SAVE save) // 'save' é uma variável para quando um jogo salvo for ser jogado, No arquivo, haverá o nível, a pontuação e as vidas em que o jogo salvo parou.
{
    // Variáveis:
    ALLEGRO_EVENT evento;
    ALLEGRO_BITMAP *gato_alt_spr_folha = NULL;
    ALLEGRO_BITMAP *gatos_rostos = NULL, *gatos_rostos_sombra = NULL;
    ALLEGRO_BITMAP *cachorro_barra_tempo = NULL, *barra_tempo = NULL;
    ALLEGRO_BITMAP *rato_cara = NULL;
    ALLEGRO_BITMAP *ganha_vida = NULL;
    ALLEGRO_BITMAP *bg_menu_ingame = NULL;
    ALLEGRO_FONT *fonte_placar = NULL;
    ALLEGRO_FONT *fonte_menu = NULL;
    ALLEGRO_SAMPLE *som_porta_block = NULL;
    ALLEGRO_SAMPLE *som_qjo = NULL, *som_perde_vida = NULL, *som_osso = NULL, *som_miado1 = NULL, *som_miado2 = NULL, *som_ganha_vida = NULL;
    ALLEGRO_SAMPLE *menu_selec = NULL, *menu_mover = NULL;
    SPRITE rato_spr = {3, 4, 1, 2, NULL};
    SPRITE gatos_spr[4]; // 0 = verm, 1 = amar, 2 = cinza, 3 = branco
    SPRITE cachorro_spr = {5, 4, 1, 2, NULL};
    CENARIO casa, jardim, lago, mal_assombrado;
    CENARIO ativo;
    RATO rato = {0, 0};
    RATO pos_inicial_rato;
    PORTA porta[PORTAS];
    GATO gatos[4]; // 0 = verm, 1 = amar, 2 = cinza, 3 = branco
    GATO pos_inicial_gato[4];
    SAVE estado_atual = save;
    char nome_arq_nivel[20] = "niveis/nivel00.txt";
    int executar = 1, jogando=1;
    int retorno = 1;
    int jogo_pausado = 0;
    int comeu_osso = 0;
    int morreu = 0;
    int atualizar = 0;
    int menu = 0;
    int direita = 0, esquerda = 0, cima = 0, baixo = 0;
    int tecla = -1; // Para funções que ocorrem uma vez e apenas ao apertar uma tecla (sem segurar)
    int cont_frames_sprite = 0, cont_frames_spr_gatos = 0, cont_osso = 0;
    int c, l;
    int i, j, k, contcima = 0, contbaixo = 0, maxqueijo;
    int g;
    int bicho_na_porta = 0, p;
    char selecao;
    int gato_morto[4] = {0,0,0,0};
    int pode_ganhar_vida = 1;
    int buffer_ptos = 0;
    int anim_vida = 0, cont_anim_vida = 0;
    float red = 249, gr = 247, bl = 128; // Para a mudanca de cores no texto do menu
    int inc = 1; // IDEM
    int posicao_pause = 1;
    int nivel_inicial = save.nivel;
    int acabou_os_niveis = 0;
    char mapa_trash[LINHAS][COLUNAS];

    // Inicializações de assets:
    casa.chao = al_load_bitmap("assets/img/jogo/tile_chao.jpg");
    if(!casa.chao)
    {
        msg_erro("Falha ao carregar tile_chao.jpg", display);
        executar = 0;
    }
    else
    {
        casa.parede = al_load_bitmap("assets/img/jogo/tile_parede.jpg");
        if(!casa.parede)
        {
            msg_erro("Falha ao carregar tile_parede.jpg", display);
            executar = 0;
        }
        else
        {
            casa.porta = al_load_bitmap("assets/img/jogo/tile_porta.jpg");
            if(!casa.porta)
            {
                msg_erro("Falha ao carregar tile_porta.jpg", display);
                executar = 0;
            }
            else
            {
                rato_spr.folha = al_load_bitmap("assets/img/jogo/rato_sprite.png");
                if(!rato_spr.folha)
                {
                    msg_erro("Falha ao carregar rato_sprite.png", display);
                    executar = 0;
                }
                else
                {
                    gatos_spr[1].folha = al_load_bitmap("assets/img/jogo/gato_amarelo_sprite.png");
                    if(!gatos_spr[1].folha)
                    {
                        msg_erro("Falha ao carregar gato_amarelo_sprite.png", display);
                        executar = 0;
                    }
                    else
                    {
                        gatos_spr[3].folha = al_load_bitmap("assets/img/jogo/gato_branco_sprite.png");
                        if(!gatos_spr[3].folha)
                        {
                            msg_erro("Falha ao carregar gato_branco_sprite.png", display);
                            executar = 0;
                        }
                        else
                        {
                            gatos_spr[2].folha = al_load_bitmap("assets/img/jogo/gato_cinza_sprite.png");
                            if(!gatos_spr[2].folha)
                            {
                                msg_erro("Falha ao carregar gato_cinza_sprite.png", display);
                                executar = 0;
                            }
                            else
                            {
                                gatos_spr[0].folha = al_load_bitmap("assets/img/jogo/gato_vermelho_sprite.png");
                                if(!gatos_spr[0].folha)
                                {
                                    msg_erro("Falha ao carregar gato_vermelho_sprite.png", display);
                                    executar = 0;
                                }
                                else
                                {
                                    gato_alt_spr_folha = al_load_bitmap("assets/img/jogo/gato_alt-preto_sprite.png");
                                    if(!gato_alt_spr_folha)
                                    {
                                        msg_erro("Falha ao carregar gato_alt-preto_sprite.png", display);
                                        executar = 0;
                                    }
                                    else
                                    {
                                        fonte_placar = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO/1.2, 0);
                                        if(!fonte_placar)
                                        {
                                            msg_erro("Falha ao carregar vcr_osd_mono.ttf", display);
                                            executar = 0;
                                        }
                                        else
                                        {
                                            fonte_menu = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO, 0);
                                            if(!fonte_menu)
                                            {
                                                msg_erro("Falha ao carregar vcr_osd_mono.ttf", display);
                                                executar = 0;
                                            }
                                            else
                                            {
                                                jardim.chao = al_load_bitmap("assets/img/jogo/tile_3_chao.jpg");
                                                if(!jardim.chao)
                                                {
                                                    msg_erro("Falha ao carregar tile_3_chao.jpg", display);
                                                    executar = 0;
                                                }
                                                else
                                                {
                                                    jardim.parede = al_load_bitmap("assets/img/jogo/tile_3_parede.jpg");
                                                    if(!jardim.parede)
                                                    {
                                                        msg_erro("Falha ao carregar tile_3_parede.jpg", display);
                                                        executar = 0;
                                                    }
                                                    else
                                                    {
                                                        jardim.porta = al_load_bitmap("assets/img/jogo/tile_3_porta.jpg");
                                                        if(!jardim.porta)
                                                        {
                                                            msg_erro("Falha ao carregar tile_3_porta.jpg", display);
                                                            executar = 0;
                                                        }
                                                        else
                                                        {
                                                            lago.chao = al_load_bitmap("assets/img/jogo/tile_2_chao.jpg");
                                                            if(!lago.chao)
                                                            {
                                                                msg_erro("Falha ao carregar tile_2_chao.jpg", display);
                                                                executar = 0;
                                                            }
                                                            else
                                                            {
                                                                lago.parede = al_load_bitmap("assets/img/jogo/tile_2_parede.jpg");
                                                                if(!lago.parede)
                                                                {
                                                                    msg_erro("Falha ao carregar tile_2_parede.jpg", display);
                                                                    executar = 0;
                                                                }
                                                                else
                                                                {
                                                                    lago.porta = al_load_bitmap("assets/img/jogo/tile_2_porta.jpg");
                                                                    if(!lago.porta)
                                                                    {
                                                                        msg_erro("Falha ao carregar tile_2_porta.jpg", display);
                                                                        executar = 0;
                                                                    }
                                                                    else
                                                                    {
                                                                        mal_assombrado.chao = al_load_bitmap("assets/img/jogo/tile_4_chao.jpg");
                                                                        if(!mal_assombrado.chao)
                                                                        {
                                                                            msg_erro("Falha ao carregar tile_4_chao.jpg", display);
                                                                            executar = 0;
                                                                        }
                                                                        else
                                                                        {
                                                                            mal_assombrado.parede = al_load_bitmap("assets/img/jogo/tile_4_parede.jpg");
                                                                            if(!mal_assombrado.parede)
                                                                            {
                                                                                msg_erro("Falha ao carregar tile_4_parede.jpg", display);
                                                                                executar = 0;
                                                                            }
                                                                            else
                                                                            {
                                                                                mal_assombrado.porta = al_load_bitmap("assets/img/jogo/tile_4_porta.jpg");
                                                                                if(!mal_assombrado.porta)
                                                                                {
                                                                                    msg_erro("Falha ao carregar tile_4_porta.jpg", display);
                                                                                    executar = 0;
                                                                                }
                                                                                else
                                                                                {
                                                                                    som_porta_block = al_load_sample("assets/audio/portablock_zapsplat.ogg");
                                                                                    if(!som_porta_block)
                                                                                    {
                                                                                        msg_erro("'portablock_zapsplat.ogg' falhou ao ser carregado.", display);
                                                                                        executar = 0;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        cachorro_spr.folha = al_load_bitmap("assets/img/jogo/cachorro_sprite.png");
                                                                                        if(!cachorro_spr.folha)
                                                                                        {
                                                                                            msg_erro("Falha ao carregar cachorro_sprite.png", display);
                                                                                            executar = 0;
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            gatos_rostos = al_load_bitmap("assets/img/jogo/gatos_rostos.png");
                                                                                            if(!gatos_rostos)
                                                                                            {
                                                                                                msg_erro("Falha ao carregar gatos_rostos.png", display);
                                                                                                executar = 0;
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                gatos_rostos_sombra = al_load_bitmap("assets/img/jogo/gato_rosto_shadow.png");
                                                                                                if(!gatos_rostos_sombra)
                                                                                                {
                                                                                                    msg_erro("Falha ao carregar gato_rosto_shadow.png", display);
                                                                                                    executar = 0;
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    cachorro_barra_tempo = al_load_bitmap("assets/img/jogo/cachorro_barra_tempo.png");
                                                                                                    if(!cachorro_barra_tempo)
                                                                                                    {
                                                                                                        msg_erro("Falha ao carregar cachorro_barra_tempo.png", display);
                                                                                                        executar = 0;
                                                                                                    }
                                                                                                    else
                                                                                                    {
                                                                                                        rato_cara = al_load_bitmap("assets/img/jogo/rato_rosto.png");
                                                                                                        if(!rato_cara)
                                                                                                        {
                                                                                                            msg_erro("Falha ao carregar rato_rosto.png", display);
                                                                                                            executar = 0;
                                                                                                        }
                                                                                                        else
                                                                                                        {
                                                                                                            som_qjo = al_load_sample("assets/audio/comeuqjo_zapsplat.ogg");
                                                                                                            if(!som_qjo)
                                                                                                            {
                                                                                                                msg_erro("Falha ao carregar comeuqjo_zapsplat.ogg", display);
                                                                                                                executar = 0;
                                                                                                            }
                                                                                                            else
                                                                                                            {
                                                                                                                som_perde_vida = al_load_sample("assets/audio/perdevida_zapsplat.ogg");
                                                                                                                if(!som_perde_vida)
                                                                                                                {
                                                                                                                    msg_erro("Falha ao carregar perdevida_zapsplat.ogg", display);
                                                                                                                    executar = 0;
                                                                                                                }
                                                                                                                else
                                                                                                                {
                                                                                                                    som_osso = al_load_sample("assets/audio/latido_zapsplat.ogg");
                                                                                                                    if(!som_osso)
                                                                                                                    {
                                                                                                                        msg_erro("Falha ao carregar latido_zapsplat.ogg", display);
                                                                                                                        executar = 0;
                                                                                                                    }
                                                                                                                    else
                                                                                                                    {
                                                                                                                        som_miado1 = al_load_sample("assets/audio/miado1_zapsplat.ogg");
                                                                                                                        if(!som_miado1)
                                                                                                                        {
                                                                                                                            msg_erro("Falha ao carregar miado1_zapsplat.ogg", display);
                                                                                                                            executar = 0;
                                                                                                                        }
                                                                                                                        else
                                                                                                                        {
                                                                                                                            som_miado2 = al_load_sample("assets/audio/miado2_zapsplat.ogg");
                                                                                                                            if(!som_miado2)
                                                                                                                            {
                                                                                                                                msg_erro("Falha ao carregar miado2_zapsplat.ogg", display);
                                                                                                                                executar = 0;
                                                                                                                            }
                                                                                                                            else
                                                                                                                            {
                                                                                                                                som_ganha_vida = al_load_sample("assets/audio/1up_stampclubp.ogg");
                                                                                                                                if(!som_ganha_vida)
                                                                                                                                {
                                                                                                                                    msg_erro("Falha ao carregar 1up_stampclubp.ogg", display);
                                                                                                                                    executar = 0;
                                                                                                                                }
                                                                                                                                else
                                                                                                                                {
                                                                                                                                    ganha_vida = al_load_bitmap("assets/img/jogo/ganha_vida.png");
                                                                                                                                    if(!ganha_vida)
                                                                                                                                    {
                                                                                                                                        msg_erro("Falha ao carregar ganha_vida.png", display);
                                                                                                                                        executar = 0;
                                                                                                                                    }
                                                                                                                                    else
                                                                                                                                    {
                                                                                                                                        bg_menu_ingame = al_load_bitmap("assets/img/jogo/bg_menu_ingame.png");
                                                                                                                                        if(!bg_menu_ingame)
                                                                                                                                        {
                                                                                                                                            msg_erro("Falha ao carregar bg_menu_ingame.png", display);
                                                                                                                                            executar = 0;
                                                                                                                                        }
                                                                                                                                        else
                                                                                                                                        {
                                                                                                                                            menu_selec = al_load_sample("assets/audio/menu_lego_sw_selecionar.ogg");
                                                                                                                                            if(!menu_selec)
                                                                                                                                            {
                                                                                                                                                msg_erro("Falha ao carregar menu_lego_sw_selecionar.ogg.", display);
                                                                                                                                                executar = 0;
                                                                                                                                            }
                                                                                                                                            else
                                                                                                                                            {
                                                                                                                                                menu_mover = al_load_sample("assets/audio/menu_lego_sw_mover.ogg");
                                                                                                                                                if(!menu_mover)
                                                                                                                                                {
                                                                                                                                                    msg_erro("Falha ao carregar menu_lego_sw_mover.ogg.", display);
                                                                                                                                                    executar = 0;
                                                                                                                                                }
                                                                                                                                            }
                                                                                                                                        }
                                                                                                                                    }
                                                                                                                                }
                                                                                                                            }
                                                                                                                        }
                                                                                                                    }
                                                                                                                }
                                                                                                            }
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    barra_tempo = al_create_bitmap(75, 19);
    al_set_target_bitmap(barra_tempo);
    al_clear_to_color(al_map_rgb(0,50,255));
    al_set_target_bitmap(al_get_backbuffer(display));

    casa.cor_fundo = al_map_rgb(135,89,51);
    jardim.cor_fundo = al_map_rgb(40,185,65);
    lago.cor_fundo = al_map_rgb(38,140,214);
    mal_assombrado.cor_fundo = al_map_rgb(0,0,0);

    while (executar==1)
    {
        //Carregar musicas dos cenarios (deve ser antes de escolher o cenário ativo):
        if(estado_atual.nivel%4 == 3)
        {
            lago.musica = NULL;
            lago.musica = al_load_audio_stream("assets/audio/sonic3_hidrocity_act1.ogg", 4, 1024);
            if(!lago.musica)
            {
                msg_erro("'sonic3_hidrocity_act1.ogg' falhou ao ser carregado.", display);
                executar = 0;
            }
        }
        else if(estado_atual.nivel%4 == 0)
        {
            mal_assombrado.musica = NULL;
            mal_assombrado.musica = al_load_audio_stream("assets/audio/evil_mortys_theme.ogg", 4, 1024);
            if(!mal_assombrado.musica)
            {
                msg_erro("'evil_mortys_theme.ogg' falhou ao ser carregado.", display);
                executar = 0;
            }
        }
        else if(estado_atual.nivel%4 == 2)
        {
            jardim.musica = NULL;
            jardim.musica = al_load_audio_stream("assets/audio/yoshis_island_flower_garden.ogg", 4, 1024);
            if(!jardim.musica)
            {
                msg_erro("'yoshis_island_flower_garden.ogg' falhou ao ser carregado.", display);
                executar = 0;
            }
        }
        else if(estado_atual.nivel%4 == 1)
        {
            casa.musica = NULL;
            casa.musica = al_load_audio_stream("assets/audio/donkey_kong_94_level_9_8.ogg", 4, 1024);
            if(!casa.musica)
            {
                msg_erro("'donkey_kong_94_level_9_8.ogg' falhou ao ser carregado.", display);
                executar = 0;
            }
        }

        //Quando abre um jogo salvo, carrega o mapa direto do save (e não do arquivo texto de nivel)
        if(!(estado_atual.nivel == nivel_inicial && save.flag == 1)) // Se o nivel atual >>não<< for o nivel no qual o jogo foi salvo
        {
             // Como eh um nivel novo, carrega o mapa do arquivo:
            arquivo_nivel(estado_atual.nivel, nome_arq_nivel);
            if(carregar_mapa(nome_arq_nivel, estado_atual.mapa, &estado_atual.rato_pos_matriz, estado_atual.gatos_matriz) == 0) // Se tiver acabado os níveis
            {
                acabou_os_niveis = 1;
                al_set_audio_stream_playing(ativo.musica, false);
                if(tela_zerar_jogo(display, queue, timer, estado_atual.pontos, estado_atual.vidas) == 0)
                {
                    retorno = 0;
                }
                executar = 0;
                jogando = 0;
            }
            else
            {
                pos_inicial_rato.x=estado_atual.rato_pos_matriz.x; //Grava posicao incial do rato no nivel como sendo a do arquivo texto
                pos_inicial_rato.y=estado_atual.rato_pos_matriz.y;

                for (g=0; g<4; g++) //Grava posicao inical do gato no nivel como sendo a do arquivo texto
                {
                    pos_inicial_gato[g].x=estado_atual.gatos_matriz[g].x;
                    pos_inicial_gato[g].y=estado_atual.gatos_matriz[g].y;
                }
            }
        }
        else // Se for o nivel no qual o jogo foi salvo, deve só pegar do arquivo nivel as posicoes iniciais do rato e dos gatos
        {
            arquivo_nivel(estado_atual.nivel, nome_arq_nivel);
            carregar_mapa(nome_arq_nivel, mapa_trash, &pos_inicial_rato, pos_inicial_gato); // Grava as posicoes iniciais apos morrer de gatos e rato como sendo as do arquivo texto
        }

        if(acabou_os_niveis == 0) // Se houver nível
        {
            jogando = 1; // Quando ele passa de nível, jogando = 0. Para voltar ao jogo, jogando = 1.

            // Alternar entre cenários:
            if(estado_atual.nivel%4 == 0) // A cada 4 níveis (4, 8, 12, 16, 20, 24, ...)
            {
                ativo = mal_assombrado;
            }
            else if(estado_atual.nivel%4 == 1) // A cada 4 níveis (1, 5, 9, ... ou 1, 6, 11, 16, 21, 26, ...)
            {
                ativo = casa;
            }
            else if(estado_atual.nivel%4 == 2) // A cada 4 niveis (2, 6, 10, 14, 18, ...)
            {
                ativo = jardim;
            }
            else if(estado_atual.nivel%4 == 3) // A cada 4 niveis (3, 7, 11, 15, ...)
            {
                ativo = lago;
            }

            if(tela_nivel_vidas(display, queue, timer, ativo.cor_fundo, estado_atual.nivel, estado_atual.vidas) == 0) // Se clicou para fechar o jogo
            {
                jogando = 0;
                executar = 0;
                retorno = 0;
            }

            //Define o volume da musica:
            al_set_audio_stream_gain(ativo.musica, VOLUME_MUSICA_JOGO);
            //Liga a musica de fundo no mixer:
            al_attach_audio_stream_to_mixer(ativo.musica, al_get_default_mixer());
            //Define que a musica de fundo vai tocar no modo repeat:
            al_set_audio_stream_playmode(ativo.musica, ALLEGRO_PLAYMODE_LOOP);

            //Calcula o número de queijos disponíveis no inicio do jogo (pode alterar caso for um jogo salvo, por isso não pode ser global)
            maxqueijo=0;
            for (i=0; i<LINHAS; i++)
            {
                j=0;
                for (j=0; j<(COLUNAS-1); j++)
                {
                    if (estado_atual.mapa[i][j]=='Q')
                    {
                        maxqueijo++;
                    }
                }
            }
            //Buffer de pontos, para ganhar 1 vida a cada 3000 pontos:
            buffer_ptos = estado_atual.pontos%3000;

            //Posiciona o rato na tela de acordo com as posicoes no nivel:
            rato.x = 5+estado_atual.rato_pos_matriz.x*TILESIZE;
            rato.y = 101+estado_atual.rato_pos_matriz.y*TILESIZE;

            //Posiciona os gatos na tela de acordo com suas posicoes no nivel:
            for(g=0; g<4; g++)
            {
                gatos[g].x = 5+estado_atual.gatos_matriz[g].x*TILESIZE;
                gatos[g].y = 101+estado_atual.gatos_matriz[g].y*TILESIZE;

                gatos_spr[g].ind_col = 1;
                gatos_spr[g].ind_lin = 2;
                gatos_spr[g].num_col = 3;
                gatos_spr[g].num_lin = 4;

                //Dá como inicial uma direção que esteja livre:
                if(estado_atual.mapa[estado_atual.gatos_matriz[g].y][estado_atual.gatos_matriz[g].x+1] != 'X' && estado_atual.mapa[estado_atual.gatos_matriz[g].y][estado_atual.gatos_matriz[g].x+1] != 'T')
                {
                    gatos[g].direcao = 1; // Direita
                }
                else if(estado_atual.mapa[estado_atual.gatos_matriz[g].y][estado_atual.gatos_matriz[g].x-1] != 'X' && estado_atual.mapa[estado_atual.gatos_matriz[g].y][estado_atual.gatos_matriz[g].x-1] != 'T')
                {
                    gatos[g].direcao = 2; // Esquerda
                }
                else if(estado_atual.mapa[estado_atual.gatos_matriz[g].y+1][estado_atual.gatos_matriz[g].x] != 'X' && estado_atual.mapa[estado_atual.gatos_matriz[g].y+1][estado_atual.gatos_matriz[g].x] != 'T')
                {
                    gatos[g].direcao = 4; // Baixo
                }
                else if(estado_atual.mapa[estado_atual.gatos_matriz[g].y-1][estado_atual.gatos_matriz[g].x] != 'X' && estado_atual.mapa[estado_atual.gatos_matriz[g].y-1][estado_atual.gatos_matriz[g].x] != 'T')
                {
                    gatos[g].direcao = 3; // Cima
                }
            }

            //Zera os comandos ativos antes de passar de nivel:
            direita = 0;
            esquerda = 0;
            cima = 0;
            baixo = 0;
        }

        while (jogando==1) //Enquanto não mudar de nível; jogando=0 quando acabam os queijos
        {
            if(morreu == 1)
            {
                if(estado_atual.vidas > 0)
                {
                    al_set_audio_stream_playing(ativo.musica, false);
                    if(tela_nivel_vidas(display, queue, timer, ativo.cor_fundo, estado_atual.nivel, estado_atual.vidas) == 0)
                    {
                        retorno = 0;
                    }
                    al_set_audio_stream_playing(ativo.musica, true);

                    //Posiciona o rato na tela de acordo com as posicoes iniciais do nivel
                    rato.x = 5+pos_inicial_rato.x*TILESIZE;
                    rato.y = 101+pos_inicial_rato.y*TILESIZE;

                    direita = 0;
                    esquerda = 0;
                    cima = 0;
                    baixo = 0;

                    //Posiciona os gatos na tela de acordo com suas posicoes iniciais no nivel:
                    for(g=0; g<4; g++)
                    {
                        gatos[g].x = 5+pos_inicial_gato[g].x*TILESIZE;
                        gatos[g].y = 101+pos_inicial_gato[g].y*TILESIZE;

                        //Dá como inicial uma direção que esteja livre:
                        if(estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x+1] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x+1] != 'T')
                        {
                            gatos[g].direcao = 1; // Direita
                        }
                        else if(estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x-1] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x-1] != 'T')
                        {
                            gatos[g].direcao = 2; // Esquerda
                        }
                        else if(estado_atual.mapa[pos_inicial_gato[g].y+1][pos_inicial_gato[g].x] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y+1][pos_inicial_gato[g].x] != 'T')
                        {
                            gatos[g].direcao = 4; // Baixo
                        }
                        else if(estado_atual.mapa[pos_inicial_gato[g].y-1][pos_inicial_gato[g].x] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y-1][pos_inicial_gato[g].x] != 'T')
                        {
                            gatos[g].direcao = 3; // Cima
                        }
                    }
                }
                else if (estado_atual.vidas==0) //Confere se ainda tem vidas
                {
                    al_set_audio_stream_playing(ativo.musica, false);
                    if(tela_game_over(display, queue, timer, estado_atual.pontos) == 0)
                    {
                        retorno = 0; // Se clicou para fechar o jogo
                    }
                    else if(tela_salvar_score(display, queue, timer, estado_atual.pontos) == 0) // Entra na tela para salvar a pontuacao no jogo
                    {
                        retorno = 0; // IDEM
                    }
                    executar=0;
                    jogando=0;
                }

                morreu = 0;
            }

            while(!al_is_event_queue_empty(queue))
            {
                al_wait_for_event(queue, &evento);

                if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                {
                    if(al_show_native_message_box(display, "Mouse Trap", "Tem certeza que quer fechar o jogo?", "Todo o progresso nao salvo sera perdido.", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL))
                    { // Se apertar em "sim" (retorno = 1)
                        executar = 0; // Fecha o jogo
                        jogando = 0;
                        retorno = 0;
                    }
                }
                else if(evento.type == ALLEGRO_EVENT_KEY_DOWN) // Apertar a tecla
                {
                    if(jogo_pausado == 0)
                    {
                        switch(evento.keyboard.keycode)
                        {
                            case ALLEGRO_KEY_TAB :
                                jogo_pausado=1; // Menu
                                al_set_audio_stream_gain(ativo.musica, 0.25*VOLUME_MUSICA_JOGO); // Diminui o volume da musica
                                break;

                            case ALLEGRO_KEY_D :
                            case ALLEGRO_KEY_RIGHT :
                                direita = 1;
                                break;

                            case ALLEGRO_KEY_A :
                            case ALLEGRO_KEY_LEFT :
                                esquerda = 1;
                                break;

                            case ALLEGRO_KEY_W :
                            case ALLEGRO_KEY_UP :
                                cima = 1;
                                break;

                            case ALLEGRO_KEY_S :
                            case ALLEGRO_KEY_DOWN :
                                baixo = 1;
                                break;

                            case ALLEGRO_KEY_B :
                                tecla = 8;
                                break;
                        }
                    }
                    else if(jogo_pausado == 1)
                    {
                        switch(evento.keyboard.keycode)
                        {
                            case ALLEGRO_KEY_UP :
                            case ALLEGRO_KEY_W :
                                if(posicao_pause > 1) // Se for maior que 1 (pra baixo de 'Voltar')
                                {
                                    posicao_pause--;
                                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                                }
                                break;

                            case ALLEGRO_KEY_DOWN :
                            case ALLEGRO_KEY_S :
                                if(posicao_pause < 4) // Se for menor que 4 (pra cima de 'Menu Inicial')
                                {
                                    posicao_pause++;
                                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                                }
                                break;

                            case ALLEGRO_KEY_ESCAPE :
                                jogo_pausado = 0;
                                al_set_audio_stream_gain(ativo.musica, VOLUME_MUSICA_JOGO);
                                al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                                posicao_pause = 1;
                                break;

                            case ALLEGRO_KEY_ENTER :
                            case ALLEGRO_KEY_PAD_ENTER :
                                al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                                switch(posicao_pause)
                                {
                                    case 1 : // Se estiver em 'Voltar'
                                        jogo_pausado = 0;
                                        al_set_audio_stream_gain(ativo.musica, VOLUME_MUSICA_JOGO); // Volta o volume da musica para o normal
                                        posicao_pause = 1;
                                        break;

                                    case 2 : // Em 'Salvar'
                                        if(tela_salvar_jogo(display, queue, timer, estado_atual) == 0) // Se pediu para fechar o jogo na tela de salvar jogo
                                        {
                                            jogando = 0;
                                            executar = 0;
                                            retorno = 0;
                                        }
                                        break;

                                    case 3 : // Em 'Menu Inicial'
                                        jogando = 0;
                                        executar = 0;
                                        retorno = 1;
                                        break;

                                    case 4 : // Em 'Sair'
                                        jogando = 0;
                                        executar = 0;
                                        retorno = 0;
                                        break;
                                }
                                break;
                        }
                    }
                }
                else if(evento.type == ALLEGRO_EVENT_KEY_UP) // Largar a tecla
                {
                    switch(evento.keyboard.keycode)
                    {
                        case ALLEGRO_KEY_D :
                        case ALLEGRO_KEY_RIGHT :
                            direita = 0;
                            break;

                        case ALLEGRO_KEY_A :
                        case ALLEGRO_KEY_LEFT :
                            esquerda = 0;
                            break;

                        case ALLEGRO_KEY_W :
                        case ALLEGRO_KEY_UP :
                            cima = 0;
                            break;

                        case ALLEGRO_KEY_S :
                        case ALLEGRO_KEY_DOWN :
                            baixo = 0;
                            break;
                    }
                }
                else if(evento.type == ALLEGRO_EVENT_TIMER)
                {
                    atualizar = 1;
                    cont_frames_sprite++;
                    cont_frames_spr_gatos++;
                    if(comeu_osso == 1 && jogo_pausado == 0)
                    {
                        cont_osso++;
                    }
                    if(anim_vida == 1)
                    {
                        cont_anim_vida++;
                        if(cont_anim_vida >= DURACAO_AVISO_VIDA*FPS)
                        {
                            anim_vida = 0;
                            cont_anim_vida = 0;
                        }
                    }
                    //Para o degrade no menu:
                    if(jogo_pausado == 1)
                    {
                        if(gr >= 247 || gr <= 61)
                            inc = inc*(-1);

                        gr = gr + (186/(FPS*VEL_CORES_MENU))*inc;
                        red = red - (5/(FPS*VEL_CORES_MENU))*inc;
                        bl = bl + (76/(FPS*VEL_CORES_MENU))*inc;
                    }
                    else if(jogo_pausado == 0)
                    {
                        inc = 1;
                        red = 249;
                        gr = 247;
                        bl = 128;
                    }
                }
            }

            for(c=0; c<27; c++) // Pega a posicao do rato e dos gatos na tela e a compara com a posicao dos tiles para obter a posicao do rato na matriz
            {
                //Rato:
                if(direita == 1)
                {
                    if(rato.x+HITBOX_X_DIR >= 5+c*TILESIZE && rato.x+HITBOX_X_DIR < 5+(c+1)*TILESIZE)
                    {
                        estado_atual.rato_pos_matriz.x = c;
                    }
                }
                else if(esquerda == 1)
                {
                    if(rato.x+HITBOX_X_ESQ >= 5+c*TILESIZE && rato.x+HITBOX_X_ESQ < 5+(c+1)*TILESIZE)
                    {
                        estado_atual.rato_pos_matriz.x = c;
                    }
                }
                else if(cima == 1 || baixo == 1)
                {
                    if(rato.x+32 >= 5+c*TILESIZE && rato.x+32 < 5+(c+1)*TILESIZE)
                    {
                        estado_atual.rato_pos_matriz.x = c;
                    }
                }
            }
            for(l=0; l<11; l++)
            {
                //Rato:
                if(cima == 1)
                {
                    if(rato.y+HITBOX_Y_CIMA >= 101+l*TILESIZE && rato.y+HITBOX_Y_CIMA < 101+(l+1)*TILESIZE)
                    {
                        estado_atual.rato_pos_matriz.y = l;
                    }
                }
                else if(baixo == 1)
                {
                    if(rato.y+HITBOX_Y_BAIXO >= 101+l*TILESIZE && rato.y+HITBOX_Y_BAIXO < 101+(l+1)*TILESIZE)
                    {
                        estado_atual.rato_pos_matriz.y = l;
                    }
                }
                else if(direita == 1 || esquerda == 1)
                {
                    if(rato.y+24 >= 101+l*TILESIZE && rato.y+24 < 101+(l+1)*TILESIZE)
                    {
                        estado_atual.rato_pos_matriz.y = l;
                    }
                }
            }

            switch (estado_atual.mapa[estado_atual.rato_pos_matriz.y][estado_atual.rato_pos_matriz.x])
            {
                case 'Q':
                    al_play_sample(som_qjo, VOLUME_SFX_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    estado_atual.pontos += 10;
                    buffer_ptos+=10; // Utilizado para dar vidas a cada 3000 pontos
                    estado_atual.mapa[estado_atual.rato_pos_matriz.y][estado_atual.rato_pos_matriz.x]=' ';
                    maxqueijo = maxqueijo - 1;
                    if (maxqueijo==0) //Quando não tem mais queijos disponiveis muda de nivel
                    {
                        estado_atual.nivel++;
                        jogando=0;
                        comeu_osso = 0;
                        cont_osso = 0;
                    }
                    break;
                case 'O':
                    al_play_sample(som_osso, VOLUME_SFX_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    estado_atual.mapa[estado_atual.rato_pos_matriz.y][estado_atual.rato_pos_matriz.x]=' ';
                    estado_atual.pontos+=25;
                    buffer_ptos+=25; // Utilizado para dar vidas a cada 3000 pontos
                    if(comeu_osso == 0)
                    {
                        comeu_osso = 1;
                    }
                    else if(comeu_osso == 1) // Se comer outro osso enquanto tem um ja ativo, aumenta o tempo que ele vai durar em 5 segundos
                    {
                        cont_osso -= DURACAO_MODO_CACHORRO*FPS;
                    }

                    al_set_audio_stream_speed(ativo.musica, VELOCIDADE_MUSICA_CACHORRO); // Acelera a música
                    break;
            }

            if(atualizar == 1)
            {
                if(cont_osso >= DURACAO_MODO_CACHORRO*FPS)
                {
                    cont_osso = 0;
                    comeu_osso = 0;
                    al_set_audio_stream_speed(ativo.musica, 1.0);
                }

                //Ganhar vidas:
                // Se todos os gatos foram comidos pelo cachorro de uma vez
                if(gato_morto[0] == 1 && gato_morto[1] == 1 && gato_morto[2] == 1 && gato_morto[3] == 1 && pode_ganhar_vida == 1)
                {
                    estado_atual.vidas++; // Ganha uma vida
                    estado_atual.pontos+=100; // Ganha 100 pontos
                    al_play_sample(som_ganha_vida, 1.5*VOLUME_SFX_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    anim_vida = 1; // 'Animacao' de ganhar vida
                    pode_ganhar_vida = 0; // So podera ganhar vida de novo quando todos os gatos respawnarem (ver abaixo)
                }
                if(gato_morto[0] == 0 && gato_morto[1] == 0 && gato_morto[2] == 0 && gato_morto[3] == 0 && pode_ganhar_vida == 0)
                {
                    pode_ganhar_vida = 1;
                }
                //A cada 3000 pontos jogados, ganha uma vida:
                if(buffer_ptos >= 3000)
                {
                    buffer_ptos = buffer_ptos-3000; // Se houveram pontos extras alem dos 3000, os "guarda"
                    estado_atual.vidas++;
                    al_play_sample(som_ganha_vida, 1.5*VOLUME_SFX_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    anim_vida = 1; // 'Animacao' de ganhar vida
                }

                for(g=0; g<4; g++)
                {
                    if(estado_atual.rato_pos_matriz.x == estado_atual.gatos_matriz[g].x && estado_atual.rato_pos_matriz.y == estado_atual.gatos_matriz[g].y) // Se for a mesma posição de um gato
                    {
                        if(comeu_osso == 0)
                        {
                            morreu = 1;
                            estado_atual.vidas--;
                            al_play_sample(som_perde_vida, VOLUME_SFX_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            estado_atual.rato_pos_matriz.x=0; // Correcao para o bug que, as vezes, descontava mais de 1 vida
                        }
                        else if(comeu_osso == 1)
                        {
                            estado_atual.pontos+=50;
                            buffer_ptos+=50; // Utilizado para dar vidas a cada 3000 pontos
                            //Some com o gato:
                            gatos[g].x = LARGURA*2;
                            gatos[g].y = ALTURA*2;
                            estado_atual.gatos_matriz[g].x = 0;
                            estado_atual.gatos_matriz[g].y = 0;
                            gato_morto[g] = 1;
                            //Alterna os miados (para nao ficar tao repetitivo):
                            if(g == 0 || g == 1)
                                al_play_sample(som_miado1, VOLUME_SFX_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            else if(g == 2 || g == 3)
                                al_play_sample(som_miado2, VOLUME_SFX_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        }
                    }

                    if(gato_morto[g] == 1 && cont_osso >= (DURACAO_MODO_CACHORRO-0.1)*FPS) //Reinicia a posição e a direção do gato, meio segundo antes do modo cachorro acabar
                    {
                        gatos[g].x = 5+pos_inicial_gato[g].x*TILESIZE;
                        gatos[g].y = 101+pos_inicial_gato[g].y*TILESIZE;
                        if(estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x+1] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x+1] != 'T')
                        {
                            gatos[g].direcao = 1; // Direita
                        }
                        else if(estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x-1] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y][pos_inicial_gato[g].x-1] != 'T')
                        {
                            gatos[g].direcao = 2; // Esquerda
                        }
                        else if(estado_atual.mapa[pos_inicial_gato[g].y+1][pos_inicial_gato[g].x] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y+1][pos_inicial_gato[g].x] != 'T')
                        {
                            gatos[g].direcao = 4; // Baixo
                        }
                        else if(estado_atual.mapa[pos_inicial_gato[g].y-1][pos_inicial_gato[g].x] != 'X' && estado_atual.mapa[pos_inicial_gato[g].y-1][pos_inicial_gato[g].x] != 'T')
                        {
                            gatos[g].direcao = 3; // Cima
                        }
                        gato_morto[g] = 0;
                    }
                }

                if(!desenhar_mapa(estado_atual.mapa, ativo))
                {
                    msg_erro("Falha ao desenhar mapa.", display);
                    jogando = 0;
                    executar = 0;
                    retorno = 0;
                }

                if(jogo_pausado == 0)
                {
                    //Posição do rato:
                    if(direita == 1)
                    {
                        if(comeu_osso == 0)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'd') == 1)
                                rato.x += VEL_RATO_JOGO;

                            rato_spr.ind_lin = 2;
                        }
                        else if(comeu_osso == 1)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'd') == 1)
                                rato.x += VEL_CACHORRO;

                            cachorro_spr.ind_lin = 2;
                        }
                    }
                    if(esquerda == 1)
                    {
                        if(comeu_osso == 0)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'e') == 1)
                                rato.x -= VEL_RATO_JOGO;

                            rato_spr.ind_lin = 1;
                        }
                        else if(comeu_osso == 1)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'e') == 1)
                                rato.x -= VEL_CACHORRO;

                            cachorro_spr.ind_lin = 1;
                        }
                    }
                    if(baixo == 1)
                    {
                        if(comeu_osso == 0)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'b') == 1)
                                rato.y += VEL_RATO_JOGO;

                            rato_spr.ind_lin = 0;
                        }
                        else if(comeu_osso == 1)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'b') == 1)
                                rato.y += VEL_CACHORRO;

                            cachorro_spr.ind_lin = 0;
                        }
                    }
                    if(cima == 1)
                    {
                        if(comeu_osso == 0)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'c') == 1)
                                rato.y -= VEL_RATO_JOGO;

                            rato_spr.ind_lin = 3;
                        }
                        else if(comeu_osso == 1)
                        {
                            if(podeMoverRato(estado_atual.mapa, rato, 'c') == 1)
                                rato.y -= VEL_CACHORRO;

                            cachorro_spr.ind_lin = 3;
                        }
                    }

                    //Posicao dos gatos na matriz:
                    for(g=0; g<4; g++)
                    {
                        for(l=0; l<LINHAS; l++)
                        {
                            for(c=0; c<COLUNAS-1; c++)
                            {
                                if(gatos[g].x == 5+c*TILESIZE && gatos[g].y == 101+l*TILESIZE)
                                {
                                    estado_atual.gatos_matriz[g].x = c;
                                    estado_atual.gatos_matriz[g].y = l;
                                }
                            }
                        }
                    }

                    //Direcao dos gatos:
                    for(g=0; g<4; g++)
                    {
                        if((gatos[g].x-5)%TILESIZE == 0 && (gatos[g].y-101)%TILESIZE == 0)
                        {
                            gatos[g].direcao = mudaDirecaoGato(estado_atual.mapa, estado_atual.gatos_matriz[g].x, estado_atual.gatos_matriz[g].y, gatos[g].direcao);
                        }
                    }

                    //Posições dos gatos:
                    for(g=0; g<4; g++)
                    {
                        if(gatos[g].direcao == 1)
                        {
                            if(comeu_osso == 0)
                            {
                                gatos[g].x += VEL_GATO;
                            }
                            else if(comeu_osso == 1)
                                gatos[g].x += VEL_GATO_OSSO;

                            gatos_spr[g].ind_lin = 2;
                        }
                        else if(gatos[g].direcao == 2)
                        {
                            if(comeu_osso == 0)
                            {
                                gatos[g].x -= VEL_GATO;
                            }
                            else if(comeu_osso == 1)
                                gatos[g].x -= VEL_GATO_OSSO;

                            gatos_spr[g].ind_lin = 1;
                        }
                        else if(gatos[g].direcao == 3)
                        {
                            if(comeu_osso == 0)
                            {
                                gatos[g].y -= VEL_GATO;
                            }
                            else if(comeu_osso == 1)
                                gatos[g].y -= VEL_GATO_OSSO;

                            gatos_spr[g].ind_lin = 3;
                        }
                        else if(gatos[g].direcao == 4)
                        {
                            if(comeu_osso == 0)
                            {
                                gatos[g].y += VEL_GATO;
                            }
                            else if(comeu_osso == 1)
                                gatos[g].y += VEL_GATO_OSSO;

                            gatos_spr[g].ind_lin = 0;
                        }
                    }

                    //Portas:
                    if (tecla==8) //Mover Portas
                    {
                    //Grava a posicao atual das portas
                        k=0;
                        i=0;
                        for (i=0; i<LINHAS; i++)
                        {
                            j=0;
                            for (j=0; j<(COLUNAS-1); j++)
                            {
                                if (estado_atual.mapa[i][j]=='T')
                                {
                                    porta[k].x=j;
                                    porta[k].y=i;
                                    k++;
                                }
                            }
                        }
                        k=0;
                        for (k=0; k<PORTAS; k++)
                        {
                            if (estado_atual.mapa[porta[k].y+1][porta[k].x+1]!='X')
                                contcima++;
                            if (estado_atual.mapa[porta[k].y-1][porta[k].x-1]!='X')
                                contbaixo++;
                        }
                        if (contcima==PORTAS)
                        {
                            //Detecta se algum gato ou rato esta na posicao de uma nova porta:
                            for(g=0; g<4; g++)
                            {
                                for(p=0; p<PORTAS; p++)
                                {
                                    if(estado_atual.gatos_matriz[g].x == porta[p].x+1 && estado_atual.gatos_matriz[g].y == porta[p].y+1)
                                    {
                                        bicho_na_porta = 1;
                                    }
                                    if(estado_atual.rato_pos_matriz.x == porta[p].x+1 && estado_atual.rato_pos_matriz.y == porta[p].y+1)
                                    {
                                        bicho_na_porta = 1;
                                    }
                                }
                            }

                            if(bicho_na_porta == 0)
                            {
                                k=0;
                                for (k=0; k<PORTAS; k++)
                                {
                                    estado_atual.mapa[porta[k].y][porta[k].x]=estado_atual.mapa[porta[k].y+1][porta[k].x+1];
                                    estado_atual.mapa[porta[k].y+1][porta[k].x+1]='T';
                                }
                            }
                            else if(bicho_na_porta == 1)
                            {
                                al_play_sample(som_porta_block, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                        }
                        else if (contbaixo==PORTAS)
                        {
                            //Detecta se algum gato ou rato esta na posicao de uma nova porta:
                            for(g=0; g<4; g++)
                            {
                                for(p=0; p<PORTAS; p++)
                                {
                                    if(estado_atual.gatos_matriz[g].x == porta[p].x-1 && estado_atual.gatos_matriz[g].y == porta[p].y-1)
                                    {
                                        bicho_na_porta = 1;
                                    }
                                    if(estado_atual.rato_pos_matriz.x == porta[p].x-1 && estado_atual.rato_pos_matriz.y == porta[p].y-1)
                                    {
                                        bicho_na_porta = 1;
                                    }
                                }
                            }

                            if(bicho_na_porta == 0)
                            {
                                k=0;
                                for (k=0; k<PORTAS; k++)
                                {
                                    estado_atual.mapa[porta[k].y][porta[k].x]=estado_atual.mapa[porta[k].y-1][porta[k].x-1];
                                    estado_atual.mapa[porta[k].y-1][porta[k].x-1]='T';
                                }
                            }
                            else if(bicho_na_porta == 1)
                            {
                                al_play_sample(som_porta_block, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                        }
                        contcima=0;
                        contbaixo=0;
                        tecla=-1;
                        bicho_na_porta = 0;
                    }

                    if(direita == 1 || esquerda == 1 || baixo == 1 || cima == 1) // Se estiver se movimentando, o sprite muda
                    {
                        if(cont_frames_sprite >= N_FRAMES_SPRITE_RATO) // A cada N_FRAMES_SPRITE_RATO, muda a coluna do sprite
                        {
                            cont_frames_sprite = 0; // Reseta a variavel contadora
                            if(comeu_osso == 0)
                            {
                                if(rato_spr.ind_col < rato_spr.num_col-1)
                                    rato_spr.ind_col++;
                                else
                                    rato_spr.ind_col = 0;
                            }
                            else if(comeu_osso == 1)
                            {
                                if(cachorro_spr.ind_col < cachorro_spr.num_col-2)
                                    cachorro_spr.ind_col++;
                                else
                                    cachorro_spr.ind_col = 0;
                            }
                        }
                    }
                    else // Se nao estiver em movimento, coloca a coluna do sprite que tenha o rato parado
                    {
                        rato_spr.ind_col = 1;
                        cachorro_spr.ind_col = 4;
                    }

                    if(cont_frames_spr_gatos >= N_FRAMES_SPRITE_RATO)
                    {
                        cont_frames_spr_gatos = 0;
                        for(g=0; g<4; g++)
                        {
                            if(gatos_spr[g].ind_col < gatos_spr[g].num_col-1)
                                gatos_spr[g].ind_col++;
                            else
                                gatos_spr[g].ind_col = 0;
                        }
                    }
                }

                al_draw_textf(fonte_placar, al_map_rgb(249, 247, 128), LARGURA/4, ALTURA/20, ALLEGRO_ALIGN_CENTER, "Pontos: %4d", estado_atual.pontos);
                al_draw_textf(fonte_placar, al_map_rgb(249, 247, 128), 3*LARGURA/4+2*al_get_bitmap_width(rato_cara), ALTURA/20, ALLEGRO_ALIGN_CENTER, "x %d", estado_atual.vidas);
                al_draw_bitmap(rato_cara, 3*LARGURA/4, ALTURA/25, 0);

                if(anim_vida == 1)
                    al_draw_bitmap(ganha_vida, 3*LARGURA/4+2*al_get_bitmap_width(rato_cara), ALTURA/20-60*(cont_anim_vida/(DURACAO_AVISO_VIDA*FPS)), 0);

                if(comeu_osso == 0)
                {
                    al_draw_scaled_bitmap(rato_spr.folha, (rato_spr.ind_col*al_get_bitmap_width(rato_spr.folha))/rato_spr.num_col, (rato_spr.ind_lin*al_get_bitmap_height(rato_spr.folha))/rato_spr.num_lin,
                                        al_get_bitmap_width(rato_spr.folha)/rato_spr.num_col, al_get_bitmap_height(rato_spr.folha)/rato_spr.num_lin, rato.x, rato.y,
                                        ESCALA_RATO_JOGO*(al_get_bitmap_width(rato_spr.folha)/rato_spr.num_col), ESCALA_RATO_JOGO*(al_get_bitmap_height(rato_spr.folha)/rato_spr.num_lin), 0);
                    for(g=0; g<4; g++)
                    {
                        al_draw_scaled_bitmap(gatos_spr[g].folha, (gatos_spr[g].ind_col*al_get_bitmap_width(gatos_spr[g].folha))/gatos_spr[g].num_col, (gatos_spr[g].ind_lin*al_get_bitmap_height(gatos_spr[g].folha))/gatos_spr[g].num_lin,
                                        al_get_bitmap_width(gatos_spr[g].folha)/gatos_spr[g].num_col, al_get_bitmap_height(gatos_spr[g].folha)/gatos_spr[g].num_lin, gatos[g].x-5, gatos[g].y-8,
                                        2*(al_get_bitmap_width(gatos_spr[g].folha)/gatos_spr[g].num_col), 2*(al_get_bitmap_height(gatos_spr[g].folha)/gatos_spr[g].num_lin), 0);
                    }
                }
                else if(comeu_osso == 1)
                {
                    al_draw_scaled_bitmap(cachorro_spr.folha, (cachorro_spr.ind_col*al_get_bitmap_width(cachorro_spr.folha))/cachorro_spr.num_col, (cachorro_spr.ind_lin*al_get_bitmap_height(cachorro_spr.folha))/cachorro_spr.num_lin,
                                        al_get_bitmap_width(cachorro_spr.folha)/cachorro_spr.num_col, al_get_bitmap_height(cachorro_spr.folha)/cachorro_spr.num_lin, rato.x-5, rato.y-12,
                                        ESCALA_CACHORRO_JOGO*(al_get_bitmap_width(cachorro_spr.folha)/cachorro_spr.num_col), ESCALA_CACHORRO_JOGO*(al_get_bitmap_height(cachorro_spr.folha)/cachorro_spr.num_lin), 0);
                    for(g=0; g<4; g++)
                    {
                        al_draw_scaled_bitmap(gato_alt_spr_folha, (gatos_spr[g].ind_col*al_get_bitmap_width(gato_alt_spr_folha))/gatos_spr[g].num_col, (gatos_spr[g].ind_lin*al_get_bitmap_height(gato_alt_spr_folha))/gatos_spr[g].num_lin,
                                        al_get_bitmap_width(gato_alt_spr_folha)/gatos_spr[g].num_col, al_get_bitmap_height(gato_alt_spr_folha)/gatos_spr[g].num_lin, gatos[g].x-5, gatos[g].y-8,
                                        2*(al_get_bitmap_width(gato_alt_spr_folha)/gatos_spr[g].num_col), 2*(al_get_bitmap_height(gato_alt_spr_folha)/gatos_spr[g].num_lin), 0);
                    }
                }

                al_draw_bitmap(gatos_rostos, ((COLUNAS-1)*TILESIZE)-al_get_bitmap_width(gatos_rostos)-al_get_bitmap_width(cachorro_barra_tempo)-40, 101+(LINHAS+0.55)*TILESIZE, 0);
                for(g=0; g<4; g++)
                {
                    if(gato_morto[g] == 1)
                        al_draw_bitmap(gatos_rostos_sombra, ((COLUNAS-1)*TILESIZE)-al_get_bitmap_width(gatos_rostos)-al_get_bitmap_width(cachorro_barra_tempo)+48*g-40, 101+(LINHAS+0.55)*TILESIZE, 0);
                }

                if(comeu_osso == 1 && cont_osso > 0)
                    al_draw_scaled_bitmap(barra_tempo, 0, 0, 75, 19, ((COLUNAS-1)*TILESIZE)-al_get_bitmap_width(cachorro_barra_tempo)+48-10, 101+((LINHAS+0.5)*TILESIZE)+13, 75*(1-(cont_osso/(DURACAO_MODO_CACHORRO*FPS))), 18, 0);

                else if(comeu_osso == 1 && cont_osso <= 0)
                    al_draw_scaled_bitmap(barra_tempo, 0, 0, 75, 19, ((COLUNAS-1)*TILESIZE)-al_get_bitmap_width(cachorro_barra_tempo)+48-10, 101+((LINHAS+0.5)*TILESIZE)+13, 75, 18, 0);

                al_draw_bitmap(cachorro_barra_tempo, ((COLUNAS-1)*TILESIZE)-al_get_bitmap_width(cachorro_barra_tempo)-10, 101+(LINHAS+0.5)*TILESIZE, 0);

                if(jogo_pausado == 1)
                {
                    al_draw_bitmap(bg_menu_ingame, 0, 0, 0);

                    switch(posicao_pause)
                    {
                        case 1 : // Posicao esta no Voltar
                            textomenu_pause(fonte_menu, 1);
                            al_draw_text(fonte_menu, al_map_rgb(round(red), round(gr), round(bl)), LARGURA/2, ALTURA/2 - 1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Voltar");
                            break;

                        case 2 :
                            textomenu_pause(fonte_menu, 2);
                            al_draw_text(fonte_menu, al_map_rgb(round(red), round(gr), round(bl)), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_CENTER, "Salvar");
                            break;

                        case 3 :
                            textomenu_pause(fonte_menu, 3);
                            al_draw_text(fonte_menu, al_map_rgb(round(red), round(gr), round(bl)), LARGURA/2, ALTURA/2 + 1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Menu Inicial");
                            break;

                        case 4 :
                            textomenu_pause(fonte_menu, 4);
                            al_draw_text(fonte_menu, al_map_rgb(round(red), round(gr), round(bl)), LARGURA/2, ALTURA/2 + 3*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Sair");
                            break;
                    }
                }

                al_flip_display();
                al_clear_to_color(al_map_rgb(0,0,0));
                atualizar = 0;
            }
        }

        //Quando passar de fase:
        al_destroy_audio_stream(ativo.musica);
        //(unico jeito de reiniciar o play da musica)
    }

    al_destroy_bitmap(rato_spr.folha);
    for(g=0; g<4; g++)
    {
        al_destroy_bitmap(gatos_spr[g].folha);
    }
    al_destroy_bitmap(gato_alt_spr_folha);
    al_destroy_bitmap(casa.chao);
    al_destroy_bitmap(casa.parede);
    al_destroy_bitmap(casa.porta);
    al_destroy_bitmap(lago.chao);
    al_destroy_bitmap(lago.parede);
    al_destroy_bitmap(lago.porta);
    al_destroy_bitmap(jardim.chao);
    al_destroy_bitmap(jardim.parede);
    al_destroy_bitmap(jardim.porta);
    al_destroy_bitmap(mal_assombrado.chao);
    al_destroy_bitmap(mal_assombrado.parede);
    al_destroy_bitmap(mal_assombrado.porta);
    al_destroy_bitmap(gatos_rostos);
    al_destroy_bitmap(gatos_rostos_sombra);
    al_destroy_bitmap(cachorro_barra_tempo);
    al_destroy_bitmap(barra_tempo);
    al_destroy_bitmap(rato_cara);
    al_destroy_bitmap(ganha_vida);
    al_destroy_bitmap(bg_menu_ingame);
    al_destroy_font(fonte_placar);
    al_destroy_font(fonte_menu);
    al_destroy_sample(som_porta_block);
    al_destroy_sample(som_qjo);
    al_destroy_sample(som_perde_vida);
    al_destroy_sample(som_osso);
    al_destroy_sample(som_miado1);
    al_destroy_sample(som_miado2);
    al_destroy_sample(som_ganha_vida);
    al_destroy_sample(menu_mover);
    al_destroy_sample(menu_selec);

    return retorno;
}

int desenhar_mapa (char mapa[LINHAS][COLUNAS], CENARIO cenario)
{
    ALLEGRO_BITMAP *queijo = NULL;
    ALLEGRO_BITMAP *osso = NULL;
    ALLEGRO_BITMAP *osso_laco = NULL; // Só tem valor estético
    int retorno = 1;
    int cont_ossos = 0;

    queijo = al_load_bitmap("assets/img/jogo/item_queijo.png");
    if(!queijo)
    {
        retorno = 0;
    }
    else
    {
        osso = al_load_bitmap("assets/img/jogo/item_osso.png");
        if(!osso)
        {
            retorno = 0;
        }
        else
        {
            osso_laco = al_load_bitmap("assets/img/jogo/item_osso_laco.png");
            if(!osso_laco)
            {
                retorno = 0;
            }
        }
    }

    al_clear_to_color(cenario.cor_fundo);

    int i, j;
    int a, b;

    for(a=0; a<LINHAS; a++)
    {
        for(b=0; b<COLUNAS-1; b++)
        {
            al_draw_bitmap(cenario.chao, 5+b*TILESIZE, 101+a*TILESIZE, 0);
        }
    }

    for (i=0; i<LINHAS; i++)
    {
        for (j=0; j<COLUNAS; j++)
        {
            switch (mapa[i][j])
            {
            case 'X':
                {
                    al_draw_bitmap (cenario.parede, 5+j*TILESIZE, 101+i*TILESIZE, 0);
                    break;
                }
            case 'T':
                {
                     al_draw_bitmap(cenario.porta, 5+j*TILESIZE, 101+i*TILESIZE, 0);
                     break;
                }
            case 'Q':
                {
                    al_draw_bitmap(queijo, 5+j*TILESIZE, 101+i*TILESIZE, 0);
                    break;
                }
            case 'O':
                {
                    cont_ossos++;
                    if(cont_ossos == 3)
                    {
                        al_draw_bitmap(osso_laco,5+j*TILESIZE, 101+i*TILESIZE, 0);
                    }
                    else
                    {
                        al_draw_bitmap(osso,5+j*TILESIZE, 101+i*TILESIZE, 0);
                    }
                    break;
                }
            }
        }
    }

    al_destroy_bitmap(osso_laco);
    al_destroy_bitmap(queijo);
    al_destroy_bitmap(osso);

    return retorno;
}

int carregar_mapa (char *nome_arquivo, char mapa[LINHAS][COLUNAS], RATO *pos_rato, GATO pos_gato[4]) //funcao que carrega mapa e pega posicao inicial do rato
{
    FILE *file;
    int i, j, retorno;
    int cont_g = 0;

    if(!(file=fopen(nome_arquivo, "r")))
    {
        retorno=0;
    }

    else
    {
        while (!feof(file))
        {
            i=0;
            for (i=0; i<LINHAS; i++)
            {
                j=0;
                for (j=0; j<(COLUNAS); j++)
                {
                    mapa[i][j]= getc(file);
                    if (mapa[i][j]=='M')
                    {
                        pos_rato->y=i;
                        pos_rato->x=j;
                    }
                    else if(mapa[i][j]=='G')
                    {
                        if(cont_g < 4)
                        {
                            pos_gato[cont_g].x = j;
                            pos_gato[cont_g].y = i;
                        }

                        cont_g++;
                    }
                }

            }
        }
        retorno=1;
    }
    fclose(file);

    return retorno;
 }

void arquivo_nivel (int nivel, char *nome_arquivo) //funcao altera o arquivo do nivel
{
    int dezena, unidade;
    char dez_char, unid_char;

    dezena=nivel/10;
    unidade=nivel%10;

    dez_char=dezena+'0'; //transforma inteiro em char
    unid_char=unidade+'0';

    nome_arquivo[12]=dez_char;
    nome_arquivo[13]=unid_char;
}

int podeMoverRato(char mapa[LINHAS][COLUNAS], RATO pos_rato, char direcao) // d = direita, e = esquerda, b = baixo, c = cima
{
    int retorno = 0;
    int l, c;
    RATO nova_posicao = {0,0}, nova_posicao_matriz = {0,0}; // ec = pra esquerda e pra cima; db = pra direita e pra baixo

    nova_posicao.x = pos_rato.x+23; // Deixa como padrao o centro do rato, para colisoes nos outros sentidos que nao o que esta sendo movido
    nova_posicao.y = pos_rato.y+23;

    if(direcao == 'd')
    {
        nova_posicao.x = pos_rato.x+HITBOX_X_DIR+VEL_RATO_JOGO;

        for(c=0; c<COLUNAS-1; c++)
        {
            if(nova_posicao.x >= 5+c*TILESIZE && nova_posicao.x < 5+(c+1)*TILESIZE)
            {
                nova_posicao_matriz.x = c;
            }
        }
        for(l=0; l<LINHAS; l++)
        {
            if(nova_posicao.y >= 101+l*TILESIZE && nova_posicao.y < 101+(l+1)*TILESIZE)
            {
                nova_posicao_matriz.y = l;
            }
        }
    }
    else if(direcao == 'e')
    {
        nova_posicao.x = pos_rato.x+HITBOX_X_ESQ-VEL_RATO_JOGO;

        for(c=0; c<COLUNAS-1; c++)
        {
            if(nova_posicao.x >= 5+c*TILESIZE && nova_posicao.x < 5+(c+1)*TILESIZE)
            {
                nova_posicao_matriz.x = c;
            }
        }
        for(l=0; l<LINHAS; l++)
        {
            if(nova_posicao.y >= 101+l*TILESIZE && nova_posicao.y < 101+(l+1)*TILESIZE)
            {
                nova_posicao_matriz.y = l;
            }
        }
    }
    else if(direcao == 'b')
    {
        nova_posicao.y = pos_rato.y+HITBOX_Y_BAIXO+VEL_RATO_JOGO;

        for(c=0; c<COLUNAS-1; c++)
        {
            if(nova_posicao.x >= 5+c*TILESIZE && nova_posicao.x < 5+(c+1)*TILESIZE)
            {
                nova_posicao_matriz.x = c;
            }
        }
        for(l=0; l<LINHAS; l++)
        {
            if(nova_posicao.y >= 101+l*TILESIZE && nova_posicao.y < 101+(l+1)*TILESIZE)
            {
                nova_posicao_matriz.y = l;
            }
        }
    }
    else if(direcao == 'c')
    {
        nova_posicao.y = pos_rato.y+HITBOX_Y_CIMA-VEL_RATO_JOGO;

        for(c=0; c<COLUNAS-1; c++)
        {
            if(nova_posicao.x >= 5+c*TILESIZE && nova_posicao.x < 5+(c+1)*TILESIZE)
            {
                nova_posicao_matriz.x = c;
            }
        }
        for(l=0; l<LINHAS; l++)
        {
            if(nova_posicao.y >= 101+l*TILESIZE && nova_posicao.y < 101+(l+1)*TILESIZE)
            {
                nova_posicao_matriz.y = l;
            }
        }
    }

    if(mapa[nova_posicao_matriz.y][nova_posicao_matriz.x] != 'X' && mapa[nova_posicao_matriz.y][nova_posicao_matriz.x] != 'T')
    {
        retorno = 1; // Pode mover
    }

    return retorno;
}

int mudaDirecaoGato(char mapa[LINHAS][COLUNAS], int col_gato, int lin_gato, int direcao_gato)
{
    // direcao_gato: 1=direita, 2=esq, 3=cima, 4=baixo
    int direcao_livre[4] = {0,0,0,0}; // 0 = direita, 1 = esq, 2 = cima, 3 = baixo (indices)
    int num_livres = 0;
    int i;
    int direcao = 0;
    int direcao_oposta;

    srand(time(NULL));

    // Ve quais direcoes estao livres:
    if(mapa[lin_gato][col_gato+1] != 'X' && mapa[lin_gato][col_gato+1] != 'T')
    {
        direcao_livre[0] = 1;
        num_livres++;
    }
    if(mapa[lin_gato][col_gato-1] != 'X' && mapa[lin_gato][col_gato-1] != 'T')
    {
        direcao_livre[1] = 1;
        num_livres++;
    }
    if(mapa[lin_gato-1][col_gato] != 'X' && mapa[lin_gato-1][col_gato] != 'T')
    {
        direcao_livre[2] = 1;
        num_livres++;
    }
    if(mapa[lin_gato+1][col_gato] != 'X' && mapa[lin_gato+1][col_gato] != 'T')
    {
        direcao_livre[3] = 1;
        num_livres++;
    }

    //Associa a direcao oposta a atual:
    if(direcao_gato == 2 || direcao_gato == 4)
    {
        direcao_oposta = direcao_gato-1;
    }
    else if(direcao_gato == 1 || direcao_gato == 3)
    {
        direcao_oposta = direcao_gato+1;
    }

    // Se estiver na borda do mapa (bug)
    if(mapa[lin_gato][col_gato] == 'T' || mapa[lin_gato][col_gato] == 'X')
    {
            direcao = direcao_oposta;
    }
    //Se so 1 estiver livre, muda para essa direcao:
    else if(num_livres == 1)
    {
        for(i=0; i<4; i++)
        {
            if(direcao_livre[i] == 1)
            {
                direcao = i+1;
            }
        }
    }
    //Se so 2 estiverem livres
    else if(num_livres == 2)
    {
        // Se forem direcoes opostas, nao muda a direcao:
        if(direcao_livre[0] == 1 && direcao_livre[1] == 1)
        {
            direcao = direcao_gato;
        }
        else if(direcao_livre[2] == 1 && direcao_livre[3] == 1)
        {
            direcao = direcao_gato;
        }
        else // Se for um canto, muda a direcao para a diferente da oposta:
        {
            do
            {
                direcao = 1+rand()%4;
            }
            while(direcao_livre[direcao-1] == 0 || direcao == direcao_oposta);
        }
    }
    else // Caso contrario, escolhe aleatoriamente a direcao, desde que: seja uma direcao livre e nao seja a direcao oposta a atual
    {
        do
        {
            direcao = 1+rand()%4;
        }
        while(direcao_livre[direcao-1] == 0 || direcao == direcao_oposta);
    }

    return direcao;
}

void textomenu_pause(ALLEGRO_FONT *fontemenu, int sel)
{
    if(sel != 1)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 - 1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Voltar");
    if(sel != 2)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_CENTER, "Salvar");
    if(sel != 3)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 + 1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Menu Inicial");
    if(sel != 4)
        al_draw_text(fontemenu, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/2 + 3*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Sair");
}

int tela_nivel_vidas(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, ALLEGRO_COLOR fundo, int nivel, int vidas)
{
    ALLEGRO_BITMAP *rato_cara = NULL;
    ALLEGRO_FONT *fontetexto = NULL;
    ALLEGRO_EVENT evento;
    int retorno = 1; // Retorna 0 se apertou para sair do jogo
    int cont_tempo = 0;

    rato_cara = al_load_bitmap("assets/img/jogo/rato_rosto.png");
    if(!rato_cara)
    {
        msg_erro("Falha ao carregar rato_rosto.png", display);
    }
    else
    {
        fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO*1.5, 0);
        if (!fontetexto)
        {
            msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
        }
    }

    al_clear_to_color(fundo);
    al_draw_bitmap(rato_cara, LARGURA/2-1.8*al_get_bitmap_width(rato_cara), ALTURA/2+ALTURA/20, 0);
    al_draw_textf(fontetexto, al_map_rgb(249, 247, 128), LARGURA/2-0.7*al_get_bitmap_width(rato_cara), ALTURA/2+ALTURA/20, ALLEGRO_ALIGN_LEFT, " x %d", vidas);
    al_draw_textf(fontetexto, al_map_rgb(249, 247, 128), LARGURA/2, ALTURA/4, ALLEGRO_ALIGN_CENTER, "Nivel %d", nivel);
    al_flip_display();

    while(cont_tempo < TEMPO_TELA_VIDAS_NIVEL*FPS)
    {
        while(!al_is_event_queue_empty(queue))
        {
            al_wait_for_event(queue, &evento);
            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                if(al_show_native_message_box(display, "Mouse Trap", "Tem certeza que quer fechar o jogo?", "Todo o progresso nao salvo sera perdido.", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL))
                { // Se apertar em "sim" (retorno = 1)
                    cont_tempo = TEMPO_TELA_VIDAS_NIVEL*FPS; // Fecha o jogo
                    retorno = 0;
                }
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                cont_tempo++;
            }
        }
    }

    al_destroy_bitmap(rato_cara);
    al_destroy_font(fontetexto);

    return retorno;
}

int tela_salvar_jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, SAVE estado_do_jogo)
{
    ALLEGRO_BITMAP *fundomenu = NULL;
    ALLEGRO_EVENT evento;
    ALLEGRO_FONT *fontetextomenor = NULL;
    ALLEGRO_FONT *fontetexto = NULL;
    ALLEGRO_FONT *fontetextosave = NULL;
    ALLEGRO_FONT *fontetextomaior = NULL;
    ALLEGRO_SAMPLE *menu_selec = NULL, *menu_mover = NULL;
    SAVE save1 = {0,0,0,0}, save2 = {0,0,0,0};
    int executar = 1;
    int retorno = 1;
    int posicao = 1; // 1 ou 2
    int atualizar = 0;
    int dim;
    float r = 249, g = 247, b = 128;
    int inc = 1;
    int tecla = -1;
    int l, h;
    int cont_salvo = 0;
    int salvou = 0;
    int i, j, gato;

    fundomenu = al_load_bitmap("assets/img/menu_fundo.jpg");
    if (!fundomenu)
    {
        msg_erro("Falha carregando menu_fundo.jpg.", display);
        executar = 0;
    }
    else
    {
        fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO, 0);
        if (!fontetexto)
        {
            msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
            executar = 0;
        }
        else
        {
            fontetextomenor = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO/2, 0);
            if (!fontetextomenor)
            {
                msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                executar = 0;
            }
            else
            {
                fontetextosave = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO/1.5, 0);
                if (!fontetextosave)
                {
                    msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                    executar = 0;
                }
                else
                {
                    menu_selec = al_load_sample("assets/audio/menu_lego_sw_selecionar.ogg");
                    if(!menu_selec)
                    {
                        msg_erro("Falha ao carregar menu_lego_sw_selecionar.ogg.", display);
                        executar = 0;
                    }
                    else
                    {
                        menu_mover = al_load_sample("assets/audio/menu_lego_sw_mover.ogg");
                        if(!menu_mover)
                        {
                            msg_erro("Falha ao carregar menu_lego_sw_mover.ogg.", display);
                            executar = 0;
                        }
                        else
                        {
                            fontetextomaior = al_load_font("assets/fontes/vcr_osd_mono.ttf", 1.5*TAMTEXTO, 0);
                            if (!fontetextomaior)
                            {
                                msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                                executar = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    checaSaves(&save1, &save2);

    while(executar == 1)
    {
        while(!al_is_event_queue_empty(queue)) // Checa todos os eventos
        {
            al_wait_for_event(queue, &evento);

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                if(al_show_native_message_box(display, "Mouse Trap", "Tem certeza que quer fechar o jogo?", "Todo o progresso nao salvo sera perdido.", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL))
                { // Se apertar em "sim" (retorna 1)
                    executar = 0;
                    retorno = 0;
                }
            }
            else if(evento.type == ALLEGRO_EVENT_KEY_DOWN) // Se o evento é pressionar uma tecla
            {
                switch(evento.keyboard.keycode)
                {
                    case ALLEGRO_KEY_ENTER :
                    case ALLEGRO_KEY_PAD_ENTER :
                        tecla = 0;
                        break;
                    case ALLEGRO_KEY_ESCAPE :
                        tecla = 3;
                        break;
                    case ALLEGRO_KEY_RIGHT :
                    case ALLEGRO_KEY_D :
                        tecla = 4;
                        break;
                    case ALLEGRO_KEY_LEFT :
                    case ALLEGRO_KEY_A :
                        tecla = 5;
                        break;
                }
                if(tecla == 3) // Se apertar ESC, volta para o menu
                {
                    al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    executar = 0;
                    retorno = 1;
                }
                else if(tecla == 4 && posicao == 1)
                {
                    posicao++;
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                else if(tecla == 5 && posicao == 2)
                {
                    posicao--;
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                else if(tecla == 0)
                {
                    if(posicao == 1) // Se apertou enter na posicao 1 e há jogo salvo nela
                    {
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        //Atualiza as posicoes dos gatos e rato na matriz:
                        gato=0;
                        for(i=0; i<LINHAS; i++)
                        {
                            for(j=0; j<COLUNAS-1; j++)
                            {
                                if(estado_do_jogo.mapa[i][j] == 'M')
                                {
                                    estado_do_jogo.mapa[i][i] = ' ';
                                    estado_do_jogo.mapa[estado_do_jogo.rato_pos_matriz.y][estado_do_jogo.rato_pos_matriz.x] = 'M'; // Nessa posição, com certeza não há queijos, pois, caso houvesse, o rato já os teria comido
                                }
                                else if(estado_do_jogo.mapa[i][j] == 'G')
                                {
                                    if(estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'X' && estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'T')
                                    {
                                        estado_do_jogo.mapa[i][j] = estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x]; // Fix para queijos nao sumirem
                                    }
                                    gato++;
                                }
                            }
                        }
                        for(gato=0; gato<4; gato++)
                        {
                            if(estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'X' && estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'T')
                            {
                                estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] = 'G';
                            }
                        }

                        salvaSave(estado_do_jogo, "saves/save1.bin"); // Salva o jogo
                        salvou = 1;
                    }
                    else if(posicao == 2)
                    {
                        al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        //Atualiza as posicoes dos gatos e rato na matriz:
                        gato=0;
                        for(i=0; i<LINHAS; i++)
                        {
                            for(j=0; j<COLUNAS-1; j++)
                            {
                                if(estado_do_jogo.mapa[i][j] == 'M')
                                {
                                    estado_do_jogo.mapa[i][i] = ' ';
                                    estado_do_jogo.mapa[estado_do_jogo.rato_pos_matriz.y][estado_do_jogo.rato_pos_matriz.x] = 'M'; // Nessa posição, com certeza não há queijos, pois, caso houvesse, o rato já os teria comido
                                }
                                else if(estado_do_jogo.mapa[i][j] == 'G')
                                {
                                    if(estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'X' && estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'T')
                                    {
                                        estado_do_jogo.mapa[i][j] = estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x]; // Fix para queijos nao sumirem
                                    }
                                    gato++;
                                }
                            }
                        }
                        for(gato=0; gato<4; gato++)
                        {
                            if(estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'X' && estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] != 'T')
                            {
                                estado_do_jogo.mapa[estado_do_jogo.gatos_matriz[gato].y][estado_do_jogo.gatos_matriz[gato].x] = 'G';
                            }
                        }
                        salvaSave(estado_do_jogo, "saves/save2.bin"); // Salva o jogo
                        salvou = 1;
                    }
                }
                tecla = -1;
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                atualizar = 1;

                if(g >= 247 || g <= 61)
                {
                   inc = inc*(-1);
                }
                g = g + (186/(FPS*VEL_CORES_MENU))*inc;
                r = r - (5/(FPS*VEL_CORES_MENU))*inc;
                b = b + (76/(FPS*VEL_CORES_MENU))*inc;

                if(salvou == 1)
                {
                    cont_salvo++;
                    if(cont_salvo >= TEMPO_JOGO_SALVO*FPS)
                    {
                        executar = 0;
                        retorno = 1;
                    }
                }
            }
        }

        if(atualizar == 1)
        {
            al_clear_to_color(al_map_rgb(66,51,31));
            dim = al_get_bitmap_height(fundomenu); // Como `fundomenu` eh quadrado, tanto faz qual dimensao se pega
            for(l=0; l<ceil((float)LARGURA/dim); l++)
            {
                for(h=0; h<ceil((float)ALTURA/dim); h++)
                {
                    al_draw_bitmap(fundomenu, dim*l, dim*h, 0);
                }
            }

            al_draw_text(fontetexto, al_map_rgb(107, 54, 172), LARGURA/2, ALTURA/6, ALLEGRO_ALIGN_CENTER, "Salvar Jogo");
            al_draw_text(fontetextosave, al_map_rgb(182, 128, 249), LARGURA/2, ALTURA/6+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Cuidado: se ja houver jogo salvo no slot, sera sobreposto.");

            if(posicao == 1)
            {
                al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 1");
                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA-LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 2");
            }
            else if(posicao == 2)
            {
                al_draw_text(fontetexto, al_map_rgb(249, 247, 128), LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 1");
                al_draw_text(fontetexto, al_map_rgb(round(r), round(g), round(b)), LARGURA-LARGURA/4, ALTURA/3, ALLEGRO_ALIGN_CENTER, "Jogo Salvo 2");
            }

            al_draw_text(fontetextomenor, al_map_rgb(249, 247, 128), LARGURA - TAMTEXTO, ALTURA - TAMTEXTO, ALLEGRO_ALIGN_RIGHT, "Voltar: ESC   Selecionar: Enter");

            if(save1.flag == 1)
            {
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "nivel: %d", save1.nivel);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "vidas: %d", save1.vidas);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA/4, ALTURA/3+1.5*TAMTEXTO+2*TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "pontos: %d", save1.pontos);
            }
            else if(save1.flag == 0)
            {
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "sem jogo");
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "salvo");
            }

            if(save2.flag == 1)
            {
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "nivel: %d", save2.nivel);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "vidas: %d", save2.vidas);
                al_draw_textf(fontetextosave, al_map_rgb(207, 95, 98), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO+2*TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "pontos: %d", save2.pontos);
            }
            else if(save2.flag == 0)
            {
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "sem jogo");
                al_draw_text(fontetextosave, al_map_rgb(160, 80, 81), LARGURA-LARGURA/4, ALTURA/3+1.5*TAMTEXTO+TAMTEXTO/1.3, ALLEGRO_ALIGN_CENTER, "salvo");
            }

            if(salvou == 1)
            {
                al_draw_text(fontetextomaior, al_map_rgb(249, 247, 128), LARGURA/2, 3*ALTURA/4, ALLEGRO_ALIGN_CENTER, "Jogo salvo!");
            }

            al_flip_display();
            atualizar = 0;
        }
    }

    al_destroy_bitmap(fundomenu);
    al_destroy_font(fontetexto);
    al_destroy_font(fontetextomenor);
    al_destroy_font(fontetextosave);
    al_destroy_font(fontetextomaior);
    al_destroy_sample(menu_selec);
    al_destroy_sample(menu_mover);

    return retorno;
}

int tela_game_over(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, int pontuacao)
{
    ALLEGRO_EVENT evento;
    ALLEGRO_BITMAP *fundomenu = NULL;
    ALLEGRO_FONT *fontetexto = NULL, *fontetextomenor = NULL;
    ALLEGRO_SAMPLE *musica_gameover = NULL;
    SPRITE cao_dormindo;
    int executar = 1, retorno = 1;
    int atualizar = 0;
    int cont_tempo = 0, cont_sprite = 0;
    int dim;
    int l, h;

    fundomenu = al_load_bitmap("assets/img/menu_fundo.jpg");
    if (!fundomenu)
    {
        msg_erro("Falha carregando menu_fundo.jpg.", display);
    }
    else
    {
        dim = al_get_bitmap_height(fundomenu);

        fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", 2*TAMTEXTO, 0);
        if (!fontetexto)
        {
            msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
        }
        else
        {
            musica_gameover = al_load_sample("assets/audio/game_over_mario1983.ogg");
            if(!musica_gameover)
            {
                msg_erro("Falha ao carregar game_over_mario1983.ogg.", display);
            }
            else
            {
                cao_dormindo.folha = NULL;
                cao_dormindo.folha = al_load_bitmap("assets/img/jogo/cao_dormindo_sprite.png");
                if(!cao_dormindo.folha)
                {
                    msg_erro("Falha ao carregar cao_dormindo_sprite.png.", display);
                }
                else
                {
                    fontetextomenor = al_load_font("assets/fontes/vcr_osd_mono.ttf", 0.7*TAMTEXTO, 0);
                    if (!fontetextomenor)
                    {
                        msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                    }
                }
            }
        }
    }

    cao_dormindo.ind_col = 0;
    cao_dormindo.ind_lin = 0;
    cao_dormindo.num_col = 2;
    cao_dormindo.num_lin = 1;

    al_play_sample(musica_gameover, VOLUME_MUSICA_JOGO, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

    while(executar == 1)
    {
        while(!al_is_event_queue_empty(queue)) // Checa todos os eventos
        {
            al_wait_for_event(queue, &evento);

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                executar = 0;
                retorno = 0;
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                atualizar = 1;

                cont_tempo++;
                if(cont_tempo >= TEMPO_GAME_OVER*FPS)
                {
                    executar = 0;
                }

                cont_sprite++;
                if(cont_sprite >= N_FRAMES_SPRITE_GAMEOVER)
                {
                    cont_sprite = 0;
                    if(cao_dormindo.ind_col == 1)
                        cao_dormindo.ind_col--;
                    else if(cao_dormindo.ind_col == 0)
                        cao_dormindo.ind_col++;
                }
            }
        }

        if(atualizar == 1)
        {
            al_clear_to_color(al_map_rgb(66,51,31));
            for(l=0; l<ceil((float)LARGURA/dim); l++)
            {
                for(h=0; h<ceil((float)ALTURA/dim); h++)
                {
                    al_draw_bitmap(fundomenu, dim*l, dim*h, 0);
                }
            }

            al_draw_text(fontetexto, al_map_rgb(250,0,100), LARGURA/2, ALTURA/4, ALLEGRO_ALIGN_CENTER, "Game Over");
            al_draw_scaled_bitmap(cao_dormindo.folha, (cao_dormindo.ind_col*al_get_bitmap_width(cao_dormindo.folha))/cao_dormindo.num_col, (cao_dormindo.ind_lin*al_get_bitmap_height(cao_dormindo.folha))/cao_dormindo.num_lin,
                                        al_get_bitmap_width(cao_dormindo.folha)/cao_dormindo.num_col, al_get_bitmap_height(cao_dormindo.folha)/cao_dormindo.num_lin, LARGURA/2-2*al_get_bitmap_width(cao_dormindo.folha), ALTURA/2+al_get_bitmap_height(cao_dormindo.folha)/2,
                                        10*(al_get_bitmap_width(cao_dormindo.folha)/cao_dormindo.num_col), 10*(al_get_bitmap_height(cao_dormindo.folha)/cao_dormindo.num_lin), 0);
            al_draw_textf(fontetextomenor, al_map_rgb(249,247,128), LARGURA/2, ALTURA/4 + 2.5*TAMTEXTO, ALLEGRO_ALIGN_CENTER, "Pontos: %d", pontuacao);
            al_flip_display();
            atualizar = 0;
        }
    }

    al_destroy_bitmap(cao_dormindo.folha);
    al_destroy_bitmap(fundomenu);
    al_destroy_font(fontetexto);
    al_destroy_sample(musica_gameover);

    return retorno;
}

int tela_salvar_score(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, int pontuacao)
{
    ALLEGRO_BITMAP *fundomenu = NULL;
    ALLEGRO_BITMAP *setas_nome = NULL;
    ALLEGRO_FONT *fonteletras = NULL, *fontetexto = NULL, *fontetextomenor = NULL;;
    ALLEGRO_EVENT evento;
    ALLEGRO_SAMPLE *menu_selec = NULL, *menu_mover = NULL;
    int retorno = 1; // Retorna 0 se apertou para sair do jogo
    int atualizar = 0, executar = 1;
    int dim, l, h;
    int posicao = 0; // 1, 2 ou 3
    char nome[4] = "AAA";
    SCORE score;
    int tecla = -1;
    float r = 249, g = 247, b = 128;
    int inc = 1;

    fundomenu = al_load_bitmap("assets/img/menu_fundo.jpg");
    if (!fundomenu)
    {
        msg_erro("Falha carregando menu_fundo.jpg.", display);
    }
    else
    {
        dim = al_get_bitmap_height(fundomenu);

        fonteletras = al_load_font("assets/fontes/vcr_osd_mono.ttf", 1.66*TAMTEXTO, 0);
        if (!fonteletras)
        {
            msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
        }
        else
        {
            fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", TAMTEXTO, 0);
            if (!fontetexto)
            {
                msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
            }
            else
            {
                setas_nome = al_load_bitmap("assets/img/jogo/setas_nome.png");
                if (!setas_nome)
                {
                    msg_erro("Falha carregando setas_nome.png.", display);
                }
                else
                {
                    fontetextomenor = al_load_font("assets/fontes/vcr_osd_mono.ttf", 0.5*TAMTEXTO, 0);
                    if (!fontetextomenor)
                    {
                        msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
                    }
                    else
                    {
                        menu_selec = al_load_sample("assets/audio/menu_lego_sw_selecionar.ogg");
                        if(!menu_selec)
                        {
                            msg_erro("Falha ao carregar menu_lego_sw_selecionar.ogg.", display);
                            executar = 0;
                        }
                        else
                        {
                            menu_mover = al_load_sample("assets/audio/menu_lego_sw_mover.ogg");
                            if(!menu_mover)
                            {
                                msg_erro("Falha ao carregar menu_lego_sw_mover.ogg.", display);
                                executar = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    while(executar == 1)
    {
        while(!al_is_event_queue_empty(queue)) // Checa todos os eventos
        {
            al_wait_for_event(queue, &evento);

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                if(al_show_native_message_box(display, "Mouse Trap", "Tem certeza que quer fechar o jogo?", "Todo o progresso nao salvo sera perdido.", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL))
                { // Se apertar em "sim" (retorna 1)
                    executar = 0;
                    retorno = 0;
                }
            }
            else if(evento.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                switch(evento.keyboard.keycode)
                {
                    case ALLEGRO_KEY_D :
                    case ALLEGRO_KEY_RIGHT :
                        tecla = 1;
                        break;

                    case ALLEGRO_KEY_A :
                    case ALLEGRO_KEY_LEFT :
                        tecla = 2;
                        break;

                    case ALLEGRO_KEY_ESCAPE :
                        tecla = 3;
                        break;

                    case ALLEGRO_KEY_ENTER :
                    case ALLEGRO_KEY_PAD_ENTER :
                        tecla = 4;
                        break;

                    case ALLEGRO_KEY_W :
                    case ALLEGRO_KEY_UP :
                        tecla = 5;
                        break;

                    case ALLEGRO_KEY_S :
                    case ALLEGRO_KEY_DOWN :
                        tecla = 6;
                        break;
                }
                if(tecla == 1)
                {
                    if(posicao < 2)
                    {
                        al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        posicao++;
                    }
                }
                else if(tecla == 2)
                {
                    if(posicao > 0)
                    {
                        al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        posicao--;
                    }
                }
                else if(tecla == 3)
                {
                    al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    executar = 0;
                }
                else if(tecla == 4)
                {
                    al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    score.pontos = pontuacao;
                    strcpy(score.nome, nome);
                    salvarScore(score);

                    executar = 0;
                }
                else if(tecla == 5) // Para cima
                {
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    if(nome[posicao] == 'A')
                    {
                        nome[posicao] = 'Z';
                    }
                    else
                    {
                        nome[posicao]--;
                    }
                }
                else if(tecla == 6) // Para baixo
                {
                    al_play_sample(menu_mover, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    if(nome[posicao] == 'Z')
                    {
                        nome[posicao] = 'A';
                    }
                    else
                    {
                        nome[posicao]++;
                    }
                }
                tecla = -1;
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                atualizar = 1;

                if(g >= 247 || g <= 61)
                {
                   inc = inc*(-1);
                }
                g = g + (186/(FPS*VEL_CORES_MENU))*inc;
                r = r - (5/(FPS*VEL_CORES_MENU))*inc;
                b = b + (76/(FPS*VEL_CORES_MENU))*inc;
            }
        }

        if(atualizar == 1)
        {
            al_clear_to_color(al_map_rgb(66,51,31));
            for(l=0; l<ceil((float)LARGURA/dim); l++)
            {
                for(h=0; h<ceil((float)ALTURA/dim); h++)
                {
                    al_draw_bitmap(fundomenu, dim*l, dim*h, 0);
                }
            }

            al_draw_text(fontetextomenor, al_map_rgb(249, 247, 128), LARGURA - TAMTEXTO, ALTURA - TAMTEXTO, ALLEGRO_ALIGN_RIGHT, "Cancelar: ESC   Confirmar: Enter");
            al_draw_text(fontetexto, al_map_rgb(107, 54, 172), LARGURA/2, ALTURA/10, ALLEGRO_ALIGN_CENTER, "Salvar Pontuacao Final");
            al_draw_textf(fontetexto, al_map_rgb(249,247,128), LARGURA/2, ALTURA/3+TAMTEXTO/1.5, ALLEGRO_ALIGN_CENTER, "%d pontos", pontuacao);
            al_draw_text(fontetexto, al_map_rgb(160, 112, 147), LARGURA/3+2.25*TAMTEXTO, ALTURA/2+TAMTEXTO/2.1, ALLEGRO_ALIGN_LEFT, "Nome: ");

            al_draw_textf(fonteletras, al_map_rgb(249,247,128), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_LEFT, "%c%c%c", nome[0], nome[1], nome[2]);

            if(posicao == 0) // 1a letra
            {
                al_draw_bitmap(setas_nome, LARGURA/2, ALTURA/2-al_get_bitmap_height(setas_nome)/4, 0);
                al_draw_textf(fonteletras, al_map_rgb(r,g,b), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_LEFT, "%c", nome[0]);
            }
            else if(posicao == 1) // 2a letra
            {
                al_draw_bitmap(setas_nome, LARGURA/2+35, ALTURA/2-al_get_bitmap_height(setas_nome)/4, 0);
                al_draw_textf(fonteletras, al_map_rgb(r,g,b), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_LEFT, " %c", nome[1]);
            }
            else if(posicao == 2) // 3a letra
            {
                al_draw_bitmap(setas_nome, LARGURA/2+68, ALTURA/2-al_get_bitmap_height(setas_nome)/4, 0);
                al_draw_textf(fonteletras, al_map_rgb(r,g,b), LARGURA/2, ALTURA/2, ALLEGRO_ALIGN_LEFT, "  %c", nome[2]);
            }

            al_flip_display();
            atualizar = 0;
        }
    }

    al_destroy_bitmap(fundomenu);
    al_destroy_font(fonteletras);
    al_destroy_font(fontetexto);
    al_destroy_bitmap(setas_nome);
    al_destroy_font(fontetextomenor);
    al_destroy_sample(menu_selec);
    al_destroy_sample(menu_mover);

    return retorno;
}

int tela_zerar_jogo(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_TIMER *timer, int pontos, int vidas)
{
    ALLEGRO_EVENT evento;
    ALLEGRO_FONT *fontetexto = NULL;
    SPRITE fireworks;
    ALLEGRO_BITMAP *fundo = NULL;
    ALLEGRO_BITMAP *canvas_preto = NULL;
    ALLEGRO_AUDIO_STREAM *musica = NULL;
    int executar = 1, retorno = 1, atualizar = 0;
    int cont_firew = 0, cont_tempo = 0;
    int canvas_y = -1*ALTURA, canvas_vel = 1, canvas_acel = 1.33;

    fontetexto = al_load_font("assets/fontes/vcr_osd_mono.ttf", 1.75*TAMTEXTO, 0);
    if (!fontetexto)
    {
        msg_erro("Falha ao carregar vcr_osd_mono.ttf.", display);
    }
    else
    {
        fireworks.folha = NULL;
        fireworks.folha = al_load_bitmap("assets/img/fireworks.png");
        if(!fireworks.folha)
        {
            msg_erro("Falha ao carregar fireworks.png.", display);
        }
        else
        {
            fundo = al_load_bitmap("assets/img/cmp.jpg");
            if(!fundo)
            {
                msg_erro("Falha ao carregar cmp.jpg.", display);
            }
            else
            {
                musica = al_load_audio_stream("assets/audio/cmp_mariokartwii.ogg", 1024, 4);
                if(!musica)
                {
                    msg_erro("Falha ao carregar cmp_mariokartwii.ogg.", display);
                }
                else
                {
                    canvas_preto = al_create_bitmap(LARGURA, ALTURA);
                    if(!canvas_preto)
                    {
                        msg_erro("Falha ao criar bitmap.", display);
                    }
                    else
                    {
                        al_set_target_bitmap(canvas_preto); // Deixa como alvo do desenho o bitmap 'canvas'
                        al_clear_to_color(al_map_rgb(0,0,0)); // Colore o bitmap
                        al_set_target_bitmap(al_get_backbuffer(display)); // Volta a deixar como padrão o desenho na janela
                    }
                }
            }
        }
    }

    al_set_audio_stream_gain(musica, VOLUME_MUSICA_MENU);
    //Liga a musica de fundo no mixer:
    al_attach_audio_stream_to_mixer(musica, al_get_default_mixer());
    //Define que a musica de fundo vai tocar no modo repeat:
    al_set_audio_stream_playmode(musica, ALLEGRO_PLAYMODE_ONCE);

    fireworks.ind_col = 0;
    fireworks.ind_lin = 0;
    fireworks.num_col = 3;
    fireworks.num_lin = 4;

    while(executar == 1)
    {
        while(!al_is_event_queue_empty(queue))
        {
            al_wait_for_event(queue, &evento);

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                if(al_show_native_message_box(display, "Mouse Trap", "Tem certeza que quer fechar o jogo?", "Todo o progresso nao salvo sera perdido.", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL))
                { // Se apertar em "sim" (retorna 1)
                    executar = 0;
                    retorno = 0;
                }
            }
            else if(evento.type == ALLEGRO_EVENT_TIMER)
            {
                atualizar = 1;

                cont_firew++;
                if(cont_firew >= N_FRAMES_FIREWORK)
                {
                    cont_firew = 0;
                    if(fireworks.ind_col < 2)
                        fireworks.ind_col++;
                    else if(fireworks.ind_col == 2)
                    {
                        fireworks.ind_col = 0;
                        if(fireworks.ind_lin < 3)
                            fireworks.ind_lin++;
                        else if(fireworks.ind_lin == 3)
                            fireworks.ind_lin = 0;
                    }
                }

                cont_tempo++;
                if(cont_tempo >= DURACAO_JOGO_ZERADO*FPS)
                {
                    executar = 0;
                }
            }
        }

        if(atualizar == 1)
        {
            al_clear_to_color(al_map_rgb(0,0,0));

            al_draw_bitmap(fundo, 0, 0, 0);
            al_draw_scaled_bitmap(fireworks.folha, (fireworks.ind_col*al_get_bitmap_width(fireworks.folha))/fireworks.num_col, (fireworks.ind_lin*al_get_bitmap_height(fireworks.folha))/fireworks.num_lin,
                                        al_get_bitmap_width(fireworks.folha)/fireworks.num_col, al_get_bitmap_height(fireworks.folha)/fireworks.num_lin, LARGURA/2-al_get_bitmap_width(fireworks.folha)/(2*fireworks.num_col), ALTURA/8,
                                        (al_get_bitmap_width(fireworks.folha)/fireworks.num_col), (al_get_bitmap_height(fireworks.folha)/fireworks.num_lin), 0);
            al_draw_text(fontetexto, al_map_rgb(0,0,255), LARGURA/2, ALTURA/10, ALLEGRO_ALIGN_CENTER, "Parabens, voce venceu!");

            if(cont_tempo >= (DURACAO_JOGO_ZERADO-0.7)*FPS)
            {
                al_draw_bitmap(canvas_preto, 0, canvas_y, 0);
                if(canvas_y < 0)
                {
                    canvas_vel+=canvas_acel; // A cada iteração, a velocidade do bloco aumenta de acordo com sua aceleração
                    canvas_y+=canvas_vel;
                }
                else
                {
                    al_clear_to_color(al_map_rgb(0,0,0));
                }
            }
            atualizar = 0;
            al_flip_display();
        }
    }

    al_destroy_bitmap(fireworks.folha);
    al_destroy_bitmap(fundo);
    al_destroy_bitmap(canvas_preto);
    al_destroy_font(fontetexto);

    if(tela_salvar_score(display, queue, timer, pontos + vidas*500) == 0) // Ganha mais 500 pontos para cada vida restante
    {
        retorno = 0;
    }

    al_destroy_audio_stream(musica);

    return retorno;
}

int main()
{
    int executar;
    char selecao;
    SAVE carregar; // Pega o retorno da função carregar_jogo
    ALLEGRO_DISPLAY *janela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *temp = NULL;
    ALLEGRO_AUDIO_STREAM *musica_de_fundo = NULL;
    ALLEGRO_SAMPLE *menu_selec = NULL;
    SAVE padrao = {1, 0, 3, 2};

    /* Precisa estar antes de todas as funções do Allegro. Como a janela está sendo criada em main
    e precisamos dela para as funções em "inicializar", deixamos al_init aqui. */
    if(!al_init()) // Executa al_init(); se falhar, retorna 0 e roda o if
    {
        msg_erro("Falha ao inicializar a Allegro.", janela);
        return 0;
    }

    janela = al_create_display(LARGURA, ALTURA);
    /* Não foi colocado na função inicializar, pois:
    janela é um pointer;
    a função al_create_display() altera o endereço ao qual janela aponta
    assim, teríamos de passar o novo valor de janela pelo retorno da função ou mudar o valor
    diretamente no endereço. */
    al_set_window_title(janela, "Mouse Trap - Trabalho Final - 2018/2");
    if(!janela)
    {
        msg_erro("Falha ao criar display.", janela);
        return -1; // 'f' para falha. Na seleção do menu, vai cair no default (fecha o jogo)
    }
    // O mesmo para o timer:
    temp = al_create_timer(1/FPS);
    if(!temp)
    {
        msg_erro("Falha ao criar temporizador", janela);
        return -1;
    }
    al_start_timer(temp);
    // E para a fila de eventos:
    fila = al_create_event_queue();
    if(!fila)
    {
        msg_erro("Falha ao criar fila de eventos.", janela);
        return -1;
    }

    executar = init_addons(janela); // Inicializa a Allegro e seus add-ons

    // Os registros de eventos devem estar após init_addons, pois registram instalações feitas na função
    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(temp));

    // Inicializa o mixer; o parametro eh o numero de samples que poderao ser tocados ao mesmo tempo:
    if(!al_reserve_samples(NUM_SAMPLES)) // So precisa ser reservado 1 vez no programa inteiro
    {
        msg_erro("Falha ao reservar amostras de audio.", janela);
        executar = 0;
    }

    //Som ao selecionar algo/sair de algo no menu:
    menu_selec = al_load_sample("assets/audio/menu_lego_sw_selecionar.ogg");
    if(!menu_selec)
    {
        msg_erro("Falha ao carregar menu_lego_sw_selecionar.ogg.", janela);
        executar = 0;
    }

    //Musica de fundo para os menus:
    musica_de_fundo = al_load_audio_stream("assets/audio/fearofdark_dancing_on_the_moon.ogg", 4, 1024);
    if(!musica_de_fundo)
    {
        msg_erro("Falha ao carregar 'fearofdark_dancing_on_the_moon.ogg'", janela);
        executar = 0;
    }

    //Define o volume da musica:
    al_set_audio_stream_gain(musica_de_fundo, VOLUME_MUSICA_MENU);
    //Liga a musica de fundo no mixer:
    al_attach_audio_stream_to_mixer(musica_de_fundo, al_get_default_mixer());
    //Define que a musica de fundo vai tocar no modo repeat:
    al_set_audio_stream_playmode(musica_de_fundo, ALLEGRO_PLAYMODE_LOOP);

    while(executar == 1)
    {
        selecao = menu(janela, fila, temp);
        switch(selecao)
        /* Legenda:
        f -> Falhou/houve erros, entra no default
        N -> Novo jogo
        C -> Carregar jogo
        H -> High Scores
        Q -> Sair do jogo
        */
        {
            case 'N' :
                al_set_audio_stream_playing(musica_de_fundo, 0);
                if(!jogo(janela, fila, temp, padrao)) // Usa o SAVE padrão, que é nível 1, 3 vidas e 0 pontos.
                { // Se retornar 0 = pediu para fechar o jogo
                    executar = 0;
                }
                else // Se escolheu para voltar ao menu inicial (a música recomeça)
                {
                    al_set_audio_stream_playing(musica_de_fundo, 1);
                }
                break;

            case 'C' :
                carregar = carregar_jogo(janela, fila, temp);
                if(carregar.flag == 1) // Se carregou um save com jogo
                {
                    al_set_audio_stream_playing(musica_de_fundo, 0);
                    //Comeca a jogar no save carregado:
                    if(!jogo(janela, fila, temp, carregar)) // Se retornar 0 = pediu para fechar o jogo
                    {
                        executar = 0;
                    }
                    else // Se escolheu para voltar ao menu de dentro do jogo
                    {
                        al_set_audio_stream_playing(musica_de_fundo, 1);
                    }
                }
                else if(carregar.flag == 0)
                {
                    executar = 0; // Se a flag for 0, sai do programa
                }
                // Se a flag for 3, ou seja, quis voltar ao menu, sai direto do switch e volta para o início do while
                al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                break;
            case 'H' :
                if(mostra_high_scores(janela, fila, temp) == 0) // Roda a funcao. Se o retorno for 0 (apertou X para fechar o programa), sai do programa
                    executar = 0;
                else
                {
                    al_play_sample(menu_selec, VOLUME_EFEITOS_MENU, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
                break;

            case 'Q' :
                executar = 0; // Sai do while, saindo do jogo
                break;

            default :
                msg_erro("Erro no menu.", janela);
                return -1; // Fecha direto o programa, alegando erro
        }
    }
    desalocar(janela, fila, temp);
    al_destroy_audio_stream(musica_de_fundo);
    al_destroy_sample(menu_selec);

    return 0;
}

