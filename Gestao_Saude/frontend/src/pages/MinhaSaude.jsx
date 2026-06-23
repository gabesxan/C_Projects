import { useEffect, useState } from 'react'
import { Link, useLocation } from 'react-router-dom'
import { apiGet, apiSend } from '../api/client'
import { formatReais } from '../money'
import { PageHeader, Spinner, Card, Badge, Button, EmptyState, StatCard } from '../components/ui'
import { ICONS, Icon } from '../components/icons'
import ConsentimentoWallet from '../components/ConsentimentoWallet'

const STATUS_COBRANCA_TONE = {
  PENDENTE: 'amber',
  AUTORIZADA: 'sky',
  PAGA: 'green',
  GLOSADA: 'red',
  CANCELADA: 'slate',
}

// Abas do portal. A chave casa com o segmento da URL (/minha-saude/<chave>),
// para que a navegacao lateral e as abas internas fiquem sempre em sincronia.
const TABS = [
  { key: '', label: 'Visão geral', icon: ICONS.health },
  { key: 'consultas', label: 'Consultas', icon: ICONS.schedule },
  { key: 'exames', label: 'Exames', icon: ICONS.lab },
  { key: 'receitas', label: 'Receitas', icon: ICONS.prescription },
  { key: 'vacinas', label: 'Vacinas', icon: ICONS.vaccine },
  { key: 'prontuarios', label: 'Prontuários', icon: ICONS.record },
  { key: 'financeiro', label: 'Financeiro', icon: ICONS.billing },
  { key: 'solicitacoes', label: 'Agendar', icon: ICONS.schedule },
  { key: 'privacidade', label: 'Privacidade', icon: ICONS.audit },
]

function useApi(path) {
  const [data, setData] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet(path).then(setData).catch((e) => setErro(e.message))
  }, [path])

  return { data, erro }
}

function Info({ label, value }) {
  return (
    <div className="rounded-lg bg-slate-50 p-3 dark:bg-slate-800">
      <p className="text-xs text-slate-500 dark:text-slate-400">{label}</p>
      <p className="mt-1 text-sm font-semibold text-slate-900 dark:text-white">{value}</p>
    </div>
  )
}

function Carteirinha({ perfil }) {
  return (
    <Card className="overflow-hidden">
      <div className="flex items-center justify-between bg-teal-700 px-5 py-4 text-white dark:bg-teal-800">
        <div>
          <p className="text-xs font-semibold uppercase text-teal-100">Carteirinha digital</p>
          <p className="mt-1 text-2xl font-bold">{perfil.nome}</p>
          <p className="text-sm text-teal-100">Paciente #{perfil.id}</p>
        </div>
        <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-white/15 ring-1 ring-white/20">
          <Icon icon={ICONS.hospital} size={24} />
        </div>
      </div>
      <div className="grid gap-3 p-5 sm:grid-cols-2 lg:grid-cols-4">
        <Info label="Documento" value={`${perfil.tipoDocumento} ${perfil.documento}`} />
        <Info label="Idade" value={`${perfil.idade} anos`} />
        <Info label="Regiao" value={`RA ${perfil.regiaoAdministrativa}`} />
        <Info label="Status" value={perfil.ativo ? 'Ativo' : 'Inativo'} />
        <Info label="Convenio" value={perfil.convenioId ? `Convenio #${perfil.convenioId}` : 'Particular'} />
        <Info label="Telefone" value={perfil.telefone} />
        <Info label="Alergias" value={perfil.alergias || 'Sem alergias registradas'} />
      </div>
    </Card>
  )
}

function ActionCard({ icon, title, text, button, primary, onClick, disabled }) {
  return (
    <Card className={`flex h-full flex-col p-5 ${disabled ? 'opacity-70' : ''}`}>
      <div className="flex h-10 w-10 items-center justify-center rounded-2xl bg-slate-100 text-teal-700 dark:bg-slate-800 dark:text-teal-300">
        <Icon icon={icon} size={20} />
      </div>
      <p className="mt-4 text-sm font-semibold text-slate-800 dark:text-slate-100">{title}</p>
      <p className="mt-1 flex-1 text-sm text-slate-500 dark:text-slate-400">{text}</p>
      <Button className="mt-4 w-full" variant={primary ? 'primary' : 'secondary'} onClick={onClick} disabled={disabled}>
        {button}
      </Button>
    </Card>
  )
}

function ListaCards({ titulo, icon, rows, erro, render, vazio = 'Nenhum registro.' }) {
  return (
    <section className="space-y-3">
      {titulo && (
        <h2 className="flex items-center gap-2 text-sm font-semibold text-slate-700 dark:text-slate-200">
          <Icon icon={icon} className="text-teal-600" size={18} />
          {titulo}
        </h2>
      )}
      {erro && <p className="text-sm text-red-600">{erro}</p>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <EmptyState icon={icon} title={vazio} description="Quando houver novidades, elas aparecem aqui." />
      )}
      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <div className="grid gap-3 md:grid-cols-2">{rows.map(render)}</div>
      )}
    </section>
  )
}

// Tabela de analitos de um exame concluido (valor, unidade, faixa e flag).
function ResultadosAnalito({ exameId }) {
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet(`/me/exames/${exameId}/resultados`).then(setRows).catch((e) => setErro(e.message))
  }, [exameId])

  if (erro) return <p className="mt-3 text-xs text-red-600">{erro}</p>
  if (rows === null) return <p className="mt-3 text-xs text-slate-400">Carregando resultados...</p>
  if (rows.length === 0) return <p className="mt-3 text-xs text-slate-400">Sem resultados estruturados para este exame.</p>

  return (
    <div className="mt-3 overflow-hidden rounded-lg border border-slate-200 dark:border-slate-700">
      <table className="w-full text-left text-xs">
        <thead className="bg-slate-50 text-slate-500 dark:bg-slate-800 dark:text-slate-400">
          <tr>
            <th className="px-3 py-2 font-medium">Analito</th>
            <th className="px-3 py-2 font-medium">Resultado</th>
            <th className="px-3 py-2 font-medium">Referência</th>
            <th className="px-3 py-2 font-medium">Situação</th>
          </tr>
        </thead>
        <tbody className="divide-y divide-slate-100 dark:divide-slate-800">
          {rows.map((r) => {
            const valor = r.valorTexto || `${r.valor}${r.unidade ? ` ${r.unidade}` : ''}`
            const faixa = r.refMin || r.refMax ? `${r.refMin} – ${r.refMax}${r.unidade ? ` ${r.unidade}` : ''}` : '—'
            return (
              <tr key={r.analitoId} className="text-slate-700 dark:text-slate-200">
                <td className="px-3 py-2 font-medium text-slate-900 dark:text-white">{r.nome}</td>
                <td className={`px-3 py-2 font-semibold ${r.foraReferencia ? 'text-red-600 dark:text-red-400' : ''}`}>{valor}</td>
                <td className="px-3 py-2 text-slate-500 dark:text-slate-400">{faixa}</td>
                <td className="px-3 py-2">
                  <Badge tone={r.foraReferencia ? 'red' : 'green'}>{r.foraReferencia ? 'Fora da faixa' : 'Normal'}</Badge>
                </td>
              </tr>
            )
          })}
        </tbody>
      </table>
    </div>
  )
}

function ExameCard({ exame }) {
  const [aberto, setAberto] = useState(false)
  const concluido = exame.status === 'CONCLUIDO'
  return (
    <Card className="p-4">
      <div className="flex items-center justify-between gap-3">
        <p className="text-sm font-semibold text-slate-900 dark:text-white">Exame #{exame.id} · Tipo {exame.tipoExame}</p>
        <Badge tone={concluido ? 'green' : 'amber'} icon={ICONS.lab}>{exame.status}</Badge>
      </div>
      {exame.resultado && <p className="mt-2 text-sm text-slate-500 dark:text-slate-400">{exame.resultado}</p>}
      {concluido ? (
        <>
          <button
            type="button"
            className="mt-2 text-xs font-semibold text-teal-700 hover:underline dark:text-teal-300"
            onClick={() => setAberto((v) => !v)}
          >
            {aberto ? 'Ocultar detalhamento' : 'Ver detalhamento por analito'}
          </button>
          {aberto && <ResultadosAnalito exameId={exame.id} />}
        </>
      ) : (
        <p className="mt-2 text-xs text-slate-400">Resultado ainda não disponível.</p>
      )}
    </Card>
  )
}

function TabBar({ active }) {
  return (
    <div className="flex gap-1 overflow-x-auto rounded-xl bg-slate-100 p-1 dark:bg-slate-800">
      {TABS.map((t) => {
        const ativo = t.key === active
        return (
          <Link
            key={t.key}
            to={`/minha-saude${t.key ? `/${t.key}` : ''}`}
            className={`flex shrink-0 items-center gap-1.5 rounded-lg px-3 py-2 text-sm font-medium transition ${
              ativo
                ? 'bg-white text-teal-700 shadow-sm dark:bg-slate-900 dark:text-teal-300'
                : 'text-slate-500 hover:text-slate-800 dark:text-slate-400 dark:hover:text-slate-200'
            }`}
          >
            <Icon icon={t.icon} size={16} />
            {t.label}
          </Link>
        )
      })}
    </div>
  )
}

function VisaoGeral({ perfil, agendamentos, exames, cobrancas }) {
  const proxima = Array.isArray(agendamentos)
    ? agendamentos.find((a) => a.status !== 'CANCELADO' && a.status !== 'REALIZADO')
    : null
  const examesConcluidos = Array.isArray(exames) ? exames.filter((e) => e.status === 'CONCLUIDO').length : 0
  const pendentes = Array.isArray(cobrancas) ? cobrancas.filter((c) => c.status === 'PENDENTE' || c.status === 'AUTORIZADA') : []
  const totalPendente = pendentes.reduce((soma, c) => soma + (c.valorCentavos || 0), 0)

  return (
    <div className="space-y-5">
      <Carteirinha perfil={perfil} />
      <div className="grid gap-3 sm:grid-cols-3">
        <StatCard
          icon={ICONS.schedule}
          label="Próxima consulta"
          value={proxima ? `${proxima.data}` : '—'}
          hint={proxima ? `${proxima.horario} · ${proxima.medicoNome || `Médico #${proxima.medicoId}`}` : 'Nenhuma agendada'}
        />
        <StatCard icon={ICONS.lab} label="Exames concluídos" value={examesConcluidos} hint="Com resultado disponível" tone="sky" />
        <StatCard
          icon={ICONS.billing}
          label="Pendências"
          value={formatReais(totalPendente)}
          hint={`${pendentes.length} cobrança(s) em aberto`}
          tone={totalPendente > 0 ? 'amber' : 'teal'}
        />
      </div>
    </div>
  )
}

function Consultas({ rows, erro }) {
  return (
    <ListaCards
      titulo="Minhas consultas"
      icon={ICONS.schedule}
      rows={rows}
      erro={erro}
      vazio="Nenhuma consulta agendada."
      render={(a) => (
        <Card key={a.id} className="p-4">
          <div className="flex items-center justify-between gap-3">
            <p className="text-sm font-semibold text-slate-900 dark:text-white">{a.data} às {a.horario}</p>
            <Badge tone="sky" icon={ICONS.schedule}>{a.status}</Badge>
          </div>
          <p className="mt-1 text-sm text-slate-600 dark:text-slate-300">{a.medicoNome || `Médico #${a.medicoId}`}</p>
          {a.especialidade && <p className="text-xs text-slate-400">{a.especialidade}</p>}
          {a.motivoCancelamento && <p className="mt-2 text-xs text-red-500">Motivo: {a.motivoCancelamento}</p>}
        </Card>
      )}
    />
  )
}

function Solicitacoes({ rows, erro, especialidades, agenda, horarios, acoes }) {
  return (
    <div className="space-y-5">
      <div className="grid gap-3 md:grid-cols-2">
        <Card className="p-5">
          <div className="flex items-center gap-2">
            <Icon icon={ICONS.schedule} className="text-teal-700" size={20} />
            <p className="text-sm font-semibold text-slate-800 dark:text-slate-100">Agendar consulta</p>
          </div>
          <div className="mt-4 grid gap-3">
            <label className="text-sm text-slate-600 dark:text-slate-300">
              Especialidade
              <select
                className="mt-1 block w-full rounded-lg border border-slate-300 px-3 py-2 text-sm dark:border-slate-700 dark:bg-slate-900"
                value={agenda.especialidade}
                onChange={(e) => acoes.setAgenda({ ...agenda, especialidade: e.target.value, horario: '' })}
              >
                <option value="">Selecione</option>
                {(especialidades || []).map((e) => (
                  <option key={e.especialidade} value={e.especialidade}>
                    {e.especialidade}
                  </option>
                ))}
              </select>
            </label>
            <label className="text-sm text-slate-600 dark:text-slate-300">
              Data
              <input
                className="mt-1 block w-full rounded-lg border border-slate-300 px-3 py-2 text-sm dark:border-slate-700 dark:bg-slate-900"
                type="date"
                value={agenda.data}
                onChange={(e) => acoes.setAgenda({ ...agenda, data: e.target.value, horario: '' })}
              />
            </label>
            <Button
              variant="secondary"
              onClick={acoes.buscarHorarios}
              disabled={!agenda.especialidade || !agenda.data || acoes.enviando === 'HORARIOS'}
            >
              {acoes.enviando === 'HORARIOS' ? 'Buscando...' : 'Ver horários disponíveis'}
            </Button>
            {horarios === null ? null : horarios.length === 0 ? (
              <p className="text-sm text-slate-500">Nenhum horário disponível para essa data.</p>
            ) : (
              <label className="text-sm text-slate-600 dark:text-slate-300">
                Horário
                <select
                  className="mt-1 block w-full rounded-lg border border-slate-300 px-3 py-2 text-sm dark:border-slate-700 dark:bg-slate-900"
                  value={agenda.horario}
                  onChange={(e) => acoes.setAgenda({ ...agenda, horario: e.target.value })}
                >
                  <option value="">Selecione</option>
                  {horarios.map((h) => (
                    <option key={h} value={h}>{h}</option>
                  ))}
                </select>
              </label>
            )}
            <Button
              onClick={acoes.confirmarAgendamento}
              disabled={!agenda.especialidade || !agenda.data || !agenda.horario || acoes.enviando === 'AGENDAMENTO'}
            >
              {acoes.enviando === 'AGENDAMENTO' ? 'Confirmando...' : 'Confirmar agendamento'}
            </Button>
          </div>
        </Card>
        <ActionCard
          icon={ICONS.alert}
          title="Pedir atendimento"
          text="Acione a equipe para ajuda de fluxo e orientação. Isso não cria triagem clínica."
          button={acoes.enviando === 'AJUDA' ? 'Enviando...' : 'Pedir ajuda'}
          primary
          onClick={() => acoes.criar('AJUDA')}
          disabled={Boolean(acoes.enviando)}
        />
      </div>
      {acoes.erroAcao && <p className="text-sm text-red-600">{acoes.erroAcao}</p>}
      {acoes.okAcao && <p className="text-sm text-teal-700">{acoes.okAcao}</p>}
      <ListaCards
        titulo="Histórico de solicitações"
        icon={ICONS.alert}
        rows={rows}
        erro={erro}
        vazio="Nenhuma solicitação enviada."
        render={(s) => (
          <Card key={s.id} className="p-4">
            <div className="flex items-center justify-between gap-3">
              <p className="text-sm font-semibold text-slate-900 dark:text-white">Protocolo #{s.id} · {s.tipo}</p>
              <Badge tone={s.status === 'ABERTA' ? 'amber' : 'green'}>{s.status}</Badge>
            </div>
            {s.mensagem && <p className="mt-1 text-sm text-slate-500 dark:text-slate-400">{s.mensagem}</p>}
            <p className="mt-1 text-xs text-slate-400">{s.criadoEm}</p>
          </Card>
        )}
      />
    </div>
  )
}

export default function MinhaSaude() {
  const location = useLocation()
  const active = location.pathname.split('/')[2] || ''
  const [erroAcao, setErroAcao] = useState('')
  const [okAcao, setOkAcao] = useState('')
  const [enviando, setEnviando] = useState('')
  const [recarregar, setRecarregar] = useState(0)
  const [agenda, setAgenda] = useState({ especialidade: '', data: '', horario: '' })
  const [horarios, setHorarios] = useState(null)

  const perfil = useApi('/me/perfil')
  const agendamentos = useApi(`/me/agendamentos?v=${recarregar}`)
  const receitas = useApi('/me/receitas')
  const vacinas = useApi('/me/vacinas')
  const exames = useApi('/me/exames')
  const prontuarios = useApi('/me/prontuarios')
  const cobrancas = useApi('/me/cobrancas')
  const solicitacoes = useApi(`/me/solicitacoes?v=${recarregar}`)
  const especialidades = useApi('/me/agendamentos/especialidades')

  async function buscarHorarios() {
    setEnviando('HORARIOS')
    setErroAcao('')
    setOkAcao('')
    try {
      const params = new URLSearchParams({ especialidade: agenda.especialidade, data: agenda.data })
      const rows = await apiGet(`/me/agendamentos/disponibilidade?${params.toString()}`)
      setHorarios(rows)
      setAgenda((v) => ({ ...v, horario: '' }))
    } catch (e) {
      setErroAcao(e.message || 'Falha ao buscar horários.')
    } finally {
      setEnviando('')
    }
  }

  async function confirmarAgendamento() {
    setEnviando('AGENDAMENTO')
    setErroAcao('')
    setOkAcao('')
    try {
      await apiSend('POST', '/me/agendamentos', agenda)
      setOkAcao('Consulta agendada. A recepção já consegue visualizar esse agendamento.')
      setAgenda({ especialidade: '', data: '', horario: '' })
      setHorarios(null)
      setRecarregar((v) => v + 1)
    } catch (e) {
      setErroAcao(e.message || 'Falha ao confirmar agendamento.')
    } finally {
      setEnviando('')
    }
  }

  async function criarSolicitacao(tipo) {
    const mensagem =
      tipo === 'AGENDAMENTO'
        ? 'Solicitação de consulta comum pelo portal do paciente.'
        : 'Pedido de ajuda/atendimento pelo portal do paciente.'
    setEnviando(tipo)
    setErroAcao('')
    setOkAcao('')
    try {
      await apiSend('POST', '/me/solicitacoes', { tipo, mensagem })
      setOkAcao('Solicitação enviada para a equipe.')
      setRecarregar((v) => v + 1)
    } catch (e) {
      setErroAcao(e.message || 'Falha de comunicação com o backend.')
    } finally {
      setEnviando('')
    }
  }

  if (perfil.erro) return <p className="text-sm text-red-600">{perfil.erro}</p>
  if (!perfil.data) return <Spinner />

  return (
    <div className="mx-auto max-w-5xl space-y-5">
      <PageHeader
        title="Minha saúde"
        subtitle="Carteirinha, consultas, exames, receitas, vacinas e orientações. A triagem clínica é feita pela equipe autorizada."
      />
      <TabBar active={active} />

      {active === '' && (
        <VisaoGeral perfil={perfil.data} agendamentos={agendamentos.data} exames={exames.data} cobrancas={cobrancas.data} />
      )}

      {active === 'consultas' && <Consultas rows={agendamentos.data} erro={agendamentos.erro} />}

      {active === 'exames' && (
        <ListaCards
          titulo="Meus exames e resultados"
          icon={ICONS.lab}
          rows={exames.data}
          erro={exames.erro}
          vazio="Nenhum exame pendente."
          render={(e) => <ExameCard key={e.id} exame={e} />}
        />
      )}

      {active === 'receitas' && (
        <ListaCards
          titulo="Minhas receitas"
          icon={ICONS.prescription}
          rows={receitas.data}
          erro={receitas.erro}
          vazio="Nenhuma receita ativa."
          render={(r) => (
            <Card key={r.id} className="p-4">
              <p className="text-sm font-semibold text-slate-900 dark:text-white">{r.medicamento}</p>
              <p className="mt-1 text-sm text-slate-500 dark:text-slate-400">{r.dosagem} · {r.frequencia}</p>
              {r.observacoes && <p className="mt-2 text-xs text-slate-500 dark:text-slate-400">{r.observacoes}</p>}
            </Card>
          )}
        />
      )}

      {active === 'vacinas' && (
        <ListaCards
          titulo="Carteira vacinal"
          icon={ICONS.vaccine}
          rows={vacinas.data}
          erro={vacinas.erro}
          vazio="Nenhuma vacina aplicada."
          render={(v) => (
            <Card key={v.id} className="p-4">
              <div className="flex items-start justify-between gap-3">
                <div>
                  <p className="text-sm font-semibold text-slate-900 dark:text-white">{v.vacinaNome}</p>
                  <p className="mt-1 text-sm text-slate-500 dark:text-slate-400">
                    Dose {v.doseNumero} · lote {v.lote}
                  </p>
                  <p className="mt-1 text-xs text-slate-400">
                    Validade {v.validade} · aplicada em {v.aplicadaEm}
                  </p>
                </div>
                <Badge tone="green" icon={ICONS.vaccine}>Aplicada</Badge>
              </div>
              {v.observacao && <p className="mt-3 text-xs text-slate-500 dark:text-slate-400">{v.observacao}</p>}
            </Card>
          )}
        />
      )}

      {active === 'prontuarios' && (
        <ListaCards
          titulo="Orientações e prontuários"
          icon={ICONS.record}
          rows={prontuarios.data}
          erro={prontuarios.erro}
          vazio="Nenhuma orientação registrada."
          render={(p) => (
            <Card key={p.id} className="p-4">
              <p className="text-sm font-semibold text-slate-900 dark:text-white">{p.data}</p>
              <p className="mt-1 text-sm text-slate-500 dark:text-slate-400">{p.conduta || p.diagnostico}</p>
            </Card>
          )}
        />
      )}

      {active === 'financeiro' && (
        <ListaCards
          titulo="Minhas cobranças"
          icon={ICONS.billing}
          rows={cobrancas.data}
          erro={cobrancas.erro}
          vazio="Nenhuma cobrança pendente."
          render={(c) => (
            <Card key={c.id} className="p-4">
              <div className="flex items-start justify-between gap-3">
                <div>
                  <p className="text-sm font-semibold text-slate-900 dark:text-white">{c.descricao || c.origem || `Cobrança #${c.id}`}</p>
                  <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">{c.forma}{c.vencimento ? ` · vence ${c.vencimento}` : ''}</p>
                </div>
                <Badge tone={STATUS_COBRANCA_TONE[c.status]} icon={ICONS.billing}>{c.status}</Badge>
              </div>
              <p className="mt-3 text-lg font-bold text-slate-900 dark:text-white">{formatReais(c.valorCentavos)}</p>
            </Card>
          )}
        />
      )}

      {active === 'solicitacoes' && (
        <Solicitacoes
          rows={solicitacoes.data}
          erro={solicitacoes.erro}
          especialidades={especialidades.data}
          agenda={agenda}
          horarios={horarios}
          acoes={{
            enviando,
            erroAcao,
            okAcao,
            criar: criarSolicitacao,
            setAgenda,
            buscarHorarios,
            confirmarAgendamento,
          }}
        />
      )}

      {active === 'privacidade' && <ConsentimentoWallet />}
    </div>
  )
}
