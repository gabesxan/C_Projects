// Catalogo de recursos listaveis. Cada um aponta para uma rota da API, as
// colunas a exibir e os papeis com acesso de LEITURA (roles), CRIACAO
// (createRoles) e REMOCAO/CANCELAMENTO (deleteRoles). Registros clinicos que
// nao podem ser apagados fisicamente tem deleteRoles vazio.
// O backend tambem barra via 403; aqui filtramos a UI para nao oferecer o que
// nao serve.

// Tons de cor por classificacao de risco (Protocolo de Manchester).
const RISCO = {
  Vermelho: 'red',
  Laranja: 'amber',
  Amarelo: 'amber',
  Verde: 'green',
  Azul: 'sky',
}
const tomRisco = (v) => RISCO[v] ?? 'slate'

export const RESOURCES = [
  {
    key: 'pacientes',
    label: 'Pacientes',
    path: '/pacientes',
    roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'],
    createRoles: ['ADMIN', 'CADASTRO'],
    deleteRoles: ['ADMIN', 'CADASTRO'],
    deleteLabel: 'Remover',
    // Clique na linha abre o detalhe do paciente.
    detail: (row) => `/paciente/${row.id}`,
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'nome', label: 'Nome' },
      { key: 'documento', label: 'Documento' },
      { key: 'idade', label: 'Idade' },
      { key: 'telefone', label: 'Telefone' },
      { key: 'regiaoAdministrativa', label: 'Regiao' },
      { key: 'alergias', label: 'Alergias', type: 'badge', tone: () => 'red' },
    ],
    createFields: [
      { name: 'nome', label: 'Nome', type: 'text' },
      { name: 'nascimento', label: 'Nascimento', type: 'date' },
      { name: 'documento', label: 'Documento (CPF ou outro)', type: 'text' },
      {
        name: 'tipo_documento',
        label: 'Tipo de documento',
        type: 'select',
        options: ['CPF', 'OUTRO'],
      },
      { name: 'telefone', label: 'Telefone', type: 'text' },
      { name: 'sexo', label: 'Sexo', type: 'select', options: ['F', 'M'] },
      { name: 'regiao', label: 'Regiao', type: 'number' },
      { name: 'responsavel', label: 'Responsavel (se menor)', type: 'text' },
      { name: 'alergias', label: 'Alergias / alertas', type: 'text' },
    ],
  },
  {
    key: 'medicos',
    label: 'Medicos',
    path: '/medicos',
    roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'],
    createRoles: ['ADMIN', 'CADASTRO'],
    deleteRoles: ['ADMIN', 'CADASTRO'],
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
    createRoles: ['ADMIN', 'MEDICO'],
    deleteRoles: ['ADMIN', 'MEDICO'],
    deleteLabel: 'Cancelar',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'horario', label: 'Horario' },
      { key: 'status', label: 'Status', type: 'badge' },
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
    roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'],
    createRoles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'],
    deleteRoles: [],
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'tipoTriagem', label: 'Tipo' },
      { key: 'pontuacao', label: 'Pontuacao' },
      { key: 'classificacao', label: 'Classificacao', type: 'badge', tone: tomRisco },
    ],
    createFields: [
      { name: 'paciente_id', label: 'Paciente ID', type: 'number' },
      { name: 'tipo', label: 'Tipo (1-5)', type: 'number' },
      { name: 'pontuacao', label: 'Pontuacao', type: 'number' },
      {
        name: 'classificacao',
        label: 'Classificacao',
        type: 'select',
        options: ['Vermelho', 'Laranja', 'Amarelo', 'Verde', 'Azul'],
      },
    ],
  },
  {
    key: 'prontuarios',
    label: 'Prontuarios',
    path: '/prontuarios',
    roles: ['ADMIN', 'MEDICO'],
    createRoles: ['ADMIN', 'MEDICO'],
    deleteRoles: [],
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
      { name: 'medico_id', label: 'Medico ID (deixe vazio: usa seu vinculo)', type: 'number' },
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
    createRoles: ['ADMIN', 'MEDICO'],
    deleteRoles: ['ADMIN', 'MEDICO'],
    deleteLabel: 'Cancelar',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'tipoExame', label: 'Tipo' },
      { key: 'dataSolicitacao', label: 'Solicitacao' },
      { key: 'status', label: 'Status', type: 'badge' },
    ],
    createFields: [
      { name: 'paciente_id', label: 'Paciente ID', type: 'number' },
      { name: 'medico_id', label: 'Medico ID (deixe vazio: usa seu vinculo)', type: 'number' },
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
    createRoles: ['ADMIN', 'MEDICO'],
    deleteRoles: ['ADMIN', 'MEDICO'],
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
      { name: 'medico_id', label: 'Medico ID (deixe vazio: usa seu vinculo)', type: 'number' },
      { name: 'medicamento', label: 'Medicamento', type: 'text' },
      { name: 'dosagem', label: 'Dosagem', type: 'text' },
      { name: 'frequencia', label: 'Frequencia', type: 'text' },
      { name: 'observacoes', label: 'Observacoes', type: 'text' },
    ],
  },
]

export function resourceByKey(key) {
  return RESOURCES.find((r) => r.key === key)
}
