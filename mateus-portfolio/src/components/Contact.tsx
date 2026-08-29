import React from 'react';
import { Code, Share2, Mail } from 'lucide-react';

export const Contact: React.FC = () => {
  return (
    <section id="contact" className="py-24 bg-slate-950 text-center">
      <div className="max-w-3xl mx-auto px-6 space-y-8">
        <h2 className="text-4xl sm:text-5xl font-extrabold text-white tracking-tight">
          Vamos construir algo?
        </h2>
        <p className="text-slate-400 text-base max-w-lg mx-auto">
          Aberto a oportunidades de estágio, projetos de engenharia e desenvolvimento de sistemas embarcados.
        </p>

        <div className="flex flex-wrap justify-center gap-4 pt-4 font-mono text-sm">
          <a
            href="https://github.com"
            target="_blank"
            rel="noreferrer"
            className="inline-flex items-center space-x-2 bg-slate-900 hover:bg-slate-800 text-white px-6 py-3.5 rounded-xl border border-slate-700 transition-colors"
          >
            <Code className="w-4 h-4 text-cyan-400" />
            <span>GitHub</span>
          </a>

          <a
            href="https://linkedin.com"
            target="_blank"
            rel="noreferrer"
            className="inline-flex items-center space-x-2 bg-slate-900 hover:bg-slate-800 text-white px-6 py-3.5 rounded-xl border border-slate-700 transition-colors"
          >
            <Share2 className="w-4 h-4 text-cyan-400" />
            <span>LinkedIn</span>
          </a>

          <a
            href="mailto:mateus@exemplo.com"
            className="inline-flex items-center space-x-2 bg-cyan-500 hover:bg-cyan-400 text-slate-950 font-bold px-6 py-3.5 rounded-xl transition-colors"
          >
            <Mail className="w-4 h-4" />
            <span>Email</span>
          </a>
        </div>
      </div>
    </section>
  );
};