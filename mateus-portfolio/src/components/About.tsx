import React from 'react';
import { GraduationCap, MapPin } from 'lucide-react';

export const About: React.FC = () => {
  return (
    <section id="about" className="py-24 bg-slate-950 border-b border-slate-800/80">
      <div className="max-w-4xl mx-auto px-6 space-y-8">
        <div className="space-y-2">
          <span className="text-xs font-mono text-cyan-400">[ PERFIL ]</span>
          <h2 className="text-3xl font-extrabold text-white">SOBRE MIM</h2>
        </div>

        <p className="text-slate-300 text-base sm:text-lg leading-relaxed font-sans">
          Sou estudante de Engenharia Aeroespacial na UFMG e tenho interesse no desenvolvimento de sistemas que conectam engenharia, software e hardware. Tenho experiência prática com sistemas embarcados, CAD, IoT, instrumentação e desenvolvimento de projetos.
        </p>

        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 pt-4 font-mono text-xs">
          <div className="bg-slate-900/80 border border-slate-800 p-4 rounded-xl flex items-center space-x-3">
            <GraduationCap className="w-5 h-5 text-cyan-400" />
            <span className="text-slate-200">Engenharia Aeroespacial — UFMG</span>
          </div>

          <div className="bg-slate-900/80 border border-slate-800 p-4 rounded-xl flex items-center space-x-3">
            <MapPin className="w-5 h-5 text-cyan-400" />
            <span className="text-slate-200">Belo Horizonte, Brasil</span>
          </div>
        </div>
      </div>
    </section>
  );
};