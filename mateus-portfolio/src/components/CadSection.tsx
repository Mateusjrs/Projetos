import React, { useState } from 'react';
import { ModelViewer } from './ModelViewer';
import { Box, Maximize2 } from 'lucide-react';

export const CadSection: React.FC = () => {
  const cadItems = [
    {
      title: 'Conjunto Estrutural Airbus',
      software: 'SolidWorks',
      description: 'Modelagem 3D detalhada de componente aeronáutico com análise de encaixes e montagem para visualização interativa.',
      modelPath: '/models/Airbus.glb',
      edrawingsUrl: '/models/Airbus.easm', // Link do eDrawings adicionado
    },
    {
      title: 'Estrutura Interna de Asa com Nervuras Aliviadas',
      software: 'CATIA / SolidWorks',
      description: 'Modelagem de asa para VANT com distribuição de nervuras otimizada e encaixes para longarinas de carbono.',
      modelPath: '/models/wing_structure.glb',
      // edrawingsUrl: '/models/wing_structure.easm', // Remova o comentário se tiver o arquivo
    },
    {
      title: 'Suporte de Célula de Carga para Bancada Aerodinâmica',
      software: 'SolidWorks',
      description: 'Estrutura mecânica projetada para isolar vibrações externas e registrar cargas de sustentação puras.',
      modelPath: '/models/loadcell_stand.glb',
      edrawingsUrl: '/models/loadcell_stand.easm', // Link do eDrawings adicionado
    },
  ];

  // Mudamos o estado para guardar o item completo ao invés de só o texto do caminho
  const [activeItem, setActiveItem] = useState(cadItems[0]);

  return (
    <section id="cad" className="py-24 bg-slate-950 border-b border-slate-800/80">
      <div className="max-w-7xl mx-auto px-6">
        <div className="flex flex-col md:flex-row md:items-end justify-between mb-12">
          <div>
            <div className="inline-flex items-center space-x-2 text-xs font-mono text-cyan-400 mb-2">
              <Box className="w-4 h-4" />
              <span>INTERFACING MECHANICAL & AEROSPACE DESIGN</span>
            </div>
            <h2 className="text-3xl sm:text-4xl font-extrabold text-white">3D & CAD GALLERY</h2>
          </div>
          <p className="text-slate-400 text-sm max-w-md mt-4 md:mt-0 font-sans">
            Explore modelos mecânicos e estruturais diretamente no navegador. Roteie, aproxime e examine os componentes.
          </p>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-12 gap-8 items-center">
          {/* Main Large Viewer */}
          <div className="lg:col-span-7 flex flex-col space-y-3">
            <div className="flex items-center justify-between">
              <h3 className="text-sm font-mono text-cyan-400 tracking-wider">VISUALIZAÇÃO ATIVA</h3>
              
              {/* Botão de Download eDrawings condicional */}
              {activeItem.edrawingsUrl && (
                <a
                  href={activeItem.edrawingsUrl}
                  download
                  className="inline-flex items-center space-x-1.5 bg-slate-800 hover:bg-slate-700 text-cyan-400 text-[10px] font-mono px-3 py-1 rounded border border-slate-700 transition-colors"
                >
                  <span>Baixar p/ eDrawings (.easm)</span>
                </a>
              )}
            </div>
            
            <div className="h-[450px] w-full border border-slate-800 rounded-xl overflow-hidden">
              <ModelViewer modelPath={activeItem.modelPath} />
            </div>
          </div>

          {/* Models list */}
          <div className="lg:col-span-5 space-y-4">
            {cadItems.map((item, idx) => (
              <div
                key={idx}
                onClick={() => setActiveItem(item)}
                className={`p-5 rounded-xl border transition-all cursor-pointer ${
                  activeItem.modelPath === item.modelPath
                    ? 'bg-slate-900 border-cyan-500 shadow-lg shadow-cyan-500/10'
                    : 'bg-slate-900/40 border-slate-800/80 hover:border-slate-700'
                }`}
              >
                <div className="flex justify-between items-start mb-2">
                  <span className="text-[11px] font-mono bg-slate-950 text-cyan-400 px-2 py-0.5 rounded border border-slate-800">
                    {item.software}
                  </span>
                  <Maximize2 className="w-4 h-4 text-slate-500" />
                </div>
                <h3 className="text-base font-bold text-white mb-1">{item.title}</h3>
                <p className="text-slate-400 text-xs leading-relaxed">{item.description}</p>
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  );
};