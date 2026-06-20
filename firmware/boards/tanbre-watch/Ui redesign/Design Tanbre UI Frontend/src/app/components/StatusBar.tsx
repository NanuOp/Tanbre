import { Wifi, Bluetooth, Radio, Zap, HardDrive, Settings } from 'lucide-react';

interface StatusBarProps {
  time: Date;
  battery: number;
  activeModule: string | null;
}

const MODULE_ICONS: Record<string, React.ReactNode> = {
  wifi: <Wifi className="w-3 h-3" />,
  bluetooth: <Bluetooth className="w-3 h-3" />,
  detector: <Zap className="w-3 h-3" />,
  sub1ghz: <Radio className="w-3 h-3" />,
  rf24: <Radio className="w-3 h-3" />,
  ir: <Zap className="w-3 h-3" />,
  storage: <HardDrive className="w-3 h-3" />,
  utils: <Settings className="w-3 h-3" />
};

export function StatusBar({ time, battery, activeModule }: StatusBarProps) {
  const timeString = time.toLocaleTimeString('en-US', {
    hour: '2-digit',
    minute: '2-digit',
    hour12: false
  });

  return (
    <div className="absolute top-0 left-0 right-0 h-[54px] border-b-2 border-white/20 bg-black px-3 flex items-center justify-between">
      {/* Left: Clock and Active Module */}
      <div className="flex items-center gap-3">
        {/* Clock */}
        <div className="text-white font-sans" style={{ fontSize: '16px' }}>
          {timeString}
        </div>

        {/* Active Module Icon */}
        {activeModule && (
          <div className="flex items-center gap-1 text-white/80">
            <div className="scale-150">
              {MODULE_ICONS[activeModule]}
            </div>
          </div>
        )}
      </div>

      {/* Right: Battery */}
      <div className="text-white font-sans" style={{ fontSize: '16px' }}>
        {battery}
      </div>
    </div>
  );
}
