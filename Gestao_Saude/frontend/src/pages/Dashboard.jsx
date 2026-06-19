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
      <p className="text-sm font-semibold text-slate-700">Acoes rapidas</p>
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
          { to: '/triagem', label: 'Triagem clinica guiada' },
          { to: '/r/agendamentos', label: 'Minha agenda' },
          { to: '/r/prontuarios', label: 'Prontuarios' },
          { to: '/laboratorio', label: 'Exames pendentes' },
        ]}
      />
      <div className="grid gap-4 md:grid-cols-3">
        <Card className="p-5">
          <p className="text-sm font-semibold text-slate-800">Agenda do dia</p>
          <p className="mt-1 text-sm text-slate-500">Acompanhe consultas e retornos do seu escopo.</p>
        </Card>
        <Card className="p-5">
          <p className="text-sm font-semibold text-slate-800">Pacientes em espera</p>
          <p className="mt-1 text-sm text-slate-500">Use a fila de recepcao e abra a ficha antes da conduta.</p>
        </Card>
        <Card className="p-5">
          <p className="text-sm font-semibold text-slate-800">Fale Conosco</p>
          <p className="mt-1 text-sm text-slate-500">atendimento@sigehdf.gov.br · (61) 3333-0000</p>
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
        <StatCard label="Triagens ativas" value={c.triagens} />
        <StatCard label="Internacoes" value={c.internacoes} />
        <StatCard label="Leitos" value={c.leitos} />
      </div>
      <Atalhos
        itens={[
          { to: '/triagem', label: 'Apoiar triagem clinica' },
          { to: '/r/prescricoes', label: 'Prescricoes a aplicar' },
          { to: '/recepcao', label: 'Pacientes em espera' },
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
    <div className="grid gap-4 md:grid-cols-3">
      <Card className="p-5 md:col-span-2">
        <p className="text-sm font-semibold text-slate-800">Sua area do paciente</p>
        <p className="mt-1 text-sm text-slate-500">
          Consulte carteirinha, proximas consultas, exames, resultados, receitas e cobrancas proprias.
        </p>
        <div className="mt-4">
          <Link to="/minha-saude">
            <Button>Abrir Minha saude</Button>
          </Link>
        </div>
      </Card>
      <Card className="p-5">
        <p className="text-sm font-semibold text-slate-800">Precisa de ajuda?</p>
        <p className="mt-1 text-sm text-slate-500">Fale com a equipe pelo atendimento do SIGEH-DF.</p>
      </Card>
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
