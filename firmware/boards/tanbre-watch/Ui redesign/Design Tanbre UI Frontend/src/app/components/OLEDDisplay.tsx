import { useState, useEffect } from 'react';
import { StatusBar } from './StatusBar';
import { MainMenuOS } from './MainMenuOS';
import { SubMenuOS } from './SubMenuOS';

const MODULES = [
  { id: 'wifi', name: 'WiFi Tools', icon: 'WIFI' },
  { id: 'bluetooth', name: 'Bluetooth', icon: 'BT' },
  { id: 'detector', name: 'Detectors', icon: 'DET' },
  { id: 'sub1ghz', name: 'Sub-1GHz', icon: 'S1G' },
  { id: 'rf24', name: '2.4GHz RF', icon: '24G' },
  { id: 'ir', name: 'Infrared', icon: 'IR' },
  { id: 'storage', name: 'Storage', icon: 'SD' },
  { id: 'utils', name: 'Utilities', icon: 'SYS' }
];

export function OLEDDisplay() {
  const [currentTime, setCurrentTime] = useState(new Date());
  const [battery, setBattery] = useState(87);
  const [activeModule, setActiveModule] = useState<string | null>(null);
  const [selectedModule, setSelectedModule] = useState<string | null>(null);
  const [isBooting, setIsBooting] = useState(true);
  const [bootProgress, setBootProgress] = useState(0);

  useEffect(() => {
    const timer = setInterval(() => {
      setCurrentTime(new Date());
    }, 1000);
    return () => clearInterval(timer);
  }, []);

  useEffect(() => {
    if (isBooting) {
      setBootProgress(0);
      const interval = setInterval(() => {
        setBootProgress((prev) => {
          if (prev >= 100) {
            clearInterval(interval);
            setTimeout(() => {
              setIsBooting(false);
            }, 300); // Small pause at 100%
            return 100;
          }
          return prev + 5;
        });
      }, 80); // ~1.6 seconds total boot time
      return () => clearInterval(interval);
    }
  }, [isBooting]);

  const handleSelectModule = (moduleId: string) => {
    setActiveModule(moduleId);
    setSelectedModule(moduleId);
  };

  const handleBack = () => {
    setSelectedModule(null);
  };

  const triggerReboot = () => {
    setIsBooting(true);
  };

  return (
    <div className="flex flex-col items-center gap-6">
      {/* OLED Screen - scaled 6x for visibility */}
      <div
        className="bg-black border-4 border-gray-700 rounded-lg shadow-2xl relative cursor-pointer"
        title="Click screen to reboot simulation"
        onClick={isBooting ? undefined : triggerReboot}
        style={{
          width: '768px',  // 128 * 6
          height: '384px'  // 64 * 6
        }}
      >
        <div
          className="relative w-full h-full bg-black overflow-hidden select-none"
          style={{
            imageRendering: 'pixelated'
          }}
        >
          {isBooting ? (
            /* Boot Screen - Designed to match 128x64 display coordinates (scaled 6x) */
            <div className="absolute inset-0 bg-black font-sans flex flex-col justify-start">
              {/* "TANBRE" centered at Y = 16 (scaled 6x = 96px) */}
              <div 
                className="absolute left-0 right-0 text-center text-white font-bold tracking-widest"
                style={{ top: '96px', fontSize: '36px', lineHeight: '36px' }}
              >
                TANBRE
              </div>

              {/* "BY G.SHIVPRASAD" centered at Y = 38 (scaled 6x = 228px) */}
              <div 
                className="absolute left-0 right-0 text-center text-white/80 tracking-wider"
                style={{ top: '228px', fontSize: '16px', lineHeight: '16px' }}
              >
                BY G.SHIVPRASAD
              </div>
            </div>
          ) : (
            <>
              {/* Status Bar - always visible */}
              <StatusBar
                time={currentTime}
                battery={battery}
                activeModule={activeModule}
              />

              {/* Main Content Area */}
              <div className="absolute top-[60px] left-0 right-0 bottom-0">
                {selectedModule ? (
                  <SubMenuOS
                    moduleId={selectedModule}
                    onBack={handleBack}
                  />
                ) : (
                  <MainMenuOS
                    modules={MODULES}
                    onSelect={handleSelectModule}
                  />
                )}
              </div>
            </>
          )}
        </div>
      </div>

      {/* Control hints */}
      <div className="flex gap-8 text-gray-500 text-sm font-sans">
        <div>UPPER: Short=Next | Long=Back</div>
        <div>LOWER: Short=Select | Long=Exit</div>
        <div className="text-cyan-600 font-semibold cursor-pointer hover:underline" onClick={triggerReboot}>[Click OLED to Reboot]</div>
      </div>
    </div>
  );
}
