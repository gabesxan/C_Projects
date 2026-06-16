import { Link } from 'react-router-dom'
import { useAuth } from '../auth/AuthContext'
import { Card, Button, papelLabel } from '../components/ui'

export default function AcessoNegado() {
  const { user } = useAuth()

  return (
    <div className="flex min-h-[60vh] items-center justify-center">
      <Card className="max-w-md p-8 text-center">
        <div className="mx-auto mb-4 flex h-14 w-14 items-center justify-center rounded-full bg-red-100 text-3xl">
          🔒
        </div>
        <h1 className="text-xl font-bold text-slate-900">Acesso negado</h1>
        <p className="mt-2 text-sm text-slate-500">
          Seu papel ({user ? papelLabel(user.papel) : '—'}) nao tem permissao para
          acessar esta area. Se acredita que isso e um engano, fale com um
          administrador.
        </p>
        <div className="mt-6">
          <Link to="/">
            <Button>Voltar ao painel</Button>
          </Link>
        </div>
      </Card>
    </div>
  )
}
