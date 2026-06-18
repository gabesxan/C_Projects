import TrocarSenhaForm from '../components/TrocarSenhaForm'

// Tela bloqueante mostrada no primeiro acesso (trocarSenha = true). So libera o
// app apos a troca; o proprio AuthContext baixa a flag ao concluir.
export default function TrocarSenhaObrigatoria() {
  return (
    <div className="flex min-h-screen items-center justify-center bg-slate-50 p-6">
      <div className="w-full max-w-md space-y-4">
        <div className="text-center">
          <div className="mx-auto mb-2 flex h-12 w-12 items-center justify-center rounded-xl bg-teal-600 text-2xl">
            🔐
          </div>
          <h1 className="text-xl font-bold text-slate-900">Defina sua senha</h1>
          <p className="mt-1 text-sm text-slate-500">
            Por seguranca, troque a senha inicial antes de continuar.
          </p>
        </div>
        <TrocarSenhaForm />
      </div>
    </div>
  )
}
