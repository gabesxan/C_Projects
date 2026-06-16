// Navegacao lateral por papel. Cada item declara os papeis que o enxergam.
// O backend tambem barra por papel (403); aqui apenas evitamos oferecer o que
// nao serve ao usuario logado.

const NAV = [
  { to: '/', label: 'Painel', icon: '🏠', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM', 'PACIENTE'], end: true },
  { to: '/minha-saude', label: 'Meus dados', icon: '💜', roles: ['PACIENTE'] },
  { to: '/r/pacientes', label: 'Pacientes', icon: '🧑‍⚕️', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/r/medicos', label: 'Médicos', icon: '🩺', roles: ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/r/triagens', label: 'Triagem', icon: '🚑', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/r/agendamentos', label: 'Agendamentos', icon: '📅', roles: ['ADMIN', 'MEDICO'] },
  { to: '/r/prontuarios', label: 'Prontuários', icon: '📄', roles: ['ADMIN', 'MEDICO'] },
  { to: '/r/exames', label: 'Exames', icon: '🧪', roles: ['ADMIN', 'MEDICO'] },
  { to: '/r/prescricoes', label: 'Prescrições', icon: '💊', roles: ['ADMIN', 'MEDICO', 'ENFERMAGEM'] },
  { to: '/relatorios', label: 'Relatórios', icon: '📊', roles: ['ADMIN', 'MEDICO'] },
  { to: '/admin/usuarios', label: 'Usuários', icon: '👥', roles: ['ADMIN'] },
  { to: '/admin/auditoria', label: 'Auditoria', icon: '🛡️', roles: ['ADMIN'] },
]

export function navForRole(papel) {
  return NAV.filter((item) => item.roles.includes(papel))
}
