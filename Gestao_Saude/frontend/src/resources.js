// Catalogo de recursos listaveis. Cada um aponta para uma rota da API, as
// colunas a exibir, os papeis com acesso de LEITURA (roles) e os papeis com
// acesso de ESCRITA (writeRoles). createFields define o formulario de criacao
// (parametros vao na query string). deleteLabel rotula a acao de remocao.
// O backend tambem barra via 403; aqui filtramos a UI para nao oferecer o que
// nao serve.
export const RESOURCES = [
  {
    key: 'pacientes',
    label: 'Pacientes',
    path: '/pacientes',
    roles: ['ADMIN', 'CADASTRO', 'MEDICO'],
    writeRoles: ['ADMIN', 'CADASTRO'],
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'nome', label: 'Nome' },
      { key: 'cpf', label: 'CPF' },
      { key: 'idade', label: 'Idade' },
      { key: 'telefone', label: 'Telefone' },
      { key: 'sexo', label: 'Sexo' },
      { key: 'regiaoAdministrativa', label: 'Regiao' },
    ],
    createFields: [
      { name: 'nome', label: 'Nome', type: 'text' },
      { name: 'cpf', label: 'CPF', type: 'text' },
      { name: 'idade', label: 'Idade', type: 'number' },
      { name: 'telefone', label: 'Telefone', type: 'text' },
      { name: 'sexo', label: 'Sexo', type: 'select', options: ['F', 'M'] },
      { name: 'regiao', label: 'Regiao', type: 'number' },
    ],
  },
  {
    key: 'medicos',
    label: 'Medicos',
    path: '/medicos',
    roles: ['ADMIN', 'CADASTRO', 'MEDICO'],
    writeRoles: ['ADMIN', 'CADASTRO'],
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'nome', label: 'Nome' },
      { key: 'crm', label: 'CRM' },
      { key: 'especialidade', label: 'Especialidade' },
      { key: 'regiaoAdministrativa', label: 'Regiao' },
    ],
    createFields: [
      { name: 'nome', label: 'Nome', type: 'text' },
      { name: 'crm', label: 'CRM', type: 'text' },
      { name: 'especialidade', label: 'Especialidade', type: 'text' },
      { name: 'regiao', label: 'Regiao', type: 'number' },
    ],
  },
  {
    key: 'agendamentos',
    label: 'Agendamentos',
    path: '/agendamentos',
    roles: ['ADMIN', 'MEDICO'],
    writeRoles: ['ADMIN', 'MEDICO'],
    deleteLabel: 'Cancelar',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'horario', label: 'Horario' },
      { key: 'status', label: 'Status' },
    ],
    createFields: [
      { name: 'paciente_id', label: 'Paciente ID', type: 'number' },
      { name: 'medico_id', label: 'Medico ID', type: 'number' },
      { name: 'data', label: 'Data', type: 'date' },
      { name: 'horario', label: 'Horario', type: 'text', placeholder: 'HH:MM' },
    ],
  },
  {
    key: 'triagens',
    label: 'Triagens',
    path: '/triagens',
    roles: ['ADMIN', 'MEDICO'],
    writeRoles: ['ADMIN', 'MEDICO'],
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'tipoTriagem', label: 'Tipo' },
      { key: 'pontuacao', label: 'Pontuacao' },
      { key: 'classificacao', label: 'Classificacao' },
    ],
    createFields: [
      { name: 'paciente_id', label: 'Paciente ID', type: 'number' },
      { name: 'tipo', label: 'Tipo (1-5)', type: 'number' },
      { name: 'pontuacao', label: 'Pontuacao', type: 'number' },
      { name: 'classificacao', label: 'Classificacao', type: 'text' },
    ],
  },
  {
    key: 'prontuarios',
    label: 'Prontuarios',
    path: '/prontuarios',
    roles: ['ADMIN', 'MEDICO'],
    writeRoles: ['ADMIN', 'MEDICO'],
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'diagnostico', label: 'Diagnostico' },
      { key: 'conduta', label: 'Conduta' },
    ],
    createFields: [
      { name: 'paciente_id', label: 'Paciente ID', type: 'number' },
      { name: 'medico_id', label: 'Medico ID', type: 'number' },
      { name: 'data', label: 'Data', type: 'date' },
      { name: 'observacoes', label: 'Observacoes', type: 'text' },
      { name: 'diagnostico', label: 'Diagnostico', type: 'text' },
      { name: 'conduta', label: 'Conduta', type: 'text' },
      { name: 'alerta_importante', label: 'Alerta', type: 'select', options: ['0', '1'] },
    ],
  },
  {
    key: 'exames',
    label: 'Exames',
    path: '/exames',
    roles: ['ADMIN', 'MEDICO'],
    writeRoles: ['ADMIN', 'MEDICO'],
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'tipoExame', label: 'Tipo' },
      { key: 'dataSolicitacao', label: 'Solicitacao' },
      { key: 'status', label: 'Status' },
    ],
    createFields: [
      { name: 'paciente_id', label: 'Paciente ID', type: 'number' },
      { name: 'medico_id', label: 'Medico ID', type: 'number' },
      { name: 'prontuario_id', label: 'Prontuario ID', type: 'number' },
      { name: 'tipo', label: 'Tipo', type: 'number' },
      { name: 'data_solicitacao', label: 'Solicitacao', type: 'date' },
      { name: 'urgente', label: 'Urgente', type: 'select', options: ['0', '1'] },
    ],
  },
  {
    key: 'prescricoes',
    label: 'Prescricoes',
    path: '/prescricoes',
    roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'],
    writeRoles: ['ADMIN', 'MEDICO'],
    deleteLabel: 'Suspender',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'medicamento', label: 'Medicamento' },
      { key: 'dosagem', label: 'Dosagem' },
      { key: 'frequencia', label: 'Frequencia' },
      { key: 'observacoes', label: 'Observacoes' },
    ],
    createFields: [
      { name: 'paciente_id', label: 'Paciente ID', type: 'number' },
      { name: 'medico_id', label: 'Medico ID', type: 'number' },
      { name: 'medicamento', label: 'Medicamento', type: 'text' },
      { name: 'dosagem', label: 'Dosagem', type: 'text' },
      { name: 'frequencia', label: 'Frequencia', type: 'text' },
      { name: 'observacoes', label: 'Observacoes', type: 'text' },
    ],
  },
]

export function resourcesForRole(papel) {
  return RESOURCES.filter((r) => r.roles.includes(papel))
}

export function resourceByKey(key) {
  return RESOURCES.find((r) => r.key === key)
}
