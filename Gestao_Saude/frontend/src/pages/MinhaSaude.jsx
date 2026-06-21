import { useEffect, useState } from 'react'
import { useLocation } from 'react-router-dom'
import { apiGet, apiSend } from '../api/client'
import { formatReais } from '../money'
import { PageHeader, Spinner, Card, Badge, Button, EmptyState } from '../components/ui'
import { ICONS, Icon } from '../components/icons'

const STATUS_COBRANCA_TONE = {
  PENDENTE: 'amber',
  AUTORIZADA: 'sky',
  PAGA: 'green',
  GLOSADA: 'red',
  CANCELADA: 'slate',
}

function useApi(path) {
  const [data, setData] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet(path).then(setData).catch((e) => setErro(e.message))
  }, [path])

  return { data, erro }
}

function Carteirinha({ perfil }) {
  return (
    <Card className="overflow-hidden">
      <div className="bg-teal-700 px-5 py-4 text-white dark:bg-teal-800">
        <div className="flex flex-col items-center justify-center gap-3 text-center">
          <div>
            <p className="text-xs font-semibold uppercase text-teal-100">Carteirinha digital</p>
            <p className="mt-1 text-2xl font-bold">{perfil.nome}</p>
            <p className="text-sm text-teal-100">Paciente #{perfil.id}</p>
          </div>
          <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-white/15 ring-1 ring-white/20">
            <Icon icon={ICONS.hospital} size={24} />
          </div>
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

function Info({ label, value }) {
  return (
    <div className="rounded-lg bg-slate-50 p-3 text-center dark:bg-slate-800">
      <p className="text-xs text-slate-500 dark:text-slate-400">{label}</p>
      <p className="mt-1 text-sm font-semibold text-slate-900 dark:text-white">{value}</p>
    </div>
  )
}

function ActionCard({ icon, title, text, button, primary, onClick, disabled }) {
  const content = (
    <>
      <div className="mx-auto flex h-10 w-10 items-center justify-center rounded-2xl bg-slate-100 text-teal-700 dark:bg-slate-800 dark:text-teal-300">
        <Icon icon={icon} size={20} />
      </div>
      <p className="mt-4 text-sm font-semibold text-slate-800 dark:text-slate-100">{title}</p>
      <p className="mx-auto mt-1 max-w-64 text-sm text-slate-500 dark:text-slate-400">{text}</p>
      {button && (
        <Button as="span" className="mt-4 w-full sm:w-auto" variant={primary ? 'primary' : 'secondary'}>
          {button}
        </Button>
      )}
    </>
  )

  return (
    <Card className={`h-full p-5 text-center transition-transform hover:-translate-y-0.5 ${disabled ? 'opacity-70' : ''}`}>
      <button
        type="button"
        onClick={onClick}
        disabled={disabled}
        className="block h-full w-full text-center disabled:cursor-not-allowed"
      >
        {content}
      </button>
    </Card>
  )
}

function AcoesPaciente({ feedback, erroAcao, enviando, onSolicitarAgendamento, onPedirAjuda }) {
  return (
    <div className="space-y-3">
      <div className="grid gap-3 md:grid-cols-2">
        <ActionCard
          icon={ICONS.schedule}
          title="Agendar consulta comum"
          text="Envie uma solicitacao para a equipe encontrar um horario eletivo."
          button={enviando === 'AGENDAMENTO' ? 'Enviando...' : 'Solicitar agendamento'}
          onClick={onSolicitarAgendamento}
          disabled={Boolean(enviando)}
        />
        <ActionCard
          icon={ICONS.alert}
          title="Pedir atendimento"
          text="Acione a equipe para ajuda de fluxo e orientacao. Isso nao cria triagem clinica."
          button={enviando === 'AJUDA' ? 'Enviando...' : 'Pedir ajuda'}
          primary
          onClick={onPedirAjuda}
          disabled={Boolean(enviando)}
        />
      </div>
      {feedback && (
        <Card className="border border-teal-200/70 bg-teal-50/80 px-4 py-3 text-center dark:border-teal-800/70 dark:bg-teal-950/40">
          <p className="text-sm font-semibold text-teal-900 dark:text-teal-100">{feedback.titulo}</p>
          <p className="mx-auto mt-1 max-w-2xl text-xs leading-5 text-teal-800 dark:text-teal-200">
            Protocolo #{feedback.id}. {feedback.texto}
          </p>
        </Card>
      )}
      {erroAcao && (
        <Card className="border border-red-200/70 bg-red-50/80 px-4 py-3 text-center dark:border-red-800/70 dark:bg-red-950/40">
          <p className="text-sm font-semibold text-red-900 dark:text-red-100">Nao foi possivel enviar</p>
          <p className="mx-auto mt-1 max-w-2xl text-xs leading-5 text-red-800 dark:text-red-200">{erroAcao}</p>
        </Card>
      )}
    </div>
  )
}

function DicasExames() {
  const dicas = [
    'Leve documento com foto.',
    'Alguns exames precisam de jejum.',
    'Confira o resultado pelo sistema.',
    'Chegue com antecedencia.',
  ]
  return (
    <Card className="p-5 text-center">
      <div className="flex items-center justify-center gap-2">
        <Icon icon={ICONS.lab} className="text-teal-600" size={19} />
        <p className="text-sm font-semibold text-slate-800 dark:text-slate-100">Dicas para exames</p>
      </div>
      <div className="mt-3 grid gap-2 sm:grid-cols-2">
        {dicas.map((dica) => (
          <div key={dica} className="rounded-lg bg-slate-50 px-3 py-2 text-sm text-slate-600 dark:bg-slate-800 dark:text-slate-300">
            {dica}
          </div>
        ))}
      </div>
    </Card>
  )
}

function ListaCards({ id, titulo, icon, rows, erro, render, vazio = 'Nenhum registro.' }) {
  return (
    <section id={id} className="scroll-mt-24 space-y-2">
      <h2 className="flex items-center justify-center gap-2 text-center text-sm font-semibold text-slate-700 dark:text-slate-200">
        <Icon icon={icon} className="text-teal-600" size={18} />
        {titulo}
      </h2>
      {erro && <p className="text-sm text-red-600">{erro}</p>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <EmptyState icon={icon} title={vazio} description="Quando houver novidades, elas aparecem aqui." />
      )}
      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <div className="mx-auto grid max-w-4xl gap-3 md:grid-cols-2">
          {rows.map(render)}
        </div>
      )}
    </section>
  )
}

function Cobrancas() {
  const { data, erro } = useApi('/me/cobrancas')
  return (
    <ListaCards
      titulo="Minhas cobrancas"
      icon={ICONS.billing}
      rows={data}
      erro={erro}
      vazio="Nenhuma cobranca pendente."
      render={(c) => (
        <Card key={c.id} className="p-4 text-center">
          <div className="flex items-start justify-between gap-3">
            <div>
              <p className="text-sm font-semibold text-slate-900 dark:text-white">{c.descricao || c.origem || `Cobranca #${c.id}`}</p>
              <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">{c.forma}{c.vencimento ? ` · vence ${c.vencimento}` : ''}</p>
            </div>
            <Badge tone={STATUS_COBRANCA_TONE[c.status]} icon={ICONS.billing}>{c.status}</Badge>
          </div>
          <p className="mt-3 text-lg font-bold text-slate-900 dark:text-white">{formatReais(c.valorCentavos)}</p>
        </Card>
      )}
    />
  )
}

export default function MinhaSaude() {
  const location = useLocation()
  const [feedback, setFeedback] = useState(null)
  const [erroAcao, setErroAcao] = useState('')
  const [enviando, setEnviando] = useState('')
  const perfil = useApi('/me/perfil')
  const agendamentos = useApi('/me/agendamentos')
  const receitas = useApi('/me/receitas')
  const exames = useApi('/me/exames')
  const prontuarios = useApi('/me/prontuarios')

  async function criarSolicitacao(tipo) {
    const mensagem =
      tipo === 'AGENDAMENTO'
        ? 'Solicitacao de consulta comum pelo portal do paciente.'
        : 'Pedido de ajuda/atendimento pelo portal do paciente.'

    setEnviando(tipo)
    setErroAcao('')
    setFeedback(null)

    try {
      const resposta = await apiSend('POST', '/me/solicitacoes', { tipo, mensagem })
      setFeedback({
        id: resposta.id,
        titulo: tipo === 'AGENDAMENTO' ? 'Solicitacao de agendamento enviada' : 'Pedido de ajuda enviado',
        texto:
          tipo === 'AGENDAMENTO'
            ? 'A equipe administrativa vai avaliar sua solicitacao e registrar o agendamento quando houver horario disponivel.'
            : 'A equipe recebeu seu pedido de apoio. A classificacao clinica continua sendo feita apenas por profissional autorizado.',
      })
    } catch (e) {
      setErroAcao(e.message || 'Falha de comunicacao com o backend.')
    } finally {
      setEnviando('')
    }
  }

  useEffect(() => {
    const secao = location.pathname.split('/')[2]
    if (!secao) return
    const alvo = document.getElementById(secao)
    if (alvo) {
      window.requestAnimationFrame(() => {
        alvo.scrollIntoView({ behavior: 'smooth', block: 'start' })
      })
    }
  }, [location.pathname, perfil.data])

  if (perfil.erro) return <p className="text-sm text-red-600">{perfil.erro}</p>
  if (!perfil.data) return <Spinner />

  return (
    <div className="mx-auto max-w-5xl space-y-5">
      <PageHeader
        title="Minha saude"
        subtitle="Carteirinha, consultas, exames, receitas e orientacoes. A triagem clinica e feita pela equipe autorizada."
        align="center"
      />
      <div className="space-y-3">
        <section id="carteirinha" className="scroll-mt-24">
          <Carteirinha perfil={perfil.data} />
        </section>
        <section id="ajuda" className="scroll-mt-24">
          <AcoesPaciente
            feedback={feedback}
            erroAcao={erroAcao}
            enviando={enviando}
            onSolicitarAgendamento={() => criarSolicitacao('AGENDAMENTO')}
            onPedirAjuda={() => criarSolicitacao('AJUDA')}
          />
        </section>
      </div>
      <ListaCards
        titulo="Proximas consultas"
        id="consultas"
        icon={ICONS.schedule}
        rows={agendamentos.data}
        erro={agendamentos.erro}
        vazio="Nenhuma consulta agendada."
        render={(a) => (
          <Card key={a.id} className="p-4 text-center">
            <p className="text-sm font-semibold text-slate-900 dark:text-white">{a.data} as {a.horario}</p>
            <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">Medico #{a.medicoId}</p>
            <div className="mt-3 flex justify-center"><Badge tone="sky" icon={ICONS.schedule}>{a.status}</Badge></div>
          </Card>
        )}
      />
      <ListaCards
        titulo="Meus exames e resultados"
        id="exames"
        icon={ICONS.lab}
        rows={exames.data}
        erro={exames.erro}
        vazio="Nenhum exame pendente."
        render={(e) => (
          <Card key={e.id} className="p-4 text-center">
            <div className="flex flex-col items-center justify-center gap-2 sm:flex-row sm:justify-between">
              <p className="text-sm font-semibold text-slate-900 dark:text-white">Exame #{e.id} · Tipo {e.tipoExame}</p>
              <Badge tone={e.status === 'CONCLUIDO' ? 'green' : 'amber'} icon={ICONS.lab}>{e.status}</Badge>
            </div>
            <p className="mx-auto mt-2 max-w-xl text-sm text-slate-500 dark:text-slate-400">{e.resultado || 'Resultado ainda nao disponivel.'}</p>
          </Card>
        )}
      />
      <ListaCards
        titulo="Minhas receitas"
        id="receitas"
        icon={ICONS.prescription}
        rows={receitas.data}
        erro={receitas.erro}
        vazio="Nenhuma receita ativa."
        render={(r) => (
          <Card key={r.id} className="p-4 text-center">
            <p className="text-sm font-semibold text-slate-900 dark:text-white">{r.medicamento}</p>
            <p className="mt-1 text-sm text-slate-500 dark:text-slate-400">{r.dosagem} · {r.frequencia}</p>
            {r.observacoes && <p className="mx-auto mt-2 max-w-xl text-xs text-slate-500 dark:text-slate-400">{r.observacoes}</p>}
          </Card>
        )}
      />
      <ListaCards
        titulo="Orientacoes e prontuarios"
        id="orientacoes"
        icon={ICONS.record}
        rows={prontuarios.data}
        erro={prontuarios.erro}
        vazio="Nenhuma orientacao registrada."
        render={(p) => (
          <Card key={p.id} className="p-4 text-center">
            <p className="text-sm font-semibold text-slate-900 dark:text-white">{p.data}</p>
            <p className="mx-auto mt-1 max-w-xl text-sm text-slate-500 dark:text-slate-400">{p.conduta || p.diagnostico}</p>
          </Card>
        )}
      />
      <section id="cobrancas" className="scroll-mt-24">
        <Cobrancas />
      </section>
      <section id="dicas" className="scroll-mt-24">
        <DicasExames />
      </section>
    </div>
  )
}
