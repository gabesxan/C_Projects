import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { useAuth } from '../auth/AuthContext'
import { apiGet } from '../api/client'
import { PageHeader, StatCard, Card, Button, papelLabel, Spinner } from '../components/ui'
import { ICONS, Icon } from '../components/icons'

// Conta ativos de um recurso; devolve null se nao houver acesso/erro.
async function contar(path) {
  try {
    const r = await apiGet(`${path}/contar`)
    return r?.ativos ?? null
  } catch {
    return null
  }
}

// Painel de atalhos com a "proxima acao" de cada papel.
function Atalhos({ itens }) {
  return (
    <Card className="p-5">
      <p className="text-sm font-semibold text-slate-700">Acoes rapidas</p>
      <div className="mt-3 grid gap-2 sm:grid-cols-2 lg:grid-cols-4">
        {itens.map((a) => (
          <Link key={a.to} to={a.to}>
            <Button variant="secondary" className="w-full justify-start">
              <Icon icon={ICONS[a.icon]} size={16} />
              {a.label}
            </Button>
          </Link>
        ))}
      </div>
    </Card>
  )
}

function PainelAdmin() {
  const [c, setC] = useState({})
  useEffect(() => {
    Promise.all(
      ['pacientes', 'medicos', 'usuarios', 'triagens', 'internacoes', 'leitos'].map(
        (k) => contar(`/${k}`).then((v) => [k, v])
      )
    ).then((pares) => setC(Object.fromEntries(pares)))
  }, [])

  return (
    <div className="space-y-6">
      <div className="grid grid-cols-2 gap-4 md:grid-cols-3">
        <StatCard label="Pacientes" value={c.pacientes} icon={ICONS.patients} tone="sky" />
        <StatCard label="Medicos" value={c.medicos} icon={ICONS.doctor} tone="teal" />
        <StatCard label="Usuarios" value={c.usuarios} icon={ICONS.users} tone="violet" />
        <StatCard label="Triagens ativas" value={c.triagens} icon={ICONS.triage} tone="amber" />
        <StatCard label="Internacoes" value={c.internacoes} icon={ICONS.admission} tone="red" />
        <StatCard label="Leitos" value={c.leitos} icon={ICONS.bed} tone="emerald" />
      </div>
      <Atalhos
        itens={[
          { to: '/admin/usuarios', label: 'Gerenciar usuarios', icon: 'users' },
          { to: '/admin/auditoria', label: 'Ver auditoria', icon: 'audit' },
          { to: '/relatorios', label: 'Relatorios', icon: 'reports' },
        ]}
      />
    </div>
  )
}

function PainelCadastro() {
  const [c, setC] = useState({})
  useEffect(() => {
    Promise.all(
      ['pacientes', 'medicos'].map((k) => contar(`/${k}`).then((v) => [k, v]))
    ).then((pares) => setC(Object.fromEntries(pares)))
  }, [])

  return (
    <div className="space-y-6">
      <div className="grid grid-cols-2 gap-4">
        <StatCard label="Pacientes" value={c.pacientes} icon={ICONS.patients} tone="sky" />
        <StatCard label="Medicos" value={c.medicos} icon={ICONS.doctor} tone="teal" />
      </div>
      <Atalhos
        itens={[
          { to: '/r/pacientes', label: 'Cadastrar paciente', icon: 'patient' },
          { to: '/r/medicos', label: 'Cadastrar medico', icon: 'doctor' },
        ]}
      />
    </div>
  )
}

function PainelMedico() {
  const [resumo, setResumo] = useState(null)
  const [erro, setErro] = useState('')
  useEffect(() => {
    apiGet('/me/resumo').then(setResumo).catch((e) => setErro(e.message))
  }, [])

  if (erro) return <p className="text-sm text-red-600">{erro}</p>
  if (!resumo) return <Spinner />

  return (
    <div className="space-y-6">
      <div className="grid grid-cols-2 gap-4 md:grid-cols-4">
        <StatCard label="Meus pacientes" value={resumo.pacientes} icon={ICONS.patients} tone="sky" />
        <StatCard label="Agendamentos" value={resumo.agendamentos} icon={ICONS.schedule} tone="teal" />
        <StatCard label="Prontuarios" value={resumo.prontuarios} icon={ICONS.record} tone="violet" />
        <StatCard label="Exames" value={resumo.exames} icon={ICONS.lab} tone="amber" />
      </div>
      <Atalhos
        itens={[
          { to: '/triagem', label: 'Triagem clinica guiada', icon: 'triage' },
          { to: '/r/agendamentos', label: 'Minha agenda', icon: 'schedule' },
          { to: '/r/prontuarios', label: 'Prontuarios', icon: 'record' },
          { to: '/laboratorio', label: 'Exames pendentes', icon: 'lab' },
        ]}
      />
      <div className="grid gap-4 md:grid-cols-3">
        <Card className="p-5">
          <Icon icon={ICONS.schedule} className="mb-3 text-teal-600" size={20} />
          <p className="text-sm font-semibold text-slate-800">Agenda do dia</p>
          <p className="mt-1 text-sm text-slate-500">Acompanhe consultas e retornos do seu escopo.</p>
        </Card>
        <Card className="p-5">
          <Icon icon={ICONS.reception} className="mb-3 text-amber-600" size={20} />
          <p className="text-sm font-semibold text-slate-800">Pacientes em espera</p>
          <p className="mt-1 text-sm text-slate-500">Use a fila de recepcao e abra a ficha antes da conduta.</p>
        </Card>
        <Card className="p-5">
          <Icon icon={ICONS.triage} className="mb-3 text-sky-600" size={20} />
          <p className="text-sm font-semibold text-slate-800">Encaminhamentos</p>
          <p className="mt-1 text-sm text-slate-500">Revise condutas, exames e agendas antes da proxima acao.</p>
        </Card>
      </div>
    </div>
  )
}

function PainelEnfermagem() {
  const [c, setC] = useState({})
  useEffect(() => {
    Promise.all(
      ['triagens', 'internacoes', 'leitos'].map((k) => contar(`/${k}`).then((v) => [k, v]))
    ).then((pares) => setC(Object.fromEntries(pares)))
  }, [])

  return (
    <div className="space-y-6">
      <div className="grid grid-cols-2 gap-4 md:grid-cols-3">
        <StatCard label="Triagens ativas" value={c.triagens} icon={ICONS.triage} tone="amber" />
        <StatCard label="Internacoes" value={c.internacoes} icon={ICONS.admission} tone="red" />
        <StatCard label="Leitos" value={c.leitos} icon={ICONS.bed} tone="emerald" />
      </div>
      <Atalhos
        itens={[
          { to: '/triagem', label: 'Apoiar triagem clinica', icon: 'triage' },
          { to: '/r/prescricoes', label: 'Prescricoes a aplicar', icon: 'prescription' },
          { to: '/recepcao', label: 'Pacientes em espera', icon: 'reception' },
        ]}
      />
    </div>
  )
}

function PainelVazio() {
  return null
}

function PainelPaciente() {
  const atalhos = [
    { to: '/minha-saude', label: 'Carteirinha digital', icon: 'patient' },
    { to: '/minha-saude/consultas', label: 'Proximas consultas', icon: 'schedule' },
    { to: '/minha-saude/exames', label: 'Exames e resultados', icon: 'lab' },
    { to: '/minha-saude/receitas', label: 'Receitas', icon: 'prescription' },
    { to: '/minha-saude/financeiro', label: 'Cobranças', icon: 'billing' },
    { to: '/minha-saude/solicitacoes', label: 'Pedir ajuda', icon: 'alert' },
  ]

  return (
    <div className="mx-auto max-w-5xl space-y-5">
      <Card className="overflow-hidden">
        <div className="px-5 py-7 text-center sm:px-8">
          <div className="mx-auto max-w-3xl">
            <div className="mx-auto flex h-12 w-12 items-center justify-center rounded-2xl bg-teal-100 text-teal-700 dark:bg-teal-950 dark:text-teal-200">
              <Icon icon={ICONS.health} size={24} />
            </div>
            <p className="mt-5 text-xl font-semibold tracking-tight text-slate-950 dark:text-white">
              Sua área do paciente
            </p>
            <p className="mx-auto mt-2 max-w-2xl text-sm leading-6 text-slate-500 dark:text-slate-400">
              Acompanhe carteirinha, consultas, exames, resultados, receitas, cobranças e orientações em um só lugar.
            </p>
            <div className="mt-5 flex flex-wrap justify-center gap-2">
              <Link to="/minha-saude">
                <Button><Icon icon={ICONS.health} size={16} />Abrir visão geral</Button>
              </Link>
              <Link to="/minha-saude/solicitacoes">
                <Button variant="secondary"><Icon icon={ICONS.alert} size={16} />Pedir ajuda</Button>
              </Link>
            </div>
          </div>
          <div className="mx-auto mt-6 grid max-w-3xl gap-2 sm:grid-cols-3">
            {['Confira suas consultas', 'Veja exames pendentes', 'Mantenha documento em mãos'].map((item) => (
              <div key={item} className="flex items-center justify-center gap-2 rounded-2xl bg-slate-50 px-3 py-2 text-sm text-slate-600 shadow-sm ring-1 ring-slate-200/70 dark:bg-slate-800/70 dark:text-slate-300 dark:ring-white/10">
                <span className="h-2 w-2 rounded-full bg-teal-500" />
                {item}
              </div>
            ))}
          </div>
        </div>
      </Card>
      <div className="mx-auto grid max-w-4xl gap-3 sm:grid-cols-2 lg:grid-cols-3">
        {atalhos.map((a) => (
          <Link key={a.to} to={a.to}>
            <Card className="h-full p-5 text-center hover:-translate-y-0.5">
              <div className="mx-auto flex h-10 w-10 items-center justify-center rounded-2xl bg-slate-100 text-teal-700 dark:bg-slate-800 dark:text-teal-200">
                <Icon icon={ICONS[a.icon]} size={20} />
              </div>
              <p className="mt-4 text-sm font-semibold text-slate-800">{a.label}</p>
              <p className="mx-auto mt-1 max-w-48 text-xs leading-5 text-slate-500">Acesso rápido à sua área pessoal.</p>
            </Card>
          </Link>
        ))}
      </div>
    </div>
  )
}

const PAINEIS = {
  ADMIN: PainelAdmin,
  CADASTRO: PainelCadastro,
  MEDICO: PainelMedico,
  ENFERMAGEM: PainelEnfermagem,
  PACIENTE: PainelPaciente,
}

export default function Dashboard() {
  const { user } = useAuth()
  const Painel = PAINEIS[user.papel] ?? PainelVazio
  const paciente = user.papel === 'PACIENTE'

  return (
    <div className="space-y-6">
      <PageHeader
        title={paciente ? 'Painel do paciente' : 'Painel'}
        subtitle={`Bem-vindo(a). Voce esta logado(a) como ${papelLabel(user.papel)}.`}
        align={paciente ? 'center' : 'left'}
      />
      <Painel />
    </div>
  )
}
