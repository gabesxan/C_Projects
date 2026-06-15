// Catalogo de recursos listaveis. Cada um aponta para uma rota da API, as
// colunas a exibir e os papeis que tem acesso de leitura (o backend tambem
// barra via 403; aqui filtramos a navegacao para nao mostrar o que nao serve).
export const RESOURCES = [
  {
    key: 'pacientes',
    label: 'Pacientes',
    path: '/pacientes',
    roles: ['ADMIN', 'CADASTRO', 'MEDICO'],
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'nome', label: 'Nome' },
      { key: 'cpf', label: 'CPF' },
      { key: 'idade', label: 'Idade' },
      { key: 'telefone', label: 'Telefone' },
      { key: 'sexo', label: 'Sexo' },
      { key: 'regiaoAdministrativa', label: 'Regiao' },
    ],
  },
  {
    key: 'medicos',
    label: 'Medicos',
    path: '/medicos',
    roles: ['ADMIN', 'CADASTRO', 'MEDICO'],
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'nome', label: 'Nome' },
      { key: 'crm', label: 'CRM' },
      { key: 'especialidade', label: 'Especialidade' },
      { key: 'regiaoAdministrativa', label: 'Regiao' },
    ],
  },
  {
    key: 'agendamentos',
    label: 'Agendamentos',
    path: '/agendamentos',
    roles: ['ADMIN', 'MEDICO'],
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'horario', label: 'Horario' },
      { key: 'status', label: 'Status' },
    ],
  },
  {
    key: 'triagens',
    label: 'Triagens',
    path: '/triagens',
    roles: ['ADMIN', 'MEDICO'],
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'tipoTriagem', label: 'Tipo' },
      { key: 'pontuacao', label: 'Pontuacao' },
      { key: 'classificacao', label: 'Classificacao' },
    ],
  },
  {
    key: 'prontuarios',
    label: 'Prontuarios',
    path: '/prontuarios',
    roles: ['ADMIN', 'MEDICO'],
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'diagnostico', label: 'Diagnostico' },
      { key: 'conduta', label: 'Conduta' },
    ],
  },
  {
    key: 'exames',
    label: 'Exames',
    path: '/exames',
    roles: ['ADMIN', 'MEDICO'],
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'tipoExame', label: 'Tipo' },
      { key: 'dataSolicitacao', label: 'Solicitacao' },
      { key: 'status', label: 'Status' },
    ],
  },
]

export function resourcesForRole(papel) {
  return RESOURCES.filter((r) => r.roles.includes(papel))
}

export function resourceByKey(key) {
  return RESOURCES.find((r) => r.key === key)
}
