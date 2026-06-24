/* eslint-disable react-refresh/only-export-components --
   co-locacao intencional do mapa ICONS com o componente Icon. */
import {
  Activity,
  AlertTriangle,
  BadgeDollarSign,
  Bed,
  Bell,
  CalendarDays,
  ClipboardCheck,
  ClipboardList,
  CreditCard,
  FileText,
  HeartPulse,
  Home,
  Hospital,
  KeyRound,
  LayoutDashboard,
  Mail,
  MapPin,
  Menu,
  Moon,
  Pill,
  Phone,
  Search,
  ShieldCheck,
  Stethoscope,
  SunMedium,
  Syringe,
  TestTube2,
  Clock3,
  User,
  UserCog,
  Users,
  X,
} from 'lucide-react'

export const ICONS = {
  dashboard: LayoutDashboard,
  bell: Bell,
  health: HeartPulse,
  reception: ClipboardCheck,
  patient: User,
  patients: Users,
  doctor: Stethoscope,
  triage: Activity,
  schedule: CalendarDays,
  record: FileText,
  lab: TestTube2,
  prescription: Pill,
  vaccine: Syringe,
  admission: Hospital,
  bed: Bed,
  finance: BadgeDollarSign,
  reports: ClipboardList,
  users: UserCog,
  audit: ShieldCheck,
  password: KeyRound,
  hospital: Hospital,
  alert: AlertTriangle,
  billing: CreditCard,
  search: Search,
  menu: Menu,
  close: X,
  moon: Moon,
  sun: SunMedium,
  home: Home,
  mail: Mail,
  phone: Phone,
  location: MapPin,
  clock: Clock3,
}

export function Icon({ icon: IconComponent, className = '', size = 18, strokeWidth = 2, ...props }) {
  if (!IconComponent) return null
  return (
    <IconComponent
      aria-hidden="true"
      className={className}
      size={size}
      strokeWidth={strokeWidth}
      {...props}
    />
  )
}
