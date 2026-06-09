CREATE TABLE pacientes (
    id INT PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    cpf VARCHAR(15) NOT NULL,
    idade INT NOT NULL,
    telefone VARCHAR(20) NOT NULL,
    sexo CHAR(1) NOT NULL,
    telefone VARCHAR(20) NOT NULL,
    regiao_administrativa INT NOT NULL,
    ativo TINYINT(1) NOT NULL
);
CREATE TABLE medicos (
    id INT PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    crm VARCHAR(20) NOT NULL,
    especialidade VARCHAR(50) NOT NULL,
    regiao_administrativa INT NOT NULL,
    ativo TINYINT(1) NOT NULL
);

CREATE TABLE triagens (
    id INT PRIMARY KEY,
    paciente_id INT NOT NULL, 
    tipo_triagem INT NOT NULL,
    pontuacao INT NOT NULL,
    classificacao VARCHAR(30) NOT NULL,
    ativo TINYINT(1) NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES paciente(id)
);

CREATE TABLE agendamentos (
    id INT PRIMARY KEY,
    paciente_id INT NOT NULL,
    medico_id INT NOT NULL,
    data VARCHAR(10) NOT NULL,
    horario VARCHAR(5) NOT NULL,
    status VARCHAR(20) NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id)
);

CREATE TABLE prontuarios (
    id INT PRIMARY KEY,
    paciente_id INT NOT NULL,
    medico_id INT NOT NULL,
    data VARCHAR(10) NOT NULL,
    observacoes VARCHAR(300) NOT NULL,
    diagnostico VARCHAR(200) NOT NULL,
    conduta VARCHAR(200) NOT NULL,
    alerta_importante TINYINT(1) NOT NULL,
    ativo TINYINT(1) NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id)
);

CREATE TABLE exames (
    id INT PRIMARY KEY,
    paciente_id INT NOT NULL,
    medico_id INT NOT NULL,
    prontuario_id INT NOT NULL,
    tipo_exame INT NOT NULL,
    data_solicitacao VARCHAR(10) NOT NULL,
    data_resultado VARCHAR(10) NOT NULL,
    resultado VARCHAR(300) NOT NULL,
    status VARCHAR(20) NOT NULL,
    urgente TINYINT(1) NOT NULL,
    ativo TINYINT(1) NOT NULL,
    FOREIGN KEY (paciente_id) REFERENCES pacientes(id),
    FOREIGN KEY (medico_id) REFERENCES medicos(id),
    FOREIGN KEY (prontuario_id) REFERENCES prontuarios(id)
)