import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'
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
  const [erro, setErro] = useState('')

  // Distribuicao (pacientes por regiao, medicos por especialidade) — uma vez.
  useEffect(() => {
    apiGet('/relatorios/distribuicao')
      .then(setDistrib)
      .catch((e) =>
        setErro(e.status === 403 ? 'Voce nao tem acesso aos relatorios.' : e.message)
      )
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
    return (
      <div className="rounded-lg bg-red-50 text-red-700 text-sm px-4 py-3">
        {erro}
      </div>
    )
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

  return (
    <div className="space-y-6">
      <h2 className="text-xl font-semibold text-slate-800">Relatorios</h2>

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
