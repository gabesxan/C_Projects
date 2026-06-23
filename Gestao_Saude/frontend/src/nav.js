// Navegacao lateral por papel. Cada item declara os papeis que o enxergam e a
// secao em que aparece (para agrupar a sidebar). O backend tambem barra por
// papel (403); aqui apenas evitamos oferecer o que nao serve ao usuario logado.
//
// Regra de caminho unico: cada tarefa tem UM caminho canonico no menu. As
// telas CRUD genericas cobertas por um fluxo sob medida ficam fora do menu
// (ex.: /r/triagens -> Triagem; /r/exames -> Laboratorio) ou restritas a quem
// realmente faz a tarefa (cadastro de pacientes/medicos: ADMIN e CADASTRO).

// Ordem de exibicao das secoes na sidebar.
export const SECTIONS = [
  'Minha saude',
  'Atendimento',
  'Clinico',
  'Internacao',
  'Farmacia',
  'Vacinacao',
  'Financeiro',
  'Administracao',
  'Conta',
]

const NAV = [
  // Painel: item de topo, sem secao (renderizado acima dos grupos).
  { to: '/', label: 'Painel', icon: 'dashboard', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM', 'PACIENTE'], end: true },

  // Portal do paciente.
  { to: '/minha-saude', label: 'Visão geral', icon: 'health', roles: ['PACIENTE'], end: true, section: 'Minha saude' },
  { to: '/minha-saude/consultas', label: 'Consultas', icon: 'schedule', roles: ['PACIENTE'], section: 'Minha saude' },
  { to: '/minha-saude/exames', label: 'Exames', icon: 'lab', roles: ['PACIENTE'], section: 'Minha saude' },
  { to: '/minha-saude/receitas', label: 'Receitas', icon: 'prescription', roles: ['PACIENTE'], section: 'Minha saude' },
  { to: '/minha-saude/vacinas', label: 'Vacinas', icon: 'vaccine', roles: ['PACIENTE'], section: 'Minha saude' },
  { to: '/minha-saude/prontuarios', label: 'Prontuários', icon: 'record', roles: ['PACIENTE'], section: 'Minha saude' },
  { to: '/minha-saude/financeiro', label: 'Financeiro', icon: 'billing', roles: ['PACIENTE'], section: 'Minha saude' },
  { to: '/minha-saude/solicitacoes', label: 'Agendar', icon: 'schedule', roles: ['PACIENTE'], section: 'Minha saude' },

  // Atendimento (recepcao, triagem e cadastros).
  { to: '/recepcao', label: 'Recepção', icon: 'reception', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'], section: 'Atendimento' },
  { to: '/triagem', label: 'Triagem', icon: 'triage', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'], section: 'Atendimento' },
  // Cadastro (registro) de pacientes/medicos: so quem cadastra. Os papeis
  // clinicos localizam o paciente pela Recepcao/Triagem e abrem a ficha.
  { to: '/r/pacientes', label: 'Pacientes', icon: 'patients', roles: ['ADMIN', 'CADASTRO'], section: 'Atendimento' },
  { to: '/r/medicos', label: 'Médicos', icon: 'doctor', roles: ['ADMIN', 'CADASTRO'], section: 'Atendimento' },

  // Clinico (agenda, prontuario, exames, prescricao).
  { to: '/r/agendamentos', label: 'Agendamentos', icon: 'schedule', roles: ['ADMIN', 'MEDICO'], section: 'Clinico' },
  { to: '/r/prontuarios', label: 'Prontuários', icon: 'record', roles: ['ADMIN', 'MEDICO'], section: 'Clinico' },
  { to: '/laboratorio', label: 'Laboratório', icon: 'lab', roles: ['ADMIN', 'MEDICO'], section: 'Clinico' },
  { to: '/r/prescricoes', label: 'Prescrições', icon: 'prescription', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'], section: 'Clinico' },

  // Internacao. As rotas do SPA usam slugs que NAO colidem com prefixos da API
  // (a API ja ocupa GET /internacoes e GET /leitos): no servidor de producao de
  // origem unica, um GET que casa com prefixo de API vira chamada de API em vez
  // de cair no index.html (fallback de SPA). Por isso /internacao e /enfermagem.
  { to: '/internacao', label: 'Internações', icon: 'admission', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'], section: 'Internacao' },
  { to: '/enfermagem', label: 'Leitos', icon: 'bed', roles: ['ADMIN', 'ENFERMAGEM', 'CADASTRO'], section: 'Internacao' },

  // Farmacia. Catalogo de medicamentos via recurso generico (/r/medicamentos);
  // estoque/movimentacoes/alertas chegam nas sub-etapas 5b/5c.
  { to: '/r/medicamentos', label: 'Medicamentos', icon: 'prescription', roles: ['ADMIN', 'ENFERMAGEM'], section: 'Farmacia' },
  { to: '/farmacia', label: 'Estoque', icon: 'hospital', roles: ['ADMIN', 'ENFERMAGEM'], section: 'Farmacia' },

  // Vacinacao. O prefixo SPA /vacinacao nao colide com a API /vacinas.
  { to: '/vacinacao', label: 'Vacinação', icon: 'vaccine', roles: ['ADMIN', 'ENFERMAGEM'], section: 'Vacinacao' },

  // Financeiro.
  { to: '/financeiro', label: 'Financeiro', icon: 'finance', roles: ['ADMIN', 'CADASTRO'], section: 'Financeiro' },

  // Administracao.
  { to: '/relatorios', label: 'Relatórios', icon: 'reports', roles: ['ADMIN', 'MEDICO'], section: 'Administracao' },
  { to: '/admin/usuarios', label: 'Usuários', icon: 'users', roles: ['ADMIN'], section: 'Administracao' },
  { to: '/admin/auditoria', label: 'Auditoria', icon: 'audit', roles: ['ADMIN'], section: 'Administracao' },

  // Conta (rodape).
  { to: '/trocar-senha', label: 'Trocar senha', icon: 'password', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM', 'PACIENTE'], section: 'Conta' },
]

export function navForRole(papel) {
  return NAV.filter((item) => item.roles.includes(papel))
}
