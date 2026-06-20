import { Card } from './ui/card';
import { ChevronRight, LucideIcon } from 'lucide-react';
import { motion } from 'motion/react';

interface MenuCardProps {
  title: string;
  description: string;
  icon: LucideIcon;
  onClick: () => void;
  accentColor?: string;
}

export function MenuCard({ title, description, icon: Icon, onClick, accentColor = 'cyan' }: MenuCardProps) {
  const colorClasses = {
    cyan: 'from-cyan-500/20 to-cyan-900/20 border-cyan-500/30 hover:border-cyan-400/50 group-hover:text-cyan-400',
    green: 'from-green-500/20 to-green-900/20 border-green-500/30 hover:border-green-400/50 group-hover:text-green-400',
    blue: 'from-blue-500/20 to-blue-900/20 border-blue-500/30 hover:border-blue-400/50 group-hover:text-blue-400',
    purple: 'from-purple-500/20 to-purple-900/20 border-purple-500/30 hover:border-purple-400/50 group-hover:text-purple-400',
    orange: 'from-orange-500/20 to-orange-900/20 border-orange-500/30 hover:border-orange-400/50 group-hover:text-orange-400',
    red: 'from-red-500/20 to-red-900/20 border-red-500/30 hover:border-red-400/50 group-hover:text-red-400',
    yellow: 'from-yellow-500/20 to-yellow-900/20 border-yellow-500/30 hover:border-yellow-400/50 group-hover:text-yellow-400',
    pink: 'from-pink-500/20 to-pink-900/20 border-pink-500/30 hover:border-pink-400/50 group-hover:text-pink-400',
  };

  const iconColorClasses = {
    cyan: 'text-cyan-400',
    green: 'text-green-400',
    blue: 'text-blue-400',
    purple: 'text-purple-400',
    orange: 'text-orange-400',
    red: 'text-red-400',
    yellow: 'text-yellow-400',
    pink: 'text-pink-400',
  };

  return (
    <motion.div
      whileHover={{ scale: 1.02 }}
      whileTap={{ scale: 0.98 }}
    >
      <Card
        className={`group relative cursor-pointer overflow-hidden bg-gradient-to-br ${colorClasses[accentColor as keyof typeof colorClasses]} border transition-all duration-300 hover:shadow-lg`}
        onClick={onClick}
      >
        <div className="p-6">
          <div className="flex items-start justify-between">
            <div className="flex-1">
              <div className="flex items-center gap-3 mb-2">
                <Icon className={`size-6 ${iconColorClasses[accentColor as keyof typeof iconColorClasses]}`} />
                <h3 className="font-semibold text-white">{title}</h3>
              </div>
              <p className="text-sm text-gray-400">{description}</p>
            </div>
            <ChevronRight className="size-5 text-gray-500 transition-transform group-hover:translate-x-1 group-hover:text-white" />
          </div>
        </div>
        <div className="absolute inset-0 bg-gradient-to-r from-transparent via-white/5 to-transparent -translate-x-full group-hover:translate-x-full transition-transform duration-700" />
      </Card>
    </motion.div>
  );
}
