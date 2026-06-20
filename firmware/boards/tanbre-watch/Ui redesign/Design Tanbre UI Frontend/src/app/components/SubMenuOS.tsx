import { useState, useEffect } from 'react';

const SUBTOOLS: Record<string, string[]> = {
  wifi: [
    'WiFi Scanner',
    'Client Sniffer',
    'Beacon Spammer',
    'Deauth Detector',
    'Evil Portal',
    'PMKID Capture'
  ],
  bluetooth: [
    'BT Scanner',
    'BLE Scanner',
    'MAC Spoof',
    'Jammer',
    'Sniffer',
    'Spam Attack'
  ],
  detector: [
    'WiFi Detector',
    'BT Detector',
    'Camera Finder',
    'GPS Tracker',
    'Bug Sweeper',
    'RF Analyzer'
  ],
  sub1ghz: [
    'Raw Record',
    'Replay Attack',
    'Brute Force',
    'Frequency Scan',
    'Jammer',
    'Protocol List'
  ],
  rf24: [
    '2.4G Scanner',
    'MouseJack',
    'Nrf Sniffer',
    'Jammer',
    'Replay',
    'Inject'
  ],
  ir: [
    'IR Learn',
    'IR Send',
    'TV-B-Gone',
    'Universal DB',
    'Brute Force',
    'IR Sniffer'
  ],
  storage: [
    'File Manager',
    'BadUSB',
    'Scripts',
    'Payload Edit',
    'SD Card Info',
    'Backup/Restore'
  ],
  utils: [
    'Settings',
    'LED Control',
    'GPIO Test',
    'Battery Info',
    'System Info',
    'Update FW'
  ]
};

interface SubMenuOSProps {
  moduleId: string;
  onBack: () => void;
}

export function SubMenuOS({ moduleId, onBack }: SubMenuOSProps) {
  const tools = SUBTOOLS[moduleId] || [];
  const [currentIndex, setCurrentIndex] = useState(0);

  useEffect(() => {
    const handleKeyPress = (e: KeyboardEvent) => {
      if (e.key === 'ArrowLeft') {
        setCurrentIndex((prev) => (prev > 0 ? prev - 1 : tools.length - 1));
      } else if (e.key === 'ArrowRight') {
        setCurrentIndex((prev) => (prev < tools.length - 1 ? prev + 1 : 0));
      } else if (e.key === 'Escape') {
        onBack();
      }
    };

    window.addEventListener('keydown', handleKeyPress);
    return () => window.removeEventListener('keydown', handleKeyPress);
  }, [currentIndex, tools.length, onBack]);

  return (
    <div className="h-full flex items-center justify-center px-6">
      {/* Current tool only */}
      <div className="text-white font-sans text-center" style={{ fontSize: '18px' }}>
        {tools[currentIndex]}
      </div>
    </div>
  );
}
