import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import { PageHeader, Card, Button, Alert, Spinner, EmptyState, Badge, StatCard } from '../components/ui'
import { SearchSelect } from '../components/FieldSelect'
import { formatReais } from '../money'
import { AlertTriangle, Boxes, CalendarClock } from 'lucide-react'
import { statusLabel } from '../usability'

const TIPO_TONE = { ENTRADA: 'green', SAIDA: 'amber', AJUSTE: 'violet' }

function Campo({ label, value, onChange, type = 'text', placeholder = '' }) {
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

// Detalhe de um medicamento: saldo, lotes, entrada, dispensacao, saida/ajuste e
// historico. Recarrega ao concluir cada operacao.
function Detalhe({ medicamento, onMudou }) {
  const [lotes, setLotes] = useState(null)
  const [movs, setMovs] = useState(null)
  const [erro, setErro] = useState('')
  const [ok, setOk] = useState('')
  const [entrada, setEntrada] = useState({ lote: '', validade: '', quantidade: '', localizacao: '' })
  const [saida, setSaida] = useState({ tipo: 'SAIDA', quantidade: '', motivo: '' })
  const [disp, setDisp] = useState({ paciente_id: '', quantidade: '', motivo: '' })

  const carregar = useCallback(() => {
    apiGet(`/medicamentos/${medicamento.id}/estoque`)
      .then((d) => { setLotes(d); setErro('') })
      .catch((e) => setErro(e.message))
    apiGet(`/medicamentos/${medicamento.id}/movimentacoes`).then(setMovs).catch((e) => setErro(e.message))
  }, [medicamento.id])

  useEffect(carregar, [carregar])

  const saldo = (lotes || []).reduce((s, l) => s + Number(l.quantidade || 0), 0)
  const baixoEstoque = saldo < Number(medicamento.estoqueMinimo || 0)

  async function enviar(fn) {
    setErro('')
    setOk('')
    try {
      await fn()
      onMudou?.()
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  function registrarEntrada(e) {
    e.preventDefault()
    if (!entrada.lote.trim() || !entrada.validade || Number(entrada.quantidade) <= 0) {
      setErro('Informe lote, validade e uma quantidade maior que zero.')
      return
    }
    enviar(async () => {
      await apiSend('POST', '/estoque', { medicamento_id: String(medicamento.id), ...entrada })
      setEntrada({ lote: '', validade: '', quantidade: '', localizacao: '' })
      setOk('Entrada registrada.')
    })
  }

  function registrarSaida(e) {
    e.preventDefault()
    if (Number(saida.quantidade) <= 0 || !saida.motivo.trim()) {
      setErro('Informe uma quantidade maior que zero e o motivo da movimentação.')
      return
    }
    enviar(async () => {
      await apiSend('POST', '/movimentacoes', { medicamento_id: String(medicamento.id), ...saida })
      setSaida({ tipo: 'SAIDA', quantidade: '', motivo: '' })
      setOk('Movimentacao registrada.')
    })
  }

  function dispensar(e) {
    e.preventDefault()
    if (!disp.paciente_id || Number(disp.quantidade) <= 0) {
      setErro('Selecione o paciente e informe uma quantidade maior que zero.')
      return
    }
    enviar(async () => {
      const r = await apiSend('POST', `/medicamentos/${medicamento.id}/dispensar`, disp)
      setDisp({ paciente_id: '', quantidade: '', motivo: '' })
      setOk(
        r?.cobrancaGerada
          ? `Dispensado. Cobranca de ${formatReais(r.valorCentavos)} lancada no financeiro.`
          : 'Dispensado (sem cobranca).',
      )
    })
  }

  return (
    <Card className="p-5 space-y-5">
      <div className="flex items-center justify-between gap-3">
        <div>
          <h3 className="text-lg font-bold text-slate-900">{medicamento.nome}</h3>
          <p className="text-sm text-slate-500">
            {medicamento.apresentacao} · {medicamento.unidade} · {formatReais(medicamento.precoCentavos)}
          </p>
        </div>
        <div className="text-right">
          <span className="text-2xl font-bold text-slate-900">{saldo}</span>
          <div className="text-xs text-slate-500">em estoque</div>
          {baixoEstoque && <Badge tone="red">Estoque baixo (min {medicamento.estoqueMinimo})</Badge>}
        </div>
      </div>

      {erro && <Alert tone="red">{erro}</Alert>}
      {ok && <Alert tone="green">{ok}</Alert>}

      <div className="grid gap-5 lg:grid-cols-3">
        <form onSubmit={registrarEntrada} className="space-y-2">
          <h4 className="text-sm font-semibold text-slate-700">Entrada de lote</h4>
          <Campo label="Lote" value={entrada.lote} onChange={(v) => setEntrada((s) => ({ ...s, lote: v }))} />
          <Campo label="Validade" type="date" value={entrada.validade} onChange={(v) => setEntrada((s) => ({ ...s, validade: v }))} />
          <Campo label="Quantidade" type="number" value={entrada.quantidade} onChange={(v) => setEntrada((s) => ({ ...s, quantidade: v }))} />
          <Campo label="Localizacao" value={entrada.localizacao} onChange={(v) => setEntrada((s) => ({ ...s, localizacao: v }))} />
          <Button type="submit">Registrar entrada</Button>
        </form>

        <form onSubmit={dispensar} className="space-y-2">
          <h4 className="text-sm font-semibold text-slate-700">Dispensar a paciente</h4>
          <div>
            <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">Paciente</span>
            <SearchSelect
              path="/pacientes/buscar"
              value={disp.paciente_id}
              onChange={(v) => setDisp((s) => ({ ...s, paciente_id: v }))}
              optionLabel={(p) => `${p.nome}${p.documento ? ` · ${p.documento}` : ''}`}
            />
          </div>
          <Campo label="Quantidade" type="number" value={disp.quantidade} onChange={(v) => setDisp((s) => ({ ...s, quantidade: v }))} />
          <Campo label="Motivo" value={disp.motivo} onChange={(v) => setDisp((s) => ({ ...s, motivo: v }))} placeholder="opcional" />
          <Button type="submit" disabled={!disp.paciente_id || !disp.quantidade}>Dispensar</Button>
        </form>

        <form onSubmit={registrarSaida} className="space-y-2">
          <h4 className="text-sm font-semibold text-slate-700">Saida / ajuste</h4>
          <label className="block">
            <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">Tipo</span>
            <select
              className="mt-1 w-full rounded-lg border border-slate-200 px-3 py-2 text-sm"
              value={saida.tipo}
              onChange={(e) => setSaida((s) => ({ ...s, tipo: e.target.value }))}
            >
              <option value="SAIDA">{statusLabel('SAIDA')}</option>
              <option value="AJUSTE">{statusLabel('AJUSTE')}</option>
            </select>
          </label>
          <Campo label="Quantidade" type="number" value={saida.quantidade} onChange={(v) => setSaida((s) => ({ ...s, quantidade: v }))} />
          <Campo label="Motivo" value={saida.motivo} onChange={(v) => setSaida((s) => ({ ...s, motivo: v }))} placeholder="perda, vencimento..." />
          <Button type="submit" variant="secondary" disabled={!saida.quantidade}>Registrar</Button>
        </form>
      </div>

      <div className="grid gap-5 lg:grid-cols-2">
        <div>
          <h4 className="mb-2 text-sm font-semibold text-slate-700">Lotes (FIFO por validade)</h4>
          {lotes == null ? <Spinner /> : lotes.length === 0 ? (
            <p className="text-sm text-slate-400">Sem lotes com saldo.</p>
          ) : (
            <ul className="space-y-1 text-sm">
              {lotes.map((l) => (
                <li key={l.id} className="flex justify-between rounded-lg border border-slate-100 px-3 py-1.5">
                  <span>{l.lote || '(sem lote)'} · val. {l.validade || '—'}</span>
                  <span className="font-semibold">{l.quantidade}</span>
                </li>
              ))}
            </ul>
          )}
        </div>
        <div>
          <h4 className="mb-2 text-sm font-semibold text-slate-700">Movimentacoes</h4>
          {movs == null ? <Spinner /> : movs.length === 0 ? (
            <p className="text-sm text-slate-400">Sem movimentacoes.</p>
          ) : (
            <ul className="space-y-1 text-sm">
              {movs.slice(0, 15).map((m) => (
                <li key={m.id} className="flex items-center justify-between rounded-lg border border-slate-100 px-3 py-1.5">
                  <span className="flex items-center gap-2">
                    <Badge tone={TIPO_TONE[m.tipo] ?? 'slate'}>{statusLabel(m.tipo)}</Badge>
                    <span className="text-slate-500">{m.motivo || '—'}</span>
                  </span>
                  <span className="font-semibold">{m.quantidade}</span>
                </li>
              ))}
            </ul>
          )}
        </div>
      </div>
    </Card>
  )
}

export default function Farmacia() {
  const [medicamentos, setMedicamentos] = useState(null)
  const [alertas, setAlertas] = useState(null)
  const [erro, setErro] = useState('')
  const [selecionadoId, setSelecionadoId] = useState(null)

  const carregar = useCallback(() => {
    Promise.all([apiGet('/medicamentos'), apiGet('/estoque/alertas')])
      .then(([lista, resumo]) => {
        setMedicamentos(lista)
        setAlertas(resumo)
        setErro('')
      })
      .catch((e) => setErro(e.message))
  }, [])

  useEffect(carregar, [carregar])

  const selecionado = (medicamentos || []).find((m) => m.id === selecionadoId) || null
  const saldos = alertas?.saldos || []
  const estoqueBaixo = alertas?.estoqueBaixo || []
  const validadeProxima = alertas?.validadeProxima || []

  return (
    <div className="space-y-6">
      <PageHeader title="Farmácia" subtitle="Estoque, entradas, dispensação e movimentações" />

      {erro && <Alert tone="red">{erro}</Alert>}

      {alertas == null ? (
        <Spinner />
      ) : (
        <>
          <div className="grid gap-4 md:grid-cols-3">
            <StatCard label="Medicamentos com saldo" value={saldos.length} icon={Boxes} tone="teal" />
            <StatCard label="Estoque baixo" value={estoqueBaixo.length} icon={AlertTriangle} tone={estoqueBaixo.length > 0 ? 'red' : 'green'} />
            <StatCard label="Validade em 30 dias" value={validadeProxima.length} icon={CalendarClock} tone={validadeProxima.length > 0 ? 'amber' : 'green'} />
          </div>

          <div className="grid gap-4 lg:grid-cols-2">
            <Card className="p-4">
              <div className="mb-3 flex items-center justify-between gap-3">
                <h2 className="text-sm font-semibold text-slate-800">Estoque baixo</h2>
                <Badge tone={estoqueBaixo.length > 0 ? 'red' : 'green'}>{estoqueBaixo.length}</Badge>
              </div>
              {estoqueBaixo.length === 0 ? (
                <p className="text-sm text-slate-400">Nenhum item abaixo do mínimo.</p>
              ) : (
                <ul className="space-y-2">
                  {estoqueBaixo.slice(0, 6).map((m) => (
                    <li key={m.medicamentoId} className="flex items-center justify-between rounded-lg border border-slate-100 px-3 py-2 text-sm">
                      <span>
                        <span className="font-semibold text-slate-800">{m.nome}</span>
                        <span className="block text-xs text-slate-400">{m.apresentacao}</span>
                      </span>
                      <span className="text-right text-xs text-slate-500">
                        <strong className="block text-sm text-red-700">{m.saldo}</strong>
                        mínimo {m.estoqueMinimo}
                      </span>
                    </li>
                  ))}
                </ul>
              )}
            </Card>

            <Card className="p-4">
              <div className="mb-3 flex items-center justify-between gap-3">
                <h2 className="text-sm font-semibold text-slate-800">Validade próxima</h2>
                <Badge tone={validadeProxima.length > 0 ? 'amber' : 'green'}>{validadeProxima.length}</Badge>
              </div>
              {validadeProxima.length === 0 ? (
                <p className="text-sm text-slate-400">Nenhum lote vence nos próximos 30 dias.</p>
              ) : (
                <ul className="space-y-2">
                  {validadeProxima.slice(0, 6).map((l) => (
                    <li key={l.id} className="flex items-center justify-between rounded-lg border border-slate-100 px-3 py-2 text-sm">
                      <span>
                        <span className="font-semibold text-slate-800">{l.nome}</span>
                        <span className="block text-xs text-slate-400">{l.lote || '(sem lote)'} · {l.localizacao || 'sem localização'}</span>
                      </span>
                      <span className="text-right text-xs text-slate-500">
                        <strong className="block text-sm text-amber-700">{l.validade}</strong>
                        qtd. {l.quantidade}
                      </span>
                    </li>
                  ))}
                </ul>
              )}
            </Card>
          </div>
        </>
      )}

      {medicamentos == null ? (
        <Spinner />
      ) : medicamentos.length === 0 ? (
        <EmptyState
          title="Nenhum medicamento"
          description="Cadastre medicamentos no catálogo para controlar o estoque."
        />
      ) : (
        <div className="grid gap-6 lg:grid-cols-[18rem_1fr]">
          <Card className="p-3">
            <ul className="space-y-1">
              {medicamentos.map((m) => (
                <li key={m.id}>
                  <button
                    type="button"
                    onClick={() => setSelecionadoId(m.id)}
                    className={`block w-full rounded-lg px-3 py-2 text-left text-sm ${
                      m.id === selecionadoId ? 'bg-teal-50 font-semibold text-teal-800' : 'hover:bg-slate-50'
                    }`}
                  >
                    {m.nome}
                    <span className="block text-xs text-slate-400">{m.apresentacao}</span>
                  </button>
                </li>
              ))}
            </ul>
          </Card>
          {selecionado ? (
            <Detalhe medicamento={selecionado} onMudou={carregar} />
          ) : (
            <Card className="p-8 text-center text-sm text-slate-400">
              Selecione um medicamento para ver o estoque.
            </Card>
          )}
        </div>
      )}
    </div>
  )
}
