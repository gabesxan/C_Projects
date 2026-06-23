import { useCallback, useEffect, useState } from 'react'
import { Link } from 'react-router-dom'
import { Syringe } from 'lucide-react'
import { apiGet } from '../api/client'
import DataTable from '../components/DataTable'
import { PageHeader, Card, Button, Alert, Spinner, StatCard } from '../components/ui'

const COLUNAS = [
  { key: 'id', label: 'ID' },
  { key: 'nome', label: 'Nome' },
  { key: 'fabricante', label: 'Fabricante' },
  { key: 'doencasAlvo', label: 'Doencas alvo' },
  { key: 'dosesPrevistas', label: 'Doses' },
  { key: 'intervaloDias', label: 'Intervalo dias' },
  { key: 'reforcoDias', label: 'Reforco dias' },
]

export default function Vacinacao() {
  const [vacinas, setVacinas] = useState(null)
  const [erro, setErro] = useState('')

  const carregar = useCallback(() => {
    apiGet('/vacinas')
      .then((dados) => {
        setVacinas(dados)
        setErro('')
      })
      .catch((e) => setErro(e.message))
  }, [])

  useEffect(carregar, [carregar])

  const totalDoses = (vacinas || []).reduce((s, v) => s + Number(v.dosesPrevistas || 0), 0)
  const comReforco = (vacinas || []).filter((v) => Number(v.reforcoDias || 0) > 0).length

  return (
    <div className="space-y-6">
      <PageHeader
        title="Vacinação"
        subtitle="Catálogo de vacinas e esquemas de dose"
        actions={<Button as={Link} to="/r/vacinas">Gerenciar catálogo</Button>}
      />

      {erro && <Alert tone="red">{erro}</Alert>}

      {vacinas == null ? (
        <Spinner />
      ) : (
        <>
          <div className="grid gap-4 md:grid-cols-3">
            <StatCard label="Vacinas ativas" value={vacinas.length} icon={Syringe} tone="teal" />
            <StatCard label="Doses previstas" value={totalDoses} tone="sky" />
            <StatCard label="Com reforço" value={comReforco} tone="green" />
          </div>

          <Card className="p-4">
            {vacinas.length === 0 ? (
              <div className="px-6 py-10 text-center">
                <p className="font-semibold text-slate-700">Nenhuma vacina cadastrada</p>
                <p className="mt-1 text-sm text-slate-500">
                  Cadastre vacinas para iniciar o módulo de vacinação.
                </p>
              </div>
            ) : (
              <DataTable columns={COLUNAS} rows={vacinas} />
            )}
          </Card>
        </>
      )}
    </div>
  )
}
