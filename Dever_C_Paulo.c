#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_REMEDIOS 100
#define MAX_HORARIOS 10
#define MAX_DIAS 7
#define ARQUIVO "medicamentos.txt"

typedef struct {
    char nome[100];
    char quantidade[50];
    char dias[MAX_DIAS][20];
    int num_dias;
    char horarios[MAX_HORARIOS][10];
    int num_horarios;
} Remedio;

Remedio lista[MAX_REMEDIOS];
int total = 0;

/* ---------- utilitários ---------- */

void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pausar() {
    printf("\nPressione ENTER para continuar...");
    limpar_buffer();
}

void limpar_tela() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* ---------- arquivo ---------- */

void salvar_arquivo() {
    FILE *f = fopen(ARQUIVO, "w");
    if (!f) {
        printf("Erro ao abrir o arquivo para salvar.\n");
        return;
    }

    fprintf(f, "========================================\n");
    fprintf(f, "   LISTA DE MEDICAMENTOS CADASTRADOS\n");
    fprintf(f, "========================================\n\n");

    for (int i = 0; i < total; i++) {
        fprintf(f, "Medicamento %d:\n", i + 1);
        fprintf(f, "  Nome      : %s\n", lista[i].nome);
        fprintf(f, "  Quantidade: %s\n", lista[i].quantidade);

        fprintf(f, "  Dias      : ");
        for (int d = 0; d < lista[i].num_dias; d++) {
            fprintf(f, "%s", lista[i].dias[d]);
            if (d < lista[i].num_dias - 1) fprintf(f, ", ");
        }
        fprintf(f, "\n");

        fprintf(f, "  Horarios  : ");
        for (int h = 0; h < lista[i].num_horarios; h++) {
            fprintf(f, "%s", lista[i].horarios[h]);
            if (h < lista[i].num_horarios - 1) fprintf(f, ", ");
        }
        fprintf(f, "\n");
        fprintf(f, "----------------------------------------\n");
    }

    fclose(f);
    printf("\n[Dados salvos em '%s' com sucesso!]\n", ARQUIVO);
}

/* ---------- carregar dados do arquivo ao iniciar ---------- */

void carregar_arquivo() {
    FILE *f = fopen(ARQUIVO, "r");
    if (!f) return;

    char linha[256];
    total = 0;

    while (fgets(linha, sizeof(linha), f) && total < MAX_REMEDIOS) {
        if (strncmp(linha, "Medicamento ", 12) != 0) continue;

        Remedio r;
        memset(&r, 0, sizeof(r));

        if (!fgets(linha, sizeof(linha), f)) break;
        sscanf(linha, "  Nome      : %99[^\n]", r.nome);

        if (!fgets(linha, sizeof(linha), f)) break;
        sscanf(linha, "  Quantidade: %49[^\n]", r.quantidade);

        if (!fgets(linha, sizeof(linha), f)) break;
        char *pos = strchr(linha, ':');
        if (pos) {
            pos++;
            while (*pos == ' ') pos++;
            char tmp[200];
            strncpy(tmp, pos, sizeof(tmp) - 1);
            tmp[sizeof(tmp)-1] = '\0';
            tmp[strcspn(tmp, "\n")] = '\0';
            char *tok = strtok(tmp, ", ");
            r.num_dias = 0;
            while (tok && r.num_dias < MAX_DIAS) {
                strcpy(r.dias[r.num_dias++], tok);
                tok = strtok(NULL, ", ");
            }
        }

        if (!fgets(linha, sizeof(linha), f)) break;
        pos = strchr(linha, ':');
        if (pos) {
            pos++;
            while (*pos == ' ') pos++;
            char tmp[200];
            strncpy(tmp, pos, sizeof(tmp) - 1);
            tmp[sizeof(tmp)-1] = '\0';
            tmp[strcspn(tmp, "\n")] = '\0';
            char *tok = strtok(tmp, ", ");
            r.num_horarios = 0;
            while (tok && r.num_horarios < MAX_HORARIOS) {
                strcpy(r.horarios[r.num_horarios++], tok);
                tok = strtok(NULL, ", ");
            }
        }

        lista[total++] = r;
    }

    fclose(f);
    if (total > 0)
        printf("[%d medicamento(s) carregado(s) do arquivo '%s']\n", total, ARQUIVO);
}

/* ---------- validação e normalização de horário ---------- */

/*
 * Aceita: "8:30", "08:30", "8:5", "08:05" etc.
 * Normaliza para o formato "HH:MM" com zeros à esquerda.
 * Retorna 1 se válido, 0 se inválido.
 */
int horario_valido_e_normalizar(char *entrada, char *saida) {
    int hh = -1, mm = -1;
    if (sscanf(entrada, "%d:%d", &hh, &mm) != 2) return 0;
    if (hh < 0 || hh > 23) return 0;
    if (mm < 0 || mm > 59) return 0;
    sprintf(saida, "%02d:%02d", hh, mm);
    return 1;
}

/* ---------- funcionalidades ---------- */

void cadastrar_remedio() {
    limpar_tela();
    if (total >= MAX_REMEDIOS) {
        printf("Limite de medicamentos atingido!\n");
        pausar();
        return;
    }

    Remedio r;
    memset(&r, 0, sizeof(r));

    printf("========================================\n");
    printf("        CADASTRAR MEDICAMENTO\n");
    printf("========================================\n\n");

    printf("Nome do medicamento: ");
    fgets(r.nome, sizeof(r.nome), stdin);
    r.nome[strcspn(r.nome, "\n")] = '\0';

    printf("Quantidade/dosagem (ex: 500mg, 1 comprimido): ");
    fgets(r.quantidade, sizeof(r.quantidade), stdin);
    r.quantidade[strcspn(r.quantidade, "\n")] = '\0';

    /* dias */
    printf("\nDias da semana disponiveis:\n");
    printf("  1-Segunda  2-Terca  3-Quarta  4-Quinta\n");
    printf("  5-Sexta    6-Sabado 7-Domingo\n");
    printf("Digite os numeros dos dias separados por espaco (ex: 1 3 5): ");

    char linha_dias[100];
    fgets(linha_dias, sizeof(linha_dias), stdin);

    const char *nomes_dias[] = {"Segunda", "Terca", "Quarta", "Quinta",
                                 "Sexta", "Sabado", "Domingo"};
    r.num_dias = 0;
    char *tok = strtok(linha_dias, " \n");
    while (tok && r.num_dias < MAX_DIAS) {
        int d = atoi(tok);
        if (d >= 1 && d <= 7) {
            int duplicado = 0;
            for (int k = 0; k < r.num_dias; k++) {
                if (strcmp(r.dias[k], nomes_dias[d - 1]) == 0) {
                    duplicado = 1;
                    break;
                }
            }
            if (!duplicado)
                strcpy(r.dias[r.num_dias++], nomes_dias[d - 1]);
        }
        tok = strtok(NULL, " \n");
    }

    if (r.num_dias == 0) {
        printf("\nNenhum dia valido informado. Cadastro cancelado.\n");
        pausar();
        return;
    }

    /* horários — o usuário escolhe quantos quer (até MAX_HORARIOS) */
    printf("\nDigite os horarios um por um (HH:MM ou H:MM).\n");
    printf("Pressione ENTER sem digitar nada para encerrar.\n\n");

    r.num_horarios = 0;
    while (r.num_horarios < MAX_HORARIOS) {
        char entrada[20];
        char normalizado[10];

        printf("  Horario %d (ENTER para encerrar): ", r.num_horarios + 1);
        fgets(entrada, sizeof(entrada), stdin);
        entrada[strcspn(entrada, "\n")] = '\0';

        /* ENTER em branco encerra */
        if (strlen(entrada) == 0) {
            if (r.num_horarios == 0) {
                printf("  Informe pelo menos um horario.\n");
                continue;
            }
            break;
        }

        if (!horario_valido_e_normalizar(entrada, normalizado)) {
            printf("  Horario invalido! Use o formato HH:MM (ex: 08:30 ou 8:30).\n");
            continue;
        }

        /* verifica duplicata */
        int dup = 0;
        for (int k = 0; k < r.num_horarios; k++) {
            if (strcmp(r.horarios[k], normalizado) == 0) {
                dup = 1;
                break;
            }
        }
        if (dup) {
            printf("  Esse horario ja foi cadastrado para este medicamento.\n");
            continue;
        }

        strcpy(r.horarios[r.num_horarios++], normalizado);

        if (r.num_horarios == MAX_HORARIOS)
            printf("  Limite de %d horarios atingido.\n", MAX_HORARIOS);
    }

    lista[total++] = r;
    printf("\nMedicamento '%s' cadastrado com sucesso!\n", r.nome);

    salvar_arquivo();
    pausar();
}

void exibir_detalhes(int idx) {
    limpar_tela();
    Remedio *r = &lista[idx];

    printf("========================================\n");
    printf("   DETALHES DO MEDICAMENTO\n");
    printf("========================================\n\n");
    printf("Nome      : %s\n", r->nome);
    printf("Quantidade: %s\n", r->quantidade);

    printf("Dias      : ");
    for (int d = 0; d < r->num_dias; d++) {
        printf("%s", r->dias[d]);
        if (d < r->num_dias - 1) printf(", ");
    }
    printf("\n");

    printf("Horarios  : ");
    for (int h = 0; h < r->num_horarios; h++) {
        printf("%s", r->horarios[h]);
        if (h < r->num_horarios - 1) printf(", ");
    }
    printf("\n");
    pausar();
}

void listar_medicamentos() {
    limpar_tela();
    printf("========================================\n");
    printf("       LISTA DE MEDICAMENTOS\n");
    printf("========================================\n\n");

    if (total == 0) {
        printf("Nenhum medicamento cadastrado.\n");
        pausar();
        return;
    }

    for (int i = 0; i < total; i++) {
        printf("  [%d] %s  (%s)\n", i + 1, lista[i].nome, lista[i].quantidade);
    }

    printf("\nDigite o numero para ver detalhes (0 para voltar): ");
    int op;
    scanf("%d", &op);
    limpar_buffer();

    if (op >= 1 && op <= total) {
        exibir_detalhes(op - 1);
    }
}

void buscar_por_horario() {
    limpar_tela();
    printf("========================================\n");
    printf("      BUSCAR POR HORARIO\n");
    printf("========================================\n\n");

    if (total == 0) {
        printf("Nenhum medicamento cadastrado.\n");
        pausar();
        return;
    }

    char todos[MAX_REMEDIOS * MAX_HORARIOS][10];
    int n_todos = 0;

    for (int i = 0; i < total; i++) {
        for (int h = 0; h < lista[i].num_horarios; h++) {
            int existe = 0;
            for (int k = 0; k < n_todos; k++) {
                if (strcmp(todos[k], lista[i].horarios[h]) == 0) {
                    existe = 1;
                    break;
                }
            }
            if (!existe)
                strcpy(todos[n_todos++], lista[i].horarios[h]);
        }
    }

    /* ordena */
    for (int i = 0; i < n_todos - 1; i++) {
        for (int j = i + 1; j < n_todos; j++) {
            if (strcmp(todos[i], todos[j]) > 0) {
                char tmp[10];
                strcpy(tmp, todos[i]);
                strcpy(todos[i], todos[j]);
                strcpy(todos[j], tmp);
            }
        }
    }

    printf("Horarios com medicamentos cadastrados:\n\n");
    for (int h = 0; h < n_todos; h++) {
        printf("  Horario: %s\n", todos[h]);
        for (int i = 0; i < total; i++) {
            for (int k = 0; k < lista[i].num_horarios; k++) {
                if (strcmp(lista[i].horarios[k], todos[h]) == 0) {
                    printf("    -> %s (%s)\n", lista[i].nome, lista[i].quantidade);
                }
            }
        }
        printf("\n");
    }

    pausar();
}

/* ---------- menu principal ---------- */

int main() {
    carregar_arquivo();
    if (total > 0) pausar();

    int opcao;

    do {
        limpar_tela();
        printf("========================================\n");
        printf("    SISTEMA DE CONTROLE DE REMEDIOS\n");
        printf("========================================\n\n");
        printf("  [1] Cadastrar medicamento\n");
        printf("  [2] Listar medicamentos\n");
        printf("  [3] Buscar por horario\n");
        printf("  [4] Sair\n\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        limpar_buffer();

        switch (opcao) {
            case 1: cadastrar_remedio();    break;
            case 2: listar_medicamentos();  break;
            case 3: buscar_por_horario();   break;
            case 4:
                printf("\nSaindo... Ate logo!\n\n");
                break;
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
                pausar();
        }
    } while (opcao != 4);

    return 0;
}
