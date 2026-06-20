import { MenuCard } from './MenuCard';
import { 
  Wifi, 
  Bluetooth, 
  Target, 
  Radio, 
  Antenna,
  Zap,
  HardDrive,
  Settings
} from 'lucide-react';

interface MainMenuProps {
  onSelectModule: (moduleId: string) => void;
}

export function MainMenu({ onSelectModule }: MainMenuProps) {
  const modules = [
    {
      id: 'wifi',
      title: '1. Wi-Fi Tools',
      description: 'Scanning, deauth, AP creation, and packet capture',
      icon: Wifi,
      color: 'cyan'
    },
    {
      id: 'bluetooth',
      title: '2. Bluetooth & BLE Tools',
      description: 'BLE scanning, GATT operations, and Bluetooth attacks',
      icon: Bluetooth,
      color: 'blue'
    },
    {
      id: 'detectors',
      title: '3. Target Detectors',
      description: 'Device detection and target identification modules',
      icon: Target,
      color: 'red'
    },
    {
      id: 'rf-sub1ghz',
      title: '4. Sub-1GHz RF Tools',
      description: 'CC1101 module for sub-GHz radio frequency operations',
      icon: Radio,
      color: 'green'
    },
    {
      id: 'rf-24ghz',
      title: '5. 2.4GHz RF Tools',
      description: 'NRF24 module for 2.4GHz wireless communications',
      icon: Antenna,
      color: 'purple'
    },
    {
      id: 'infrared',
      title: '6. Infrared (IR) Tools',
      description: 'IR transmission, receiving, and remote control emulation',
      icon: Zap,
      color: 'orange'
    },
    {
      id: 'storage',
      title: '7. Storage, BadUSB & Scripting',
      description: 'File management, USB attacks, and script execution',
      icon: HardDrive,
      color: 'yellow'
    },
    {
      id: 'utilities',
      title: '8. Utilities & Core System',
      description: 'Clock, system info, settings, and reverse shell',
      icon: Settings,
      color: 'pink'
    }
  ];

  return (
    <div className="container mx-auto px-4 py-8">
      <div className="mb-8">
        <h1 className="text-3xl font-bold text-white mb-2">Main Menu</h1>
        <p className="text-gray-400">Select a module to access offensive security tools</p>
      </div>
      
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-4">
        {modules.map((module) => (
          <MenuCard
            key={module.id}
            title={module.title}
            description={module.description}
            icon={module.icon}
            accentColor={module.color}
            onClick={() => onSelectModule(module.id)}
          />
        ))}
      </div>
    </div>
  );
}
