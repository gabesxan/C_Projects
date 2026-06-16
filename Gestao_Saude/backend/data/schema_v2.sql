PRAGMA foreign_keys = ON;

CREATE TABLE pacientes (
    id INTEGER PRIMARY KEY,
    nome TEXT NOT NULL,
    cpf TEXT NOT NULL,
    idade INTEGER NOT NULL,
    telefone TEXT NOT NULL,
    sexo TEXT NOT NULL,
    regiao_administrativa INTEGER NOT NULL,
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

CREATE TABLE triagens (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    tipo_triagem INTEGER NOT NULL,
    pontuacao INTEGER NOT NULL,
    classificacao TEXT NOT NULL,
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

CREATE TABLE leitos (
    id INTEGER PRIMARY KEY,
    ala_id INTEGER NOT NULL,
    numero INTEGER NOT NULL,
    ocupado INTEGER NOT NULL,
    paciente_id INTEGER NOT NULL,
    ativo INTEGER NOT NULL,
    FOREIGN KEY (ala_id) REFERENCES alas(id)
);

CREATE TABLE internacoes (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    ala_id INTEGER NOT NULL,
    leito_id INTEGER NOT NULL,
    data_entrada TEXT NOT NULL,
    data_alta TEXT NOT NULL,
    status TEXT NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (ala_id) REFERENCES alas(id),
    FOREIGN KEY (leito_id) REFERENCES leitos(id)
);

CREATE TABLE prontuarios (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    data TEXT NOT NULL,
    observacoes TEXT NOT NULL,
    diagnostico TEXT NOT NULL,
    conduta TEXT NOT NULL,
    alerta_importante INTEGER NOT NULL,
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
    ativo INTEGER NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id),
    FOREIGN KEY (prontuario_id) REFERENCES prontuarios(id)
);

CREATE TABLE prescricoes (
    id INTEGER PRIMARY KEY,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    medicamento TEXT NOT NULL,
    dosagem TEXT NOT NULL,
    frequencia TEXT NOT NULL,
    observacoes TEXT NOT NULL,
    ativo INTEGER NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id)
);

-- Usuarios de acesso ao sistema (login criado pelo administrador).
-- papel: ADMIN, CADASTRO, MEDICO, ENFERMAGEM, PACIENTE.
-- paciente_id / medico_id vinculam o usuario a uma entidade (0 = nenhum).
-- Sem FK nesses vinculos: 0 representa "nenhum" e nao existiria como chave.
CREATE TABLE usuarios (
    id INTEGER PRIMARY KEY,
    login TEXT NOT NULL UNIQUE,
    senha_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    papel TEXT NOT NULL,
    paciente_id INTEGER NOT NULL,
    medico_id INTEGER NOT NULL,
    ativo INTEGER NOT NULL
);
