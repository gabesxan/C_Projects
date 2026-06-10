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
