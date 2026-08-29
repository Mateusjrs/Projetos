import React from 'react';
import { Code, Cpu, Box, Layers, Wind, Calculator } from 'lucide-react';

export const SkillsSection: React.FC = () => {
  const softwareSkills = ['C', 'C++', 'Python', 'MATLAB', 'Git', 'GitHub', 'Linux', 'FreeRTOS'];
  const hardwareSkills = ['ESP32', 'Sensores', 'CAN', 'OBD-II', 'RS485', 'Modbus', 'MQTT', 'Sistemas Embarcados'];

  return (
    <section id="skills" className="py-24 bg-slate-950/60 border-b border-slate-800/80">
      <div className="max-w-7xl mx-auto px-6">
        
        {/* ============================================================== */}
        {/* CABEÇALHO DA SEÇÃO SOFTWARE & HARDWARE */}
        {/* ============================================================== */}
        <div className="text-center max-w-2xl mx-auto mb-16 space-y-2">
          <span className="text-xs font-mono text-cyan-400 tracking-wider uppercase">
            [ Stack Técnico ]
          </span>
          <h2 className="text-3xl font-extrabold text-white">SOFTWARE & HARDWARE</h2>
          <p className="text-slate-400 text-sm">
            Domínio multidisciplinar integrando programação de baixo nível e arquitetura de hardware.
          </p>
        </div>

        {/* ============================================================== */}
        {/* GRIDS: SOFTWARE E HARDWARE */}
        {/* ============================================================== */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8 mb-24">
          
          {/* Software Box */}
          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-8 hover:border-slate-700 transition-colors">
            <div className="flex items-center space-x-3 mb-6">
              <div className="p-3 bg-cyan-950/50 border border-cyan-800/50 rounded-xl text-cyan-400">
                <Code className="w-6 h-6" />
              </div>
              <div>
                <h3 className="text-xl font-bold text-white">SOFTWARE</h3>
                <span className="text-xs font-mono text-slate-400">Desenvolvimento & Análise</span>
              </div>
            </div>

            <div className="grid grid-cols-2 gap-3">
              {softwareSkills.map((skill) => (
                <div key={skill} className="bg-slate-950/80 border border-slate-800 px-4 py-3 rounded-lg flex items-center justify-between">
                  <span className="text-sm font-mono text-slate-200">{skill}</span>
                  <span className="w-1.5 h-1.5 rounded-full bg-cyan-400" />
                </div>
              ))}
            </div>
          </div>

          {/* Hardware Box */}
          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-8 hover:border-slate-700 transition-colors">
            <div className="flex items-center space-x-3 mb-6">
              <div className="p-3 bg-cyan-950/50 border border-cyan-800/50 rounded-xl text-cyan-400">
                <Cpu className="w-6 h-6" />
              </div>
              <div>
                <h3 className="text-xl font-bold text-white">HARDWARE</h3>
                <span className="text-xs font-mono text-slate-400">Embarcados & Protocolos</span>
              </div>
            </div>

            <div className="grid grid-cols-2 gap-3">
              {hardwareSkills.map((skill) => (
                <div key={skill} className="bg-slate-950/80 border border-slate-800 px-4 py-3 rounded-lg flex items-center justify-between">
                  <span className="text-sm font-mono text-slate-200">{skill}</span>
                  <span className="w-1.5 h-1.5 rounded-full bg-cyan-400" />
                </div>
              ))}
            </div>
          </div>
        </div>

        {/* ============================================================== */}
        {/* CABEÇALHO DA SEÇÃO MODELAGEM 3D & ENGENHARIA */}
        {/* ============================================================== */}
        <div className="text-center max-w-2xl mx-auto mb-16 space-y-2">
          <span className="text-xs font-mono text-cyan-400 tracking-wider uppercase">
            [ Stack de Engenharia ]
          </span>
          <h2 className="text-3xl font-extrabold text-white">MODELAGEM 3D & ANÁLISE</h2>
          <p className="text-slate-400 text-sm">
            Projeto mecânico, simulação de elementos finitos (FEA) e aerodinâmica computacional (CFD) voltados para alta performance.
          </p>
        </div>

        {/* ============================================================== */}
        {/* GRIDS: MODELAGEM E ENGENHARIA */}
        {/* ============================================================== */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          
          {/* Card 1: CAD & Modelagem */}
          <div className="bg-slate-900/60 p-6 rounded-xl border border-slate-800 hover:border-cyan-500/30 transition-colors group">
            <div className="flex items-center gap-3 mb-4">
              <div className="p-2 bg-cyan-500/10 rounded-lg group-hover:bg-cyan-500/20 transition-colors">
                <Box className="w-5 h-5 text-cyan-400" />
              </div>
              <h4 className="text-white font-bold tracking-wide">Design & CAD 3D</h4>
            </div>
            <div className="flex flex-wrap gap-2">
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">SolidWorks</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">AutoCAD</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Modelagem Paramétrica</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Engenharia Reversa</span>
            </div>
          </div>

          {/* Card 2: FEA & Simulação */}
          <div className="bg-slate-900/60 p-6 rounded-xl border border-slate-800 hover:border-cyan-500/30 transition-colors group">
            <div className="flex items-center gap-3 mb-4">
              <div className="p-2 bg-cyan-500/10 rounded-lg group-hover:bg-cyan-500/20 transition-colors">
                <Layers className="w-5 h-5 text-cyan-400" />
              </div>
              <h4 className="text-white font-bold tracking-wide">Análise Estrutural (FEA)</h4>
            </div>
            <div className="flex flex-wrap gap-2">
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Ansys Mechanical</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Análise Estática</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Fadiga & Vibração</span>
            </div>
          </div>

          {/* Card 3: Aerodinâmica & CFD */}
          <div className="bg-slate-900/60 p-6 rounded-xl border border-slate-800 hover:border-cyan-500/30 transition-colors group">
            <div className="flex items-center gap-3 mb-4">
              <div className="p-2 bg-cyan-500/10 rounded-lg group-hover:bg-cyan-500/20 transition-colors">
                <Wind className="w-5 h-5 text-cyan-400" />
              </div>
              <h4 className="text-white font-bold tracking-wide">Dinâmica de Fluidos (CFD)</h4>
            </div>
            <div className="flex flex-wrap gap-2">
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">XFLR5</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Ansys Fluent</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Método dos Painéis (VLM)</span>
            </div>
          </div>

          {/* Card 4: Matemática & Computação */}
          <div className="bg-slate-900/60 p-6 rounded-xl border border-slate-800 hover:border-cyan-500/30 transition-colors group">
            <div className="flex items-center gap-3 mb-4">
              <div className="p-2 bg-cyan-500/10 rounded-lg group-hover:bg-cyan-500/20 transition-colors">
                <Calculator className="w-5 h-5 text-cyan-400" />
              </div>
              <h4 className="text-white font-bold tracking-wide">Análise Computacional</h4>
            </div>
            <div className="flex flex-wrap gap-2">
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">MATLAB / Simulink</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">Python (NumPy/SciPy)</span>
              <span className="text-xs font-mono bg-slate-950 text-slate-300 px-3 py-1.5 rounded border border-slate-800">LaTeX</span>
            </div>
          </div>
          
        </div>
      </div>
    </section>
  );
};