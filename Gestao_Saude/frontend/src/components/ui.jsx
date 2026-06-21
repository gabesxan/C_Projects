// Primitivos visuais compartilhados do SIGEH-DF.
// Tema hospitalar: alto contraste, badges de estado, feedback claro e
// indicacao de proxima acao. Tudo via classes utilitarias do Tailwind.
import { AlertTriangle, CheckCircle2, ClipboardList, Info, Loader2 } from 'lucide-react'

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
  slate: 'bg-slate-100/80 text-slate-700 ring-slate-200 dark:bg-slate-800/80 dark:text-slate-200 dark:ring-slate-600',
  teal: 'bg-teal-100/80 text-teal-800 ring-teal-200 dark:bg-teal-950/80 dark:text-teal-200 dark:ring-teal-700',
  emerald: 'bg-emerald-100/80 text-emerald-800 ring-emerald-200 dark:bg-emerald-950/80 dark:text-emerald-200 dark:ring-emerald-700',
  sky: 'bg-sky-100/80 text-sky-800 ring-sky-200 dark:bg-sky-950/80 dark:text-sky-200 dark:ring-sky-700',
  violet: 'bg-violet-100/80 text-violet-800 ring-violet-200 dark:bg-violet-950/80 dark:text-violet-200 dark:ring-violet-700',
  amber: 'bg-amber-100/80 text-amber-900 ring-amber-200 dark:bg-amber-950/80 dark:text-amber-200 dark:ring-amber-700',
  red: 'bg-red-100/80 text-red-800 ring-red-200 dark:bg-red-950/80 dark:text-red-200 dark:ring-red-700',
  green: 'bg-green-100/80 text-green-800 ring-green-200 dark:bg-green-950/80 dark:text-green-200 dark:ring-green-700',
}

export function Badge({ tone = 'slate', icon: Icon, children }) {
  return (
    <span
      className={`inline-flex items-center gap-1.5 rounded-full px-2.5 py-0.5 text-xs font-semibold ring-1 ring-inset ${
        TONES[tone] ?? TONES.slate
      }`}
    >
      {Icon && <Icon size={13} strokeWidth={2.2} />}
      {children}
    </span>
  )
}

// Badge de status ativo/inativo a partir de um booleano/0-1.
export function StatusBadge({ ativo }) {
  const on = ativo === true || ativo === 1
  return <Badge tone={on ? 'green' : 'red'} icon={on ? CheckCircle2 : AlertTriangle}>{on ? 'Ativo' : 'Inativo'}</Badge>
}

export function Card({ children, className = '' }) {
  return (
    <div className={`premium-card rounded-2xl bg-white/85 shadow-[0_18px_50px_rgba(15,23,42,0.07)] ring-1 ring-slate-200/70 backdrop-blur-xl transition-all duration-200 dark:bg-slate-900/78 dark:ring-white/10 ${className}`}>
      {children}
    </div>
  )
}

// Cartao de indicador (KPI) com rotulo, valor e dica opcional.
export function StatCard({ label, value, hint, icon: Icon, tone = 'teal' }) {
  return (
    <Card className="p-5">
      <div className="flex items-start justify-between gap-3">
        <p className="text-sm font-medium text-slate-500 dark:text-slate-400">{label}</p>
        {Icon && (
          <span className={`flex h-9 w-9 items-center justify-center rounded-xl ${TONES[tone] ?? TONES.teal}`}>
            <Icon size={18} />
          </span>
        )}
      </div>
      <p className="mt-2 text-3xl font-bold tracking-tight text-slate-950 dark:text-white">
        {value ?? <span className="text-slate-300">—</span>}
      </p>
      {hint && <p className="mt-2 text-xs font-medium text-teal-700 dark:text-teal-300">{hint}</p>}
    </Card>
  )
}

export function PageHeader({ title, subtitle, actions, align = 'left' }) {
  const centered = align === 'center'
  return (
    <div
      className={`animate-enter flex flex-wrap gap-3 ${
        centered ? 'items-center justify-center text-center' : 'items-end justify-between'
      }`}
    >
      <div className={centered ? 'mx-auto max-w-3xl' : ''}>
        <h1 className="text-2xl font-semibold tracking-tight text-slate-950 dark:text-white">{title}</h1>
        {subtitle && (
          <p className={`mt-1 text-sm text-slate-500 dark:text-slate-400 ${centered ? 'mx-auto max-w-2xl' : ''}`}>
            {subtitle}
          </p>
        )}
      </div>
      {actions && <div className={`flex items-center gap-2 ${centered ? 'w-full justify-center' : ''}`}>{actions}</div>}
    </div>
  )
}

export function Button({ variant = 'primary', className = '', as: Component = 'button', ...props }) {
  const base =
    'inline-flex items-center justify-center gap-2 rounded-xl px-4 py-2 text-sm font-semibold transition-all duration-200 disabled:cursor-not-allowed disabled:opacity-60 focus:outline-none focus-visible:ring-2 focus-visible:ring-offset-1 active:scale-[0.98]'
  const variants = {
    primary: 'bg-teal-600 text-white shadow-[0_12px_28px_rgba(13,148,136,0.28)] hover:bg-teal-700 focus-visible:ring-teal-500',
    secondary:
      'bg-white/80 text-slate-700 shadow-sm ring-1 ring-inset ring-slate-300/80 hover:bg-white focus-visible:ring-slate-400 dark:bg-slate-900/80 dark:text-slate-100 dark:ring-slate-600 dark:hover:bg-slate-800',
    danger:
      'bg-white/80 text-red-700 shadow-sm ring-1 ring-inset ring-red-200 hover:bg-red-50 focus-visible:ring-red-400 dark:bg-slate-900/80 dark:text-red-200 dark:ring-red-700 dark:hover:bg-red-950',
  }
  return <Component className={`${base} ${variants[variant]} ${className}`} {...props} />
}

export function Alert({ tone = 'red', children }) {
  const tones = {
    red: 'bg-red-50 text-red-800 ring-red-200 dark:bg-red-950 dark:text-red-100 dark:ring-red-800',
    amber: 'bg-amber-50 text-amber-900 ring-amber-200 dark:bg-amber-950 dark:text-amber-100 dark:ring-amber-800',
    teal: 'bg-teal-50 text-teal-800 ring-teal-200 dark:bg-teal-950 dark:text-teal-100 dark:ring-teal-800',
  }
  return (
    <div className={`flex items-start gap-2 rounded-2xl px-4 py-3 text-sm ring-1 ring-inset ${tones[tone]}`}>
      {tone === 'red' ? <AlertTriangle className="mt-0.5 shrink-0" size={16} /> : <Info className="mt-0.5 shrink-0" size={16} />}
      {children}
    </div>
  )
}

export function Spinner({ label = 'Carregando...' }) {
  return (
    <div className="flex items-center gap-2 text-sm text-slate-400 dark:text-slate-300">
      <Loader2 className="animate-spin text-teal-600" size={16} />
      {label}
    </div>
  )
}

// Estado vazio com icone, titulo e (opcional) proxima acao.
export function EmptyState({ title, description, action, icon: Icon = ClipboardList }) {
  return (
    <Card className="flex flex-col items-center gap-2 px-6 py-12 text-center">
      <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-slate-100 text-slate-500 shadow-inner dark:bg-slate-800 dark:text-slate-300">
        <Icon size={22} />
      </div>
      <p className="font-semibold text-slate-700">{title}</p>
      {description && <p className="max-w-md text-sm text-slate-500">{description}</p>}
      {action && <div className="mt-2">{action}</div>}
    </Card>
  )
}
