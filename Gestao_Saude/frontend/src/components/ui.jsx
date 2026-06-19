// Primitivos visuais compartilhados do SIGEH-DF.
// Tema hospitalar: alto contraste, badges de estado, feedback claro e
// indicacao de proxima acao. Tudo via classes utilitarias do Tailwind.

// --- Papeis: rotulo legivel + tom de cor para badge ---------------------
export const PAPEL_INFO = {
  ADMIN: { label: 'Administrador', tone: 'violet' },
  CADASTRO: { label: 'Cadastro', tone: 'sky' },
  MEDICO: { label: 'Médico', tone: 'teal' },
  ENFERMAGEM: { label: 'Enfermagem', tone: 'emerald' },
  PACIENTE: { label: 'Paciente', tone: 'amber' },
}

export function papelLabel(papel) {
  return PAPEL_INFO[papel]?.label ?? papel
}

const TONES = {
  slate: 'bg-slate-100 text-slate-700 ring-slate-200 dark:bg-slate-800 dark:text-slate-200 dark:ring-slate-600',
  teal: 'bg-teal-100 text-teal-800 ring-teal-200 dark:bg-teal-950 dark:text-teal-200 dark:ring-teal-700',
  emerald: 'bg-emerald-100 text-emerald-800 ring-emerald-200 dark:bg-emerald-950 dark:text-emerald-200 dark:ring-emerald-700',
  sky: 'bg-sky-100 text-sky-800 ring-sky-200 dark:bg-sky-950 dark:text-sky-200 dark:ring-sky-700',
  violet: 'bg-violet-100 text-violet-800 ring-violet-200 dark:bg-violet-950 dark:text-violet-200 dark:ring-violet-700',
  amber: 'bg-amber-100 text-amber-900 ring-amber-200 dark:bg-amber-950 dark:text-amber-200 dark:ring-amber-700',
  red: 'bg-red-100 text-red-800 ring-red-200 dark:bg-red-950 dark:text-red-200 dark:ring-red-700',
  green: 'bg-green-100 text-green-800 ring-green-200 dark:bg-green-950 dark:text-green-200 dark:ring-green-700',
}

export function Badge({ tone = 'slate', children }) {
  return (
    <span
      className={`inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-semibold ring-1 ring-inset ${
        TONES[tone] ?? TONES.slate
      }`}
    >
      {children}
    </span>
  )
}

// Badge de status ativo/inativo a partir de um booleano/0-1.
export function StatusBadge({ ativo }) {
  const on = ativo === true || ativo === 1
  return <Badge tone={on ? 'green' : 'red'}>{on ? 'Ativo' : 'Inativo'}</Badge>
}

export function Card({ children, className = '' }) {
  return (
    <div className={`rounded-lg bg-white shadow-sm ring-1 ring-slate-200 dark:bg-slate-900 dark:ring-slate-700 ${className}`}>
      {children}
    </div>
  )
}

// Cartao de indicador (KPI) com rotulo, valor e dica opcional.
export function StatCard({ label, value, hint }) {
  return (
    <Card className="p-5">
      <p className="text-sm font-medium text-slate-500 dark:text-slate-400">{label}</p>
      <p className="mt-1 text-3xl font-bold text-slate-900 dark:text-white">
        {value ?? <span className="text-slate-300">—</span>}
      </p>
      {hint && <p className="mt-2 text-xs font-medium text-teal-700 dark:text-teal-300">{hint}</p>}
    </Card>
  )
}

export function PageHeader({ title, subtitle, actions }) {
  return (
    <div className="flex flex-wrap items-end justify-between gap-3">
      <div>
        <h1 className="text-2xl font-bold tracking-tight text-slate-900 dark:text-white">{title}</h1>
        {subtitle && <p className="mt-1 text-sm text-slate-500 dark:text-slate-400">{subtitle}</p>}
      </div>
      {actions && <div className="flex items-center gap-2">{actions}</div>}
    </div>
  )
}

export function Button({ variant = 'primary', className = '', ...props }) {
  const base =
    'inline-flex items-center justify-center gap-1.5 rounded-lg px-4 py-2 text-sm font-semibold transition disabled:opacity-60 disabled:cursor-not-allowed focus:outline-none focus-visible:ring-2 focus-visible:ring-offset-1'
  const variants = {
    primary: 'bg-teal-600 text-white hover:bg-teal-700 focus-visible:ring-teal-500',
    secondary:
      'bg-white text-slate-700 ring-1 ring-inset ring-slate-300 hover:bg-slate-50 focus-visible:ring-slate-400 dark:bg-slate-900 dark:text-slate-100 dark:ring-slate-600 dark:hover:bg-slate-800',
    danger:
      'bg-white text-red-700 ring-1 ring-inset ring-red-200 hover:bg-red-50 focus-visible:ring-red-400 dark:bg-slate-900 dark:text-red-200 dark:ring-red-700 dark:hover:bg-red-950',
  }
  return <button className={`${base} ${variants[variant]} ${className}`} {...props} />
}

export function Alert({ tone = 'red', children }) {
  const tones = {
    red: 'bg-red-50 text-red-800 ring-red-200 dark:bg-red-950 dark:text-red-100 dark:ring-red-800',
    amber: 'bg-amber-50 text-amber-900 ring-amber-200 dark:bg-amber-950 dark:text-amber-100 dark:ring-amber-800',
    teal: 'bg-teal-50 text-teal-800 ring-teal-200 dark:bg-teal-950 dark:text-teal-100 dark:ring-teal-800',
  }
  return (
    <div className={`rounded-lg px-4 py-3 text-sm ring-1 ring-inset ${tones[tone]}`}>
      {children}
    </div>
  )
}

export function Spinner({ label = 'Carregando...' }) {
  return (
    <div className="flex items-center gap-2 text-sm text-slate-400 dark:text-slate-300">
      <span className="h-4 w-4 animate-spin rounded-full border-2 border-slate-300 border-t-teal-600" />
      {label}
    </div>
  )
}

// Estado vazio com icone, titulo e (opcional) proxima acao.
export function EmptyState({ title, description, action }) {
  return (
    <Card className="flex flex-col items-center gap-2 px-6 py-12 text-center">
      <div className="flex h-12 w-12 items-center justify-center rounded-full bg-slate-100 text-2xl dark:bg-slate-800">
        📋
      </div>
      <p className="font-semibold text-slate-700">{title}</p>
      {description && <p className="max-w-md text-sm text-slate-500">{description}</p>}
      {action && <div className="mt-2">{action}</div>}
    </Card>
  )
}
