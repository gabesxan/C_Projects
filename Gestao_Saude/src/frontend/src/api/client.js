// Cliente HTTP do SIGEH-DF.
// - Prefixo "/api" e reescrito pelo proxy do Vite para o backend em C.
// - Autenticacao por TOKEN de sessao: o login devolve um token opaco que vai
//   no header 'Authorization: Bearer <token>' de toda requisicao. A senha so
//   trafega uma vez, no corpo do POST /sessao (nunca na URL nem no storage).
// - As escritas (POST/DELETE) enviam os parametros no corpo como JSON.

// Em dev, "/api" e reescrito pelo proxy do Vite para o backend (:8080).
// Em producao, o proprio servidor C serve o front e a API na mesma origem,
// entao as chamadas vao para a raiz.
const BASE = import.meta.env.DEV ? '/api' : ''

// Token em memoria; persistido em sessionStorage para sobreviver a reload.
let token = null

export function setToken(novo) {
  token = novo
  if (novo) {
    sessionStorage.setItem('sigeh_token', novo)
  } else {
    sessionStorage.removeItem('sigeh_token')
  }
}

export function loadToken() {
  if (token === null) {
    token = sessionStorage.getItem('sigeh_token')
  }
  return token
}

export function clearToken() {
  token = null
  sessionStorage.removeItem('sigeh_token')
}

function authHeaders() {
  return token ? { Authorization: `Bearer ${token}` } : {}
}

export class ApiError extends Error {
  constructor(status, body) {
    super((body && body.erro) || `HTTP ${status}`)
    this.status = status
    this.body = body
  }
}

async function safeJson(res) {
  try {
    return await res.json()
  } catch {
    return null
  }
}

// Abre uma sessao: envia login/senha no CORPO (JSON), guarda o token e devolve
// o perfil retornado pela API ({ papel, login, pacienteId, medicoId }).
export async function apiLogin(login, senha) {
  const res = await fetch(BASE + '/sessao', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ login, senha }),
  })
  if (!res.ok) throw new ApiError(res.status, await safeJson(res))
  const data = await res.json()
  setToken(data.token)
  return data
}

// Encerra a sessao no servidor (best-effort) e descarta o token local.
export async function apiLogout() {
  try {
    if (token) {
      await fetch(BASE + '/sessao', { method: 'DELETE', headers: { ...authHeaders() } })
    }
  } catch {
    // logout e best-effort: mesmo offline, limpamos o token local.
  } finally {
    clearToken()
  }
}

// GET autenticado que devolve o JSON ja parseado.
export async function apiGet(path) {
  const res = await fetch(BASE + path, { headers: { ...authHeaders() } })
  if (!res.ok) throw new ApiError(res.status, await safeJson(res))
  return res.json()
}

// POST/DELETE: os parametros vao no CORPO como JSON (nunca na URL). Quando nao
// ha parametros (acoes simples como /checkins/{id}/chamar), envia sem corpo.
export async function apiSend(method, path, params = {}) {
  const temCorpo = params && Object.keys(params).length > 0
  const res = await fetch(BASE + path, {
    method,
    headers: {
      ...authHeaders(),
      ...(temCorpo ? { 'Content-Type': 'application/json' } : {}),
    },
    body: temCorpo ? JSON.stringify(params) : undefined,
  })
  if (!res.ok) throw new ApiError(res.status, await safeJson(res))
  return safeJson(res)
}

// --- Anexos -----------------------------------------------------------------
// Documentos vinculados a uma entidade (exame, paciente, etc.). O binario fica
// no backend; o front so troca metadados e base64, nunca o caminho fisico.

// Converte um File para base64 puro (sem o prefixo "data:<mime>;base64,").
export function arquivoParaBase64(file) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader()
    reader.onload = () => {
      const r = String(reader.result)
      const virgula = r.indexOf(',')
      resolve(virgula >= 0 ? r.slice(virgula + 1) : r)
    }
    reader.onerror = () => reject(reader.error || new Error('falha ao ler arquivo'))
    reader.readAsDataURL(file)
  })
}

// Lista os metadados dos anexos de uma entidade.
export function listarAnexos(entidade, id) {
  return apiGet(`/anexos/${encodeURIComponent(entidade)}/${id}`)
}

// Envia um novo anexo (corpo JSON com o conteudo em base64).
export function enviarAnexo({ entidade, entidadeId, nome, mime, conteudoB64 }) {
  return apiSend('POST', '/anexos', { entidade, entidadeId, nome, mime, conteudoB64 })
}

// Baixa o binario do anexo e dispara o download no navegador. O nome do arquivo
// vem do cabecalho Content-Disposition (o front nunca conhece o caminho fisico).
export async function baixarAnexo(id) {
  const res = await fetch(BASE + `/anexos/${id}/conteudo`, { headers: { ...authHeaders() } })
  if (!res.ok) throw new ApiError(res.status, await safeJson(res))
  const blob = await res.blob()
  const disp = res.headers.get('Content-Disposition') || ''
  const m = /filename="?([^"]+)"?/.exec(disp)
  const nome = m ? m[1] : `anexo-${id}`
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = nome
  document.body.appendChild(a)
  a.click()
  a.remove()
  URL.revokeObjectURL(url)
}

// Remove um anexo. Acao destrutiva: o motivo e obrigatorio (vai no corpo JSON,
// convertido para query string pelo backend legado).
export function removerAnexo(id, motivo) {
  return apiSend('DELETE', `/anexos/${id}`, { motivo })
}

// --- LGPD: carteira de consentimentos e relatorio de acessos ----------------
// Tudo escopado ao paciente autenticado (rotas /me). O historico de
// consentimentos e imutavel: revogar nao apaga, apenas muda o status.

// Lista os consentimentos do proprio paciente (concedidos e revogados).
export function listarMeusConsentimentos() {
  return apiGet('/me/consentimentos')
}

// Revoga um consentimento proprio. O motivo e obrigatorio (acao sensivel) e
// vai no corpo JSON. So afeta consentimentos do paciente autenticado.
export function revogarConsentimento(id, motivo) {
  return apiSend('POST', `/me/consentimentos/${id}/revogar`, { motivo })
}

// Relatorio LGPD de acessos aos dados do proprio paciente (quem acessou o que,
// quando e com qual detalhe), derivado da trilha de auditoria.
export function listarMeuRelatorioAcessos() {
  return apiGet('/me/relatorio-acessos')
}

// --- Fila do medico: assumir o proximo e ver quem atender -------------------

// Fila de consulta que o medico pode assumir (proximos a atender).
export function listarFilaConsulta() {
  return apiGet('/me/fila')
}

// O medico assume um check-in da fila (vincula o atendimento a ele).
export function assumirCheckin(id) {
  return apiSend('POST', `/checkins/${id}/assumir`)
}

// Pacientes que o medico ja assumiu e deve atender agora.
export function listarMeusAtendimentos() {
  return apiGet('/me/atendimentos')
}

// --- Notificacoes in-app (todos os papeis) ----------------------------------

// Lista as notificacoes do usuario autenticado (mais recentes primeiro).
export function listarNotificacoes() {
  return apiGet('/me/notificacoes')
}

// Conta as notificacoes nao lidas: { naoLidas }.
export function contarNotificacoes() {
  return apiGet('/me/notificacoes/contar')
}

// Marca uma notificacao como lida.
export function marcarNotificacaoLida(id) {
  return apiSend('POST', `/me/notificacoes/${id}/lida`)
}

// Marca todas as notificacoes do usuario como lidas.
export function marcarTodasNotificacoesLidas() {
  return apiSend('POST', '/me/notificacoes/lidas')
}
