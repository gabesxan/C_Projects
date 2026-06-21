import { useEffect, useState } from 'react'
import { useNavigate, useParams } from 'react-router-dom'
import { apiGet, apiSend } from '../api/client'
import { useAuth } from '../auth/AuthContext'
import DataTable from '../components/DataTable'
import {
  PageHeader,
  Card,
  Button,
  Alert,
  Spinner,
  Badge,
  StatusBadge,
} from '../components/ui'
import { ICONS, Icon } from '../components/icons'

const inputEvo =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30 outline-none'

// Formulario de evolucao de enfermagem (texto + sinais vitais).
function FormEvolucao({ pacienteId, onSalvo }) {
  const [aberto, setAberto] = useState(false)
  const [v, setV] = useState({ texto: '', pressao: '', temperatura: '', freq_cardiaca: '', saturacao: '' })
  const [erro, setErro] = useState('')

  function set(k, val) { setV((s) => ({ ...s, [k]: val })) }

  async function submit(e) {
    e.preventDefault()
    setErro('')
    if (!v.texto.trim()) { setErro('Texto da evolucao e obrigatorio.'); return }
    try {
      await apiSend('POST', `/pacientes/${pacienteId}/evolucao`, v)
      setV({ texto: '', pressao: '', temperatura: '', freq_cardiaca: '', saturacao: '' })
      setAberto(false)
      onSalvo()
    } catch (err) {
      setErro(err.message)
    }
  }

  if (!aberto) return <Button variant="secondary" onClick={() => setAberto(true)}>+ Evolucao de enfermagem</Button>

  return (
    <Card className="p-5">
      <form onSubmit={submit} className="space-y-3">
        {erro && <Alert>{erro}</Alert>}
        <label className="text-sm text-slate-600">Evolucao
          <textarea className={inputEvo} rows={2} value={v.texto} onChange={(e) => set('texto', e.target.value)} />
        </label>
        <div className="grid grid-cols-2 gap-3 sm:grid-cols-4">
          <label className="text-sm text-slate-600">PA
            <input className={inputEvo} placeholder="120/80" value={v.pressao} onChange={(e) => set('pressao', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Temp
            <input className={inputEvo} placeholder="36.5" value={v.temperatura} onChange={(e) => set('temperatura', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">FC
            <input className={inputEvo} placeholder="80" value={v.freq_cardiaca} onChange={(e) => set('freq_cardiaca', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Sat
            <input className={inputEvo} placeholder="98" value={v.saturacao} onChange={(e) => set('saturacao', e.target.value)} />
          </label>
        </div>
        <div className="flex gap-2">
          <Button type="submit">Salvar</Button>
          <Button type="button" variant="secondary" onClick={() => setAberto(false)}>Cancelar</Button>
        </div>
      </form>
    </Card>
  )
}

const RISCO = {
  Vermelho: 'red',
  Laranja: 'amber',
  Amarelo: 'amber',
  Verde: 'green',
  Azul: 'sky',
}
const tomRisco = (v) => RISCO[v] ?? 'slate'

// Secoes do historico clinico e suas colunas.
const SECOES = [
  {
    titulo: 'Triagens',
    chave: 'triagens',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'tipoTriagem', label: 'Tipo' },
      { key: 'pontuacao', label: 'Pontuacao' },
      { key: 'classificacao', label: 'Classificacao', type: 'badge', tone: tomRisco },
    ],
  },
  {
    titulo: 'Agendamentos',
    chave: 'agendamentos',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'horario', label: 'Horario' },
      { key: 'status', label: 'Status', type: 'badge' },
    ],
  },
  {
    titulo: 'Prontuarios (todas as versoes)',
    chave: 'prontuarios',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'data', label: 'Data' },
      { key: 'diagnostico', label: 'Diagnostico' },
      { key: 'conduta', label: 'Conduta' },
      { key: 'versao', label: 'Versao' },
      { key: 'vigente', label: 'Vigente', type: 'status' },
      { key: 'justificativa', label: 'Justificativa' },
    ],
  },
  {
    titulo: 'Exames',
    chave: 'exames',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'tipoExame', label: 'Tipo' },
      { key: 'dataSolicitacao', label: 'Solicitacao' },
      { key: 'status', label: 'Status', type: 'badge' },
      { key: 'resultado', label: 'Resultado' },
    ],
  },
  {
    titulo: 'Prescricoes',
    chave: 'prescricoes',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'medicamento', label: 'Medicamento' },
      { key: 'dosagem', label: 'Dosagem' },
      { key: 'frequencia', label: 'Frequencia' },
      { key: 'via', label: 'Via' },
      { key: 'observacoes', label: 'Observacoes' },
    ],
  },
  {
    titulo: 'Medicamentos administrados (enfermagem)',
    chave: 'administracoes',
    columns: [
      { key: 'criadoEm', label: 'Quando' },
      { key: 'medicamento', label: 'Medicamento' },
      { key: 'por', label: 'Por' },
      { key: 'observacao', label: 'Observacao' },
    ],
  },
  {
    titulo: 'Evolucao de enfermagem',
    chave: 'evolucoes',
    columns: [
      { key: 'criadoEm', label: 'Quando' },
      { key: 'autor', label: 'Autor' },
      { key: 'texto', label: 'Evolucao' },
      { key: 'pressao', label: 'PA' },
      { key: 'temperatura', label: 'Temp' },
      { key: 'freqCardiaca', label: 'FC' },
      { key: 'saturacao', label: 'Sat' },
    ],
  },
]

function Info({ label, value }) {
  return (
    <div>
      <p className="text-xs font-medium uppercase tracking-wide text-slate-400">{label}</p>
      <p className="text-sm font-medium text-slate-800">{value || '—'}</p>
    </div>
  )
}

// Mostra o convenio atual do paciente e permite (ADMIN/CADASTRO) defini-lo.
function ConvenioPaciente({ pacienteId, convenioId, podeEditar, onAtualizado }) {
  const [convenios, setConvenios] = useState([])
  const [editando, setEditando] = useState(false)
  const [sel, setSel] = useState('')
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet('/convenios').then(setConvenios).catch(() => setConvenios([]))
  }, [])

  const atual = convenios.find((c) => c.id === convenioId)
  const nome = convenioId ? (atual ? atual.nome : `#${convenioId}`) : 'Particular (sem convenio)'

  async function salvar() {
    setErro('')
    try {
      await apiSend('POST', `/pacientes/${pacienteId}/convenio`, { convenio_id: sel || 0 })
      setEditando(false)
      onAtualizado()
    } catch (e) {
      setErro(e.message)
    }
  }

  return (
    <Card className="p-5">
      <div className="flex flex-wrap items-center justify-between gap-3">
        <div>
          <p className="text-xs font-medium uppercase tracking-wide text-slate-400">Convenio</p>
          <p className="text-sm font-medium text-slate-800">{nome}</p>
        </div>
        {podeEditar && !editando && (
          <Button variant="secondary" onClick={() => { setSel(convenioId ? String(convenioId) : ''); setEditando(true) }}>
            Definir convenio
          </Button>
        )}
      </div>
      {editando && (
        <div className="mt-3 flex flex-wrap items-end gap-2">
          <select
            className="rounded-lg border border-slate-300 px-3 py-1.5 text-sm focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30 outline-none"
            value={sel}
            onChange={(e) => setSel(e.target.value)}
          >
            <option value="">Particular (sem convenio)</option>
            {convenios.map((c) => (
              <option key={c.id} value={c.id}>{c.nome}</option>
            ))}
          </select>
          <Button onClick={salvar}>Salvar</Button>
          <Button variant="secondary" onClick={() => setEditando(false)}>Cancelar</Button>
        </div>
      )}
      {erro && <p className="mt-2 text-sm text-red-600">{erro}</p>}
    </Card>
  )
}

export default function PacienteDetalhe() {
  const { id } = useParams()
  const navigate = useNavigate()
  const { user } = useAuth()
  const [paciente, setPaciente] = useState(null)
  const [historico, setHistorico] = useState(null)
  const [erro, setErro] = useState('')
  const [semHistorico, setSemHistorico] = useState(false)

  const podeEvoluir = ['ADMIN', 'MEDICO', 'ENFERMAGEM'].includes(user.papel)
  const podeConvenio = ['ADMIN', 'CADASTRO'].includes(user.papel)

  function carregarPaciente() {
    apiGet(`/pacientes/${id}`)
      .then(setPaciente)
      .catch((e) => setErro(e.status === 404 ? 'Paciente nao encontrado.' : e.message))
  }

  function carregarHistorico() {
    apiGet(`/pacientes/${id}/historico`)
      .then(setHistorico)
      .catch((e) => {
        if (e.status === 403) setSemHistorico(true)
        else setErro(e.message)
      })
  }

  useEffect(() => {
    carregarPaciente()
    carregarHistorico()
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [id])

  if (erro) return <Alert>{erro}</Alert>
  if (!paciente) return <Spinner />

  const menor = paciente.idade < 18

  return (
    <div className="space-y-6">
      <PageHeader
        title={paciente.nome}
        subtitle={`${paciente.idade} anos • ${paciente.tipoDocumento} ${paciente.documento}`}
        actions={
          <Button variant="secondary" onClick={() => navigate('/r/pacientes')}>
            ← Voltar
          </Button>
        }
      />

      {/* Alergias / alertas clinicos em destaque. */}
      {paciente.alergias ? (
        <Alert tone="red">
          <span className="inline-flex items-center gap-1 font-semibold">
            <Icon icon={ICONS.alert} size={16} />
            Alergias / alertas:
          </span>{' '}
          {paciente.alergias}
        </Alert>
      ) : (
        <Alert tone="teal">Sem alergias registradas.</Alert>
      )}

      <Card className="p-5">
        <div className="mb-4 flex items-center gap-2">
          <StatusBadge ativo={paciente.ativo} />
          {menor && <Badge tone="amber">Menor de idade</Badge>}
        </div>
        <div className="grid grid-cols-2 gap-4 md:grid-cols-3">
          <Info label="Nascimento" value={paciente.nascimento} />
          <Info label="Idade" value={`${paciente.idade} anos`} />
          <Info label="Sexo" value={paciente.sexo} />
          <Info label="Telefone" value={paciente.telefone} />
          <Info label="Regiao administrativa" value={paciente.regiaoAdministrativa} />
          <Info label="Documento" value={`${paciente.documento} (${paciente.tipoDocumento})`} />
          {menor && <Info label="Responsavel" value={paciente.responsavel} />}
        </div>
      </Card>

      <ConvenioPaciente
        pacienteId={paciente.id}
        convenioId={paciente.convenioId}
        podeEditar={podeConvenio}
        onAtualizado={carregarPaciente}
      />

      <div>
        <div className="mb-3 flex items-center justify-between">
          <h2 className="text-lg font-bold text-slate-900">Historico clinico</h2>
          {!semHistorico && podeEvoluir && (
            <FormEvolucao pacienteId={paciente.id} onSalvo={carregarHistorico} />
          )}
        </div>
        {semHistorico ? (
          <Alert tone="amber">
            Seu papel nao tem acesso ao historico clinico deste paciente.
          </Alert>
        ) : !historico ? (
          <Spinner />
        ) : (
          <div className="space-y-5">
            {SECOES.map((s) => {
              const linhas = historico[s.chave] ?? []
              return (
                <section key={s.chave} className="space-y-2">
                  <h3 className="text-sm font-semibold text-slate-600">
                    {s.titulo}{' '}
                    <span className="font-normal text-slate-400">({linhas.length})</span>
                  </h3>
                  {linhas.length > 0 ? (
                    <DataTable columns={s.columns} rows={linhas} />
                  ) : (
                    <p className="text-sm text-slate-400">Nenhum registro.</p>
                  )}
                </section>
              )
            })}
          </div>
        )}
      </div>
    </div>
  )
}
