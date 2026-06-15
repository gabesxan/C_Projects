import { useEffect, useState } from 'react'
import { useAuth } from '../auth/AuthContext'
import { apiGet } from '../api/client'

function Card({ label, value }) {
  return (
    <div className="bg-white rounded-xl shadow p-5">
      <p className="text-sm text-slate-500">{label}</p>
      <p className="text-3xl font-semibold text-slate-800">{value}</p>
    </div>
  )
}

export default function Dashboard() {
  const { user } = useAuth()
  const [resumo, setResumo] = useState(null)
  const [erro, setErro] = useState('')

  // Medico tem um resumo proprio (contagens escopadas) em /me/resumo.
  useEffect(() => {
    if (user?.papel === 'MEDICO') {
      apiGet('/me/resumo')
        .then(setResumo)
        .catch((e) => setErro(e.message))
    }
  }, [user])

  return (
    <div className="space-y-8">
      <section>
        <h2 className="text-sm font-medium text-slate-500 mb-2">Sessao</h2>
        <div className="bg-white rounded-xl shadow p-5 text-sm text-slate-700 space-y-1">
          <p>
            Papel: <span className="font-medium">{user.papel}</span>
          </p>
          <p>pacienteId: {user.pacienteId}</p>
          <p>medicoId: {user.medicoId}</p>
        </div>
      </section>

      {user.papel === 'MEDICO' && (
        <section>
          <h2 className="text-sm font-medium text-slate-500 mb-2">Meu resumo</h2>
          {erro && <p className="text-sm text-red-600">{erro}</p>}
          {resumo ? (
            <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
              <Card label="Pacientes" value={resumo.pacientes} />
              <Card label="Agendamentos" value={resumo.agendamentos} />
              <Card label="Prontuarios" value={resumo.prontuarios} />
              <Card label="Exames" value={resumo.exames} />
            </div>
          ) : (
            !erro && <p className="text-sm text-slate-400">Carregando...</p>
          )}
        </section>
      )}
    </div>
  )
}
