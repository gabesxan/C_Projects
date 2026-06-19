import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'
import { formatReais } from '../money'
import { PageHeader, Spinner, Card, Badge, Button, EmptyState } from '../components/ui'

const STATUS_COBRANCA_TONE = {
  PENDENTE: 'amber',
  AUTORIZADA: 'sky',
  PAGA: 'green',
  GLOSADA: 'red',
  CANCELADA: 'slate',
}

function useApi(path) {
  const [data, setData] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet(path).then(setData).catch((e) => setErro(e.message))
  }, [path])

  return { data, erro }
}

function Carteirinha({ perfil }) {
  return (
    <Card className="overflow-hidden">
      <div className="bg-teal-700 px-5 py-4 text-white dark:bg-teal-800">
        <div className="flex items-start justify-between gap-4">
          <div>
            <p className="text-xs font-semibold uppercase text-teal-100">Carteirinha digital</p>
            <p className="mt-1 text-2xl font-bold">{perfil.nome}</p>
            <p className="text-sm text-teal-100">Paciente #{perfil.id}</p>
          </div>
          <div className="flex h-12 w-12 items-center justify-center rounded-lg bg-white/15 text-2xl">⚕</div>
        </div>
      </div>
      <div className="grid gap-3 p-5 sm:grid-cols-2 lg:grid-cols-4">
        <Info label="Documento" value={`${perfil.tipoDocumento} ${perfil.documento}`} />
        <Info label="Idade" value={`${perfil.idade} anos`} />
        <Info label="Regiao" value={`RA ${perfil.regiaoAdministrativa}`} />
        <Info label="Status" value={perfil.ativo ? 'Ativo' : 'Inativo'} />
        <Info label="Convenio" value={perfil.convenioId ? `Convenio #${perfil.convenioId}` : 'Particular'} />
        <Info label="Telefone" value={perfil.telefone} />
        <Info label="Alergias" value={perfil.alergias || 'Sem alergias registradas'} />
      </div>
    </Card>
  )
}

function Info({ label, value }) {
  return (
    <div className="rounded-lg bg-slate-50 p-3 dark:bg-slate-800">
      <p className="text-xs text-slate-500">{label}</p>
      <p className="mt-1 text-sm font-semibold text-slate-900">{value}</p>
    </div>
  )
}

function AcoesPaciente() {
  return (
    <div className="grid gap-3 md:grid-cols-3">
      <Card className="p-5">
        <p className="text-sm font-semibold text-slate-800">Agendar consulta comum</p>
        <p className="mt-1 text-sm text-slate-500">Solicite um horario eletivo com a recepcao.</p>
        <Button className="mt-4" variant="secondary">Solicitar agendamento</Button>
      </Card>
      <Card className="p-5">
        <p className="text-sm font-semibold text-slate-800">Pedir atendimento</p>
        <p className="mt-1 text-sm text-slate-500">Para ajuda ou orientacao de fluxo, acione a equipe.</p>
        <Button className="mt-4">Pedir ajuda</Button>
      </Card>
      <Card className="p-5">
        <p className="text-sm font-semibold text-slate-800">Fale Conosco</p>
        <p className="mt-1 text-sm text-slate-500">atendimento@sigehdf.gov.br</p>
        <p className="text-sm text-slate-500">(61) 3333-0000</p>
      </Card>
    </div>
  )
}

function DicasExames() {
  const dicas = [
    'Leve documento com foto.',
    'Alguns exames precisam de jejum.',
    'Confira o resultado pelo sistema.',
    'Chegue com antecedencia.',
  ]
  return (
    <Card className="p-5">
      <p className="text-sm font-semibold text-slate-800">Dicas para exames</p>
      <div className="mt-3 grid gap-2 sm:grid-cols-2">
        {dicas.map((dica) => (
          <div key={dica} className="rounded-lg bg-slate-50 px-3 py-2 text-sm text-slate-600 dark:bg-slate-800">
            {dica}
          </div>
        ))}
      </div>
    </Card>
  )
}

function ContatoCompleto() {
  return (
    <Card className="p-5">
      <p className="text-sm font-semibold text-slate-800">Fale Conosco</p>
      <div className="mt-3 grid gap-3 sm:grid-cols-2 lg:grid-cols-4">
        <Info label="Email" value="atendimento@sigehdf.gov.br" />
        <Info label="Telefone" value="(61) 3333-0000" />
        <Info label="Endereco" value="Brasilia, DF" />
        <Info label="Horario" value="Segunda a sexta, 8h as 18h" />
      </div>
    </Card>
  )
}

function ListaCards({ titulo, rows, erro, render, vazio = 'Nenhum registro.' }) {
  return (
    <section className="space-y-2">
      <h2 className="text-sm font-semibold text-slate-700">{titulo}</h2>
      {erro && <p className="text-sm text-red-600">{erro}</p>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <EmptyState title={vazio} description="Quando houver novidades, elas aparecem aqui." />
      )}
      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <div className="grid gap-3 md:grid-cols-2">
          {rows.map(render)}
        </div>
      )}
    </section>
  )
}

function Cobrancas() {
  const { data, erro } = useApi('/me/cobrancas')
  return (
    <ListaCards
      titulo="Minhas cobrancas"
      rows={data}
      erro={erro}
      vazio="Nenhuma cobranca pendente."
      render={(c) => (
        <Card key={c.id} className="p-4">
          <div className="flex items-start justify-between gap-3">
            <div>
              <p className="text-sm font-semibold text-slate-900">{c.descricao || c.origem || `Cobranca #${c.id}`}</p>
              <p className="mt-1 text-xs text-slate-500">{c.forma}{c.vencimento ? ` · vence ${c.vencimento}` : ''}</p>
            </div>
            <Badge tone={STATUS_COBRANCA_TONE[c.status]}>{c.status}</Badge>
          </div>
          <p className="mt-3 text-lg font-bold text-slate-900">{formatReais(c.valorCentavos)}</p>
        </Card>
      )}
    />
  )
}

export default function MinhaSaude() {
  const perfil = useApi('/me/perfil')
  const agendamentos = useApi('/me/agendamentos')
  const receitas = useApi('/me/receitas')
  const exames = useApi('/me/exames')
  const prontuarios = useApi('/me/prontuarios')

  if (perfil.erro) return <p className="text-sm text-red-600">{perfil.erro}</p>
  if (!perfil.data) return <Spinner />

  return (
    <div className="space-y-6">
      <PageHeader
        title="Minha saude"
        subtitle="Carteirinha, consultas, exames, receitas e orientacoes. A triagem clinica e feita pela equipe autorizada."
      />
      <Carteirinha perfil={perfil.data} />
      <AcoesPaciente />
      <ListaCards
        titulo="Proximas consultas"
        rows={agendamentos.data}
        erro={agendamentos.erro}
        vazio="Nenhuma consulta agendada."
        render={(a) => (
          <Card key={a.id} className="p-4">
            <p className="text-sm font-semibold text-slate-900">{a.data} as {a.horario}</p>
            <p className="mt-1 text-xs text-slate-500">Medico #{a.medicoId}</p>
            <div className="mt-3"><Badge tone="sky">{a.status}</Badge></div>
          </Card>
        )}
      />
      <ListaCards
        titulo="Meus exames e resultados"
        rows={exames.data}
        erro={exames.erro}
        vazio="Nenhum exame pendente."
        render={(e) => (
          <Card key={e.id} className="p-4">
            <div className="flex items-start justify-between gap-3">
              <p className="text-sm font-semibold text-slate-900">Exame #{e.id} · Tipo {e.tipoExame}</p>
              <Badge tone={e.status === 'CONCLUIDO' ? 'green' : 'amber'}>{e.status}</Badge>
            </div>
            <p className="mt-2 text-sm text-slate-500">{e.resultado || 'Resultado ainda nao disponivel.'}</p>
          </Card>
        )}
      />
      <ListaCards
        titulo="Minhas receitas"
        rows={receitas.data}
        erro={receitas.erro}
        vazio="Nenhuma receita ativa."
        render={(r) => (
          <Card key={r.id} className="p-4">
            <p className="text-sm font-semibold text-slate-900">{r.medicamento}</p>
            <p className="mt-1 text-sm text-slate-500">{r.dosagem} · {r.frequencia}</p>
            {r.observacoes && <p className="mt-2 text-xs text-slate-500">{r.observacoes}</p>}
          </Card>
        )}
      />
      <ListaCards
        titulo="Orientacoes e prontuarios"
        rows={prontuarios.data}
        erro={prontuarios.erro}
        vazio="Nenhuma orientacao registrada."
        render={(p) => (
          <Card key={p.id} className="p-4">
            <p className="text-sm font-semibold text-slate-900">{p.data}</p>
            <p className="mt-1 text-sm text-slate-500">{p.conduta || p.diagnostico}</p>
          </Card>
        )}
      />
      <Cobrancas />
      <DicasExames />
      <ContatoCompleto />
    </div>
  )
}
