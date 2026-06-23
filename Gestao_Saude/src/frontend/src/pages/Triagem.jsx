import { useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { apiGet, apiSend } from '../api/client'
import { PageHeader, Card, Button, Alert, Spinner, Badge, EmptyState } from '../components/ui'
import { ICONS, Icon } from '../components/icons'

const RISCO = {
  Vermelho: 'red',
  Laranja: 'amber',
  Amarelo: 'amber',
  Verde: 'green',
  Azul: 'sky',
}

const inputCls =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-2 text-sm outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30'

const tomRisco = (v) => RISCO[v] ?? 'slate'
const classePorPeso = (peso) => (peso >= 5 ? 'Vermelho' : peso >= 4 ? 'Laranja' : peso >= 3 ? 'Amarelo' : peso >= 2 ? 'Verde' : 'Azul')

function Stepper({ etapa }) {
  const etapas = ['Paciente', 'Sinais', 'Decisao']
  return (
    <Card className="p-3">
      <div className="grid gap-2 sm:grid-cols-3">
        {etapas.map((nome, idx) => {
          const ativo = idx + 1 <= etapa
          return (
            <div key={nome} className={`rounded-xl px-3 py-2 text-sm ${ativo ? 'bg-teal-50 text-teal-800 dark:bg-teal-950/60 dark:text-teal-100' : 'bg-slate-50 text-slate-500 dark:bg-slate-800'}`}>
              <span className="mr-2 inline-flex h-6 w-6 items-center justify-center rounded-full bg-white text-xs font-bold shadow-sm dark:bg-slate-900">{idx + 1}</span>
              {nome}
            </div>
          )
        })}
      </div>
    </Card>
  )
}

function BuscaPaciente({ onSelecionar }) {
  const [q, setQ] = useState('')
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  async function buscar(e) {
    e.preventDefault()
    setErro('')
    setRows(null)
    try {
      setRows(await apiGet(`/pacientes/buscar?q=${encodeURIComponent(q)}`))
    } catch (err) {
      setErro(err.message)
    }
  }

  return (
    <Card className="p-5">
      <div className="flex items-start justify-between gap-4">
        <div>
          <p className="text-sm font-semibold text-slate-700">1. Escolher paciente</p>
          <p className="mt-1 text-sm text-slate-500">Busque pelo nome ou documento antes de iniciar a avaliacao clinica.</p>
        </div>
        <Badge tone="teal" icon={ICONS.doctor}>Profissional</Badge>
      </div>
      <form onSubmit={buscar} className="mt-4 flex gap-2">
        <input
          className={`${inputCls} mt-0`}
          placeholder="Nome ou documento"
          value={q}
          onChange={(e) => setQ(e.target.value)}
        />
        <Button type="submit"><Icon icon={ICONS.search} size={16} />Buscar</Button>
      </form>
      {erro && <div className="mt-3"><Alert>{erro}</Alert></div>}
      {Array.isArray(rows) && rows.length === 0 && (
        <p className="mt-3 text-sm text-slate-500">Nenhum paciente encontrado.</p>
      )}
      {Array.isArray(rows) && rows.length > 0 && (
        <ul className="mt-4 divide-y divide-slate-100 rounded-lg border border-slate-200 bg-white dark:border-slate-700 dark:bg-slate-900">
          {rows.map((p) => (
            <li key={p.id} className="flex items-center justify-between gap-3 px-4 py-3">
              <div>
                <p className="text-sm font-semibold text-slate-900">{p.nome}</p>
                <p className="text-xs text-slate-500">
                  Paciente #{p.id} · {p.idade} anos · {p.tipoDocumento} {p.documento}
                </p>
              </div>
              <Button variant="secondary" onClick={() => onSelecionar(p)}>Selecionar</Button>
            </li>
          ))}
        </ul>
      )}
    </Card>
  )
}

function PainelPaciente({ paciente, onLimpar }) {
  return (
    <Card className="p-5">
      <div className="flex flex-wrap items-center justify-between gap-3">
        <div>
          <p className="text-xs font-semibold uppercase text-teal-700 dark:text-teal-300">Paciente selecionado</p>
          <p className="mt-1 text-xl font-bold text-slate-900">{paciente.nome}</p>
          <p className="text-sm text-slate-500">
            #{paciente.id} · {paciente.idade} anos · {paciente.tipoDocumento} {paciente.documento}
          </p>
        </div>
        <div className="flex items-center gap-2">
          {paciente.alergias ? <Badge tone="red" icon={ICONS.alert}>{paciente.alergias}</Badge> : <Badge tone="green">Sem alergias</Badge>}
          <Button variant="secondary" onClick={onLimpar}>Trocar</Button>
        </div>
      </div>
    </Card>
  )
}

function SeletorProblemas({ especialidade, titulo, selecionados, onToggle }) {
  const [problemas, setProblemas] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    if (!especialidade?.id) return
    queueMicrotask(() => {
      setProblemas(null)
      apiGet(`/especialidades/${especialidade.id}/problemas`)
        .then(setProblemas)
        .catch((e) => setErro(e.message))
    })
  }, [especialidade?.id])

  if (!especialidade) return null
  if (erro) return <Alert>{erro}</Alert>
  if (!problemas) return <Spinner label="Carregando problemas clinicos..." />

  return (
    <div>
      <div className="mb-2 flex items-center justify-between gap-2">
        <p className="text-sm font-semibold text-slate-700">{titulo}</p>
        <Badge tone="slate" icon={ICONS.doctor}>{especialidade.nome}</Badge>
      </div>
      <div className="grid gap-2 sm:grid-cols-2">
        {problemas.map((p) => {
          const ativo = selecionados.some((s) => s.id === p.id)
          return (
            <button
              key={p.id}
              type="button"
              onClick={() => onToggle(p, especialidade)}
              className={`rounded-lg border px-3 py-3 text-left text-sm transition ${
                ativo
                  ? 'border-teal-500 bg-teal-50 ring-2 ring-teal-500/20 dark:bg-teal-950/50'
                  : 'border-slate-200 bg-white hover:border-teal-300 dark:border-slate-700 dark:bg-slate-900'
              }`}
            >
              <span className="flex items-center justify-between gap-3">
                <span className="font-medium text-slate-800">{p.nome}</span>
                <Badge tone={tomRisco(classePorPeso(p.pesoRisco))} icon={ICONS.triage}>{classePorPeso(p.pesoRisco)}</Badge>
              </span>
              {p.exameSugerido && <span className="mt-1 block text-xs text-slate-500">{p.exameSugerido}</span>}
            </button>
          )
        })}
      </div>
    </div>
  )
}

function ResumoClinico({ selecionados }) {
  const maiorPeso = selecionados.reduce((max, p) => Math.max(max, p.pesoRisco), 1)
  const classificacao = classePorPeso(maiorPeso)
  const exames = [...new Set(selecionados.map((p) => p.exameSugerido).filter(Boolean))]
  // Sem useMemo manual: o React Compiler ja memoiza; a lista de selecionados e
  // pequena. (Evita o erro de lint "memoization could not be preserved".)
  const mapa = new Map()
  selecionados.forEach((p) => mapa.set(p.especialidadeNome, (mapa.get(p.especialidadeNome) || 0) + p.pesoRisco))
  const porEspecialidade = [...mapa.entries()].sort((a, b) => b[1] - a[1])

  return (
    <Card className="p-5">
      <p className="flex items-center gap-2 text-sm font-semibold text-slate-700">
        <Icon icon={ICONS.triage} className="text-teal-600" size={18} />
        Resumo calculado
      </p>
      <div className="mt-4 grid gap-3 sm:grid-cols-3">
        <div className="rounded-lg bg-slate-50 p-3 dark:bg-slate-800">
          <p className="text-xs text-slate-500">Risco</p>
          <div className="mt-1"><Badge tone={tomRisco(classificacao)} icon={ICONS.alert}>{classificacao}</Badge></div>
        </div>
        <div className="rounded-lg bg-slate-50 p-3 dark:bg-slate-800">
          <p className="text-xs text-slate-500">Prioridade</p>
          <p className="mt-1 text-lg font-bold text-slate-900">{maiorPeso}</p>
        </div>
        <div className="rounded-lg bg-slate-50 p-3 dark:bg-slate-800">
          <p className="text-xs text-slate-500">Especialidade provavel</p>
          <p className="mt-1 text-sm font-semibold text-slate-900">{porEspecialidade[0]?.[0] || 'A definir'}</p>
        </div>
      </div>
      <div className="mt-4 flex flex-wrap gap-2">
        {exames.length ? exames.map((e) => <Badge key={e} tone="sky" icon={ICONS.lab}>{e}</Badge>) : <Badge icon={ICONS.lab}>Sem exame sugerido</Badge>}
      </div>
    </Card>
  )
}

function Resultado({ triagem, avaliacao, exames, paciente, especialidades, onNova }) {
  const [agenda, setAgenda] = useState({ data: '', horario: '', especialidade: avaliacao?.especialidadeProvavel || '' })
  const [msg, setMsg] = useState('')
  const [erro, setErro] = useState('')

  async function agendar(path) {
    setMsg('')
    setErro('')
    if (!agenda.data || !agenda.horario || !agenda.especialidade) {
      setErro('Selecione data, horário e especialidade.')
      return
    }
    try {
      const r = await apiSend('POST', path, agenda)
      setMsg(r?.agendado || r?.encaminhado ? 'Acao registrada com sucesso.' : r?.motivo || 'Acao processada.')
    } catch (err) {
      setErro(err.message)
    }
  }

  return (
    <div className="space-y-4">
      <Card className="p-5">
        <div className="flex flex-wrap items-start justify-between gap-3">
          <div>
            <p className="text-sm font-semibold text-slate-700">Avaliacao clinica</p>
            <p className="mt-1 text-sm text-slate-500">{avaliacao.justificativa}</p>
          </div>
          <Badge tone={tomRisco(avaliacao.classificacao)} icon={ICONS.alert}>{avaliacao.classificacao}</Badge>
        </div>
        <div className="mt-4 grid gap-3 md:grid-cols-3">
          <Info label="Prioridade" value={avaliacao.prioridade} />
          <Info label="Especialidade provavel" value={avaliacao.especialidadeProvavel} />
          <Info label="Problemas registrados" value={triagem.problemas?.length || 0} />
        </div>
        <p className="mt-4 rounded-lg bg-slate-50 p-3 text-sm text-slate-600 dark:bg-slate-800">
          {avaliacao.proximosPassos}
        </p>
      </Card>

      <Card className="p-5">
        <p className="text-sm font-semibold text-slate-700">Exames sugeridos</p>
        <div className="mt-3 flex flex-wrap gap-2">
          {exames.examesSugeridos?.length
            ? exames.examesSugeridos.map((e) => <Badge key={e.nome} tone="sky" icon={ICONS.lab}>{e.nome}</Badge>)
            : <Badge icon={ICONS.lab}>Sem exame sugerido</Badge>}
        </div>
      </Card>

      <Card className="p-5">
        <p className="text-sm font-semibold text-slate-700">Acao final</p>
        {erro && <div className="mt-3"><Alert>{erro}</Alert></div>}
        {msg && <div className="mt-3"><Alert tone="teal">{msg}</Alert></div>}
        <div className="mt-4 grid gap-3 sm:grid-cols-3">
          <label className="text-sm text-slate-600">Data
            <input type="date" className={inputCls} value={agenda.data} onChange={(e) => setAgenda((s) => ({ ...s, data: e.target.value }))} />
          </label>
          <label className="text-sm text-slate-600">Horario
            <input type="time" step="1800" className={inputCls} value={agenda.horario} onChange={(e) => setAgenda((s) => ({ ...s, horario: e.target.value }))} />
          </label>
          <label className="text-sm text-slate-600">Especialidade
            <select className={inputCls} value={agenda.especialidade} onChange={(e) => setAgenda((s) => ({ ...s, especialidade: e.target.value }))}>
              <option value="">Selecione...</option>
              {especialidades.map((e) => <option key={e.id} value={e.nome}>{e.nome}</option>)}
            </select>
          </label>
        </div>
        <div className="mt-4 flex flex-wrap gap-2">
          <Button disabled={!agenda.data || !agenda.horario} onClick={() => agendar(`/triagens/${triagem.id}/agendar`)}><Icon icon={ICONS.schedule} size={16} />Agendar</Button>
          <Button disabled={!agenda.data || !agenda.horario || !agenda.especialidade} variant="secondary" onClick={() => agendar(`/triagens/${triagem.id}/encaminhar`)}><Icon icon={ICONS.doctor} size={16} />Encaminhar</Button>
          <Link to={`/paciente/${paciente.id}`}><Button variant="secondary"><Icon icon={ICONS.record} size={16} />Abrir ficha</Button></Link>
          <Button variant="secondary" onClick={onNova}>Nova triagem</Button>
        </div>
      </Card>
    </div>
  )
}

function Info({ label, value }) {
  return (
    <div className="rounded-lg bg-slate-50 p-3 dark:bg-slate-800">
      <p className="text-xs text-slate-500">{label}</p>
      <p className="mt-1 text-sm font-semibold text-slate-900">{value}</p>
    </div>
  )
}

export default function Triagem() {
  const [paciente, setPaciente] = useState(null)
  const [especialidades, setEspecialidades] = useState(null)
  const [principalId, setPrincipalId] = useState('')
  const [suspeitaId, setSuspeitaId] = useState('')
  const [selecionados, setSelecionados] = useState([])
  const [dados, setDados] = useState({ queixa: '', observacoes: '', pressao: '', temperatura: '', freq_cardiaca: '', saturacao: '' })
  const [erro, setErro] = useState('')
  const [salvando, setSalvando] = useState(false)
  const [resultado, setResultado] = useState(null)

  useEffect(() => {
    apiGet('/especialidades').then((rows) => {
      setEspecialidades(rows)
      setPrincipalId(String(rows[0]?.id || ''))
    }).catch((e) => setErro(e.message))
  }, [])

  const principal = especialidades?.find((e) => String(e.id) === String(principalId))
  const suspeita = especialidades?.find((e) => String(e.id) === String(suspeitaId))

  function setCampo(k, v) {
    setDados((s) => ({ ...s, [k]: v }))
  }

  function toggleProblema(problema, especialidade) {
    setSelecionados((rows) => {
      if (rows.some((p) => p.id === problema.id)) return rows.filter((p) => p.id !== problema.id)
      return [...rows, { ...problema, especialidadeNome: especialidade.nome, especialidadeId: especialidade.id }]
    })
  }

  async function registrar(e) {
    e.preventDefault()
    setErro('')
    if (!paciente) return setErro('Selecione o paciente.')
    if (!principal) return setErro('Escolha a especialidade principal.')
    if (!selecionados.length) return setErro('Selecione ao menos um problema ou sintoma.')
    setSalvando(true)
    try {
      const r = await apiSend('POST', '/triagens', {
        paciente_id: paciente.id,
        especialidade_principal_id: principal.id,
        ...dados,
      })
      const triagemId = r.triagemId || r.id
      for (const p of selecionados) {
        await apiSend('POST', `/triagens/${triagemId}/problemas`, {
          problema_id: p.id,
          principal: p.especialidadeId === principal.id ? '1' : '0',
        })
      }
      const [triagem, avaliacao, exames] = await Promise.all([
        apiGet(`/triagens/${triagemId}`),
        apiGet(`/triagens/${triagemId}/avaliacao`),
        apiGet(`/triagens/${triagemId}/exames-sugeridos`),
      ])
      setResultado({ triagem, avaliacao, exames })
    } catch (err) {
      setErro(err.message)
    } finally {
      setSalvando(false)
    }
  }

  function reiniciar() {
    setPaciente(null)
    setSelecionados([])
    setDados({ queixa: '', observacoes: '', pressao: '', temperatura: '', freq_cardiaca: '', saturacao: '' })
    setResultado(null)
    setErro('')
  }

  if (!especialidades) return <Spinner label="Carregando catalogo clinico..." />

  return (
    <div className="space-y-5">
      <PageHeader
        title="Triagem clinica"
        subtitle="Fluxo guiado para profissional autorizado, com risco e encaminhamento calculados pelos sinais selecionados."
        actions={paciente && !resultado && <Button variant="secondary" onClick={reiniciar}>Reiniciar</Button>}
      />
      <Stepper etapa={!paciente ? 1 : resultado ? 3 : 2} />

      {erro && <Alert>{erro}</Alert>}
      {!paciente && <BuscaPaciente onSelecionar={setPaciente} />}

      {paciente && <PainelPaciente paciente={paciente} onLimpar={reiniciar} />}

      {paciente && !resultado && (
        <form onSubmit={registrar} className="space-y-5">
          <Card className="p-5">
            <p className="text-sm font-semibold text-slate-700">2. Especialidade e sinais</p>
            <div className="mt-4 grid gap-4 md:grid-cols-2">
              <label className="text-sm text-slate-600">Especialidade principal
                <select className={inputCls} value={principalId} onChange={(e) => setPrincipalId(e.target.value)}>
                  {especialidades.map((e) => <option key={e.id} value={e.id}>{e.nome}</option>)}
                </select>
              </label>
              <label className="text-sm text-slate-600">Adicionar suspeita de outra especialidade
                <select className={inputCls} value={suspeitaId} onChange={(e) => setSuspeitaId(e.target.value)}>
                  <option value="">Nenhuma</option>
                  {especialidades.filter((e) => String(e.id) !== String(principalId)).map((e) => (
                    <option key={e.id} value={e.id}>{e.nome}</option>
                  ))}
                </select>
              </label>
            </div>
            <div className="mt-5 space-y-5">
              <SeletorProblemas
                titulo="Problemas da especialidade principal"
                especialidade={principal}
                selecionados={selecionados}
                onToggle={toggleProblema}
              />
              <SeletorProblemas
                titulo="Suspeita adicional"
                especialidade={suspeita}
                selecionados={selecionados}
                onToggle={toggleProblema}
              />
            </div>
          </Card>

          <Card className="p-5">
            <p className="text-sm font-semibold text-slate-700">3. Queixa, sinais vitais e observacoes</p>
            <div className="mt-4 grid gap-4 md:grid-cols-3">
              <label className="md:col-span-2 text-sm text-slate-600">Queixa principal
                <input className={inputCls} value={dados.queixa} onChange={(e) => setCampo('queixa', e.target.value)} />
              </label>
              <label className="text-sm text-slate-600">Pressao
                <input className={inputCls} value={dados.pressao} onChange={(e) => setCampo('pressao', e.target.value)} />
              </label>
              <label className="text-sm text-slate-600">Temperatura
                <input className={inputCls} value={dados.temperatura} onChange={(e) => setCampo('temperatura', e.target.value)} />
              </label>
              <label className="text-sm text-slate-600">Freq. cardiaca
                <input className={inputCls} value={dados.freq_cardiaca} onChange={(e) => setCampo('freq_cardiaca', e.target.value)} />
              </label>
              <label className="text-sm text-slate-600">Saturacao
                <input className={inputCls} value={dados.saturacao} onChange={(e) => setCampo('saturacao', e.target.value)} />
              </label>
              <label className="md:col-span-3 text-sm text-slate-600">Observacoes
                <textarea className={inputCls} rows={3} value={dados.observacoes} onChange={(e) => setCampo('observacoes', e.target.value)} />
              </label>
            </div>
          </Card>

          {selecionados.length > 0 ? (
            <ResumoClinico selecionados={selecionados} />
          ) : (
            <EmptyState icon={ICONS.triage} title="Selecione problemas clinicos" description="A avaliacao aparece assim que sinais ou sintomas forem marcados." />
          )}

          <div className="flex justify-end">
            <Button type="submit" disabled={salvando}><Icon icon={ICONS.triage} size={16} />{salvando ? 'Registrando...' : 'Registrar triagem clinica'}</Button>
          </div>
        </form>
      )}

      {resultado && (
        <Resultado
          paciente={paciente}
          triagem={resultado.triagem}
          avaliacao={resultado.avaliacao}
          exames={resultado.exames}
          especialidades={especialidades}
          onNova={reiniciar}
        />
      )}
    </div>
  )
}
