/*
 * Ferramenta de seed do SIGEH-DF.
 * Recria o banco oficial a partir do schema versionado e popula dados minimos
 * para o sistema ser usavel: um usuario por papel (com senha com hash) e alguns
 * cadastros de exemplo. Reaproveita os repositories reais do backend.
 *
 * Uso (a partir de src/backend/web/):
 *   ./build/seed [caminho_schema] [caminho_banco]
 * Padroes: ../data/schema_v3.sql e ../data/sigeh_v3.db (caminho default do db).
 */

#include "database.h"
#include "usuario_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "ala_repository.h"
#include "leito_repository.h"
#include "checkin_repository.h"
#include "triagem_repository.h"
#include "agendamento_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "analito_repository.h"
#include "financeiro_repository.h"
#include "lote_repository.h"
#include "medicamento_repository.h"
#include "estoque_repository.h"
#include "prescricao_repository.h"
#include "enfermagem_repository.h"
#include "farmacia_service.h"
#include "vacina_repository.h"
#include "internacao_repository.h"
#include "solicitacao_repository.h"

#include <stdio.h>
#include <stdlib.h>

#define SCHEMA_PADRAO "../data/schema_v3.sql"

static int etapa_ok(const char *etapa, int ok)
{
    if (!ok)
    {
        fprintf(stderr, "falha ao popular etapa: %s\n", etapa);
        return 0;
    }
    printf("  ok: %s\n", etapa);
    return 1;
}

/* Cria um usuario e relata no stdout; aborta o seed em caso de falha. */
static int criar_usuario(const char *nome, const char *login, const char *senha,
                         const char *papel, int paciente_id, int medico_id)
{
    if (usuario_repo_criar(nome, login, senha, papel, paciente_id, medico_id) != 1)
    {
        fprintf(stderr, "falha ao criar usuario '%s'\n", login);
        return 0;
    }

    printf("  usuario: %-10s papel=%-10s senha=%s\n", login, papel, senha);
    return 1;
}

/*
 * Complementa o seed narrativo acima com uma massa maior e reproduzivel.
 * Os registros usam chaves naturais identificaveis (CPF/CRM/nome com prefixo
 * "Demonstracao") para facilitar a apresentacao sem depender de IDs fixos.
 *
 * Sessoes nao sao semeadas porque sao efemeras. Anexos tambem ficam vazios:
 * inserir apenas metadados sem os arquivos correspondentes criaria links
 * quebrados e uma demonstracao enganosa.
 */
static int popular_demonstracao_completa(void)
{
    const char *scripts[] = {
        /* Completa os catalogos-base ate 30 opcoes. */
        "WITH RECURSIVE n(i) AS (SELECT 8 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO especialidades_clinicas(nome,ativo) "
        "SELECT CASE i "
        "WHEN 8 THEN 'Dermatologia' WHEN 9 THEN 'Endocrinologia' "
        "WHEN 10 THEN 'Ginecologia' WHEN 11 THEN 'Oftalmologia' "
        "WHEN 12 THEN 'Otorrinolaringologia' WHEN 13 THEN 'Urologia' "
        "WHEN 14 THEN 'Nefrologia' WHEN 15 THEN 'Reumatologia' "
        "WHEN 16 THEN 'Hematologia' WHEN 17 THEN 'Oncologia' "
        "WHEN 18 THEN 'Psiquiatria' WHEN 19 THEN 'Infectologia' "
        "WHEN 20 THEN 'Geriatria' WHEN 21 THEN 'Cirurgia Geral' "
        "WHEN 22 THEN 'Cirurgia Vascular' WHEN 23 THEN 'Cirurgia Toracica' "
        "WHEN 24 THEN 'Anestesiologia' WHEN 25 THEN 'Medicina Intensiva' "
        "WHEN 26 THEN 'Medicina da Familia' WHEN 27 THEN 'Alergologia' "
        "WHEN 28 THEN 'Nutrologia' WHEN 29 THEN 'Proctologia' "
        "ELSE 'Medicina do Trabalho' END,1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<23) "
        "INSERT INTO medicos(nome,crm,especialidade,regiao_administrativa,ativo) "
        "SELECT printf('Dr(a). Especialista %02d',i),printf('CRM-DF %04d',1100+i),"
        "(SELECT nome FROM especialidades_clinicas WHERE id=7+i),1+((i*3)%31),1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<25) "
        "INSERT INTO pacientes(nome,nascimento,documento,tipo_documento,telefone,sexo,"
        "regiao_administrativa,responsavel,alergias,convenio_id,ativo) "
        "SELECT printf('Paciente Demonstracao %02d',i),"
        "date('1958-01-15','+'||(i*640)||' days'),printf('900000%05d',i),'CPF',"
        "printf('6198%07d',1000000+i),CASE WHEN i%2=0 THEN 'F' ELSE 'M' END,"
        "1+((i*5)%31),'',CASE i%6 WHEN 0 THEN 'Penicilina' WHEN 1 THEN 'Dipirona' "
        "WHEN 2 THEN 'Iodo' ELSE '' END,0,1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<28) "
        "INSERT INTO alas(nome,tipo,total_leitos,leitos_ocupados,ativo) "
        "SELECT printf('Ala Demonstracao %02d',i),1+(i%6),2,0,1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO leitos(ala_id,numero,status,paciente_id,ativo) "
        "SELECT 3+((i-1)%28),300+i,"
        "CASE i%10 WHEN 0 THEN 'MANUTENCAO' WHEN 1 THEN 'HIGIENIZACAO' "
        "ELSE 'DISPONIVEL' END,0,1 FROM n;",

        /* 30 usuarios no total: 6 contas principais + 24 perfis de consulta. */
        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<24) "
        "INSERT INTO usuarios(nome,login,senha_hash,salt,papel,paciente_id,medico_id,"
        "ativo,trocar_senha) "
        "SELECT printf('Usuario Demonstracao %02d',i),printf('demo%02d',i),"
        "u.senha_hash,u.salt,CASE i%4 WHEN 0 THEN 'ENFERMAGEM' "
        "WHEN 1 THEN 'CADASTRO' WHEN 2 THEN 'MEDICO' ELSE 'PACIENTE' END,"
        "CASE WHEN i%4=3 THEN 5+i ELSE 0 END,"
        "CASE WHEN i%4=2 THEN 7+i ELSE 0 END,1,0 FROM n CROSS JOIN usuarios u "
        "WHERE u.login='admin';"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO checkins(paciente_id,senha,destino,status,rechamadas,motivo,criado_em) "
        "SELECT 1+((i-1)%30),printf('D%03d',i),"
        "CASE WHEN i%3=0 THEN 'CONSULTA' ELSE 'TRIAGEM' END,"
        "CASE i%5 WHEN 0 THEN 'ENCERRADO' WHEN 1 THEN 'AGUARDANDO' "
        "WHEN 2 THEN 'EM_ATENDIMENTO' WHEN 3 THEN 'FALTOU' ELSE 'CANCELADO' END,"
        "i%3,CASE WHEN i%5=4 THEN 'Cancelado a pedido do paciente' ELSE '' END,"
        "datetime('2026-06-01','+'||(i-1)||' days','+'||(7+i%10)||' hours') FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO triagens(paciente_id,profissional_id,especialidade_principal_id,"
        "tipo_triagem,pontuacao,classificacao,prioridade,itens,justificativa,queixa,"
        "observacoes,pressao,temperatura,freq_cardiaca,saturacao,status,data_hora,"
        "versao,raiz_id,vigente,ativo) "
        "SELECT 1+((i-1)%30),4,1+(i%7),1+(i%5),2+(i%8),"
        "CASE i%5 WHEN 0 THEN 'Emergencia' WHEN 1 THEN 'Muito prioritario' "
        "WHEN 2 THEN 'Prioritario' WHEN 3 THEN 'Pouco prioritario' ELSE 'Nao prioritario' END,"
        "5-(i%5),'avaliacao_clinica,sinais_vitais','Classificacao baseada no protocolo',"
        "CASE i%4 WHEN 0 THEN 'Dor e mal-estar' WHEN 1 THEN 'Febre persistente' "
        "WHEN 2 THEN 'Tontura e nausea' ELSE 'Retorno para avaliacao' END,"
        "'Paciente consciente e orientado',printf('%d/%d',110+i%35,70+i%20),"
        "printf('%.1f',36.2+(i%7)*0.3),printf('%d',68+i%42),printf('%d',92+i%8),"
        "CASE WHEN i%4=0 THEN 'FINALIZADA' ELSE 'EM_TRIAGEM' END,"
        "datetime('2026-06-01','+'||(i-1)||' days','+8 hours'),1,0,1,1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO triagem_problemas(triagem_id,problema_id,especialidade_id,"
        "principal,observacao,ativo) "
        "SELECT 4+i,1+((i-1)%33),"
        "(SELECT especialidade_id FROM problemas_clinicos WHERE id=1+((i-1)%33)),"
        "1,'Problema principal identificado na triagem demonstrativa',1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO agendamentos(paciente_id,medico_id,especialidade,data,horario,status,"
        "motivo_cancelamento) "
        "SELECT 1+((i-1)%30),1+((i-1)%30),"
        "(SELECT especialidade FROM medicos WHERE id=1+((i-1)%30)),"
        "date('2026-06-20','+'||i||' days'),printf('%02d:%02d',8+(i%9),(i%2)*30),"
        "CASE i%5 WHEN 0 THEN 'CONCLUIDO' WHEN 1 THEN 'AGENDADO' "
        "WHEN 2 THEN 'AGENDADO' WHEN 3 THEN 'CANCELADO' ELSE 'REMANEJADO' END,"
        "CASE WHEN i%5=3 THEN 'Indisponibilidade informada' ELSE '' END FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO prontuarios(paciente_id,medico_id,data,observacoes,diagnostico,"
        "conduta,alerta_importante,versao,raiz_id,vigente,justificativa,ativo) "
        "SELECT 1+((i-1)%30),1+((i-1)%30),date('2026-05-20','+'||i||' days'),"
        "'Consulta demonstrativa com anamnese e exame fisico registrados',"
        "CASE i%5 WHEN 0 THEN 'Hipertensao em acompanhamento' "
        "WHEN 1 THEN 'Infeccao viral de vias aereas' WHEN 2 THEN 'Dor musculoesqueletica' "
        "WHEN 3 THEN 'Cefaleia tensional' ELSE 'Acompanhamento preventivo' END,"
        "'Orientacoes, tratamento sintomatico e retorno programado',i%7=0,1,0,1,'',1 FROM n;",

        /* Laboratorio: 30 analitos e paineis, alem de exames em varios estados. */
        "WITH RECURSIVE n(i) AS (SELECT 4 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO analitos(codigo,nome,unidade,valor_ref_min,valor_ref_max,metodo,ativo) "
        "SELECT printf('AN%02d',i),printf('Analito Demonstracao %02d',i),"
        "CASE i%4 WHEN 0 THEN 'mg/dL' WHEN 1 THEN 'U/L' WHEN 2 THEN 'mmol/L' ELSE '%' END,"
        "10+i,50+i*2,'Metodo automatizado',1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 4 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO painel_analitos(tipo_exame,analito_id,ordem) "
        "SELECT 1+((i-1)%7),i,i FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO exames(paciente_id,medico_id,prontuario_id,tipo_exame,data_solicitacao,"
        "data_resultado,resultado,status,urgente,resultado_critico,motivo_cancelamento,"
        "versao,raiz_id,vigente,justificativa,ativo) "
        "SELECT 1+((i-1)%30),1+((i-1)%30),3+i,1+((i-1)%7),"
        "date('2026-06-01','+'||i||' days'),"
        "CASE WHEN i%4=0 THEN date('2026-06-02','+'||i||' days') ELSE '' END,"
        "CASE WHEN i%4=0 THEN 'Resultado liberado para demonstracao' ELSE '' END,"
        "CASE i%6 WHEN 0 THEN 'RESULTADO' WHEN 1 THEN 'SOLICITADO' "
        "WHEN 2 THEN 'AUTORIZADO' WHEN 3 THEN 'COLETADO' "
        "WHEN 4 THEN 'EM_ANALISE' ELSE 'CANCELADO' END,i%5=0,i%11=0,"
        "CASE WHEN i%6=5 THEN 'Paciente solicitou reagendamento' ELSE '' END,1,0,1,'',1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO exame_resultados_analitos(exame_id,analito_id,valor_numerico,"
        "valor_texto,fora_referencia,observacao) "
        "SELECT 2+i,1+((i-1)%30),25+i*1.7,'',i%6=0,"
        "CASE WHEN i%6=0 THEN 'Valor requer correlacao clinica' ELSE 'Dentro do esperado' END "
        "FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO prescricoes(paciente_id,medico_id,medicamento,dosagem,frequencia,"
        "via,duracao,observacoes,motivo_suspensao,ativo) "
        "SELECT 1+((i-1)%30),1+((i-1)%30),"
        "CASE i%6 WHEN 0 THEN 'Losartana' WHEN 1 THEN 'Paracetamol' "
        "WHEN 2 THEN 'Omeprazol' WHEN 3 THEN 'Amoxicilina' "
        "WHEN 4 THEN 'Ibuprofeno' ELSE 'Loratadina' END,"
        "CASE i%3 WHEN 0 THEN '500mg' WHEN 1 THEN '20mg' ELSE '50mg' END,"
        "CASE i%3 WHEN 0 THEN '8/8h' WHEN 1 THEN '1x/dia' ELSE '12/12h' END,"
        "'VO',printf('%d dias',3+(i%12)),'Reavaliar em caso de piora','',1 FROM n;",

        /* Financeiro com convenios, lotes e cobrancas em diferentes estados. */
        "WITH RECURSIVE n(i) AS (SELECT 3 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO convenios(nome,cobertura_pct,ativo) "
        "SELECT printf('Convenio Demonstracao %02d',i),50+(i%6)*10,1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 2 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO lotes(convenio_id,status,criado_em,fechado_em) "
        "SELECT i,CASE i%3 WHEN 0 THEN 'PAGO' WHEN 1 THEN 'ABERTO' ELSE 'FECHADO' END,"
        "datetime('2026-05-01','+'||i||' days'),"
        "CASE WHEN i%3=1 THEN '' ELSE date('2026-06-01','+'||i||' days') END FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO cobrancas(paciente_id,convenio_id,forma,origem,descricao,"
        "valor_centavos,status,motivo,lote_id,vencimento,guia,guia_validade,"
        "coberto_centavos,copart_centavos,criado_em) "
        "SELECT 1+((i-1)%30),CASE WHEN i%3=0 THEN 0 ELSE 1+(i%29) END,"
        "CASE WHEN i%3=0 THEN 'PARTICULAR' ELSE 'CONVENIO' END,"
        "CASE i%4 WHEN 0 THEN 'consulta' WHEN 1 THEN 'exame' "
        "WHEN 2 THEN 'internacao' ELSE 'medicamento' END,"
        "printf('Cobranca demonstrativa %02d',i),12000+i*1750,"
        "CASE i%5 WHEN 0 THEN 'PAGA' WHEN 1 THEN 'PENDENTE' "
        "WHEN 2 THEN 'AUTORIZADA' WHEN 3 THEN 'GLOSADA' ELSE 'CANCELADA' END,"
        "CASE WHEN i%5=3 THEN 'Divergencia de autorizacao' ELSE '' END,"
        "CASE WHEN i%3=0 THEN 0 ELSE 1+(i%30) END,date('2026-07-01','+'||i||' days'),"
        "CASE WHEN i%3=0 THEN '' ELSE printf('GUIA-DEMO-%04d',i) END,"
        "date('2026-09-01','+'||i||' days'),"
        "CASE WHEN i%3=0 THEN 0 ELSE (12000+i*1750)*8/10 END,"
        "CASE WHEN i%3=0 THEN 12000+i*1750 ELSE (12000+i*1750)*2/10 END,"
        "datetime('2026-06-01','+'||i||' days') FROM n;",

        /* Farmacia e vacinacao. */
        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO medicamentos(nome,apresentacao,unidade,estoque_minimo,"
        "preco_centavos,ativo) "
        "SELECT printf('Medicamento Demonstracao %02d',i),"
        "CASE i%3 WHEN 0 THEN 'comprimido 500mg' WHEN 1 THEN 'solucao oral' "
        "ELSE 'frasco ampola' END,CASE i%3 WHEN 0 THEN 'comprimido' "
        "WHEN 1 THEN 'ml' ELSE 'frasco' END,5+(i%20),200+i*135,1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO estoque_itens(medicamento_id,lote,validade,quantidade,localizacao) "
        "SELECT 4+i,printf('DEMO-%04d',i),date('2027-01-01','+'||(i*12)||' days'),"
        "10+i*4,printf('Farmacia %c - Prateleira %02d',65+(i%3),1+(i%12)) FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO movimentacoes(medicamento_id,estoque_item_id,tipo,quantidade,motivo,"
        "prescricao_id,usuario_id,usuario_login) "
        "SELECT 4+i,4+i,CASE i%4 WHEN 0 THEN 'AJUSTE' WHEN 1 THEN 'ENTRADA' "
        "WHEN 2 THEN 'SAIDA' ELSE 'ENTRADA' END,2+(i%15),"
        "CASE i%4 WHEN 0 THEN 'Inventario demonstrativo' WHEN 2 THEN 'Dispensacao' "
        "ELSE 'Reposicao programada' END,CASE WHEN i%4=2 THEN 2+i ELSE NULL END,"
        "1,'admin' FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO vacinas(nome,fabricante,doencas_alvo,doses_previstas,intervalo_dias,"
        "reforco_dias,medicamento_id,ativo) "
        "SELECT printf('Vacina Demonstracao %02d',i),"
        "CASE i%4 WHEN 0 THEN 'Butantan' WHEN 1 THEN 'Fiocruz' "
        "WHEN 2 THEN 'Bio-Manguinhos' ELSE 'Fabricante demonstrativo' END,"
        "printf('Prevencao demonstrativa %02d',i),1+(i%3),CASE WHEN i%3=0 THEN 30 ELSE 0 END,"
        "CASE WHEN i%4=0 THEN 365 ELSE 0 END,4+i,1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO aplicacoes_vacinas(paciente_id,vacina_id,medicamento_id,dose_numero,"
        "lote,validade,aplicador_usuario_id,aplicador_login,observacao,aplicada_em) "
        "SELECT 1+((i-1)%30),1+i,4+i,1,printf('DEMO-%04d',i),"
        "date('2027-01-01','+'||(i*12)||' days'),4,'enfermagem',"
        "'Aplicacao registrada para demonstracao',"
        "datetime('2026-05-20','+'||i||' days','+10 hours') FROM n;",

        /* Internacao historica, transferencias e registros de enfermagem. */
        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO internacoes(paciente_id,ala_id,leito_id,data_entrada,data_alta,status,"
        "responsavel,resumo_alta,diagnostico_final,orientacoes) "
        "SELECT 1+((i-1)%30),3+((i-1)%28),7+i,date('2026-04-01','+'||i||' days'),"
        "date('2026-04-03','+'||i||' days'),'ALTA',printf('Equipe assistencial %02d',i),"
        "'Evolucao favoravel durante a permanencia','Condicao clinica estabilizada',"
        "'Manter acompanhamento ambulatorial e sinais de alerta' FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO transferencias(internacao_id,leito_origem,leito_destino,data,responsavel) "
        "SELECT 2+i,7+i,7+((i)%30)+1,date('2026-04-02','+'||i||' days'),"
        "printf('Enfermagem demonstracao %02d',i) FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO leito_status_historico(leito_id,status,responsavel,criado_em) "
        "SELECT 7+i,CASE i%4 WHEN 0 THEN 'HIGIENIZACAO' WHEN 1 THEN 'OCUPADO' "
        "WHEN 2 THEN 'DISPONIVEL' ELSE 'MANUTENCAO' END,'seed demonstracao',"
        "datetime('2026-04-01','+'||i||' days') FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO evolucoes_enfermagem(paciente_id,autor_login,texto,pressao,temperatura,"
        "freq_cardiaca,saturacao,criado_em) "
        "SELECT 1+((i-1)%30),'enfermagem',"
        "'Evolucao demonstrativa: paciente estavel, consciente e orientado',"
        "printf('%d/%d',110+i%30,70+i%15),printf('%.1f',36.1+(i%6)*0.2),"
        "printf('%d',70+i%30),printf('%d',94+i%6),"
        "datetime('2026-06-01','+'||i||' days','+12 hours') FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO administracoes(prescricao_id,paciente_id,usuario_id,usuario_login,"
        "observacao,criado_em) "
        "SELECT 2+i,1+((i-1)%30),4,'enfermagem',"
        "'Medicamento administrado conforme prescricao, sem intercorrencias',"
        "datetime('2026-06-01','+'||i||' days','+14 hours') FROM n;",

        /* Portal do paciente, LGPD e trilha de auditoria. */
        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO solicitacoes_paciente(paciente_id,tipo,mensagem,status,criado_em,"
        "atualizado_em) "
        "SELECT 1+((i-1)%30),CASE WHEN i%2=0 THEN 'AGENDAMENTO' ELSE 'AJUDA' END,"
        "printf('Solicitacao demonstrativa %02d para atendimento do portal',i),"
        "CASE i%4 WHEN 0 THEN 'CONCLUIDA' WHEN 1 THEN 'ABERTA' "
        "WHEN 2 THEN 'EM_ANALISE' ELSE 'CANCELADA' END,"
        "datetime('2026-06-01','+'||i||' days'),"
        "CASE WHEN i%4=1 THEN '' ELSE datetime('2026-06-02','+'||i||' days') END FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO consentimentos(paciente_id,finalidade,versao_termo,status,"
        "concedido_em,revogado_em,motivo_revogacao,criado_em,ativo) "
        "SELECT 1+((i-1)%30),CASE i%4 WHEN 0 THEN 'Compartilhamento para pesquisa' "
        "WHEN 1 THEN 'Comunicacoes de saude' WHEN 2 THEN 'Atendimento assistencial' "
        "ELSE 'Acesso ao portal do paciente' END,'1.0',"
        "CASE WHEN i%5=0 THEN 'REVOGADO' ELSE 'CONCEDIDO' END,"
        "datetime('2026-05-01','+'||i||' days'),"
        "CASE WHEN i%5=0 THEN datetime('2026-06-01','+'||i||' days') ELSE '' END,"
        "CASE WHEN i%5=0 THEN 'Revogado a pedido do titular' ELSE '' END,"
        "datetime('2026-05-01','+'||i||' days'),1 FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO auditoria(usuario_id,usuario_login,acao,entidade,entidade_id,detalhe,"
        "criado_em) "
        "SELECT 1,'admin',CASE i%5 WHEN 0 THEN 'CONSENTIMENTO_REVOGADO' "
        "WHEN 1 THEN 'PACIENTE_CONSULTADO' WHEN 2 THEN 'EXAME_CRIADO' "
        "WHEN 3 THEN 'INTERNACAO_ALTA' ELSE 'ESTOQUE_MOVIMENTADO' END,"
        "CASE i%5 WHEN 0 THEN 'consentimento' WHEN 1 THEN 'paciente' "
        "WHEN 2 THEN 'exame' WHEN 3 THEN 'internacao' ELSE 'medicamento' END,"
        "i,'Registro gerado pela massa oficial de demonstracao',"
        "datetime('2026-06-01','+'||i||' days','+16 hours') FROM n;"

        "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i<30) "
        "INSERT INTO notificacoes(usuario_id,papel,titulo,mensagem,tipo,entidade,"
        "entidade_id,lida,criado_em) "
        "SELECT i,(SELECT papel FROM usuarios WHERE id=i),"
        "CASE i%4 WHEN 0 THEN 'Atualizacao de atendimento' "
        "WHEN 1 THEN 'Novo item na fila' WHEN 2 THEN 'Resultado disponivel' "
        "ELSE 'Comunicado do SIGEH-DF' END,"
        "printf('Notificacao demonstrativa %02d com informacoes do fluxo hospitalar.',i),"
        "CASE i%4 WHEN 0 THEN 'ATENDIMENTO' WHEN 1 THEN 'FILA' "
        "WHEN 2 THEN 'EXAME' ELSE 'INFO' END,"
        "CASE i%4 WHEN 0 THEN 'agendamento' WHEN 1 THEN 'checkin' "
        "WHEN 2 THEN 'exame' ELSE 'sistema' END,i,i%3=0,"
        "datetime('2026-06-20','+'||(i%5)||' days','+'||(7+i%12)||' hours') FROM n;"
    };
    size_t i;

    for (i = 0; i < sizeof(scripts) / sizeof(scripts[0]); i++)
    {
        if (db_executar(scripts[i]) != 1)
        {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    const char *schema = argc > 1 ? argv[1] : SCHEMA_PADRAO;
    char senha[16];
    char json[1024];
    int agendamentoId = 0;
    int medicoId = 0;
    int aplicacaoId = 0;
    int loteFinanceiroId = 0;

    /* O segundo argumento permite gerar um banco em outro caminho (testes). */
    if (argc > 2 && db_definir_caminho(argv[2]) != 1)
    {
        fprintf(stderr, "caminho de banco invalido: %s\n", argv[2]);
        return 1;
    }

    printf("Recriando banco em %s a partir de %s\n", db_caminho(), schema);

    if (db_resetar_com_schema(schema) != 1)
    {
        fprintf(stderr, "falha ao resetar banco com schema %s\n", schema);
        return 1;
    }

    /* Cadastros-base. O portal de paciente so oferta especialidades com medico
     * ativo, entao deixamos varias especialidades disponiveis para testes. */
    if (!etapa_ok("medicos",
        medico_repo_criar("Dra. Helena Prado", "CRM-DF 1001", "Clinico Geral", 1) == 1 &&
        medico_repo_criar("Dr. Marcos Vieira", "CRM-DF 1002", "Cardiologia", 1) == 1 &&
        medico_repo_criar("Dra. Livia Rocha", "CRM-DF 1003", "Ortopedia", 2) == 1 &&
        medico_repo_criar("Dr. Paulo Nunes", "CRM-DF 1004", "Pediatria", 5) == 1 &&
        medico_repo_criar("Dra. Aline Torres", "CRM-DF 1005", "Pneumologia", 1) == 1 &&
        medico_repo_criar("Dr. Renato Lima", "CRM-DF 1006", "Neurologia", 3) == 1 &&
        medico_repo_criar("Dra. Camila Bento", "CRM-DF 1007", "Gastroenterologia", 4) == 1))
    {
        return 1;
    }

    if (!etapa_ok("pacientes",
        paciente_repo_criar("Joao da Silva", "1981-05-12", "12345678900", "CPF",
                            "61999990000", "M", 1, "", "Penicilina") == 1 &&
        paciente_repo_criar("Maria Oliveira", "1990-08-22", "98765432100", "CPF",
                            "61988887777", "F", 1, "", "") == 1 &&
        paciente_repo_criar("Ana Souza", "2016-04-09", "55566677788", "CPF",
                            "61977776666", "F", 5, "Carla Souza", "Dipirona") == 1 &&
        paciente_repo_criar("Carlos Mendes", "1974-11-30", "44433322211", "CPF",
                            "61966665555", "M", 2, "", "") == 1 &&
        paciente_repo_criar("Beatriz Lima", "1955-02-18", "33322211100", "CPF",
                            "61955554444", "F", 3, "", "Iodo") == 1))
    {
        return 1;
    }

    if (!etapa_ok("alas e leitos",
        ala_repo_criar("Ala A - Clinica Medica", 1, 4) == 1 &&
        ala_repo_criar("Ala B - Pediatria", 2, 3) == 1 &&
        leito_repo_criar(1, 101) == 1 &&
        leito_repo_criar(1, 102) == 1 &&
        leito_repo_criar(1, 103) == 1 &&
        leito_repo_criar(1, 104) == 1 &&
        leito_repo_criar(2, 201) == 1 &&
        leito_repo_criar(2, 202) == 1 &&
        leito_repo_criar(2, 203) == 1 &&
        leito_repo_registrar_status(4, "MANUTENCAO", "seed") == 1))
    {
        return 1;
    }

    /* Um usuario por papel. PACIENTE/MEDICO ficam vinculados aos cadastros. */
    printf("Usuarios criados:\n");
    if (!criar_usuario("Administrador", "admin", "admin123", "ADMIN", 0, 0) ||
        !criar_usuario("Recepcao", "cadastro", "cadastro123", "CADASTRO", 0, 0) ||
        !criar_usuario("Dra. Helena Prado", "medico", "medico123", "MEDICO", 0, 1) ||
        !criar_usuario("Equipe Enfermagem", "enfermagem", "enfermagem123",
                       "ENFERMAGEM", 0, 0) ||
        !criar_usuario("Joao da Silva", "paciente", "paciente123", "PACIENTE", 1, 0) ||
        !criar_usuario("Maria Oliveira", "paciente2", "paciente123", "PACIENTE", 2, 0))
    {
        return 1;
    }

    /* Contas de demonstracao nao forcam troca no 1o acesso (mantem as
     * credenciais documentadas utilizaveis). A troca obrigatoria vale para
     * usuarios criados depois pelo admin. */
    db_executar("UPDATE usuarios SET trocar_senha = 0;");

    /* Recepcao/fila: uma senha em atendimento, uma aguardando e uma encerrada. */
    if (!etapa_ok("recepcao e check-in",
        checkin_repo_criar(1, "TRIAGEM", senha, sizeof(senha)) == 1 &&
        checkin_repo_chamar(1) == 1 &&
        checkin_repo_criar(2, "CONSULTA", senha, sizeof(senha)) == 1 &&
        checkin_repo_criar(3, "TRIAGEM", senha, sizeof(senha)) == 1 &&
        checkin_repo_chamar(3) == 1 &&
        checkin_repo_encerrar(3) == 1))
    {
        return 1;
    }

    if (!etapa_ok("triagem clinica",
        triagem_repo_criar_clinica(1, 4, 1, 3, "Pouco prioritario",
            "febre,mal-estar", "Febre e mal-estar ha 2 dias",
            "Hidratado, sem sinais de gravidade", "120/80", "37.8", "82", "98") > 0 &&
        triagem_repo_adicionar_problema(1, 22, 1, "febre") == 1 &&
        triagem_repo_criar_clinica(2, 4, 3, 5, "Emergencia",
            "dor_toracica,falta_ar", "Dor no peito e falta de ar",
            "Encaminhar para cardiologia", "150/95", "36.7", "108", "94") > 0 &&
        triagem_repo_adicionar_problema(2, 1, 1, "dor no peito") == 1 &&
        triagem_repo_criar_clinica(3, 4, 5, 3, "Prioritario",
            "febre_infantil,vomito", "Febre infantil com vomitos",
            "Acompanhada pela responsavel", "100/60", "38.4", "110", "97") > 0 &&
        triagem_repo_adicionar_problema(3, 17, 1, "febre infantil") == 1 &&
        triagem_repo_criar_clinica(4, 4, 2, 4, "Muito prioritario",
            "fratura_suspeita", "Queda com dor intensa no tornozelo",
            "Imobilizar e solicitar imagem", "130/85", "36.5", "96", "99") > 0 &&
        triagem_repo_adicionar_problema(4, 8, 1, "fratura suspeita") == 1))
    {
        return 1;
    }

    if (!etapa_ok("agenda de consultas",
        agendamento_repo_criar_por_especialidade(1, "Clinico Geral", "2026-07-01", "08:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(2, "Cardiologia", "2026-07-01", "09:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(3, "Pediatria", "2026-07-02", "10:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(4, "Ortopedia", "2026-07-03", "11:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(5, "Neurologia", "2026-07-04", "14:00", &agendamentoId, &medicoId) == 1))
    {
        return 1;
    }

    if (!etapa_ok("prontuarios e prescricoes",
        prontuario_repo_criar(1, 1, "2026-07-01", "Consulta inicial",
            "Sindrome gripal leve", "Hidratacao e sintomaticos", 0) == 1 &&
        prontuario_repo_criar(2, 2, "2026-07-01", "Atendimento cardiologico",
            "Dor toracica em investigacao", "Solicitar ECG e exames laboratoriais", 1) == 1 &&
        prontuario_repo_criar(4, 3, "2026-07-03", "Avaliacao ortopedica",
            "Entorse de tornozelo", "Imobilizacao e analgesia", 0) == 1 &&
        prescricao_repo_criar(1, 1, "Paracetamol", "750mg", "8/8h", "VO",
            "5 dias", "Usar se dor ou febre") == 1 &&
        prescricao_repo_criar(2, 2, "AAS", "100mg", "1x/dia", "VO",
            "15 dias", "Apos avaliacao cardiologica") == 1 &&
        administracao_criar(1, 4, "enfermagem", "Dose administrada sem intercorrencias") == 1 &&
        evolucao_criar(4, "enfermagem", "Paciente orientado, dor controlada apos imobilizacao.",
            "125/82", "36.6", "88", "98") == 1))
    {
        return 1;
    }

    if (!etapa_ok("laboratorio e exames",
        analito_criar("HGB", "Hemoglobina", "g/dL", 12.0, 16.0, "Automatizado") == 1 &&
        analito_criar("GLI", "Glicemia", "mg/dL", 70.0, 99.0, "Enzimatico") == 1 &&
        analito_criar("PCR", "Proteina C Reativa", "mg/L", 0.0, 5.0, "Imunoturbidimetria") == 1 &&
        painel_adicionar_analito(1, 1, 1) == 1 &&
        painel_adicionar_analito(1, 2, 2) == 1 &&
        painel_adicionar_analito(1, 3, 3) == 1 &&
        exame_repo_criar(2, 2, 2, 1, "2026-07-01", 1) == 1 &&
        exame_repo_atualizar_status(1, "AUTORIZADO") == 1 &&
        exame_repo_atualizar_status(1, "COLETADO") == 1 &&
        exame_repo_registrar_resultado_analito(1, 1, 13.8, "", "") == 1 &&
        exame_repo_registrar_resultado_analito(1, 2, 112.0, "", "Levemente elevada") == 1 &&
        exame_repo_registrar_resultado_analito(1, 3, 8.5, "", "Inflamacao discreta") == 1 &&
        exame_repo_registrar_resultado(1, "Sem evidencia de evento agudo; manter acompanhamento.", 0) == 1 &&
        exame_repo_criar(4, 3, 3, 3, "2026-07-03", 0) == 1))
    {
        return 1;
    }

    if (!etapa_ok("financeiro",
        convenio_criar("Saude DF Basico", 80) == 1 &&
        convenio_criar("Particular Assistido", 0) == 1 &&
        paciente_repo_definir_convenio(2, 1) == 1 &&
        paciente_repo_definir_convenio(5, 1) == 1 &&
        cobranca_criar(1, 0, "PARTICULAR", "consulta", "Consulta clinica particular",
            18000, "2026-07-10", "", "") == 1 &&
        cobranca_atualizar_status(1, "PAGA", "") == 1 &&
        cobranca_criar(2, 1, "CONVENIO", "consulta", "Consulta cardiologica",
            32000, "2026-07-20", "GUIA-DF-001", "2026-08-20") == 1 &&
        cobranca_atualizar_status(2, "AUTORIZADA", "") == 1 &&
        (loteFinanceiroId = lote_criar(1)) > 0 &&
        lote_adicionar_cobranca(loteFinanceiroId, 2) == 1 &&
        lote_fechar(loteFinanceiroId) == 1))
    {
        return 1;
    }

    if (!etapa_ok("farmacia e estoque",
        medicamento_criar("Dipirona", "500mg comprimido", "comprimido", 20, 150) == 1 &&
        medicamento_criar("Amoxicilina", "500mg capsula", "capsula", 15, 280) == 1 &&
        medicamento_criar("Vacina Influenza", "dose unica", "dose", 5, 0) == 1 &&
        medicamento_criar("Insulina NPH", "frasco 10ml", "frasco", 8, 4200) == 1 &&
        estoque_entrada(1, "DIP-A1", "2027-01-31", 120, "Farmacia A", 1, "admin") == 1 &&
        estoque_entrada(2, "AMX-B1", "2027-03-31", 60, "Farmacia A", 1, "admin") == 1 &&
        estoque_entrada(3, "FLU-26", "2026-12-31", 40, "Geladeira 1", 1, "admin") == 1 &&
        estoque_entrada(4, "INS-N1", "2026-09-30", 6, "Geladeira 2", 1, "admin") == 1 &&
        farmacia_service_dispensar(1, 1, 10, "dispensacao pos-consulta", 4,
            "enfermagem", json, sizeof(json)) == 1))
    {
        return 1;
    }

    if (!etapa_ok("vacinacao",
        vacina_criar("Influenza campanha 2026", "Butantan", "Gripe", 1, 0, 365, 3) == 1 &&
        vacina_aplicar(1, 1, 1, "FLU-26", "2026-12-31", 4, "enfermagem",
            "Aplicada em campanha interna", &aplicacaoId) == 1 &&
        vacina_aplicar(2, 1, 1, "FLU-26", "2026-12-31", 4, "enfermagem",
            "Aplicada antes da consulta cardiologica", &aplicacaoId) == 1))
    {
        return 1;
    }

    if (!etapa_ok("internacao e enfermagem",
        internacao_repo_criar(5, 1, 1, "2026-07-02", "Dra. Helena Prado") == 1 &&
        internacao_repo_criar(3, 2, 5, "2026-07-03", "Dr. Paulo Nunes") == 1 &&
        internacao_repo_dar_alta(2, "2026-07-05", "Quadro febril controlado",
            "Virose infantil", "Retornar se febre persistir") == 1 &&
        evolucao_criar(5, "enfermagem", "Paciente em observacao, sem queixas no momento.",
            "135/84", "36.8", "78", "97") == 1))
    {
        return 1;
    }

    if (!etapa_ok("solicitacoes do paciente",
        solicitacao_repo_criar(1, "AGENDAMENTO", "Desejo reagendar retorno clinico.", NULL) == 1 &&
        solicitacao_repo_criar(2, "AJUDA", "Tenho duvida sobre preparo do exame.", NULL) == 1))
    {
        return 1;
    }

    if (!etapa_ok("massa completa de demonstracao", popular_demonstracao_completa()))
    {
        return 1;
    }

    /* Garante o banco na ultima versao de schema (no-op se ja estiver). */
    if (db_migrar() != 1)
    {
        fprintf(stderr, "falha ao aplicar migracoes de schema\n");
        return 1;
    }

    printf("Seed concluido.\n");
    return 0;
}
