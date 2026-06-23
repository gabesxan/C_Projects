import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor, fireEvent } from '@testing-library/react'
import userEvent from '@testing-library/user-event'

vi.mock('../api/client', () => ({
  listarAnexos: vi.fn(),
  enviarAnexo: vi.fn(),
  baixarAnexo: vi.fn(),
  removerAnexo: vi.fn(),
  arquivoParaBase64: vi.fn(),
}))
import {
  listarAnexos,
  enviarAnexo,
  baixarAnexo,
  removerAnexo,
  arquivoParaBase64,
} from '../api/client'
import AnexoPanel from './AnexoPanel'

function arquivoPdf(nome = 'laudo.pdf', tamanho = 1024) {
  const f = new File(['x'], nome, { type: 'application/pdf' })
  Object.defineProperty(f, 'size', { value: tamanho })
  return f
}

// fireEvent.change ignora o atributo accept do input, permitindo testar a
// validacao de MIME do componente de forma deterministica.
function selecionarArquivo(file) {
  fireEvent.change(document.querySelector('input[type=file]'), { target: { files: [file] } })
}

beforeEach(() => {
  vi.clearAllMocks()
  listarAnexos.mockResolvedValue([])
  arquivoParaBase64.mockResolvedValue('QkFTRTY0')
})

afterEach(() => {
  vi.restoreAllMocks()
})

describe('AnexoPanel', () => {
  it('renderiza titulo, formatos aceitos e limite', async () => {
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    expect(screen.getByText('Anexos')).toBeInTheDocument()
    expect(screen.getByText(/Formatos aceitos: PDF, PNG, JPEG/)).toBeInTheDocument()
    await waitFor(() => expect(listarAnexos).toHaveBeenCalledWith('exame', 1))
  })

  it('mostra estado vazio quando nao ha anexos', async () => {
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    expect(await screen.findByText('Nenhum anexo enviado.')).toBeInTheDocument()
  })

  it('renderiza um item da lista com nome, mime e autor', async () => {
    listarAnexos.mockResolvedValue([
      { id: 7, nome: 'laudo.pdf', mime: 'application/pdf', tamanho: 2048, autorLogin: 'ana', criadoEm: '2026-06-23' },
    ])
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    expect(await screen.findByText('laudo.pdf')).toBeInTheDocument()
    expect(screen.getByText('PDF')).toBeInTheDocument()
    expect(screen.getByText(/ana/)).toBeInTheDocument()
  })

  it('valida MIME nao permitido antes de enviar', async () => {
    const user = userEvent.setup()
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    await screen.findByText('Nenhum anexo enviado.')

    const txt = new File(['x'], 'nota.txt', { type: 'text/plain' })
    selecionarArquivo(txt)
    await user.click(screen.getByRole('button', { name: 'Enviar' }))

    expect(await screen.findByText('Formato não permitido.')).toBeInTheDocument()
    expect(enviarAnexo).not.toHaveBeenCalled()
  })

  it('valida tamanho acima de 5 MB antes de enviar', async () => {
    const user = userEvent.setup()
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    await screen.findByText('Nenhum anexo enviado.')

    const grande = arquivoPdf('grande.pdf', 6 * 1024 * 1024)
    selecionarArquivo(grande)
    await user.click(screen.getByRole('button', { name: 'Enviar' }))

    expect(await screen.findByText('Arquivo maior que 5 MB.')).toBeInTheDocument()
    expect(enviarAnexo).not.toHaveBeenCalled()
  })

  it('chama enviarAnexo com os dados corretos em um upload valido', async () => {
    enviarAnexo.mockResolvedValue({ id: 9, tamanho: 1024 })
    const user = userEvent.setup()
    render(<AnexoPanel entidade="exame" entidadeId={42} />)
    await screen.findByText('Nenhum anexo enviado.')

    selecionarArquivo(arquivoPdf())
    await user.click(screen.getByRole('button', { name: 'Enviar' }))

    await waitFor(() =>
      expect(enviarAnexo).toHaveBeenCalledWith({
        entidade: 'exame',
        entidadeId: 42,
        nome: 'laudo.pdf',
        mime: 'application/pdf',
        conteudoB64: 'QkFTRTY0',
      }),
    )
    expect(await screen.findByText('Anexo enviado com sucesso.')).toBeInTheDocument()
  })

  it('mostra mensagem amigavel no erro 413', async () => {
    enviarAnexo.mockRejectedValue({ status: 413, message: 'too large' })
    const user = userEvent.setup()
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    await screen.findByText('Nenhum anexo enviado.')

    selecionarArquivo(arquivoPdf())
    await user.click(screen.getByRole('button', { name: 'Enviar' }))

    expect(await screen.findByText('Arquivo maior que 5 MB.')).toBeInTheDocument()
  })

  it('exige motivo para remover', async () => {
    listarAnexos.mockResolvedValue([
      { id: 7, nome: 'laudo.pdf', mime: 'application/pdf', tamanho: 2048, autorLogin: 'ana', criadoEm: '2026-06-23' },
    ])
    const user = userEvent.setup()
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    await screen.findByText('laudo.pdf')

    // Sem motivo: nao chama o client.
    vi.spyOn(window, 'prompt').mockReturnValue('   ')
    await user.click(screen.getByRole('button', { name: 'Remover' }))
    expect(await screen.findByText('Informe o motivo da remoção.')).toBeInTheDocument()
    expect(removerAnexo).not.toHaveBeenCalled()

    // Com motivo: chama removerAnexo.
    window.prompt.mockReturnValue('documento duplicado')
    removerAnexo.mockResolvedValue({})
    await user.click(screen.getByRole('button', { name: 'Remover' }))
    await waitFor(() => expect(removerAnexo).toHaveBeenCalledWith(7, 'documento duplicado'))
  })

  it('baixa o anexo ao clicar em Baixar', async () => {
    listarAnexos.mockResolvedValue([
      { id: 7, nome: 'laudo.pdf', mime: 'application/pdf', tamanho: 2048, autorLogin: 'ana', criadoEm: '2026-06-23' },
    ])
    baixarAnexo.mockResolvedValue(undefined)
    const user = userEvent.setup()
    render(<AnexoPanel entidade="exame" entidadeId={1} />)
    await screen.findByText('laudo.pdf')

    await user.click(screen.getByRole('button', { name: 'Baixar' }))
    await waitFor(() => expect(baixarAnexo).toHaveBeenCalledWith(7))
  })

  it('em modo disabled nao mostra envio nem remocao', async () => {
    listarAnexos.mockResolvedValue([
      { id: 7, nome: 'laudo.pdf', mime: 'application/pdf', tamanho: 2048, autorLogin: 'ana', criadoEm: '2026-06-23' },
    ])
    render(<AnexoPanel entidade="exame" entidadeId={1} disabled />)
    await screen.findByText('laudo.pdf')
    expect(screen.queryByRole('button', { name: 'Enviar' })).not.toBeInTheDocument()
    expect(screen.queryByRole('button', { name: 'Remover' })).not.toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'Baixar' })).toBeInTheDocument()
  })
})
