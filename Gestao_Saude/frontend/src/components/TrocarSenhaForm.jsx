import { useState } from 'react'
import { apiSend } from '../api/client'
import { useAuth } from '../auth/AuthContext'
import { Card, Button, Alert } from './ui'

const inputCls =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-2 text-sm focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30 outline-none'

// Formulario de troca de senha. Serve tanto a troca voluntaria quanto a
// obrigatoria no primeiro acesso (onTrocada permite reagir ao sucesso).
export default function TrocarSenhaForm({ onTrocada }) {
  const { confirmarTrocaSenha } = useAuth()
  const [atual, setAtual] = useState('')
  const [nova, setNova] = useState('')
  const [confirma, setConfirma] = useState('')
  const [erro, setErro] = useState('')
  const [ok, setOk] = useState(false)
  const [enviando, setEnviando] = useState(false)

  async function submit(e) {
    e.preventDefault()
    setErro('')
    if (nova.length < 6) {
      setErro('A nova senha deve ter ao menos 6 caracteres.')
      return
    }
    if (nova !== confirma) {
      setErro('A confirmacao nao confere com a nova senha.')
      return
    }
    setEnviando(true)
    try {
      await apiSend('POST', '/me/senha', { senha_atual: atual, senha_nova: nova })
      confirmarTrocaSenha()
      setOk(true)
      setAtual('')
      setNova('')
      setConfirma('')
      if (onTrocada) onTrocada()
    } catch (err) {
      setErro(err.status === 400 ? 'Senha atual incorreta ou nova senha invalida.' : err.message)
    } finally {
      setEnviando(false)
    }
  }

  return (
    <Card className="p-5">
      <form onSubmit={submit} className="space-y-3">
        {erro && <Alert>{erro}</Alert>}
        {ok && <Alert tone="teal">Senha alterada com sucesso.</Alert>}
        <label className="block text-sm text-slate-600">
          Senha atual
          <input type="password" autoComplete="current-password" className={inputCls}
            value={atual} onChange={(e) => setAtual(e.target.value)} required />
        </label>
        <label className="block text-sm text-slate-600">
          Nova senha
          <input type="password" autoComplete="new-password" className={inputCls}
            value={nova} onChange={(e) => setNova(e.target.value)} required />
        </label>
        <label className="block text-sm text-slate-600">
          Confirmar nova senha
          <input type="password" autoComplete="new-password" className={inputCls}
            value={confirma} onChange={(e) => setConfirma(e.target.value)} required />
        </label>
        <Button type="submit" disabled={enviando}>
          {enviando ? 'Salvando...' : 'Trocar senha'}
        </Button>
      </form>
    </Card>
  )
}
