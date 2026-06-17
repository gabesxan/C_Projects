import { useEffect, useState } from 'react'
import { useNavigate, useParams } from 'react-router-dom'
import { apiGet } from '../api/client'
import DataTable from '../components/DataTable'
import {
  PageHeader,
  Card,
  Button,
  Alert,
  Spinner,
  Badge,
  StatusBadge,
} from '../components/ui'

const RISCO = {
  Vermelho: 'red',
  Laranja: 'amber',
  Amarelo: 'amber',
  Verde: 'green',
  Azul: 'sky',
}
const tomRisco = (v) => RISCO[v] ?? 'slate'

// Secoes do historico clinico e suas colunas.
const SECOES = [
  {
    titulo: 'Triagens',
    chave: 'triagens',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'tipoTriagem', label: 'Tipo' },
      { key: 'pontuacao', label: 'Pontuacao' },
      { key: 'classificacao', label: 'Classificacao', type: 'badge', tone: tomRisco },
    ],
  },
  {
    titulo: 'Agendamentos',
    chave: 'agendamentos',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'horario', label: 'Horario' },
      { key: 'status', label: 'Status', type: 'badge' },
    ],
  },
  {
    titulo: 'Prontuarios',
    chave: 'prontuarios',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'diagnostico', label: 'Diagnostico' },
      { key: 'conduta', label: 'Conduta' },
    ],
  },
  {
    titulo: 'Exames',
    chave: 'exames',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'tipoExame', label: 'Tipo' },
      { key: 'dataSolicitacao', label: 'Solicitacao' },
      { key: 'status', label: 'Status', type: 'badge' },
      { key: 'resultado', label: 'Resultado' },
    ],
  },
  {
    titulo: 'Prescricoes',
    chave: 'prescricoes',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'medicamento', label: 'Medicamento' },
      { key: 'dosagem', label: 'Dosagem' },
      { key: 'frequencia', label: 'Frequencia' },
      { key: 'observacoes', label: 'Observacoes' },
    ],
  },
]

function Info({ label, value }) {
  return (
    <div>
      <p className="text-xs font-medium uppercase tracking-wide text-slate-400">{label}</p>
      <p className="text-sm font-medium text-slate-800">{value || '—'}</p>
    </div>
  )
}

export default function PacienteDetalhe() {
  const { id } = useParams()
  const navigate = useNavigate()
  const [paciente, setPaciente] = useState(null)
  const [historico, setHistorico] = useState(null)
  const [erro, setErro] = useState('')
  const [semHistorico, setSemHistorico] = useState(false)

  useEffect(() => {
    apiGet(`/pacientes/${id}`)
      .then(setPaciente)
      .catch((e) => setErro(e.status === 404 ? 'Paciente nao encontrado.' : e.message))
    apiGet(`/pacientes/${id}/historico`)
      .then(setHistorico)
      .catch((e) => {
        if (e.status === 403) setSemHistorico(true)
        else setErro(e.message)
      })
  }, [id])

  if (erro) return <Alert>{erro}</Alert>
  if (!paciente) return <Spinner />

  const menor = paciente.idade < 18

  return (
    <div className="space-y-6">
      <PageHeader
        title={paciente.nome}
        subtitle={`${paciente.idade} anos • ${paciente.tipoDocumento} ${paciente.documento}`}
        actions={
          <Button variant="secondary" onClick={() => navigate('/r/pacientes')}>
            ← Voltar
          </Button>
        }
      />

      {/* Alergias / alertas clinicos em destaque. */}
      {paciente.alergias ? (
        <Alert tone="red">
          <span className="font-semibold">⚠️ Alergias / alertas:</span> {paciente.alergias}
        </Alert>
      ) : (
        <Alert tone="teal">Sem alergias registradas.</Alert>
      )}

      <Card className="p-5">
        <div className="mb-4 flex items-center gap-2">
          <StatusBadge ativo={paciente.ativo} />
          {menor && <Badge tone="amber">Menor de idade</Badge>}
        </div>
        <div className="grid grid-cols-2 gap-4 md:grid-cols-3">
          <Info label="Nascimento" value={paciente.nascimento} />
          <Info label="Idade" value={`${paciente.idade} anos`} />
          <Info label="Sexo" value={paciente.sexo} />
          <Info label="Telefone" value={paciente.telefone} />
          <Info label="Regiao administrativa" value={paciente.regiaoAdministrativa} />
          <Info label="Documento" value={`${paciente.documento} (${paciente.tipoDocumento})`} />
          {menor && <Info label="Responsavel" value={paciente.responsavel} />}
        </div>
      </Card>

      <div>
        <h2 className="mb-3 text-lg font-bold text-slate-900">Historico clinico</h2>
        {semHistorico ? (
          <Alert tone="amber">
            Seu papel nao tem acesso ao historico clinico deste paciente.
          </Alert>
        ) : !historico ? (
          <Spinner />
        ) : (
          <div className="space-y-5">
            {SECOES.map((s) => {
              const linhas = historico[s.chave] ?? []
              return (
                <section key={s.chave} className="space-y-2">
                  <h3 className="text-sm font-semibold text-slate-600">
                    {s.titulo}{' '}
                    <span className="font-normal text-slate-400">({linhas.length})</span>
                  </h3>
                  {linhas.length > 0 ? (
                    <DataTable columns={s.columns} rows={linhas} />
                  ) : (
                    <p className="text-sm text-slate-400">Nenhum registro.</p>
                  )}
                </section>
              )
            })}
          </div>
        )}
      </div>
    </div>
  )
}
