PRAGMA foreign_keys = ON;

-- =========================================================================
-- SIGEH-DF - schema_v3
-- -------------------------------------------------------------------------
-- Evolucao compativel do schema_v2: mantem as colunas usadas pelos
-- repositories atuais (nada quebra) e prepara a base do sistema:
--   - usuarios ganham nome e data de criacao (cadastro de usuario real);
--   - tabela de auditoria para rastrear acoes sensiveis (login, alta, etc.);
--   - indices para as buscas mais frequentes.
-- Remodelagens mais profundas (nascimento do paciente, enum de status de
-- leito, maquina de estados de exame...) acontecem nas etapas dos seus
-- respectivos modulos, junto com repository e UI.
-- =========================================================================

-- Pacientes. A idade NAO e armazenada: e calculada a partir de 'nascimento'.
-- 'documento' guarda o CPF ou um documento alternativo, distinguido por
-- 'tipo_documento' ('CPF' | 'OUTRO'). 'responsavel' e obrigatorio para menores
-- de idade. 'alergias' destaca alertas clinicos. Historico nunca e apagado
-- fisicamente: a baixa e logica (ativo = 0).
CREATE TABLE pacientes (
    id INTEGER PRIMARY KEY,
    nome TEXT NOT NULL,
    nascimento TEXT NOT NULL,                 -- YYYY-MM-DD
    documento TEXT NOT NULL,
    tipo_documento TEXT NOT NULL DEFAULT 'CPF',
    telefone TEXT NOT NULL,
    sexo TEXT NOT NULL,
    regiao_administrativa INTEGER NOT NULL,
    responsavel TEXT NOT NULL DEFAULT '',
    alergias TEXT NOT NULL DEFAULT '',
    convenio_id INTEGER NOT NULL DEFAULT 0,   -- 0 = particular
    ativo INTEGER NOT NULL
);

CREATE TABLE medicos (
    id INTEGER PRIMARY KEY,
    nome TEXT NOT NULL,
    crm TEXT NOT NULL,
    especialidade TEXT NOT NULL,
    regiao_administrativa INTEGER NOT NULL,
    ativo INTEGER NOT NULL
);

-- Triagens. Alem da classificacao de risco, registra a queixa principal e os
-- sinais vitais basicos (texto livre para nao perder informacao do protocolo).
CREATE TABLE triagens (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    tipo_triagem INTEGER NOT NULL,
    pontuacao INTEGER NOT NULL,
    classificacao TEXT NOT NULL,
    itens TEXT NOT NULL DEFAULT '',
    justificativa TEXT NOT NULL DEFAULT '',
    queixa TEXT NOT NULL DEFAULT '',
    pressao TEXT NOT NULL DEFAULT '',
    temperatura TEXT NOT NULL DEFAULT '',
    freq_cardiaca TEXT NOT NULL DEFAULT '',
    saturacao TEXT NOT NULL DEFAULT '',
    versao INTEGER NOT NULL DEFAULT 1,
    raiz_id INTEGER NOT NULL DEFAULT 0,
    vigente INTEGER NOT NULL DEFAULT 1,
    ativo INTEGER NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id)
);

CREATE TABLE agendamentos (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    data TEXT NOT NULL,
    horario TEXT NOT NULL,
    status TEXT NOT NULL,
    motivo_cancelamento TEXT NOT NULL DEFAULT '',
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id)
);

CREATE TABLE alas (
    id INTEGER PRIMARY KEY,
    nome TEXT NOT NULL,
    tipo INTEGER NOT NULL,
    total_leitos INTEGER NOT NULL,
    leitos_ocupados INTEGER NOT NULL,
    ativo INTEGER NOT NULL
);

-- Leitos. 'status' substitui o antigo booleano 'ocupado' por um enum:
-- DISPONIVEL | OCUPADO | HIGIENIZACAO | MANUTENCAO | BLOQUEADO.
-- paciente_id (0 = nenhum) guarda quem ocupa o leito no momento.
CREATE TABLE leitos (
    id INTEGER PRIMARY KEY,
    ala_id INTEGER NOT NULL,
    numero INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'DISPONIVEL',
    paciente_id INTEGER NOT NULL DEFAULT 0,
    ativo INTEGER NOT NULL,
    FOREIGN KEY (ala_id) REFERENCES alas(id)
);

-- Historico de mudanca de status dos leitos (nunca apagado).
CREATE TABLE leito_status_historico (
    id INTEGER PRIMARY KEY,
    leito_id INTEGER NOT NULL,
    status TEXT NOT NULL,
    responsavel TEXT NOT NULL DEFAULT '',
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (leito_id) REFERENCES leitos(id)
);

-- Internacoes. Registra responsavel pela admissao e, na alta, o resumo
-- clinico, o diagnostico final e as orientacoes ao paciente.
CREATE TABLE internacoes (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    ala_id INTEGER NOT NULL,
    leito_id INTEGER NOT NULL,
    data_entrada TEXT NOT NULL,
    data_alta TEXT NOT NULL,
    status TEXT NOT NULL,
    responsavel TEXT NOT NULL DEFAULT '',
    resumo_alta TEXT NOT NULL DEFAULT '',
    diagnostico_final TEXT NOT NULL DEFAULT '',
    orientacoes TEXT NOT NULL DEFAULT '',
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (ala_id) REFERENCES alas(id),
    FOREIGN KEY (leito_id) REFERENCES leitos(id)
);

-- Transferencias de leito durante uma internacao (origem/destino/responsavel).
CREATE TABLE transferencias (
    id INTEGER PRIMARY KEY,
    internacao_id INTEGER NOT NULL,
    leito_origem INTEGER NOT NULL,
    leito_destino INTEGER NOT NULL,
    data TEXT NOT NULL,
    responsavel TEXT NOT NULL DEFAULT '',
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (internacao_id) REFERENCES internacoes(id)
);

-- Prontuarios versionados: registro clinico imutavel. Uma retificacao NAO
-- altera nem apaga a linha original; cria uma nova versao (vigente = 1) e marca
-- a anterior como vigente = 0 (preservada). 'raiz_id' agrupa as versoes de um
-- mesmo registro; 'justificativa' explica a retificacao.
CREATE TABLE prontuarios (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    data TEXT NOT NULL,
    observacoes TEXT NOT NULL,
    diagnostico TEXT NOT NULL,
    conduta TEXT NOT NULL,
    alerta_importante INTEGER NOT NULL,
    versao INTEGER NOT NULL DEFAULT 1,
    raiz_id INTEGER NOT NULL DEFAULT 0,
    vigente INTEGER NOT NULL DEFAULT 1,
    justificativa TEXT NOT NULL DEFAULT '',
    ativo INTEGER NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id)
);

CREATE TABLE exames (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    prontuario_id INTEGER NOT NULL,
    tipo_exame INTEGER NOT NULL,
    data_solicitacao TEXT NOT NULL,
    data_resultado TEXT NOT NULL,
    resultado TEXT NOT NULL,
    status TEXT NOT NULL,
    urgente INTEGER NOT NULL,
    resultado_critico INTEGER NOT NULL DEFAULT 0,
    motivo_cancelamento TEXT NOT NULL DEFAULT '',
    versao INTEGER NOT NULL DEFAULT 1,
    raiz_id INTEGER NOT NULL DEFAULT 0,
    vigente INTEGER NOT NULL DEFAULT 1,
    justificativa TEXT NOT NULL DEFAULT '',
    ativo INTEGER NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id),
    FOREIGN KEY (prontuario_id) REFERENCES prontuarios(id)
);

-- Laboratorio: catalogo de analitos (cada medida de um exame, ex.: Hemoglobina,
-- Glicose) com unidade e faixa de referencia. A faixa alimenta, na etapa de
-- resultado estruturado, o sinalizador de valor fora do normal.
CREATE TABLE analitos (
    id INTEGER PRIMARY KEY,
    codigo TEXT NOT NULL,
    nome TEXT NOT NULL,
    unidade TEXT NOT NULL DEFAULT '',
    valor_ref_min REAL NOT NULL DEFAULT 0,
    valor_ref_max REAL NOT NULL DEFAULT 0,
    metodo TEXT NOT NULL DEFAULT '',
    ativo INTEGER NOT NULL DEFAULT 1
);

-- Painel: liga um tipo de exame (mesmo codigo usado em exames.tipo_exame) ao
-- conjunto de analitos que o compoem, na ordem de apresentacao do laudo.
CREATE TABLE painel_analitos (
    id INTEGER PRIMARY KEY,
    tipo_exame INTEGER NOT NULL,
    analito_id INTEGER NOT NULL,
    ordem INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY (analito_id) REFERENCES analitos(id)
);

CREATE TABLE prescricoes (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    medicamento TEXT NOT NULL,
    dosagem TEXT NOT NULL,
    frequencia TEXT NOT NULL,
    via TEXT NOT NULL DEFAULT '',
    duracao TEXT NOT NULL DEFAULT '',
    observacoes TEXT NOT NULL,
    motivo_suspensao TEXT NOT NULL DEFAULT '',
    ativo INTEGER NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id)
);

-- Usuarios de acesso ao sistema (login criado pelo administrador).
-- papel: ADMIN, CADASTRO, MEDICO, ENFERMAGEM, PACIENTE.
-- paciente_id / medico_id vinculam o usuario a uma entidade (0 = nenhum).
-- Sem FK nesses vinculos: 0 representa "nenhum" e nao existiria como chave.
-- nome: nome de exibicao do usuario (cadastro de usuario real).
-- criado_em: carimbo de criacao (UTC) para fins de cadastro/auditoria.
-- tentativas_invalidas / bloqueado_ate: endurecimento de login. Apos N erros
-- consecutivos o login fica bloqueado ate o horario (UTC) em bloqueado_ate;
-- um login bem-sucedido zera ambos.
CREATE TABLE usuarios (
    id INTEGER PRIMARY KEY,
    nome TEXT NOT NULL DEFAULT '',
    login TEXT NOT NULL UNIQUE,
    senha_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    papel TEXT NOT NULL,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    ativo INTEGER NOT NULL,
    tentativas_invalidas INTEGER NOT NULL DEFAULT 0,
    bloqueado_ate TEXT NOT NULL DEFAULT '',
    -- trocar_senha: usuario criado pelo admin comeca com 1 e precisa definir uma
    -- nova senha no primeiro acesso; zera apos a troca.
    trocar_senha INTEGER NOT NULL DEFAULT 0,
    criado_em TEXT NOT NULL DEFAULT (datetime('now'))
);

-- Sessoes de acesso: cada login bem-sucedido cria um token opaco com validade.
-- O cliente envia 'Authorization: Bearer <token>' (a senha nao trafega mais a
-- cada requisicao). Sessoes expiradas/encerradas sao removidas. Reconstruivel.
CREATE TABLE sessoes (
    token TEXT PRIMARY KEY,
    usuario_id INTEGER NOT NULL,
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    expira_em TEXT NOT NULL
);

-- Trilha de auditoria das acoes sensiveis do sistema (req. de seguranca).
-- Guarda quem fez (usuario_id/login no momento da acao, pois o usuario pode
-- ser desativado depois), o que fez (acao), sobre qual entidade e um detalhe
-- livre em texto. Nunca e apagada.
CREATE TABLE auditoria (
    id INTEGER PRIMARY KEY,
    usuario_id INTEGER NOT NULL,
    usuario_login TEXT NOT NULL,
    acao TEXT NOT NULL,
    entidade TEXT NOT NULL,
    entidade_id INTEGER NOT NULL,
    detalhe TEXT NOT NULL,
    criado_em TEXT NOT NULL DEFAULT (datetime('now'))
);

-- Convenios (planos de saude). Paciente sem convenio = particular.
CREATE TABLE convenios (
    id INTEGER PRIMARY KEY,
    nome TEXT NOT NULL,
    -- cobertura_pct: % do valor coberto pelo convenio (0-100). A diferenca e a
    -- coparticipacao do paciente.
    cobertura_pct INTEGER NOT NULL DEFAULT 100,
    ativo INTEGER NOT NULL DEFAULT 1
);

-- Cobrancas (financeiro). Vinculam um atendimento (origem em texto livre) ao
-- paciente e a forma de pagamento (PARTICULAR/CONVENIO). valor em centavos.
-- status: PENDENTE, AUTORIZADA, PAGA, GLOSADA, CANCELADA. Nunca apagada.
CREATE TABLE cobrancas (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    convenio_id INTEGER NOT NULL DEFAULT 0,
    forma TEXT NOT NULL,
    origem TEXT NOT NULL DEFAULT '',
    descricao TEXT NOT NULL DEFAULT '',
    valor_centavos INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'PENDENTE',
    motivo TEXT NOT NULL DEFAULT '',
    -- lote_id: lote de faturamento ao qual a cobranca pertence (0 = nenhum).
    lote_id INTEGER NOT NULL DEFAULT 0,
    -- vencimento: data limite de pagamento (YYYY-MM-DD; vazio = sem vencimento).
    vencimento TEXT NOT NULL DEFAULT '',
    -- guia / validade: autorizacao do convenio para o procedimento.
    guia TEXT NOT NULL DEFAULT '',
    guia_validade TEXT NOT NULL DEFAULT '',
    -- Divisao do valor: parte coberta pelo convenio e coparticipacao do paciente
    -- (calculadas na criacao a partir de convenios.cobertura_pct).
    coberto_centavos INTEGER NOT NULL DEFAULT 0,
    copart_centavos INTEGER NOT NULL DEFAULT 0,
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id)
);

-- Lotes de faturamento: agrupam cobrancas CONVENIO autorizadas de um mesmo
-- convenio para envio/pagamento. status: ABERTO, FECHADO, PAGO. Pagar o lote
-- quita (PAGA) todas as cobrancas que ele contem.
CREATE TABLE lotes (
    id INTEGER PRIMARY KEY,
    convenio_id INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'ABERTO',
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    fechado_em TEXT NOT NULL DEFAULT '',
    FOREIGN KEY (convenio_id) REFERENCES convenios(id)
);

-- Check-in / recepcao: confirma a chegada do paciente, gera uma senha de fila
-- e o direciona para TRIAGEM ou CONSULTA. status: AGUARDANDO, EM_ATENDIMENTO,
-- ENCERRADO. Nunca apagado (historico de fluxo).
CREATE TABLE checkins (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    senha TEXT NOT NULL,
    destino TEXT NOT NULL,
    -- status: AGUARDANDO, EM_ATENDIMENTO, ENCERRADO, FALTOU, CANCELADO.
    status TEXT NOT NULL DEFAULT 'AGUARDANDO',
    -- rechamadas: quantas vezes a senha foi chamada de novo (paciente nao veio).
    rechamadas INTEGER NOT NULL DEFAULT 0,
    -- motivo: justificativa do cancelamento (vazio nos demais estados).
    motivo TEXT NOT NULL DEFAULT '',
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id)
);

-- Administracao de medicamentos pela enfermagem (MAR). Cada linha registra
-- que uma prescricao foi aplicada, por quem e quando. Nunca e apagada.
CREATE TABLE administracoes (
    id INTEGER PRIMARY KEY,
    prescricao_id INTEGER NOT NULL,
    paciente_id INTEGER NOT NULL,
    usuario_id INTEGER NOT NULL,
    usuario_login TEXT NOT NULL DEFAULT '',
    observacao TEXT NOT NULL DEFAULT '',
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (prescricao_id) REFERENCES prescricoes(id),
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id)
);

-- Evolucao de enfermagem: nota clinica + sinais vitais ao longo da estadia.
CREATE TABLE evolucoes_enfermagem (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    autor_login TEXT NOT NULL DEFAULT '',
    texto TEXT NOT NULL,
    pressao TEXT NOT NULL DEFAULT '',
    temperatura TEXT NOT NULL DEFAULT '',
    freq_cardiaca TEXT NOT NULL DEFAULT '',
    saturacao TEXT NOT NULL DEFAULT '',
    criado_em TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id)
);

-- Indices das buscas mais frequentes (listagens por vinculo e por paciente).
CREATE INDEX idx_agendamentos_medico ON agendamentos(medico_id);
CREATE INDEX idx_agendamentos_paciente ON agendamentos(paciente_id);
CREATE INDEX idx_prontuarios_medico ON prontuarios(medico_id);
CREATE INDEX idx_prontuarios_paciente ON prontuarios(paciente_id);
CREATE INDEX idx_exames_medico ON exames(medico_id);
CREATE INDEX idx_exames_paciente ON exames(paciente_id);
CREATE INDEX idx_prescricoes_paciente ON prescricoes(paciente_id);
CREATE INDEX idx_triagens_paciente ON triagens(paciente_id);
CREATE INDEX idx_auditoria_entidade ON auditoria(entidade, entidade_id);

-- Nao permite dois pacientes ATIVOS com o mesmo CPF (documento alternativo
-- nao entra na regra). Indice parcial: a unicidade so vale para CPF ativo.
CREATE UNIQUE INDEX idx_pacientes_cpf_ativo
    ON pacientes(documento)
    WHERE ativo = 1 AND tipo_documento = 'CPF';

-- Indices das features mais recentes (financeiro e sessoes). Tambem aplicados
-- a bancos antigos pela migracao v2 (ver backend/web/database/migracoes.c).
CREATE INDEX idx_cobrancas_paciente ON cobrancas(paciente_id);
CREATE INDEX idx_cobrancas_lote ON cobrancas(lote_id);
CREATE INDEX idx_sessoes_expira ON sessoes(expira_em);

-- Laboratorio: busca dos analitos de um painel e unicidade do codigo do analito
-- (somente entre ativos; um codigo desativado pode ser reaproveitado).
CREATE INDEX idx_painel_tipo ON painel_analitos(tipo_exame);
CREATE INDEX idx_painel_analito ON painel_analitos(analito_id);
CREATE UNIQUE INDEX idx_analitos_codigo_ativo ON analitos(codigo) WHERE ativo = 1;

-- Versao do schema. Mantenha em sincronia com LATEST_VERSION em migracoes.c:
-- um banco recem-criado ja nasce na ultima versao (as migracoes nao re-rodam).
PRAGMA user_version = 6;
