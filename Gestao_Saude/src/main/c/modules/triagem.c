#include "triagem.h"

void classificarTriagem(int pontuacao, char classificacao[])
{
    if (pontuacao >= 8)
        strcpy(classificacao, "Emergencia");
    else if (pontuacao >= 5)
        strcpy(classificacao, "Muito prioritario");
    else if (pontuacao >= 3)
        strcpy(classificacao, "Prioritario");
    else if (pontuacao >= 1)
        strcpy(classificacao, "Comum");
    else
        strcpy(classificacao, "Orientacao basica");
}

int excluirTriagem(int id)
{
    for (int i = 0; i < totalTriagens; i++)
    {
        if (triagens[i].id == id && triagens[i].ativo == 1)
        {
            triagens[i].ativo = 0;
            return 1;
        }
    }

    return 0;
}

void exibirNomeTipoTriagem(int tipoTriagem)
{
    switch (tipoTriagem)
    {
    case TRIAGEM_GERAL:
        printf("Geral");
        break;
    case TRIAGEM_ORTOPEDIA:
        printf("Ortopedia");
        break;
    case TRIAGEM_CARDIOLOGIA:
        printf("Cardiologia");
        break;
    case TRIAGEM_PNEUMOLOGIA:
        printf("Pneumologia");
        break;
    case TRIAGEM_PEDIATRIA:
        printf("Pediatria");
        break;
    default:
        printf("Desconhecido");
        break;
    }
}

int selecionarTipoTriagem(void)
{
    int tipoTriagem;

    printf("\n=============================================\n");
    printf("SELECIONE O TIPO DE TRIAGEM\n");
    printf("=============================================\n");
    printf("1. Geral\n");
    printf("2. Ortopedia\n");
    printf("3. Cardiologia\n");
    printf("4. Pneumologia\n");
    printf("5. Pediatria\n");
    printf("0. Cancelar\n");
    printf("---------------------------------------------\n");
    printf("Escolha uma opcao: ");
    scanf("%d", &tipoTriagem);

    if (tipoTriagem < 0 || tipoTriagem > 5)
    {
        printf("\nTipo de triagem invalido.\n");
        return -1;
    }

    return tipoTriagem;
}

int submenuTriagemGeral(int idadePaciente)
{
    int febre;
    int faltaAr;
    int dorIntensa;
    int pressaoAlta;
    int pontuacao = 0;

    printf("\nTRIAGEM GERAL\n");

    printf("Febre? (1-Sim / 0-Nao): ");
    scanf("%d", &febre);

    printf("Falta de ar? (1-Sim / 0-Nao): ");
    scanf("%d", &faltaAr);

    printf("Dor intensa? (1-Sim / 0-Nao): ");
    scanf("%d", &dorIntensa);

    printf("Pressao alta? (1-Sim / 0-Nao): ");
    scanf("%d", &pressaoAlta);

    if (febre == 1)
        pontuacao += 2;
    if (faltaAr == 1)
        pontuacao += 5;
    if (dorIntensa == 1)
        pontuacao += 4;
    if (pressaoAlta == 1)
        pontuacao += 2;
    if (idadePaciente >= 60 || idadePaciente <= 5)
        pontuacao += 2;

    return pontuacao;
}

int submenuTriagemOrtopedia(int idadePaciente)
{
    int suspeitaFratura;
    int deformidadeVisivel;
    int dificuldadeMovimento;
    int dorIntensa;
    int inchacoIntenso;
    int sangramento;
    int perdaSensibilidade;
    int pontuacao = 0;

    printf("\nTRIAGEM ORTOPEDICA\n");

    printf("Suspeita de fratura? (1-Sim / 0-Nao): ");
    scanf("%d", &suspeitaFratura);

    printf("Deformidade visivel? (1-Sim / 0-Nao): ");
    scanf("%d", &deformidadeVisivel);

    printf("Dificuldade ou impossibilidade de andar/mover? (1-Sim / 0-Nao): ");
    scanf("%d", &dificuldadeMovimento);

    printf("Dor intensa? (1-Sim / 0-Nao): ");
    scanf("%d", &dorIntensa);

    printf("Inchaco intenso? (1-Sim / 0-Nao): ");
    scanf("%d", &inchacoIntenso);

    printf("Sangramento? (1-Sim / 0-Nao): ");
    scanf("%d", &sangramento);

    printf("Perda de sensibilidade? (1-Sim / 0-Nao): ");
    scanf("%d", &perdaSensibilidade);

    if (suspeitaFratura == 1)
        pontuacao += 5;
    if (deformidadeVisivel == 1)
        pontuacao += 5;
    if (dificuldadeMovimento == 1)
        pontuacao += 4;
    if (dorIntensa == 1)
        pontuacao += 4;
    if (inchacoIntenso == 1)
        pontuacao += 2;
    if (sangramento == 1)
        pontuacao += 3;
    if (perdaSensibilidade == 1)
        pontuacao += 5;
    if (idadePaciente >= 60 || idadePaciente <= 5)
        pontuacao += 2;

    return pontuacao;
}

int submenuTriagemCardiologia(int idadePaciente)
{
    int dorPeito;
    int faltaAr;
    int desmaio;
    int palpitacaoIntensa;
    int pressaoAlta;
    int pontuacao = 0;

    printf("\nTRIAGEM CARDIOLOGICA\n");

    printf("Dor no peito? (1-Sim / 0-Nao): ");
    scanf("%d", &dorPeito);

    printf("Falta de ar? (1-Sim / 0-Nao): ");
    scanf("%d", &faltaAr);

    printf("Desmaio ou quase desmaio? (1-Sim / 0-Nao): ");
    scanf("%d", &desmaio);

    printf("Palpitacao intensa? (1-Sim / 0-Nao): ");
    scanf("%d", &palpitacaoIntensa);

    printf("Pressao alta? (1-Sim / 0-Nao): ");
    scanf("%d", &pressaoAlta);

    if (dorPeito == 1)
        pontuacao += 5;
    if (faltaAr == 1)
        pontuacao += 4;
    if (desmaio == 1)
        pontuacao += 5;
    if (palpitacaoIntensa == 1)
        pontuacao += 3;
    if (pressaoAlta == 1)
        pontuacao += 2;
    if (idadePaciente >= 60)
        pontuacao += 2;

    return pontuacao;
}

int submenuTriagemPneumologia(int idadePaciente)
{
    int faltaAr;
    int chiadoPeito;
    int tosseIntensa;
    int dorRespirar;
    int saturacaoBaixa;
    int febre;
    int pontuacao = 0;

    printf("\nTRIAGEM PNEUMOLOGICA\n");

    printf("Falta de ar? (1-Sim / 0-Nao): ");
    scanf("%d", &faltaAr);

    printf("Chiado no peito? (1-Sim / 0-Nao): ");
    scanf("%d", &chiadoPeito);

    printf("Tosse intensa? (1-Sim / 0-Nao): ");
    scanf("%d", &tosseIntensa);

    printf("Dor ao respirar? (1-Sim / 0-Nao): ");
    scanf("%d", &dorRespirar);

    printf("Saturacao baixa? (1-Sim / 0-Nao): ");
    scanf("%d", &saturacaoBaixa);

    printf("Febre? (1-Sim / 0-Nao): ");
    scanf("%d", &febre);

    if (faltaAr == 1)
        pontuacao += 5;
    if (chiadoPeito == 1)
        pontuacao += 3;
    if (tosseIntensa == 1)
        pontuacao += 2;
    if (dorRespirar == 1)
        pontuacao += 3;
    if (saturacaoBaixa == 1)
        pontuacao += 5;
    if (febre == 1)
        pontuacao += 2;
    if (idadePaciente >= 60 || idadePaciente <= 5)
        pontuacao += 2;

    return pontuacao;
}

int submenuTriagemPediatria(int idadePaciente)
{
    int febreAlta;
    int vomitosPersistentes;
    int convulsao;
    int prostracaoIntensa;
    int dificuldadeRespirar;
    int pontuacao = 0;

    printf("\nTRIAGEM PEDIATRICA\n");

    printf("Febre alta? (1-Sim / 0-Nao): ");
    scanf("%d", &febreAlta);

    printf("Vomitos persistentes? (1-Sim / 0-Nao): ");
    scanf("%d", &vomitosPersistentes);

    printf("Convulsao? (1-Sim / 0-Nao): ");
    scanf("%d", &convulsao);

    printf("Prostracao intensa? (1-Sim / 0-Nao): ");
    scanf("%d", &prostracaoIntensa);

    printf("Dificuldade para respirar? (1-Sim / 0-Nao): ");
    scanf("%d", &dificuldadeRespirar);

    if (febreAlta == 1)
        pontuacao += 3;
    if (vomitosPersistentes == 1)
        pontuacao += 3;
    if (convulsao == 1)
        pontuacao += 6;
    if (prostracaoIntensa == 1)
        pontuacao += 5;
    if (dificuldadeRespirar == 1)
        pontuacao += 5;
    if (idadePaciente <= 5)
        pontuacao += 2;

    return pontuacao;
}

void menuTriagem(void)
{
    int caso7;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE TRIAGEM\n");
        printf("=============================================\n");
        printf("1. Realizar Triagem\n");
        printf("2. Listar Triagens\n");
        printf("3. Excluir Triagem\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &caso7);

        switch (caso7)
        {
        case 1:
        {
            int pacienteId;
            int idadePaciente = 0;
            int pacienteEncontrado = 0;
            int tipoTriagem;
            int pontuacao = 0;

            if (totalTriagens >= MAX_TRIAGENS)
            {
                printf("\nLimite de triagens atingido.\n");
                break;
            }

            printf("\nID do paciente: ");
            scanf("%d", &pacienteId);

            for (int i = 0; i < totalPacientes; i++)
            {
                if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
                {
                    pacienteEncontrado = 1;
                    idadePaciente = pacientes[i].idade;
                    break;
                }
            }

            if (pacienteEncontrado == 0)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            tipoTriagem = selecionarTipoTriagem();

            if (tipoTriagem == 0)
            {
                printf("\nTriagem cancelada.\n");
                break;
            }

            if (tipoTriagem == -1)
            {
                break;
            }

            switch (tipoTriagem)
            {
            case TRIAGEM_GERAL:
                pontuacao = submenuTriagemGeral(idadePaciente);
                break;
            case TRIAGEM_ORTOPEDIA:
                pontuacao = submenuTriagemOrtopedia(idadePaciente);
                break;
            case TRIAGEM_CARDIOLOGIA:
                pontuacao = submenuTriagemCardiologia(idadePaciente);
                break;
            case TRIAGEM_PNEUMOLOGIA:
                pontuacao = submenuTriagemPneumologia(idadePaciente);
                break;
            case TRIAGEM_PEDIATRIA:
                pontuacao = submenuTriagemPediatria(idadePaciente);
                break;
            default:
                printf("\nTipo de triagem invalido.\n");
                break;
            }

            triagens[totalTriagens].id = totalTriagens + 1;
            triagens[totalTriagens].pacienteId = pacienteId;
            triagens[totalTriagens].tipoTriagem = tipoTriagem;
            triagens[totalTriagens].pontuacao = pontuacao;
            triagens[totalTriagens].ativo = 1;

            classificarTriagem(
                triagens[totalTriagens].pontuacao,
                triagens[totalTriagens].classificacao);

            printf("\nTriagem registrada com sucesso.\n");
            printf("Tipo de triagem: ");
            exibirNomeTipoTriagem(triagens[totalTriagens].tipoTriagem);
            printf("\nPontuacao: %d\n", triagens[totalTriagens].pontuacao);
            printf("Classificacao: %s\n", triagens[totalTriagens].classificacao);

            totalTriagens++;
            break;
        }

        case 2:
        {
            int encontrouAtiva = 0;

            if (totalTriagens == 0)
            {
                printf("\nNenhuma triagem cadastrada.\n");
                break;
            }

            printf("\nLista de Triagens:\n");

            for (int i = 0; i < totalTriagens; i++)
            {
                if (triagens[i].ativo == 1)
                {
                    encontrouAtiva = 1;
                    printf("\nID: %d\n", triagens[i].id);
                    printf("Paciente ID: %d\n", triagens[i].pacienteId);
                    printf("Tipo de triagem: ");
                    exibirNomeTipoTriagem(triagens[i].tipoTriagem);
                    printf("\nPontuacao: %d\n", triagens[i].pontuacao);
                    printf("Classificacao: %s\n", triagens[i].classificacao);
                }
            }

            if (encontrouAtiva == 0)
            {
                printf("\nNenhuma triagem ativa cadastrada.\n");
            }

            break;
        }

        case 3:
        {
            int idBusca;

            printf("\nDigite o ID da triagem que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirTriagem(idBusca) == 1)
            {
                printf("\nTriagem removida com sucesso.\n");
            }
            else
            {
                printf("\nTriagem nao encontrada ou ja inativa.\n");
            }

            break;
        }

        case 0:
            printf("\nVoltando ao menu principal...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (caso7 != 0);
}
