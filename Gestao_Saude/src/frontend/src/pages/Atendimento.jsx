import { useCallback, useEffect, useState } from 'react'
import { apiSend, listarFilaConsulta, listarMeusAtendimentos, assumirCheckin } from '../api/client'
import { PageHeader, Card, Button, Badge, Spinner, EmptyState, Alert } from '../components/ui'
import { ICONS, Icon } from '../components/icons'

const CLASSIF_TONE = {
  Vermelho: 'red',
  Laranja: 'amber',
  Amarelo: 'amber',
  Verde: 'green',
  Azul: 'sky',
}

function PacienteCard({ c, acao, rotulo, variante, enviandoId }) {
  return (
    <Card className="p-4">
      <div className="flex items-start justify-between gap-3">
        <div>
          <p className="text-sm font-semibold text-slate-900 dark:text-white">
            Senha {c.senha} · {c.pacienteNome || `Paciente #${c.pacienteId}`}
          </p>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">Espera: {c.esperaMinutos} min</p>
        </div>
        <div className="flex flex-col items-end gap-1">
          {c.classificacao && <Badge tone={CLASSIF_TONE[c.classificacao] || 'slate'}>{c.classificacao}</Badge>}
          {c.prioridade >= 4 && <Badge tone="red">prioridade {c.prioridade}</Badge>}
        </div>
      </div>
      <Button
        variant={variante}
        className="mt-4 w-full"
        disabled={enviandoId === c.id}
        onClick={() => acao(c)}
      >
        {enviandoId === c.id ? 'Processando...' : rotulo}
      </Button>
    </Card>
  )
}

export default function Atendimento() {
  const [fila, setFila] = useState(null)
  const [atendimentos, setAtendimentos] = useState(null)
  const [erro, setErro] = useState('')
  const [enviandoId, setEnviandoId] = useState(0)

  const carregar = useCallback(() => {
    listarFilaConsulta()
      .then(setFila)
      .catch((e) => setErro(e.message || 'Falha ao carregar a fila.'))
    listarMeusAtendimentos()
      .then(setAtendimentos)
      .catch((e) => setErro(e.message || 'Falha ao carregar atendimentos.'))
  }, [])

  useEffect(() => { carregar() }, [carregar])

  async function assumir(c) {
    if (enviandoId) return
    setEnviandoId(c.id)
    setErro('')
    try {
      await assumirCheckin(c.id)
      carregar()
    } catch (e) {
      setErro(e.message || 'Falha ao assumir o paciente.')
    } finally {
      setEnviandoId(0)
    }
  }

  async function encerrar(c) {
    if (enviandoId) return
    setEnviandoId(c.id)
    setErro('')
    try {
      await apiSend('POST', `/checkins/${c.id}/encerrar`)
      carregar()
    } catch (e) {
      setErro(e.message || 'Falha ao encerrar o atendimento.')
    } finally {
      setEnviandoId(0)
    }
  }

  return (
    <div className="space-y-6">
      <PageHeader
        title="Atendimento"
        subtitle="Assuma o próximo paciente da fila de consulta e acompanhe quem você deve atender."
      />

      {erro && <Alert>{erro}</Alert>}

      <section className="space-y-3">
        <h2 className="flex items-center gap-2 text-sm font-semibold text-slate-700 dark:text-slate-200">
          <Icon icon={ICONS.reception} className="text-teal-600" size={18} />
          Próximos a atender (fila de consulta)
        </h2>
        {fila === null && <Spinner />}
        {Array.isArray(fila) && fila.length === 0 && (
          <EmptyState icon={ICONS.reception} title="Fila vazia" description="Nenhum paciente aguardando consulta." />
        )}
        {Array.isArray(fila) && fila.length > 0 && (
          <div className="grid gap-3 md:grid-cols-2">
            {fila.map((c) => (
              <PacienteCard key={c.id} c={c} acao={assumir} rotulo="Assumir paciente" variante="primary" enviandoId={enviandoId} />
            ))}
          </div>
        )}
      </section>

      <section className="space-y-3">
        <h2 className="flex items-center gap-2 text-sm font-semibold text-slate-700 dark:text-slate-200">
          <Icon icon={ICONS.doctor} className="text-teal-600" size={18} />
          Meus atendimentos em andamento
        </h2>
        {atendimentos === null && <Spinner />}
        {Array.isArray(atendimentos) && atendimentos.length === 0 && (
          <EmptyState icon={ICONS.doctor} title="Nenhum atendimento" description="Assuma um paciente da fila para começar." />
        )}
        {Array.isArray(atendimentos) && atendimentos.length > 0 && (
          <div className="grid gap-3 md:grid-cols-2">
            {atendimentos.map((c) => (
              <PacienteCard key={c.id} c={c} acao={encerrar} rotulo="Encerrar atendimento" variante="danger" enviandoId={enviandoId} />
            ))}
          </div>
        )}
      </section>
    </div>
  )
}
