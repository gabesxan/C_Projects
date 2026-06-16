import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { useAuth } from '../auth/AuthContext'
import { apiGet } from '../api/client'
import { PageHeader, StatCard, Card, Button, papelLabel, Spinner } from '../components/ui'

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
      <p className="text-sm font-semibold text-slate-700">Proximos passos</p>
      <div className="mt-3 flex flex-wrap gap-2">
        {itens.map((a) => (
          <Link key={a.to} to={a.to}>
            <Button variant="secondary">{a.label}</Button>
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
        <StatCard label="Pacientes" value={c.pacientes} />
        <StatCard label="Medicos" value={c.medicos} />
        <StatCard label="Usuarios" value={c.usuarios} />
        <StatCard label="Triagens ativas" value={c.triagens} />
        <StatCard label="Internacoes" value={c.internacoes} />
        <StatCard label="Leitos" value={c.leitos} />
      </div>
      <Atalhos
        itens={[
          { to: '/admin/usuarios', label: 'Gerenciar usuarios' },
          { to: '/admin/auditoria', label: 'Ver auditoria' },
          { to: '/relatorios', label: 'Relatorios' },
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
        <StatCard label="Pacientes" value={c.pacientes} />
        <StatCard label="Medicos" value={c.medicos} />
      </div>
      <Atalhos
        itens={[
          { to: '/r/pacientes', label: 'Cadastrar paciente' },
          { to: '/r/medicos', label: 'Cadastrar medico' },
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
        <StatCard label="Meus pacientes" value={resumo.pacientes} />
        <StatCard label="Agendamentos" value={resumo.agendamentos} />
        <StatCard label="Prontuarios" value={resumo.prontuarios} />
        <StatCard label="Exames" value={resumo.exames} />
      </div>
      <Atalhos
        itens={[
          { to: '/r/triagens', label: 'Fila de triagem' },
          { to: '/r/agendamentos', label: 'Minha agenda' },
          { to: '/r/prontuarios', label: 'Prontuarios' },
        ]}
      />
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
        <StatCard label="Triagens ativas" value={c.triagens} />
        <StatCard label="Internacoes" value={c.internacoes} />
        <StatCard label="Leitos" value={c.leitos} />
      </div>
      <Atalhos
        itens={[
          { to: '/r/triagens', label: 'Registrar triagem' },
          { to: '/r/prescricoes', label: 'Prescricoes a aplicar' },
        ]}
      />
    </div>
  )
}

function PainelVazio() {
  return null
}

function PainelPaciente() {
  return (
    <Card className="p-6">
      <p className="text-slate-700">
        Acompanhe suas receitas, exames e prontuarios na area do paciente.
      </p>
      <div className="mt-4">
        <Link to="/minha-saude">
          <Button>Ir para Meus dados</Button>
        </Link>
      </div>
    </Card>
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

  return (
    <div className="space-y-6">
      <PageHeader
        title="Painel"
        subtitle={`Bem-vindo(a). Voce esta logado(a) como ${papelLabel(user.papel)}.`}
      />
      <Painel />
    </div>
  )
}
