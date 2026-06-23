import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import { useAuth } from '../auth/AuthContext'
import { PageHeader, Card, Button, Alert, Spinner, EmptyState, Badge } from '../components/ui'

const STATUS_TONE = {
  SOLICITADO: 'slate',
  AUTORIZADO: 'sky',
  COLETADO: 'amber',
  EM_ANALISE: 'violet',
  CONCLUIDO: 'green',
  CANCELADO: 'red',
}

const PROXIMO_STATUS = {
  SOLICITADO: 'AUTORIZADO',
  AUTORIZADO: 'COLETADO',
  COLETADO: 'EM_ANALISE',
}

const emptyAnalitoForm = {
  codigo: '',
  nome: '',
  unidade: '',
  ref_min: '',
  ref_max: '',
  metodo: '',
}

function statusTone(status) {
  return STATUS_TONE[status] ?? 'slate'
}

function examLabel(exame) {
  if (!exame) return ''
  return `#${exame.id} · Paciente ${exame.pacienteId} · Tipo ${exame.tipoExame}`
}

function resultadoPorAnalito(resultados, analitoId) {
  return resultados.find((r) => Number(r.analitoId) === Number(analitoId))
}

function TextInput({ label, value, onChange, type = 'text', placeholder = '' }) {
  return (
    <label className="block">
      <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">{label}</span>
      <input
        className="mt-1 w-full rounded-lg border border-slate-200 px-3 py-2 text-sm outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-100"
        type={type}
        value={value}
        placeholder={placeholder}
        onChange={(e) => onChange(e.target.value)}
      />
    </label>
  )
}

function SelectInput({ label, value, onChange, children }) {
  return (
    <label className="block">
      <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">{label}</span>
      <select
        className="mt-1 w-full rounded-lg border border-slate-200 px-3 py-2 text-sm outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-100"
        value={value}
        onChange={(e) => onChange(e.target.value)}
      >
        {children}
      </select>
    </label>
  )
}

function AnalitoCatalogo({ analitos, onCriado, onErro }) {
  const [form, setForm] = useState(emptyAnalitoForm)
  const [salvando, setSalvando] = useState(false)

  function setCampo(campo, valor) {
    setForm((atual) => ({ ...atual, [campo]: valor }))
  }

  async function salvar(e) {
    e.preventDefault()
    setSalvando(true)
    try {
      await apiSend('POST', '/analitos', form)
      setForm(emptyAnalitoForm)
      onCriado()
    } catch (err) {
      onErro(err.message)
    } finally {
      setSalvando(false)
    }
  }

  return (
    <Card className="p-5">
      <div className="mb-4 flex items-center justify-between gap-3">
        <div>
          <h2 className="text-lg font-bold text-slate-900">Catalogo de analitos</h2>
          <p className="text-sm text-slate-500">{analitos.length} ativo(s)</p>
        </div>
        <Badge tone="teal">Admin</Badge>
      </div>

      <form className="grid gap-3 sm:grid-cols-2" onSubmit={salvar}>
        <TextInput label="Codigo" value={form.codigo} onChange={(v) => setCampo('codigo', v)} />
        <TextInput label="Nome" value={form.nome} onChange={(v) => setCampo('nome', v)} />
        <TextInput label="Unidade" value={form.unidade} onChange={(v) => setCampo('unidade', v)} />
        <TextInput label="Metodo" value={form.metodo} onChange={(v) => setCampo('metodo', v)} />
        <TextInput label="Ref. min" type="number" value={form.ref_min} onChange={(v) => setCampo('ref_min', v)} />
        <TextInput label="Ref. max" type="number" value={form.ref_max} onChange={(v) => setCampo('ref_max', v)} />
        <div className="sm:col-span-2">
          <Button type="submit" disabled={salvando}>
            {salvando ? 'Salvando...' : 'Criar analito'}
          </Button>
        </div>
      </form>
    </Card>
  )
}

function PainelAdmin({ exame, analitos, painel, onAtualizar, onErro }) {
  const [analitoId, setAnalitoId] = useState('')
  const [ordem, setOrdem] = useState('')
  const [salvando, setSalvando] = useState(false)

  const disponiveis = analitos.filter(
    (a) => !painel.some((p) => Number(p.id) === Number(a.id)),
  )
  const analitoSelecionado = disponiveis.some((a) => String(a.id) === analitoId)
    ? analitoId
    : (disponiveis[0]?.id ? String(disponiveis[0].id) : '')

  async function vincular(e) {
    e.preventDefault()
    if (!exame || !analitoSelecionado) return
    setSalvando(true)
    try {
      await apiSend('POST', `/paineis/${exame.tipoExame}/analitos`, {
        analito_id: analitoSelecionado,
        ordem: ordem || String(painel.length + 1),
      })
      setOrdem('')
      await onAtualizar()
    } catch (err) {
      onErro(err.message)
    } finally {
      setSalvando(false)
    }
  }

  if (!exame) return null

  return (
    <Card className="p-5">
      <div className="mb-4 flex items-center justify-between gap-3">
        <div>
          <h2 className="text-lg font-bold text-slate-900">Painel do tipo {exame.tipoExame}</h2>
          <p className="text-sm text-slate-500">{painel.length} analito(s) vinculado(s)</p>
        </div>
        <Badge tone="sky">Tipo {exame.tipoExame}</Badge>
      </div>

      <form className="grid gap-3 sm:grid-cols-[minmax(0,1fr)_120px_auto]" onSubmit={vincular}>
        <SelectInput label="Analito" value={analitoSelecionado} onChange={setAnalitoId}>
          {disponiveis.length === 0 && <option value="">Sem analitos disponiveis</option>}
          {disponiveis.map((a) => (
            <option key={a.id} value={a.id}>{a.codigo} · {a.nome}</option>
          ))}
        </SelectInput>
        <TextInput label="Ordem" type="number" value={ordem} onChange={setOrdem} />
        <div className="flex items-end">
          <Button type="submit" disabled={!analitoSelecionado || salvando}>
            Vincular
          </Button>
        </div>
      </form>
    </Card>
  )
}

export default function Laboratorio() {
  const { user } = useAuth()
  const [exames, setExames] = useState(null)
  const [selecionadoId, setSelecionadoId] = useState('')
  const [analitos, setAnalitos] = useState([])
  const [painel, setPainel] = useState([])
  const [resultados, setResultados] = useState([])
  const [erro, setErro] = useState('')
  const [carregandoDetalhe, setCarregandoDetalhe] = useState(false)
  const [resultadoTexto, setResultadoTexto] = useState('')
  const [valorPorAnalito, setValorPorAnalito] = useState({})
  const [observacaoPorAnalito, setObservacaoPorAnalito] = useState({})

  const selecionado = exames?.find((e) => String(e.id) === String(selecionadoId))
  const podeAdminCatalogo = user.papel === 'ADMIN'
  const podeLancarAnalito = selecionado && ['COLETADO', 'EM_ANALISE'].includes(selecionado.status)
  const podeConcluir = selecionado && ['COLETADO', 'EM_ANALISE'].includes(selecionado.status)
  const podeRetificar = selecionado?.status === 'CONCLUIDO'

  const carregarAnalitos = useCallback(async () => {
    const lista = await apiGet('/analitos')
    setAnalitos(lista)
  }, [])

  const carregarExames = useCallback(async () => {
    const lista = await apiGet('/exames')
    setExames(lista)
    setSelecionadoId((atual) => {
      if (lista.some((e) => String(e.id) === String(atual))) return atual
      return lista[0]?.id ? String(lista[0].id) : ''
    })
  }, [])

  async function recarregarExamesSelecionando(proximoId) {
    const lista = await apiGet('/exames')
    setExames(lista)
    if (proximoId) {
      setSelecionadoId(String(proximoId))
      return
    }
    setSelecionadoId((atual) => {
      if (lista.some((e) => String(e.id) === String(atual))) return atual
      return lista[0]?.id ? String(lista[0].id) : ''
    })
  }

  const carregarDetalhe = useCallback(async () => {
    if (!selecionado) {
      setPainel([])
      setResultados([])
      return
    }
    setCarregandoDetalhe(true)
    try {
      const [painelAtual, resultadosAtuais] = await Promise.all([
        apiGet(`/paineis/${selecionado.tipoExame}/analitos`),
        apiGet(`/exames/${selecionado.id}/resultados-analitos`),
      ])
      setPainel(painelAtual)
      setResultados(resultadosAtuais)
      setResultadoTexto(selecionado.resultado || '')
      const valores = {}
      const observacoes = {}
      painelAtual.forEach((a) => {
        const r = resultadoPorAnalito(resultadosAtuais, a.id)
        valores[a.id] = r?.valorTexto || (r?.valor != null ? String(r.valor) : '')
        observacoes[a.id] = r?.observacao || ''
      })
      setValorPorAnalito(valores)
      setObservacaoPorAnalito(observacoes)
    } catch (err) {
      setErro(err.message)
    } finally {
      setCarregandoDetalhe(false)
    }
  }, [selecionado])

  useEffect(() => {
    queueMicrotask(() => {
      carregarExames().catch((err) => setErro(err.message))
      carregarAnalitos().catch(() => setAnalitos([]))
    })
  }, [carregarAnalitos, carregarExames])

  useEffect(() => {
    queueMicrotask(() => carregarDetalhe())
  }, [carregarDetalhe])

  async function atualizarTudo() {
    await carregarExames()
    await carregarAnalitos()
    await carregarDetalhe()
  }

  async function avancarStatus() {
    if (!selecionado) return
    const proximo = PROXIMO_STATUS[selecionado.status]
    if (!proximo) return
    try {
      await apiSend('POST', `/exames/${selecionado.id}/status`, { valor: proximo })
      setErro('')
      await carregarExames()
    } catch (err) {
      setErro(err.message)
    }
  }

  async function salvarAnalito(analito) {
    if (!selecionado) return
    const valor = valorPorAnalito[analito.id]
    if (!valor || Number.isNaN(Number(valor))) {
      setErro('Informe um valor numerico para o analito.')
      return
    }
    try {
      await apiSend('POST', `/exames/${selecionado.id}/resultados-analitos`, {
        analito_id: String(analito.id),
        valor: String(valor),
        valor_texto: String(valor),
        observacao: observacaoPorAnalito[analito.id] || '',
      })
      setErro('')
      await carregarDetalhe()
    } catch (err) {
      setErro(err.message)
    }
  }

  async function retificarAnalito(analito) {
    if (!selecionado) return
    const valor = valorPorAnalito[analito.id]
    if (!valor || Number.isNaN(Number(valor))) {
      setErro('Informe um valor numerico para retificar.')
      return
    }
    const justificativa = window.prompt(`Justificativa para retificar ${analito.codigo}:`)
    if (justificativa === null) return
    if (!justificativa.trim()) {
      setErro('Justificativa e obrigatoria.')
      return
    }
    try {
      await apiSend('POST', `/exames/${selecionado.id}/resultados-analitos/retificar`, {
        analito_id: String(analito.id),
        valor: String(valor),
        valor_texto: String(valor),
        observacao: observacaoPorAnalito[analito.id] || '',
        justificativa,
      })
      setErro('')
      const lista = await apiGet('/exames')
      setExames(lista)
      const novaVersao = lista
        .filter((e) =>
          e.pacienteId === selecionado.pacienteId &&
          e.medicoId === selecionado.medicoId &&
          e.prontuarioId === selecionado.prontuarioId &&
          e.tipoExame === selecionado.tipoExame)
        .sort((a, b) => b.id - a.id)[0]
      setSelecionadoId(novaVersao?.id ? String(novaVersao.id) : '')
    } catch (err) {
      setErro(err.message)
    }
  }

  async function concluirExame() {
    if (!selecionado) return
    if (!resultadoTexto.trim()) {
      setErro('Informe o laudo textual para concluir.')
      return
    }
    try {
      await apiSend('POST', `/exames/${selecionado.id}/resultado`, {
        resultado: resultadoTexto,
        critico: resultados.some((r) => r.foraReferencia) ? '1' : '0',
      })
      setErro('')
      await recarregarExamesSelecionando(selecionado.id)
    } catch (err) {
      setErro(err.message)
    }
  }

  return (
    <div className="space-y-5">
      <PageHeader
        title="Laboratorio"
        subtitle="Exames, paineis e resultados estruturados por analito"
        actions={exames && <Badge tone="slate">{exames.length} exame(s)</Badge>}
      />

      {erro && <Alert>{erro}</Alert>}
      {exames === null && <Spinner />}

      {exames?.length === 0 && (
        <EmptyState title="Sem exames" description="Os exames solicitados aparecem aqui." />
      )}

      {exames?.length > 0 && (
        <div className="grid gap-5 xl:grid-cols-[360px_minmax(0,1fr)]">
          <Card className="p-5">
            <div className="mb-4 flex items-center justify-between">
              <h2 className="text-lg font-bold text-slate-900">Fila de exames</h2>
              <Button variant="secondary" className="px-3 py-1" onClick={carregarExames}>
                Atualizar
              </Button>
            </div>
            <div className="space-y-2">
              {exames.map((exame) => (
                <button
                  key={exame.id}
                  className={[
                    'w-full rounded-lg border px-3 py-3 text-left transition',
                    String(selecionadoId) === String(exame.id)
                      ? 'border-teal-300 bg-teal-50'
                      : 'border-slate-200 bg-white hover:bg-slate-50',
                  ].join(' ')}
                  onClick={() => setSelecionadoId(String(exame.id))}
                >
                  <div className="flex items-center justify-between gap-2">
                    <span className="font-semibold text-slate-900">Exame #{exame.id}</span>
                    <Badge tone={statusTone(exame.status)}>{exame.status}</Badge>
                  </div>
                  <p className="mt-1 text-sm text-slate-500">
                    Paciente {exame.pacienteId} · Medico {exame.medicoId}
                  </p>
                  <p className="text-xs text-slate-400">
                    Tipo {exame.tipoExame} · {exame.dataSolicitacao}
                  </p>
                </button>
              ))}
            </div>
          </Card>

          <div className="space-y-5">
            <Card className="p-5">
              <div className="flex flex-wrap items-start justify-between gap-3">
                <div>
                  <h2 className="text-xl font-bold text-slate-900">{examLabel(selecionado)}</h2>
                  {selecionado && (
                    <div className="mt-2 flex flex-wrap gap-2">
                      <Badge tone={statusTone(selecionado.status)}>{selecionado.status}</Badge>
                      <Badge tone="sky">Tipo {selecionado.tipoExame}</Badge>
                      <Badge tone="slate">Prontuario {selecionado.prontuarioId}</Badge>
                    </div>
                  )}
                </div>
                {PROXIMO_STATUS[selecionado?.status] && (
                  <Button onClick={avancarStatus}>
                    Marcar {PROXIMO_STATUS[selecionado.status]}
                  </Button>
                )}
              </div>
            </Card>

            {podeAdminCatalogo && (
              <div className="grid gap-5 lg:grid-cols-2">
                <AnalitoCatalogo analitos={analitos} onCriado={atualizarTudo} onErro={setErro} />
                <PainelAdmin
                  exame={selecionado}
                  analitos={analitos}
                  painel={painel}
                  onAtualizar={carregarDetalhe}
                  onErro={setErro}
                />
              </div>
            )}

            <Card className="p-5">
              <div className="mb-4 flex flex-wrap items-center justify-between gap-3">
                <div>
                  <h2 className="text-lg font-bold text-slate-900">Resultados por analito</h2>
                  <p className="text-sm text-slate-500">{painel.length} item(ns) no painel</p>
                </div>
                {carregandoDetalhe && <Spinner label="Carregando painel..." />}
              </div>

              {painel.length === 0 && (
                <EmptyState title="Painel vazio" description="Vincule analitos ao tipo do exame." />
              )}

              {painel.length > 0 && (
                <div className="overflow-x-auto">
                  <table className="min-w-full text-sm">
                    <thead>
                      <tr className="border-b border-slate-200 text-left text-slate-500">
                        <th className="px-3 py-2 font-semibold">Analito</th>
                        <th className="px-3 py-2 font-semibold">Referencia</th>
                        <th className="px-3 py-2 font-semibold">Valor</th>
                        <th className="px-3 py-2 font-semibold">Observacao</th>
                        <th className="px-3 py-2 font-semibold">Status</th>
                        <th className="px-3 py-2 font-semibold">Acoes</th>
                      </tr>
                    </thead>
                    <tbody>
                      {painel.map((analito) => {
                        const resultado = resultadoPorAnalito(resultados, analito.id)
                        return (
                          <tr key={analito.id} className="border-b border-slate-100 last:border-0">
                            <td className="px-3 py-3">
                              <p className="font-semibold text-slate-900">{analito.codigo}</p>
                              <p className="text-xs text-slate-500">{analito.nome}</p>
                            </td>
                            <td className="px-3 py-3 text-slate-600">
                              {analito.refMin} a {analito.refMax} {analito.unidade}
                            </td>
                            <td className="px-3 py-3">
                              <input
                                className="w-28 rounded-lg border border-slate-200 px-3 py-2 outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-100"
                                type="number"
                                value={valorPorAnalito[analito.id] || ''}
                                disabled={!podeLancarAnalito && !podeRetificar}
                                onChange={(e) => setValorPorAnalito((atual) => ({
                                  ...atual,
                                  [analito.id]: e.target.value,
                                }))}
                              />
                            </td>
                            <td className="px-3 py-3">
                              <input
                                className="w-48 rounded-lg border border-slate-200 px-3 py-2 outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-100"
                                value={observacaoPorAnalito[analito.id] || ''}
                                disabled={!podeLancarAnalito && !podeRetificar}
                                onChange={(e) => setObservacaoPorAnalito((atual) => ({
                                  ...atual,
                                  [analito.id]: e.target.value,
                                }))}
                              />
                            </td>
                            <td className="px-3 py-3">
                              {resultado ? (
                                <Badge tone={resultado.foraReferencia ? 'red' : 'green'}>
                                  {resultado.foraReferencia ? 'Fora da faixa' : 'Normal'}
                                </Badge>
                              ) : (
                                <Badge tone="slate">Pendente</Badge>
                              )}
                            </td>
                            <td className="px-3 py-3">
                              {podeLancarAnalito && (
                                <Button className="px-3 py-1" onClick={() => salvarAnalito(analito)}>
                                  Salvar
                                </Button>
                              )}
                              {podeRetificar && (
                                <Button
                                  variant="secondary"
                                  className="px-3 py-1"
                                  onClick={() => retificarAnalito(analito)}
                                >
                                  Retificar
                                </Button>
                              )}
                            </td>
                          </tr>
                        )
                      })}
                    </tbody>
                  </table>
                </div>
              )}
            </Card>

            <Card className="p-5">
              <div className="mb-3 flex flex-wrap items-center justify-between gap-3">
                <h2 className="text-lg font-bold text-slate-900">Laudo</h2>
                {podeConcluir && <Button onClick={concluirExame}>Concluir exame</Button>}
              </div>
              <textarea
                className="min-h-28 w-full rounded-lg border border-slate-200 px-3 py-2 text-sm outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-100"
                value={resultadoTexto}
                disabled={!podeConcluir}
                onChange={(e) => setResultadoTexto(e.target.value)}
              />
            </Card>
          </div>
        </div>
      )}
    </div>
  )
}
