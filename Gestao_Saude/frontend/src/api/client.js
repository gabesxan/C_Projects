// Cliente HTTP do SIGEH-DF.
// - Prefixo "/api" e reescrito pelo proxy do Vite para o backend em C.
// - Autenticacao HTTP Basic: o header vai em toda requisicao.
// - A API le parametros pela QUERY STRING, inclusive nos POST.

const BASE = '/api'

// Credencial em memoria; persistida em sessionStorage para sobreviver a reload.
let credentials = null

export function setCredentials(login, senha) {
  const basic = 'Basic ' + btoa(`${login}:${senha}`)
  credentials = { login, basic }
  sessionStorage.setItem('sigeh_auth', JSON.stringify({ login, senha }))
}

export function loadCredentials() {
  const raw = sessionStorage.getItem('sigeh_auth')
  if (raw) {
    const { login, senha } = JSON.parse(raw)
    setCredentials(login, senha)
  }
  return credentials
}

export function clearCredentials() {
  credentials = null
  sessionStorage.removeItem('sigeh_auth')
}

function authHeaders() {
  return credentials ? { Authorization: credentials.basic } : {}
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

// GET autenticado que devolve o JSON ja parseado.
export async function apiGet(path) {
  const res = await fetch(BASE + path, { headers: { ...authHeaders() } })
  if (!res.ok) throw new ApiError(res.status, await safeJson(res))
  return res.json()
}

// POST/DELETE: os parametros viram query string (contrato do backend).
export async function apiSend(method, path, params = {}) {
  const qs = new URLSearchParams(params).toString()
  const url = BASE + path + (qs ? `?${qs}` : '')
  const res = await fetch(url, { method, headers: { ...authHeaders() } })
  if (!res.ok) throw new ApiError(res.status, await safeJson(res))
  return safeJson(res)
}
