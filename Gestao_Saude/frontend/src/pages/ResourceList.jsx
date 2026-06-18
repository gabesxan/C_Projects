import { useCallback, useEffect, useState } from 'react'
import { useNavigate, useParams } from 'react-router-dom'
import { apiGet, apiSend } from '../api/client'
import { useAuth } from '../auth/AuthContext'
import { resourceByKey } from '../resources'
import DataTable from '../components/DataTable'
import ResourceForm from '../components/ResourceForm'
import AcessoNegado from './AcessoNegado'
import { PageHeader, Alert, Spinner, EmptyState, Badge } from '../components/ui'

export default function ResourceList() {
  const { key } = useParams()
  const navigate = useNavigate()
  const { user } = useAuth()
  const recurso = resourceByKey(key)
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  const carregar = useCallback(() => {
    if (!recurso) return
    setRows(null)
    setErro('')
    apiGet(recurso.path)
      .then(setRows)
      .catch((e) => {
        if (e.status === 403) setErro('Voce nao tem acesso a este recurso.')
        else setErro(e.message)
      })
  }, [recurso])

  useEffect(() => {
    carregar()
  }, [carregar])

  if (!recurso) {
    return <EmptyState title="Recurso desconhecido" description="Verifique o endereco." />
  }

  // Guarda de acesso por papel (espelha a politica do backend).
  if (!recurso.roles.includes(user.papel)) {
    return <AcessoNegado />
  }

  const podeCriar = recurso.createRoles?.includes(user.papel)
  const podeDeletar = recurso.deleteRoles?.includes(user.papel)
  const podeRetificar = recurso.retificavel && podeCriar
  const podeAdministrar =
    recurso.administravel && ['ADMIN', 'MEDICO', 'ENFERMAGEM'].includes(user.papel)

  async function administrar(row) {
    const observacao = window.prompt(`Confirmar administracao de "${row.medicamento}". Observacao (opcional):`)
    if (observacao === null) return
    try {
      await apiSend('POST', `${recurso.path}/${row.id}/administrar`, { observacao })
      setErro('')
      window.alert('Administracao registrada.')
    } catch (e) {
      setErro(e.message)
    }
  }

  async function retificar(row) {
    const diagnostico = window.prompt('Diagnostico (corrigido):', row.diagnostico || '')
    if (diagnostico === null) return
    const conduta = window.prompt('Conduta (corrigida):', row.conduta || '')
    if (conduta === null || !conduta.trim()) {
      setErro('Conduta e obrigatoria na retificacao.')
      return
    }
    const justificativa = window.prompt('Justificativa da retificacao:')
    if (justificativa === null) return
    if (!justificativa.trim()) {
      setErro('Justificativa e obrigatoria.')
      return
    }
    const data = new Date().toISOString().slice(0, 10)
    try {
      await apiSend('POST', `${recurso.path}/${row.id}/retificar`, {
        data,
        observacoes: row.observacoes || '',
        diagnostico,
        conduta,
        alerta_importante: row.alertaImportante ? '1' : '0',
        justificativa,
      })
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  async function remover(row) {
    let params = {}
    // Acoes que exigem motivo (cancelar/suspender) pedem a justificativa.
    if (recurso.requireReason) {
      const motivo = window.prompt(`Motivo para ${recurso.deleteLabel.toLowerCase()} #${row.id}:`)
      if (motivo === null) return
      if (!motivo.trim()) {
        setErro('Motivo e obrigatorio.')
        return
      }
      params = { motivo }
    } else if (!window.confirm(`Confirmar: ${recurso.deleteLabel} #${row.id}?`)) {
      return
    }
    try {
      await apiSend('DELETE', `${recurso.path}/${row.id}`, params)
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  return (
    <div className="space-y-5">
      <PageHeader
        title={recurso.label}
        subtitle={
          Array.isArray(rows) ? `${rows.length} registro(s)` : 'Carregando registros...'
        }
        actions={
          Array.isArray(rows) && (
            <Badge tone="slate">{rows.length}</Badge>
          )
        }
      />

      {podeCriar && <ResourceForm recurso={recurso} onCreated={carregar} />}

      {erro && <Alert>{erro}</Alert>}
      {!erro && rows === null && <Spinner />}

      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <EmptyState
          title="Nenhum registro"
          description={
            podeCriar
              ? 'Comece criando o primeiro registro com o botao acima.'
              : 'Ainda nao ha registros para exibir.'
          }
        />
      )}

      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <DataTable
          columns={recurso.columns}
          rows={rows}
          onDelete={podeDeletar ? remover : undefined}
          onAction={podeRetificar ? retificar : podeAdministrar ? administrar : undefined}
          actionLabel={podeRetificar ? 'Retificar' : 'Administrar'}
          onRowClick={recurso.detail ? (row) => navigate(recurso.detail(row)) : undefined}
          deleteLabel={recurso.deleteLabel}
        />
      )}
    </div>
  )
}
