import { PageHeader } from '../components/ui'
import TrocarSenhaForm from '../components/TrocarSenhaForm'

// Troca de senha voluntaria (qualquer papel), dentro do layout autenticado.
export default function TrocarSenha() {
  return (
    <div className="mx-auto max-w-md space-y-6">
      <PageHeader
        title="Trocar senha"
        subtitle="Defina uma nova senha de acesso individual."
      />
      <TrocarSenhaForm />
    </div>
  )
}
