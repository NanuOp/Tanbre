import { useState, useEffect } from 'react';

interface Module {
  id: string;
  name: string;
  icon: string;
}

interface MainMenuOSProps {
  modules: Module[];
  onSelect: (moduleId: string) => void;
}

export function MainMenuOS({ modules, onSelect }: MainMenuOSProps) {
  const [currentIndex, setCurrentIndex] = useState(0);

  useEffect(() => {
    const handleKeyPress = (e: KeyboardEvent) => {
      if (e.key === 'ArrowLeft') {
        setCurrentIndex((prev) => (prev > 0 ? prev - 1 : modules.length - 1));
      } else if (e.key === 'ArrowRight') {
        setCurrentIndex((prev) => (prev < modules.length - 1 ? prev + 1 : 0));
      } else if (e.key === 'Enter') {
        onSelect(modules[currentIndex].id);
      }
    };

    window.addEventListener('keydown', handleKeyPress);
    return () => window.removeEventListener('keydown', handleKeyPress);
  }, [currentIndex, modules, onSelect]);

  const currentModule = modules[currentIndex];

  return (
    <div className="h-full flex items-center justify-center px-6">
      {/* Module Name Only */}
      <div className="text-white font-sans text-center" style={{ fontSize: '20px' }}>
        {currentModule.name}
      </div>
    </div>
  );
}
