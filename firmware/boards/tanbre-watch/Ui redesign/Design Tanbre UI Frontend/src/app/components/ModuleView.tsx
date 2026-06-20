import { Button } from './ui/button';
import { Card } from './ui/card';
import { ArrowLeft, Play, Square, RefreshCw } from 'lucide-react';
import { Tabs, TabsContent, TabsList, TabsTrigger } from './ui/tabs';
import { Badge } from './ui/badge';

interface ModuleViewProps {
  moduleId: string;
  onBack: () => void;
}

interface SubModule {
  name: string;
  status: 'idle' | 'running' | 'ready';
  description: string;
}

const moduleData: Record<string, { title: string; subModules: SubModule[] }> = {
  wifi: {
    title: 'Wi-Fi Tools',
    subModules: [
      { name: 'Wi-Fi Scanner', status: 'ready', description: 'Scan for nearby access points and stations' },
      { name: 'Deauthentication Attack', status: 'idle', description: 'Disconnect clients from access points' },
      { name: 'Evil Twin AP', status: 'idle', description: 'Create rogue access point' },
      { name: 'Packet Monitor', status: 'idle', description: 'Capture and analyze Wi-Fi packets' },
      { name: 'Handshake Capture', status: 'idle', description: 'Capture WPA/WPA2 handshakes' },
      { name: 'Beacon Spam', status: 'idle', description: 'Broadcast fake SSIDs' }
    ]
  },
  bluetooth: {
    title: 'Bluetooth & BLE Tools',
    subModules: [
      { name: 'BLE Scanner', status: 'ready', description: 'Scan for BLE devices and services' },
      { name: 'GATT Browser', status: 'idle', description: 'Explore GATT characteristics' },
      { name: 'BLE Sniffer', status: 'idle', description: 'Monitor BLE communications' },
      { name: 'Bluetooth Jammer', status: 'idle', description: 'Jam Bluetooth signals' },
      { name: 'Apple AirTag Spoof', status: 'idle', description: 'Spoof Apple AirTag broadcasts' }
    ]
  },
  detectors: {
    title: 'Target Detectors',
    subModules: [
      { name: 'Camera Detector', status: 'ready', description: 'Detect hidden cameras via lens reflection' },
      { name: 'Bug Detector', status: 'idle', description: 'RF bug and transmitter detection' },
      { name: 'GPS Tracker Detector', status: 'idle', description: 'Find GPS tracking devices' },
      { name: 'Wireless Device Scanner', status: 'idle', description: 'Detect all wireless transmitters' }
    ]
  },
  'rf-sub1ghz': {
    title: 'Sub-1GHz RF Tools (CC1101)',
    subModules: [
      { name: 'RF Signal Scanner', status: 'ready', description: 'Scan 300-928 MHz frequencies' },
      { name: 'Signal Analyzer', status: 'idle', description: 'Analyze captured RF signals' },
      { name: 'Signal Replay', status: 'idle', description: 'Replay captured signals' },
      { name: 'Garage Door Attack', status: 'idle', description: 'Rolling code attack' },
      { name: 'Car Key Fob Capture', status: 'idle', description: 'Capture and replay key fobs' }
    ]
  },
  'rf-24ghz': {
    title: '2.4GHz RF Tools (NRF24)',
    subModules: [
      { name: '2.4GHz Scanner', status: 'ready', description: 'Scan 2.4GHz spectrum' },
      { name: 'Mouse Jacker', status: 'idle', description: 'Attack wireless keyboards/mice' },
      { name: 'Drone Controller', status: 'idle', description: 'Control 2.4GHz drones' },
      { name: 'Packet Injection', status: 'idle', description: 'Inject custom packets' }
    ]
  },
  infrared: {
    title: 'Infrared (IR) Tools',
    subModules: [
      { name: 'IR Receiver', status: 'ready', description: 'Capture IR remote signals' },
      { name: 'IR Transmitter', status: 'idle', description: 'Send IR commands' },
      { name: 'Universal Remote', status: 'idle', description: 'Database of IR codes' },
      { name: 'TV-B-Gone', status: 'idle', description: 'Turn off TVs automatically' }
    ]
  },
  storage: {
    title: 'Storage, BadUSB & Scripting',
    subModules: [
      { name: 'File Browser', status: 'ready', description: 'Browse SD card and flash storage' },
      { name: 'BadUSB Payloads', status: 'idle', description: 'Execute HID attack scripts' },
      { name: 'Rubber Ducky Scripts', status: 'idle', description: 'Run Ducky Script payloads' },
      { name: 'Script Editor', status: 'idle', description: 'Create and edit scripts' },
      { name: 'Payload Library', status: 'idle', description: 'Manage attack payloads' }
    ]
  },
  utilities: {
    title: 'Utilities & Core System',
    subModules: [
      { name: 'Clock / Time', status: 'ready', description: 'RTC and NTP time sync' },
      { name: 'System Info', status: 'ready', description: 'CPU, RAM, flash, battery stats' },
      { name: 'Settings Menu', status: 'ready', description: 'Configure device settings' },
      { name: 'Reverse Shell', status: 'idle', description: 'Remote terminal access' },
      { name: 'Firmware Update', status: 'idle', description: 'OTA firmware updates' },
      { name: 'LED Control', status: 'idle', description: 'Configure LED indicators' }
    ]
  }
};

export function ModuleView({ moduleId, onBack }: ModuleViewProps) {
  const module = moduleData[moduleId];

  if (!module) {
    return (
      <div className="container mx-auto px-4 py-8">
        <Button onClick={onBack} variant="ghost" className="mb-4 text-cyan-400">
          <ArrowLeft className="size-4 mr-2" />
          Back to Menu
        </Button>
        <div className="text-white">Module not found</div>
      </div>
    );
  }

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'running': return 'bg-green-500';
      case 'ready': return 'bg-cyan-500';
      default: return 'bg-gray-500';
    }
  };

  const getStatusBadge = (status: string) => {
    switch (status) {
      case 'running': return <Badge className="bg-green-500/20 text-green-400 border-green-500/50">Running</Badge>;
      case 'ready': return <Badge className="bg-cyan-500/20 text-cyan-400 border-cyan-500/50">Ready</Badge>;
      default: return <Badge variant="outline" className="text-gray-400">Idle</Badge>;
    }
  };

  return (
    <div className="container mx-auto px-4 py-8">
      <Button 
        onClick={onBack} 
        variant="ghost" 
        className="mb-6 text-cyan-400 hover:text-cyan-300 hover:bg-cyan-900/20"
      >
        <ArrowLeft className="size-4 mr-2" />
        Back to Menu
      </Button>

      <div className="mb-8">
        <h1 className="text-3xl font-bold text-white mb-2">{module.title}</h1>
        <p className="text-gray-400">Available tools and operations</p>
      </div>

      <Tabs defaultValue="tools" className="w-full">
        <TabsList className="bg-black/40 border border-cyan-900/50">
          <TabsTrigger value="tools" className="data-[state=active]:bg-cyan-900/30 data-[state=active]:text-cyan-400">
            Tools
          </TabsTrigger>
          <TabsTrigger value="logs" className="data-[state=active]:bg-cyan-900/30 data-[state=active]:text-cyan-400">
            Logs
          </TabsTrigger>
          <TabsTrigger value="settings" className="data-[state=active]:bg-cyan-900/30 data-[state=active]:text-cyan-400">
            Settings
          </TabsTrigger>
        </TabsList>

        <TabsContent value="tools" className="mt-6">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {module.subModules.map((subModule, index) => (
              <Card 
                key={index}
                className="bg-black/40 border-gray-700 hover:border-cyan-500/50 transition-colors"
              >
                <div className="p-5">
                  <div className="flex items-start justify-between mb-3">
                    <div className="flex items-center gap-2">
                      <div className={`size-2 rounded-full ${getStatusColor(subModule.status)}`} />
                      <h3 className="font-semibold text-white">{subModule.name}</h3>
                    </div>
                    {getStatusBadge(subModule.status)}
                  </div>
                  <p className="text-sm text-gray-400 mb-4">{subModule.description}</p>
                  <div className="flex gap-2">
                    {subModule.status === 'running' ? (
                      <Button size="sm" variant="destructive" className="flex-1">
                        <Square className="size-3 mr-2" />
                        Stop
                      </Button>
                    ) : (
                      <Button size="sm" className="flex-1 bg-cyan-600 hover:bg-cyan-700">
                        <Play className="size-3 mr-2" />
                        Start
                      </Button>
                    )}
                    <Button size="sm" variant="outline" className="border-gray-600">
                      <RefreshCw className="size-3" />
                    </Button>
                  </div>
                </div>
              </Card>
            ))}
          </div>
        </TabsContent>

        <TabsContent value="logs" className="mt-6">
          <Card className="bg-black/60 border-gray-700">
            <div className="p-6">
              <div className="font-sans text-sm space-y-1">
                <div className="text-green-400">[12:34:56] System initialized</div>
                <div className="text-cyan-400">[12:34:57] {module.title} module loaded</div>
                <div className="text-yellow-400">[12:34:58] Awaiting user input...</div>
                <div className="text-gray-500">[12:35:00] Ready for operations</div>
              </div>
            </div>
          </Card>
        </TabsContent>

        <TabsContent value="settings" className="mt-6">
          <Card className="bg-black/40 border-gray-700">
            <div className="p-6">
              <h3 className="text-white font-semibold mb-4">Module Settings</h3>
              <div className="space-y-4 text-gray-300">
                <div className="flex items-center justify-between">
                  <span>Auto-start on boot</span>
                  <div className="size-10 rounded bg-gray-700" />
                </div>
                <div className="flex items-center justify-between">
                  <span>Verbose logging</span>
                  <div className="size-10 rounded bg-gray-700" />
                </div>
                <div className="flex items-center justify-between">
                  <span>Save results to SD</span>
                  <div className="size-10 rounded bg-gray-700" />
                </div>
              </div>
            </div>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  );
}
