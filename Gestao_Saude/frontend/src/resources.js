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

// Rotulos das opcoes dos campos de referencia (FK por nome). Cada funcao recebe
// uma linha da API e devolve o texto exibido na selecao.
const rotuloPaciente = (p) => `${p.nome}${p.documento ? ` · ${p.documento}` : ''}`
const rotuloMedico = (m) => `${m.nome}${m.especialidade ? ` · ${m.especialidade}` : ''}`
const rotuloProntuario = (p) =>
  `#${p.id} · ${p.diagnostico || p.data || 'sem diagnostico'}`

// Campos de referencia reutilizaveis: buscam opcoes na API e enviam o id.
// Paciente usa BUSCA pela rota nao escopada /pacientes/buscar: o GET /pacientes
// e escopado por papel (o medico so ve quem tem agendamento com ele), o que
// esconderia pacientes validos para criar registros clinicos.
const fkPaciente = {
  name: 'paciente_id',
  label: 'Paciente',
  type: 'ref',
  searchPath: '/pacientes/buscar',
  optionLabel: rotuloPaciente,
}
const fkMedicoVinculo = {
  name: 'medico_id',
  label: 'Medico',
  type: 'ref',
  path: '/medicos',
  optionLabel: rotuloMedico,
  allowEmpty: true,
  emptyLabel: 'Usar meu vinculo',
}

export const RESOURCES = [
  {
    key: 'pacientes',
    label: 'Pacientes',
    path: '/pacientes',
    roles: ['ADMIN', 'CADASTRO'],
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
    roles: ['ADMIN', 'CADASTRO'],
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
    key: 'medicamentos',
    label: 'Medicamentos',
    path: '/medicamentos',
    roles: ['ADMIN', 'ENFERMAGEM'],
    createRoles: ['ADMIN', 'ENFERMAGEM'],
    deleteRoles: ['ADMIN', 'ENFERMAGEM'],
    deleteLabel: 'Desativar',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'nome', label: 'Nome' },
      { key: 'apresentacao', label: 'Apresentacao' },
      { key: 'unidade', label: 'Unidade' },
      { key: 'estoqueMinimo', label: 'Estoque minimo' },
    ],
    createFields: [
      { name: 'nome', label: 'Nome', type: 'text' },
      { name: 'apresentacao', label: 'Apresentacao', type: 'text' },
      { name: 'unidade', label: 'Unidade', type: 'text' },
      { name: 'estoque_minimo', label: 'Estoque minimo', type: 'number' },
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
    requireReason: true,
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteNome', label: 'Paciente' },
      { key: 'medicoNome', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'horario', label: 'Horario' },
      { key: 'status', label: 'Status', type: 'badge' },
    ],
    createFields: [
      fkPaciente,
      { ...fkMedicoVinculo, allowEmpty: false, emptyLabel: undefined },
      { name: 'data', label: 'Data', type: 'date' },
      { name: 'horario', label: 'Horario', type: 'text', placeholder: 'HH:MM' },
    ],
  },
  {
    key: 'triagens',
    label: 'Triagens',
    path: '/triagens',
    roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'],
    // A triagem e registrada pelo fluxo guiado (/triagem), que deriva a
    // classificacao do checklist; nao ha criacao manual nesta lista.
    createRoles: [],
    deleteRoles: [],
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteId', label: 'Paciente' },
      { key: 'tipoTriagem', label: 'Tipo' },
      { key: 'queixa', label: 'Queixa' },
      { key: 'pontuacao', label: 'Nivel' },
      { key: 'classificacao', label: 'Classificacao', type: 'badge', tone: tomRisco },
    ],
  },
  {
    key: 'prontuarios',
    label: 'Prontuarios',
    path: '/prontuarios',
    roles: ['ADMIN', 'MEDICO'],
    createRoles: ['ADMIN', 'MEDICO'],
    deleteRoles: [],
    // Registro clinico imutavel: a correcao e por retificacao versionada.
    retificavel: true,
    deleteLabel: 'Remover',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteNome', label: 'Paciente' },
      { key: 'medicoNome', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'diagnostico', label: 'Diagnostico' },
      { key: 'conduta', label: 'Conduta' },
      { key: 'versao', label: 'Versao' },
    ],
    createFields: [
      fkPaciente,
      fkMedicoVinculo,
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
    requireReason: true,
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteNome', label: 'Paciente' },
      { key: 'medicoNome', label: 'Medico' },
      { key: 'tipoExame', label: 'Tipo' },
      { key: 'dataSolicitacao', label: 'Solicitacao' },
      { key: 'status', label: 'Status', type: 'badge' },
    ],
    createFields: [
      fkPaciente,
      fkMedicoVinculo,
      {
        name: 'prontuario_id',
        label: 'Prontuario',
        type: 'ref',
        path: '/prontuarios',
        optionLabel: rotuloProntuario,
        // So oferece prontuarios do paciente escolhido (quando ja selecionado).
        filter: (row, valores) =>
          !valores?.paciente_id || String(row.pacienteId) === String(valores.paciente_id),
      },
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
    requireReason: true,
    // Enfermagem registra a administracao (MAR) do medicamento.
    administravel: true,
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'pacienteNome', label: 'Paciente' },
      { key: 'medicoNome', label: 'Medico' },
      { key: 'medicamento', label: 'Medicamento' },
      { key: 'dosagem', label: 'Dosagem' },
      { key: 'frequencia', label: 'Frequencia' },
      { key: 'via', label: 'Via' },
      { key: 'duracao', label: 'Duracao' },
      { key: 'observacoes', label: 'Observacoes' },
    ],
    createFields: [
      fkPaciente,
      fkMedicoVinculo,
      { name: 'medicamento', label: 'Medicamento', type: 'text' },
      { name: 'dosagem', label: 'Dosagem', type: 'text' },
      { name: 'frequencia', label: 'Frequencia', type: 'text' },
      { name: 'via', label: 'Via', type: 'text' },
      { name: 'duracao', label: 'Duracao', type: 'text' },
      { name: 'observacoes', label: 'Observacoes', type: 'text' },
    ],
  },
]

export function resourceByKey(key) {
  return RESOURCES.find((r) => r.key === key)
}
