import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAuth } from '../auth/AuthContext'
import { Alert, Button } from '../components/ui'
import { ICONS, Icon } from '../components/icons'

export default function Login() {
  const { login } = useAuth()
  const navigate = useNavigate()
  const [loginName, setLoginName] = useState('')
  const [senha, setSenha] = useState('')
  const [erro, setErro] = useState('')
  const [carregando, setCarregando] = useState(false)

  async function handleSubmit(e) {
    e.preventDefault()
    setErro('')
    setCarregando(true)
    try {
      await login(loginName.trim(), senha)
      navigate('/')
    } catch (err) {
      if (err.status === 401) setErro('Login ou senha invalidos.')
      else if (err.status === 429) setErro('Acesso bloqueado por tentativas invalidas. Tente novamente em alguns minutos.')
      else setErro(err.message)
    } finally {
      setCarregando(false)
    }
  }

  return (
    <div className="grid min-h-screen lg:grid-cols-2">
      {/* Painel de marca (lado esquerdo) */}
      <div className="relative hidden overflow-hidden p-12 text-white lg:flex lg:flex-col lg:justify-between">
        <div className="absolute inset-0 bg-slate-950" />
        <div className="absolute inset-0 bg-[radial-gradient(circle_at_22%_12%,rgba(20,184,166,0.35),transparent_28rem),radial-gradient(circle_at_80%_50%,rgba(59,130,246,0.16),transparent_22rem)]" />
        <div className="flex items-center gap-3">
          <div className="relative flex h-11 w-11 items-center justify-center rounded-2xl bg-white/12 text-teal-100 ring-1 ring-white/15">
            <Icon icon={ICONS.hospital} size={23} />
          </div>
          <div className="relative">
            <p className="text-lg font-bold leading-tight">SIGEH-DF</p>
            <p className="text-sm text-teal-100">Gestao Hospitalar</p>
          </div>
        </div>

        <div className="relative space-y-4">
          <h2 className="text-4xl font-semibold leading-tight tracking-tight">
            Cuidado coordenado,
            <br />
            do acolhimento a alta.
          </h2>
          <p className="max-w-md text-teal-100">
            Triagem, prontuario, exames, prescricoes, leitos e internacoes em um
            so lugar — com acesso controlado por papel.
          </p>
        </div>

        <p className="relative text-xs text-teal-200">
          Acesso restrito a profissionais autorizados.
        </p>
      </div>

      {/* Formulario (lado direito) */}
      <div className="flex items-center justify-center bg-slate-50 p-6 dark:bg-slate-950">
        <form onSubmit={handleSubmit} className="premium-card w-full max-w-sm space-y-6 rounded-3xl p-7 shadow-xl ring-1 ring-slate-200/70">
          <div className="text-center lg:hidden">
            <div className="mx-auto mb-2 flex h-12 w-12 items-center justify-center rounded-2xl bg-teal-600 text-white shadow-lg">
              <Icon icon={ICONS.hospital} size={24} />
            </div>
            <h1 className="text-xl font-bold text-slate-900">SIGEH-DF</h1>
          </div>

          <div>
            <h1 className="text-2xl font-semibold tracking-tight text-slate-950 dark:text-white">Entrar</h1>
            <p className="mt-1 text-sm text-slate-500">
              Use suas credenciais individuais.
            </p>
          </div>

          {erro && <Alert>{erro}</Alert>}

          <div className="space-y-1">
            <label className="text-sm font-medium text-slate-700">Login</label>
            <input
              type="text"
              value={loginName}
              onChange={(e) => setLoginName(e.target.value)}
              autoFocus
              required
              autoComplete="username"
              className="w-full rounded-lg border border-slate-300 px-3 py-2 outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30"
            />
          </div>

          <div className="space-y-1">
            <label className="text-sm font-medium text-slate-700">Senha</label>
            <input
              type="password"
              value={senha}
              onChange={(e) => setSenha(e.target.value)}
              required
              autoComplete="current-password"
              className="w-full rounded-lg border border-slate-300 px-3 py-2 outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30"
            />
          </div>

          <Button type="submit" disabled={carregando} className="w-full py-2.5">
            {carregando ? 'Entrando...' : 'Entrar'}
          </Button>
        </form>
      </div>
    </div>
  )
}
