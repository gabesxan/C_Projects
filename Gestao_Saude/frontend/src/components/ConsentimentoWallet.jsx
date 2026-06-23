import { useCallback, useEffect, useState } from 'react'
import {
  listarMeusConsentimentos,
  revogarConsentimento,
  listarMeuRelatorioAcessos,
} from '../api/client'
import { Card, Badge, Button, Spinner, EmptyState, Alert } from './ui'
import { ICONS, Icon } from './icons'

// Rotulos legiveis para as acoes/entidades da trilha de auditoria, para que o
// relatorio de acessos fique compreensivel ao paciente (e nao em "jargao").
const ACAO_LABEL = {
  CRIAR: 'Cadastrou',
  ATUALIZAR: 'Atualizou',
  EVOLUCAO: 'Registrou evolução',
  REVOGAR: 'Revogou',
  AGENDAR: 'Agendou',
  REAGENDAR: 'Reagendou',
  CANCELAR: 'Cancelou',
  CHECKIN: 'Recepcionou',
  CHAMAR: 'Chamou na fila',
  TRIAGEM: 'Fez triagem',
  RECLASSIFICAR: 'Reclassificou triagem',
  PRESCREVER: 'Prescreveu',
  ADMINISTRAR: 'Administrou medicação',
  EXAME_RESULTADO: 'Lançou resultado de exame',
  EXAME_STATUS: 'Atualizou status de exame',
  RETIFICAR: 'Retificou',
  COBRANCA: 'Lançou cobrança',
  RELATORIO_ACESSOS: 'Consultou relatório de acessos',
}

const ENTIDADE_LABEL = {
  paciente: 'cadastro',
  exame: 'exame',
  prontuario: 'prontuário',
  prescricao: 'prescrição',
  triagem: 'triagem',
  agendamento: 'agendamento',
  consentimento: 'consentimento',
  cobranca: 'cobrança',
  checkin: 'recepção',
  internacao: 'internação',
  solicitacao_paciente: 'solicitação',
}

function StatusConsentimento({ status }) {
  const revogado = status === 'REVOGADO'
  return (
    <Badge tone={revogado ? 'red' : 'green'} icon={revogado ? ICONS.alert : ICONS.audit}>
      {revogado ? 'Revogado' : 'Concedido'}
    </Badge>
  )
}

function ConsentimentoCard({ consentimento, onRevogar, revogando }) {
  const c = consentimento
  const concedido = c.status === 'CONCEDIDO'
  return (
    <Card className="p-4">
      <div className="flex items-start justify-between gap-3">
        <div>
          <p className="text-sm font-semibold text-slate-900 dark:text-white">{c.finalidade}</p>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">Termo {c.versaoTermo}</p>
        </div>
        <StatusConsentimento status={c.status} />
      </div>
      <dl className="mt-3 space-y-1 text-xs text-slate-500 dark:text-slate-400">
        <div className="flex justify-between gap-2">
          <dt>Concedido em</dt>
          <dd className="font-medium text-slate-700 dark:text-slate-200">{c.concedidoEm || '—'}</dd>
        </div>
        {c.status === 'REVOGADO' && (
          <div className="flex justify-between gap-2">
            <dt>Revogado em</dt>
            <dd className="font-medium text-slate-700 dark:text-slate-200">{c.revogadoEm || '—'}</dd>
          </div>
        )}
      </dl>
      {c.status === 'REVOGADO' && c.motivoRevogacao && (
        <p className="mt-2 text-xs text-red-600 dark:text-red-400">Motivo: {c.motivoRevogacao}</p>
      )}
      {concedido && (
        <Button
          variant="danger"
          className="mt-4 w-full"
          onClick={() => onRevogar(c)}
          disabled={revogando === c.id}
        >
          {revogando === c.id ? 'Revogando...' : 'Revogar consentimento'}
        </Button>
      )}
    </Card>
  )
}

function GrupoConsentimentos({ titulo, icon, rows }) {
  if (!rows || rows.length === 0) return null
  return (
    <section className="space-y-3">
      <h3 className="flex items-center gap-2 text-sm font-semibold text-slate-700 dark:text-slate-200">
        <Icon icon={icon} className="text-teal-600" size={16} />
        {titulo} ({rows.length})
      </h3>
      <div className="grid gap-3 md:grid-cols-2">{rows}</div>
    </section>
  )
}

function AcessoCard({ acesso }) {
  const a = acesso
  const acao = ACAO_LABEL[a.acao] || a.acao
  const entidade = ENTIDADE_LABEL[a.entidade] || a.entidade
  return (
    <Card className="p-4">
      <div className="flex items-start justify-between gap-3">
        <p className="text-sm font-semibold text-slate-900 dark:text-white">
          {acao} <span className="font-normal text-slate-500 dark:text-slate-400">— {entidade}</span>
        </p>
        <Badge tone="sky">{a.criadoEm}</Badge>
      </div>
      <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">
        Por <span className="font-medium text-slate-700 dark:text-slate-200">{a.usuarioLogin || `usuário #${a.usuarioId}`}</span>
        {a.entidadeId ? ` · registro #${a.entidadeId}` : ''}
      </p>
      {a.detalhe && <p className="mt-2 text-xs text-slate-500 dark:text-slate-400">{a.detalhe}</p>}
    </Card>
  )
}

export default function ConsentimentoWallet() {
  const [consentimentos, setConsentimentos] = useState(null)
  const [erroConsent, setErroConsent] = useState('')
  const [acessos, setAcessos] = useState(null)
  const [erroAcessos, setErroAcessos] = useState('')
  const [revogando, setRevogando] = useState(0)
  const [erroAcao, setErroAcao] = useState('')
  const [okAcao, setOkAcao] = useState('')

  // Carga assincrona: o estado so muda nos callbacks da promise (nunca de
  // forma sincrona dentro do efeito), seguindo o padrao do portal.
  const carregarConsentimentos = useCallback(() => {
    listarMeusConsentimentos()
      .then((rows) => {
        setConsentimentos(rows)
        setErroConsent('')
      })
      .catch((e) => setErroConsent(e.message || 'Falha ao carregar consentimentos.'))
  }, [])

  const carregarAcessos = useCallback(() => {
    listarMeuRelatorioAcessos()
      .then((r) => {
        setAcessos(Array.isArray(r?.acessos) ? r.acessos : [])
        setErroAcessos('')
      })
      .catch((e) => setErroAcessos(e.message || 'Falha ao carregar relatório de acessos.'))
  }, [])

  useEffect(() => {
    carregarConsentimentos()
    carregarAcessos()
  }, [carregarConsentimentos, carregarAcessos])

  async function revogar(c) {
    const motivo = window.prompt('Informe o motivo da revogação deste consentimento:')
    if (motivo === null) return // cancelou o prompt
    if (!motivo.trim()) {
      setErroAcao('Informe o motivo da revogação.')
      setOkAcao('')
      return
    }
    setRevogando(c.id)
    setErroAcao('')
    setOkAcao('')
    try {
      await revogarConsentimento(c.id, motivo.trim())
      setOkAcao('Consentimento revogado.')
      carregarConsentimentos()
      carregarAcessos()
    } catch (e) {
      setErroAcao(e.message || 'Falha ao revogar consentimento.')
    } finally {
      setRevogando(0)
    }
  }

  const concedidos = Array.isArray(consentimentos)
    ? consentimentos.filter((c) => c.status === 'CONCEDIDO')
    : []
  const revogados = Array.isArray(consentimentos)
    ? consentimentos.filter((c) => c.status === 'REVOGADO')
    : []

  return (
    <div className="space-y-8">
      <section className="space-y-3">
        <div>
          <h2 className="flex items-center gap-2 text-sm font-semibold text-slate-700 dark:text-slate-200">
            <Icon icon={ICONS.audit} className="text-teal-600" size={18} />
            Meus consentimentos (LGPD)
          </h2>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">
            Você pode revogar a qualquer momento. O histórico é mantido para fins de auditoria.
          </p>
        </div>

        {erroAcao && <Alert tone="red">{erroAcao}</Alert>}
        {okAcao && <Alert tone="teal">{okAcao}</Alert>}

        {erroConsent && <Alert tone="red">{erroConsent}</Alert>}
        {!erroConsent && consentimentos === null && <Spinner label="Carregando consentimentos..." />}
        {!erroConsent && Array.isArray(consentimentos) && consentimentos.length === 0 && (
          <EmptyState
            icon={ICONS.audit}
            title="Nenhum consentimento registrado."
            description="Quando você conceder consentimentos, eles aparecem aqui."
          />
        )}
        {!erroConsent && consentimentos !== null && consentimentos.length > 0 && (
          <div className="space-y-6">
            <GrupoConsentimentos
              titulo="Concedidos"
              icon={ICONS.audit}
              rows={concedidos.map((c) => (
                <ConsentimentoCard key={c.id} consentimento={c} onRevogar={revogar} revogando={revogando} />
              ))}
            />
            <GrupoConsentimentos
              titulo="Revogados"
              icon={ICONS.alert}
              rows={revogados.map((c) => (
                <ConsentimentoCard key={c.id} consentimento={c} onRevogar={revogar} revogando={revogando} />
              ))}
            />
          </div>
        )}
      </section>

      <section className="space-y-3">
        <div>
          <h2 className="flex items-center gap-2 text-sm font-semibold text-slate-700 dark:text-slate-200">
            <Icon icon={ICONS.reports} className="text-teal-600" size={18} />
            Quem acessou meus dados
          </h2>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">
            Registro de acessos aos seus dados, com quem, quando e qual ação.
          </p>
        </div>

        {erroAcessos && <Alert tone="red">{erroAcessos}</Alert>}
        {!erroAcessos && acessos === null && <Spinner label="Carregando relatório de acessos..." />}
        {!erroAcessos && Array.isArray(acessos) && acessos.length === 0 && (
          <EmptyState
            icon={ICONS.reports}
            title="Nenhum acesso registrado."
            description="Acessos aos seus dados pela equipe aparecem aqui."
          />
        )}
        {!erroAcessos && Array.isArray(acessos) && acessos.length > 0 && (
          <div className="grid gap-3 md:grid-cols-2">
            {acessos.map((a) => (
              <AcessoCard key={a.id} acesso={a} />
            ))}
          </div>
        )}
      </section>
    </div>
  )
}
