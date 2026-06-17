import { useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import {
  PageHeader,
  Card,
  Button,
  Alert,
  Spinner,
  Badge,
  EmptyState,
} from '../components/ui'

const RISCO = {
  Vermelho: 'red',
  Laranja: 'amber',
  Amarelo: 'amber',
  Verde: 'green',
  Azul: 'sky',
}
const tomRisco = (v) => RISCO[v] ?? 'slate'

const TIPOS = [
  { v: 1, label: 'Clinico Geral' },
  { v: 2, label: 'Ortopedia' },
  { v: 3, label: 'Cardiologia' },
  { v: 4, label: 'Pneumologia' },
  { v: 5, label: 'Pediatria' },
]

const inputCls =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30 outline-none'

// --- Passo 1a: buscar paciente existente -------------------------------
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
      <p className="text-sm font-semibold text-slate-700">Buscar paciente</p>
      <form onSubmit={buscar} className="mt-3 flex gap-2">
        <input
          className={`${inputCls} mt-0`}
          placeholder="Nome ou documento"
          value={q}
          onChange={(e) => setQ(e.target.value)}
        />
        <Button type="submit">Buscar</Button>
      </form>

      {erro && <div className="mt-3"><Alert>{erro}</Alert></div>}
      {Array.isArray(rows) && rows.length === 0 && (
        <p className="mt-3 text-sm text-slate-500">Nenhum paciente encontrado.</p>
      )}
      {Array.isArray(rows) && rows.length > 0 && (
        <ul className="mt-3 divide-y divide-slate-100">
          {rows.map((p) => (
            <li key={p.id} className="flex items-center justify-between py-2">
              <div>
                <p className="text-sm font-medium text-slate-800">{p.nome}</p>
                <p className="text-xs text-slate-500">
                  {p.idade} anos • {p.tipoDocumento} {p.documento}
                </p>
              </div>
              <Button variant="secondary" onClick={() => onSelecionar(p)}>
                Selecionar
              </Button>
            </li>
          ))}
        </ul>
      )}
    </Card>
  )
}

// --- Passo 1b: cadastrar paciente no fluxo (gera acesso) ----------------
function NovoPaciente({ onCriado }) {
  const [aberto, setAberto] = useState(false)
  const [v, setV] = useState({
    nome: '', nascimento: '', documento: '', tipo_documento: 'CPF',
    telefone: '', sexo: 'F', regiao: '', responsavel: '', alergias: '',
  })
  const [erro, setErro] = useState('')
  const [salvando, setSalvando] = useState(false)

  function set(k, val) { setV((s) => ({ ...s, [k]: val })) }

  async function submit(e) {
    e.preventDefault()
    setErro('')
    setSalvando(true)
    try {
      const cred = await apiSend('POST', '/triagem/pacientes', v)
      const paciente = await apiGet(`/pacientes/${cred.pacienteId}`)
      onCriado(paciente, cred)
    } catch (err) {
      setErro(err.status === 400 ? 'Dados invalidos (verifique nascimento, documento, responsavel de menor ou CPF ja cadastrado).' : err.message)
    } finally {
      setSalvando(false)
    }
  }

  if (!aberto) {
    return (
      <Card className="p-5">
        <p className="text-sm font-semibold text-slate-700">Paciente novo?</p>
        <p className="mt-1 text-sm text-slate-500">
          Cadastre no proprio fluxo de triagem; o acesso do paciente e gerado
          automaticamente.
        </p>
        <div className="mt-3">
          <Button onClick={() => setAberto(true)}>+ Cadastrar paciente</Button>
        </div>
      </Card>
    )
  }

  return (
    <Card className="p-5">
      <form onSubmit={submit} className="space-y-4">
        <p className="text-sm font-semibold text-slate-700">Novo paciente</p>
        {erro && <Alert>{erro}</Alert>}
        <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
          <label className="text-sm text-slate-600">Nome
            <input className={inputCls} value={v.nome} onChange={(e) => set('nome', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">Nascimento
            <input type="date" className={inputCls} value={v.nascimento} onChange={(e) => set('nascimento', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">Documento
            <input className={inputCls} value={v.documento} onChange={(e) => set('documento', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">Tipo de documento
            <select className={inputCls} value={v.tipo_documento} onChange={(e) => set('tipo_documento', e.target.value)}>
              <option>CPF</option><option>OUTRO</option>
            </select>
          </label>
          <label className="text-sm text-slate-600">Telefone
            <input className={inputCls} value={v.telefone} onChange={(e) => set('telefone', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">Sexo
            <select className={inputCls} value={v.sexo} onChange={(e) => set('sexo', e.target.value)}>
              <option>F</option><option>M</option>
            </select>
          </label>
          <label className="text-sm text-slate-600">Regiao
            <input type="number" className={inputCls} value={v.regiao} onChange={(e) => set('regiao', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Responsavel (se menor)
            <input className={inputCls} value={v.responsavel} onChange={(e) => set('responsavel', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Alergias / alertas
            <input className={inputCls} value={v.alergias} onChange={(e) => set('alergias', e.target.value)} />
          </label>
        </div>
        <div className="flex gap-2">
          <Button type="submit" disabled={salvando}>{salvando ? 'Salvando...' : 'Cadastrar e gerar acesso'}</Button>
          <Button type="button" variant="secondary" onClick={() => setAberto(false)}>Cancelar</Button>
        </div>
      </form>
    </Card>
  )
}

// Credenciais geradas, exibidas com destaque (precisam ser anotadas/repassadas).
function Credenciais({ cred }) {
  return (
    <Alert tone="teal">
      <p className="font-semibold">Acesso do paciente criado — anote e repasse:</p>
      <p className="mt-1 font-mono text-sm">
        login: <strong>{cred.login}</strong> &nbsp;•&nbsp; senha: <strong>{cred.senha}</strong>
      </p>
    </Alert>
  )
}

// --- Passo 2: registrar triagem ----------------------------------------
function FormTriagem({ paciente, onRegistrada }) {
  const [v, setV] = useState({
    tipo: 1, classificacao: 'Verde', pontuacao: 3,
    queixa: '', pressao: '', temperatura: '', freq_cardiaca: '', saturacao: '',
  })
  const [erro, setErro] = useState('')
  const [salvando, setSalvando] = useState(false)

  function set(k, val) { setV((s) => ({ ...s, [k]: val })) }

  async function submit(e) {
    e.preventDefault()
    setErro('')
    setSalvando(true)
    try {
      await apiSend('POST', '/triagens', { paciente_id: paciente.id, ...v })
      onRegistrada()
    } catch (err) {
      setErro(err.message)
    } finally {
      setSalvando(false)
    }
  }

  return (
    <Card className="p-5">
      <form onSubmit={submit} className="space-y-4">
        <p className="text-sm font-semibold text-slate-700">Registrar triagem</p>
        {erro && <Alert>{erro}</Alert>}
        <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
          <label className="text-sm text-slate-600">Tipo
            <select className={inputCls} value={v.tipo} onChange={(e) => set('tipo', Number(e.target.value))}>
              {TIPOS.map((t) => <option key={t.v} value={t.v}>{t.label}</option>)}
            </select>
          </label>
          <label className="text-sm text-slate-600">Classificacao de risco
            <select className={inputCls} value={v.classificacao} onChange={(e) => set('classificacao', e.target.value)}>
              {Object.keys(RISCO).map((c) => <option key={c}>{c}</option>)}
            </select>
          </label>
          <label className="text-sm text-slate-600">Pontuacao
            <input type="number" className={inputCls} value={v.pontuacao} onChange={(e) => set('pontuacao', Number(e.target.value))} />
          </label>
          <label className="text-sm text-slate-600 lg:col-span-3">Queixa principal
            <input className={inputCls} value={v.queixa} onChange={(e) => set('queixa', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Pressao
            <input className={inputCls} placeholder="120/80" value={v.pressao} onChange={(e) => set('pressao', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Temperatura
            <input className={inputCls} placeholder="36.5" value={v.temperatura} onChange={(e) => set('temperatura', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Freq. cardiaca
            <input className={inputCls} placeholder="80" value={v.freq_cardiaca} onChange={(e) => set('freq_cardiaca', e.target.value)} />
          </label>
          <label className="text-sm text-slate-600">Saturacao
            <input className={inputCls} placeholder="98" value={v.saturacao} onChange={(e) => set('saturacao', e.target.value)} />
          </label>
        </div>
        <Button type="submit" disabled={salvando}>{salvando ? 'Registrando...' : 'Registrar triagem'}</Button>
      </form>
    </Card>
  )
}

// --- Passo 3: proximo passo (risco, sugestoes, agendar) ----------------
function ProximoPasso({ paciente }) {
  const [aval, setAval] = useState(null)
  const [medicos, setMedicos] = useState(null)
  const [exames, setExames] = useState(null)
  const [erro, setErro] = useState('')
  const [agenda, setAgenda] = useState({ data: '', horario: '' })
  const [resultado, setResultado] = useState('')

  // Carrega avaliacao/sugestoes uma vez (apos a triagem registrada).
  useEffect(() => {
    apiGet(`/triagem/${paciente.id}/avaliacao`).then(setAval).catch((e) => setErro(e.message))
    apiGet(`/triagem/${paciente.id}/medicos`).then(setMedicos).catch(() => {})
    apiGet(`/triagem/${paciente.id}/exames`).then(setExames).catch(() => {})
  }, [paciente.id])

  async function agendar(e) {
    e.preventDefault()
    setResultado('')
    try {
      const r = await apiSend('POST', `/triagem/${paciente.id}/agendar`, agenda)
      setResultado(`Agendado com medico #${r.medicoId} em ${r.data} ${r.horario}.`)
    } catch (err) {
      setResultado(err.status === 409 ? 'Sem medico/horario disponivel.' : err.message)
    }
  }

  if (erro) return <Alert>{erro}</Alert>
  if (!aval) return <Spinner label="Avaliando triagem..." />

  return (
    <Card className="p-5 space-y-4">
      <div className="flex flex-wrap items-center gap-3">
        <span className="text-sm font-semibold text-slate-700">Risco:</span>
        <Badge tone={tomRisco(aval.classificacao)}>{aval.classificacao}</Badge>
        <span className="text-sm text-slate-500">
          Prioridade {aval.prioridade} • Especialidade provavel:{' '}
          <strong>{aval.especialidadeProvavel}</strong>
        </span>
      </div>

      <div>
        <p className="text-sm font-semibold text-slate-700">Medicos sugeridos</p>
        {medicos?.medicos?.length ? (
          <ul className="mt-1 flex flex-wrap gap-2">
            {medicos.medicos.map((m) => (
              <li key={m.id}><Badge tone="teal">{m.nome} • {m.especialidade}</Badge></li>
            ))}
          </ul>
        ) : (
          <p className="text-sm text-slate-400">Nenhum medico disponivel na regiao/especialidade.</p>
        )}
      </div>

      <div>
        <p className="text-sm font-semibold text-slate-700">Exames sugeridos</p>
        {exames?.examesSugeridos?.length ? (
          <ul className="mt-1 flex flex-wrap gap-2">
            {exames.examesSugeridos.map((ex) => <li key={ex}><Badge tone="sky">{ex}</Badge></li>)}
          </ul>
        ) : (
          <p className="text-sm text-slate-400">Sem exames sugeridos.</p>
        )}
      </div>

      <div>
        <p className="text-sm font-semibold text-slate-700">Proximo passo: agendar atendimento</p>
        <form onSubmit={agendar} className="mt-2 flex flex-wrap items-end gap-2">
          <label className="text-sm text-slate-600">Data
            <input type="date" className={`${inputCls} w-auto`} value={agenda.data} onChange={(e) => setAgenda((a) => ({ ...a, data: e.target.value }))} required />
          </label>
          <label className="text-sm text-slate-600">Horario
            <input className={`${inputCls} w-auto`} placeholder="09:00" value={agenda.horario} onChange={(e) => setAgenda((a) => ({ ...a, horario: e.target.value }))} required />
          </label>
          <Button type="submit">Agendar</Button>
        </form>
        {resultado && <p className="mt-2 text-sm text-slate-600">{resultado}</p>}
      </div>
    </Card>
  )
}

export default function Triagem() {
  const [paciente, setPaciente] = useState(null)
  const [cred, setCred] = useState(null)
  const [registrada, setRegistrada] = useState(false)

  function selecionar(p, credenciais = null) {
    setPaciente(p)
    setCred(credenciais)
    setRegistrada(false)
  }

  function reiniciar() {
    setPaciente(null)
    setCred(null)
    setRegistrada(false)
  }

  return (
    <div className="space-y-6">
      <PageHeader
        title="Triagem"
        subtitle="Busque ou cadastre o paciente, classifique o risco e defina o proximo passo."
        actions={paciente && <Button variant="secondary" onClick={reiniciar}>Nova triagem</Button>}
      />

      {!paciente ? (
        <div className="grid gap-4 lg:grid-cols-2">
          <BuscaPaciente onSelecionar={(p) => selecionar(p)} />
          <NovoPaciente onCriado={(p, c) => selecionar(p, c)} />
        </div>
      ) : (
        <div className="space-y-4">
          {cred && <Credenciais cred={cred} />}

          <Card className="p-5">
            <div className="flex items-center justify-between">
              <div>
                <p className="text-lg font-bold text-slate-900">{paciente.nome}</p>
                <p className="text-sm text-slate-500">
                  {paciente.idade} anos • {paciente.tipoDocumento} {paciente.documento}
                </p>
              </div>
              {paciente.alergias
                ? <Badge tone="red">⚠️ {paciente.alergias}</Badge>
                : <Badge tone="green">Sem alergias</Badge>}
            </div>
          </Card>

          {!registrada ? (
            <FormTriagem paciente={paciente} onRegistrada={() => setRegistrada(true)} />
          ) : (
            <ProximoPasso paciente={paciente} />
          )}
        </div>
      )}

      {!paciente && (
        <EmptyState
          title="Comece a triagem"
          description="Selecione um paciente existente ou cadastre um novo para classificar o risco."
        />
      )}
    </div>
  )
}
