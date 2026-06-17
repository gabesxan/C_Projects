import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'
import { PageHeader, Alert, StatCard } from '../components/ui'
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  BarElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js'
import { Bar } from 'react-chartjs-2'

ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend)

const COR = '#0284c7'

const baseOptions = {
  responsive: true,
  plugins: { legend: { display: false } },
  scales: { y: { beginAtZero: true, ticks: { precision: 0 } } },
}

// Datas padrao: primeiro e ultimo dia do mes atual (YYYY-MM-DD).
function fmt(d) {
  return d.toISOString().slice(0, 10)
}
const agora = new Date()
const PRIMEIRO_DIA = fmt(new Date(agora.getFullYear(), agora.getMonth(), 1))
const ULTIMO_DIA = fmt(new Date(agora.getFullYear(), agora.getMonth() + 1, 0))

function ChartCard({ titulo, children }) {
  return (
    <div className="bg-white rounded-xl shadow p-5">
      <h3 className="text-sm font-medium text-slate-600 mb-3">{titulo}</h3>
      {children}
    </div>
  )
}

export default function Relatorios() {
  const [distrib, setDistrib] = useState(null)
  const [inicio, setInicio] = useState(PRIMEIRO_DIA)
  const [fim, setFim] = useState(ULTIMO_DIA)
  const [periodo, setPeriodo] = useState(null)
  const [triagens, setTriagens] = useState(null)
  const [internacoes, setInternacoes] = useState(null)
  const [ocupacao, setOcupacao] = useState(null)
  const [erro, setErro] = useState('')

  // Distribuicao (pacientes por regiao, medicos por especialidade) — uma vez.
  useEffect(() => {
    apiGet('/relatorios/distribuicao')
      .then(setDistrib)
      .catch((e) =>
        setErro(e.status === 403 ? 'Voce nao tem acesso aos relatorios.' : e.message)
      )
    apiGet('/relatorios/triagens').then(setTriagens).catch(() => {})
    apiGet('/relatorios/internacoes').then(setInternacoes).catch(() => {})
    apiGet('/relatorios/ocupacao').then(setOcupacao).catch(() => {})
  }, [])

  // Relatorio por periodo — refaz quando as datas mudam.
  useEffect(() => {
    if (!inicio || !fim) return
    setPeriodo(null)
    apiGet(`/relatorios/agendamentos?inicio=${inicio}&fim=${fim}`)
      .then(setPeriodo)
      .catch((e) => setErro(e.message))
  }, [inicio, fim])

  if (erro) {
    return <Alert>{erro}</Alert>
  }

  const dataRegiao = distrib && {
    labels: distrib.pacientesPorRegiao.map((r) => `RA ${r.regiao}`),
    datasets: [{ label: 'Pacientes', data: distrib.pacientesPorRegiao.map((r) => r.total), backgroundColor: COR }],
  }
  const dataEspec = distrib && {
    labels: distrib.medicosPorEspecialidade.map((m) => m.especialidade),
    datasets: [{ label: 'Medicos', data: distrib.medicosPorEspecialidade.map((m) => m.total), backgroundColor: COR }],
  }
  const dataPeriodo = periodo && {
    labels: periodo.porDia.map((d) => d.data),
    datasets: [{ label: 'Agendamentos', data: periodo.porDia.map((d) => d.total), backgroundColor: COR }],
  }
  const dataTriagens = triagens && {
    labels: triagens.map((t) => t.classificacao),
    datasets: [{ label: 'Triagens', data: triagens.map((t) => t.total), backgroundColor: '#dc2626' }],
  }
  const dataInternacoes = internacoes && {
    labels: internacoes.map((i) => i.status),
    datasets: [{ label: 'Internacoes', data: internacoes.map((i) => i.total), backgroundColor: '#7c3aed' }],
  }

  return (
    <div className="space-y-6">
      <PageHeader
        title="Relatorios"
        subtitle="Ocupacao de leitos, distribuicao, triagens, internacoes e agendamentos."
      />

      {ocupacao && (
        <div className="grid grid-cols-2 gap-4 md:grid-cols-4">
          <StatCard label="Taxa de ocupacao" value={`${ocupacao.taxaOcupacao}%`} />
          <StatCard label="Leitos ocupados" value={ocupacao.ocupados} />
          <StatCard label="Leitos disponiveis" value={ocupacao.disponiveis} />
          <StatCard label="Em higienizacao" value={ocupacao.higienizacao} />
        </div>
      )}

      <div className="grid md:grid-cols-2 gap-6">
        <ChartCard titulo="Triagens por classificacao de risco">
          {dataTriagens ? (
            triagens.length ? (
              <Bar data={dataTriagens} options={baseOptions} />
            ) : (
              <p className="text-sm text-slate-400">Sem dados.</p>
            )
          ) : (
            <p className="text-sm text-slate-400">Carregando...</p>
          )}
        </ChartCard>

        <ChartCard titulo="Internacoes por status">
          {dataInternacoes ? (
            internacoes.length ? (
              <Bar data={dataInternacoes} options={baseOptions} />
            ) : (
              <p className="text-sm text-slate-400">Sem dados.</p>
            )
          ) : (
            <p className="text-sm text-slate-400">Carregando...</p>
          )}
        </ChartCard>
      </div>

      <div className="grid md:grid-cols-2 gap-6">
        <ChartCard titulo="Pacientes por regiao administrativa">
          {dataRegiao ? (
            distrib.pacientesPorRegiao.length ? (
              <Bar data={dataRegiao} options={baseOptions} />
            ) : (
              <p className="text-sm text-slate-400">Sem dados.</p>
            )
          ) : (
            <p className="text-sm text-slate-400">Carregando...</p>
          )}
        </ChartCard>

        <ChartCard titulo="Medicos por especialidade">
          {dataEspec ? (
            distrib.medicosPorEspecialidade.length ? (
              <Bar data={dataEspec} options={baseOptions} />
            ) : (
              <p className="text-sm text-slate-400">Sem dados.</p>
            )
          ) : (
            <p className="text-sm text-slate-400">Carregando...</p>
          )}
        </ChartCard>
      </div>

      <ChartCard titulo="Agendamentos por dia (periodo)">
        <div className="flex flex-wrap items-end gap-4 mb-4">
          <label className="text-sm text-slate-600">
            Inicio
            <input
              type="date"
              value={inicio}
              onChange={(e) => setInicio(e.target.value)}
              className="mt-1 block rounded-lg border border-slate-300 px-3 py-1.5 text-sm"
            />
          </label>
          <label className="text-sm text-slate-600">
            Fim
            <input
              type="date"
              value={fim}
              onChange={(e) => setFim(e.target.value)}
              className="mt-1 block rounded-lg border border-slate-300 px-3 py-1.5 text-sm"
            />
          </label>
          {periodo && (
            <span className="text-sm text-slate-500">
              Total no periodo: <span className="font-semibold">{periodo.total}</span>
            </span>
          )}
        </div>

        {dataPeriodo ? (
          periodo.porDia.length ? (
            <Bar data={dataPeriodo} options={baseOptions} />
          ) : (
            <p className="text-sm text-slate-400">Nenhum agendamento no periodo.</p>
          )
        ) : (
          <p className="text-sm text-slate-400">Carregando...</p>
        )}
      </ChartCard>
    </div>
  )
}
