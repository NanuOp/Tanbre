import { useEffect, useState } from 'react';
import { Battery, Cpu, HardDrive, Clock } from 'lucide-react';

export function Header() {
  const [currentTime, setCurrentTime] = useState(new Date());
  const [systemStats] = useState({
    cpuFreq: '240 MHz',
    ramUsage: '68%',
    flashSize: '16 MB',
    batteryPercent: 87,
    batteryVoltage: '4.15V'
  });

  useEffect(() => {
    const timer = setInterval(() => {
      setCurrentTime(new Date());
    }, 1000);
    return () => clearInterval(timer);
  }, []);

  const getBatteryColor = (percent: number) => {
    if (percent > 60) return 'text-green-400';
    if (percent > 30) return 'text-yellow-400';
    return 'text-red-400';
  };

  return (
    <header className="border-b border-cyan-900/50 bg-black/40 backdrop-blur-sm">
      <div className="container mx-auto px-4 py-3">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="text-2xl font-bold bg-gradient-to-r from-cyan-400 to-green-400 bg-clip-text text-transparent">
              TANBRE
            </div>
            <div className="text-xs text-cyan-400/60">ESP32-S3</div>
          </div>

          <div className="flex items-center gap-6 text-sm">
            <div className="flex items-center gap-2 text-cyan-400">
              <Clock className="size-4" />
              <span className="font-sans">
                {currentTime.toLocaleTimeString('en-US', { hour12: false })}
              </span>
            </div>

            <div className="flex items-center gap-2 text-green-400">
              <Cpu className="size-4" />
              <span className="font-sans text-xs">{systemStats.cpuFreq}</span>
            </div>

            <div className="flex items-center gap-2 text-purple-400">
              <HardDrive className="size-4" />
              <span className="font-sans text-xs">RAM: {systemStats.ramUsage}</span>
            </div>

            <div className={`flex items-center gap-2 ${getBatteryColor(systemStats.batteryPercent)}`}>
              <Battery className="size-4" />
              <span className="font-sans text-xs">
                {systemStats.batteryPercent}% ({systemStats.batteryVoltage})
              </span>
            </div>
          </div>
        </div>
      </div>
    </header>
  );
}
