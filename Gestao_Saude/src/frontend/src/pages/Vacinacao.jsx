import { useCallback, useEffect, useMemo, useState } from 'react'
import { Link } from 'react-router-dom'
import { ClipboardCheck, Syringe } from 'lucide-react'
import { apiGet, apiSend } from '../api/client'
import DataTable from '../components/DataTable'
import { SearchSelect } from '../components/FieldSelect'
import { PageHeader, Card, Button, Alert, Spinner, StatCard } from '../components/ui'

const COLUNAS_VACINAS = [
  { key: 'id', label: 'ID' },
  { key: 'nome', label: 'Nome' },
  { key: 'fabricante', label: 'Fabricante' },
  { key: 'doencasAlvo', label: 'Doencas alvo' },
  { key: 'dosesPrevistas', label: 'Doses' },
  { key: 'medicamentoId', label: 'Estoque' },
]

const COLUNAS_APLICACOES = [
  { key: 'id', label: 'ID' },
  { key: 'pacienteNome', label: 'Paciente' },
  { key: 'vacinaNome', label: 'Vacina' },
  { key: 'doseNumero', label: 'Dose' },
  { key: 'lote', label: 'Lote' },
  { key: 'validade', label: 'Validade' },
  { key: 'aplicadorLogin', label: 'Aplicador' },
  { key: 'aplicadaEm', label: 'Aplicada em' },
]

const CAMPO =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm disabled:bg-slate-100 disabled:text-slate-400'

const FORM_INICIAL = {
  paciente_id: '',
  vacina_id: '',
  dose_numero: '1',
  lote: '',
  validade: '',
  observacao: '',
}

function pacienteLabel(p) {
  return `${p.nome}${p.documento ? ` - ${p.documento}` : ''}`
}

function loteValue(lote) {
  return `${lote.lote}||${lote.validade}`
}

export default function Vacinacao() {
  const [vacinas, setVacinas] = useState(null)
  const [aplicacoes, setAplicacoes] = useState(null)
  const [lotes, setLotes] = useState([])
  const [form, setForm] = useState(FORM_INICIAL)
  const [erro, setErro] = useState('')
  const [sucesso, setSucesso] = useState('')
  const [salvando, setSalvando] = useState(false)

  const carregar = useCallback(() => {
    Promise.all([apiGet('/vacinas'), apiGet('/aplicacoes-vacinas')])
      .then(([listaVacinas, listaAplicacoes]) => {
        setVacinas(listaVacinas)
        setAplicacoes(listaAplicacoes)
        setErro('')
      })
      .catch((e) => setErro(e.message))
  }, [])

  useEffect(carregar, [carregar])

  const vacinaSelecionada = useMemo(
    () => (vacinas || []).find((v) => String(v.id) === String(form.vacina_id)) || null,
    [form.vacina_id, vacinas],
  )

  useEffect(() => {
    let vivo = true
    const medicamentoId = vacinaSelecionada?.medicamentoId
    if (!medicamentoId) {
      return () => { vivo = false }
    }
    apiGet(`/medicamentos/${medicamentoId}/estoque`)
      .then((rows) => { if (vivo) setLotes(Array.isArray(rows) ? rows : []) })
      .catch((e) => { if (vivo) { setLotes([]); setErro(e.message) } })
    return () => { vivo = false }
  }, [vacinaSelecionada])

  function alterar(campo, valor) {
    setSucesso('')
    setErro('')
    if (campo === 'vacina_id') {
      setLotes([])
    }
    setForm((atual) => {
      if (campo === 'vacina_id') {
        return { ...atual, vacina_id: valor, lote: '', validade: '' }
      }
      return { ...atual, [campo]: valor }
    })
  }

  function escolherLote(valor) {
    const [lote, validade] = valor.split('||')
    setForm((atual) => ({ ...atual, lote: lote || '', validade: validade || '' }))
  }

  async function aplicar(e) {
    e.preventDefault()
    setSalvando(true)
    setErro('')
    setSucesso('')
    try {
      await apiSend('POST', '/aplicacoes-vacinas', form)
      setForm(FORM_INICIAL)
      setLotes([])
      setSucesso('Vacina aplicada e estoque atualizado.')
      carregar()
    } catch (err) {
      setErro(err.message)
    } finally {
      setSalvando(false)
    }
  }

  const comEstoque = (vacinas || []).filter((v) => Number(v.medicamentoId || 0) > 0).length

  return (
    <div className="space-y-6">
      <PageHeader
        title="Vacinação"
        subtitle="Aplicação de vacinas com lote, validade e baixa de estoque"
        actions={<Button as={Link} to="/r/vacinas">Gerenciar catálogo</Button>}
      />

      {erro && <Alert tone="red">{erro}</Alert>}
      {sucesso && <Alert tone="teal">{sucesso}</Alert>}

      {vacinas == null || aplicacoes == null ? (
        <Spinner />
      ) : (
        <>
          <div className="grid gap-4 md:grid-cols-3">
            <StatCard label="Vacinas ativas" value={vacinas.length} icon={Syringe} tone="teal" />
            <StatCard label="Com estoque" value={comEstoque} tone="sky" />
            <StatCard label="Aplicações" value={aplicacoes.length} icon={ClipboardCheck} tone="green" />
          </div>

          <Card className="p-5">
            <form onSubmit={aplicar} className="grid gap-4 lg:grid-cols-6">
              <label className="lg:col-span-2">
                <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">Paciente</span>
                <SearchSelect
                  path="/pacientes/buscar"
                  value={form.paciente_id}
                  onChange={(v) => alterar('paciente_id', v)}
                  optionLabel={pacienteLabel}
                />
              </label>

              <label className="lg:col-span-2">
                <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">Vacina</span>
                <select
                  value={form.vacina_id}
                  onChange={(e) => alterar('vacina_id', e.target.value)}
                  className={CAMPO}
                  required
                >
                  <option value="">Selecione...</option>
                  {vacinas.map((v) => (
                    <option key={v.id} value={v.id}>
                      {v.nome}{v.medicamentoId ? '' : ' - sem estoque'}
                    </option>
                  ))}
                </select>
              </label>

              <label>
                <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">Dose</span>
                <input
                  type="number"
                  min="1"
                  value={form.dose_numero}
                  onChange={(e) => alterar('dose_numero', e.target.value)}
                  className={CAMPO}
                  required
                />
              </label>

              <label>
                <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">Lote</span>
                <select
                  value={form.lote && form.validade ? `${form.lote}||${form.validade}` : ''}
                  onChange={(e) => escolherLote(e.target.value)}
                  className={CAMPO}
                  disabled={!vacinaSelecionada?.medicamentoId}
                  required
                >
                  <option value="">Selecione...</option>
                  {lotes.map((lote) => (
                    <option key={lote.id} value={loteValue(lote)}>
                      {lote.lote} - {lote.validade} ({lote.quantidade})
                    </option>
                  ))}
                </select>
              </label>

              <label className="lg:col-span-5">
                <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">Observação</span>
                <input
                  value={form.observacao}
                  onChange={(e) => alterar('observacao', e.target.value)}
                  className={CAMPO}
                />
              </label>

              <div className="flex items-end">
                <Button type="submit" disabled={salvando || !form.paciente_id || !form.vacina_id || !form.lote}>
                  {salvando ? 'Aplicando...' : 'Aplicar'}
                </Button>
              </div>
            </form>
          </Card>

          <Card className="p-4">
            <h2 className="mb-3 text-sm font-semibold text-slate-700">Aplicações registradas</h2>
            {aplicacoes.length === 0 ? (
              <p className="px-2 py-6 text-sm text-slate-500">Nenhuma aplicação registrada.</p>
            ) : (
              <DataTable columns={COLUNAS_APLICACOES} rows={aplicacoes} />
            )}
          </Card>

          <Card className="p-4">
            <h2 className="mb-3 text-sm font-semibold text-slate-700">Catálogo</h2>
            {vacinas.length === 0 ? (
              <p className="px-2 py-6 text-sm text-slate-500">Nenhuma vacina cadastrada.</p>
            ) : (
              <DataTable columns={COLUNAS_VACINAS} rows={vacinas} />
            )}
          </Card>
        </>
      )}
    </div>
  )
}
