// Navegacao lateral por papel. Cada item declara os papeis que o enxergam.
// O backend tambem barra por papel (403); aqui apenas evitamos oferecer o que
// nao serve ao usuario logado.

const NAV = [
  { to: '/', label: 'Painel', icon: 'dashboard', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM', 'PACIENTE'], end: true },
  { to: '/minha-saude', label: 'Visão geral', icon: 'health', roles: ['PACIENTE'], end: true },
  { to: '/minha-saude/carteirinha', label: 'Carteirinha', icon: 'patient', roles: ['PACIENTE'] },
  { to: '/minha-saude/consultas', label: 'Consultas', icon: 'schedule', roles: ['PACIENTE'] },
  { to: '/minha-saude/exames', label: 'Exames', icon: 'lab', roles: ['PACIENTE'] },
  { to: '/minha-saude/receitas', label: 'Receitas', icon: 'prescription', roles: ['PACIENTE'] },
  { to: '/minha-saude/cobrancas', label: 'Cobranças', icon: 'billing', roles: ['PACIENTE'] },
  { to: '/minha-saude/dicas', label: 'Dicas de exames', icon: 'record', roles: ['PACIENTE'] },
  { to: '/minha-saude/ajuda', label: 'Ajuda', icon: 'alert', roles: ['PACIENTE'] },
  { to: '/recepcao', label: 'Recepcao', icon: 'reception', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/r/pacientes', label: 'Pacientes', icon: 'patients', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/r/medicos', label: 'Médicos', icon: 'doctor', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/triagem', label: 'Triagem', icon: 'triage', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/r/agendamentos', label: 'Agendamentos', icon: 'schedule', roles: ['ADMIN', 'MEDICO'] },
  { to: '/r/prontuarios', label: 'Prontuários', icon: 'record', roles: ['ADMIN', 'MEDICO'] },
  { to: '/laboratorio', label: 'Laboratório', icon: 'lab', roles: ['ADMIN', 'MEDICO'] },
  { to: '/r/prescricoes', label: 'Prescrições', icon: 'prescription', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/internacao', label: 'Internações', icon: 'admission', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/enfermagem', label: 'Leitos', icon: 'bed', roles: ['ADMIN', 'ENFERMAGEM', 'CADASTRO'] },
  { to: '/financeiro', label: 'Financeiro', icon: 'finance', roles: ['ADMIN', 'CADASTRO'] },
  { to: '/relatorios', label: 'Relatórios', icon: 'reports', roles: ['ADMIN', 'MEDICO'] },
  { to: '/admin/usuarios', label: 'Usuários', icon: 'users', roles: ['ADMIN'] },
  { to: '/admin/auditoria', label: 'Auditoria', icon: 'audit', roles: ['ADMIN'] },
  { to: '/trocar-senha', label: 'Trocar senha', icon: 'password', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM', 'PACIENTE'] },
]

export function navForRole(papel) {
  return NAV.filter((item) => item.roles.includes(papel))
}
