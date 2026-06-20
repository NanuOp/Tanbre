import { useState } from 'react';
import { OLEDDisplay } from './components/OLEDDisplay';

export default function App() {
  return (
    <div className="min-h-screen bg-gray-950 flex items-center justify-center p-8">
      <OLEDDisplay />
    </div>
  );
}
