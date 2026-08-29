import React from 'react';
import { ModelViewer } from './ModelViewer';
import { ArrowDown, Code, ChevronRight, Binary } from 'lucide-react';
import { motion } from 'framer-motion';

const container = {
  hidden: { opacity: 0 },
  show: {
    opacity: 1,
    transition: { staggerChildren: 0.15, delayChildren: 0.2 }
  }
};

const item = {
  hidden: { opacity: 0, y: 20 },
  show: { opacity: 1, y: 0, transition: { duration: 0.6 } }
};

export const Hero: React.FC = () => {
  return (
    <section className="relative min-h-screen flex items-center pt-20 pb-20 overflow-hidden bg-slate-950 text-white border-b border-slate-800/50">
      {/* Grid Pattern Dinâmico */}
      <div className="absolute inset-0 bg-[linear-gradient(to_right,#1e293b20_1px,transparent_1px),linear-gradient(to_bottom,#1e293b20_1px,transparent_1px)] bg-[size:4rem_4rem] [mask-image:radial-gradient(ellipse_60%_50%_at_50%_0%,#000_70%,transparent_100%)] pointer-events-none" />
      
      {/* Glow Sutil no Fundo */}
      <div className="absolute top-1/4 left-1/4 w-96 h-96 bg-cyan-500/10 rounded-full blur-[120px] pointer-events-none" />

      <div className="max-w-7xl mx-auto px-6 grid grid-cols-1 lg:grid-cols-12 gap-12 items-center relative z-10 w-full">
        
        {/* Coluna Texto Animada */}
        <motion.div 
          variants={container}
          initial="hidden"
          animate="show"
          className="lg:col-span-7 space-y-7"
        >
          <motion.div variants={item} className="inline-flex items-center space-x-2 px-3 py-1.5 rounded-full bg-slate-900/80 backdrop-blur-md border border-slate-800 text-xs font-mono text-cyan-400 shadow-lg shadow-cyan-900/20">
            <Binary className="w-3.5 h-3.5" />
            <span>UNIVERSIDADE FEDERAL DE MINAS GERAIS (UFMG)</span>
          </motion.div>

          <motion.h1 variants={item} className="text-5xl sm:text-7xl font-extrabold tracking-tight leading-tight">
            <span className="bg-clip-text text-transparent bg-gradient-to-r from-white via-slate-200 to-slate-500">
              MATEUS JUNIO
            </span>
          </motion.h1>

          <motion.p variants={item} className="text-lg sm:text-xl font-mono text-cyan-400 tracking-wide font-medium">
            Engenharia Aeroespacial | CAD | Embedded Systems | IoT
          </motion.p>

          <motion.p variants={item} className="text-slate-300 text-base sm:text-lg max-w-2xl leading-relaxed font-sans font-light">
            Estudante de Engenharia Aeroespacial na UFMG, interessado no desenvolvimento de sistemas que integram software, hardware, modelagem e engenharia.
          </motion.p>

          <motion.div variants={item} className="pt-4 flex flex-wrap gap-4">
            <a href="#projects" className="group inline-flex items-center space-x-2 bg-cyan-500 hover:bg-cyan-400 text-slate-950 font-bold px-7 py-3.5 rounded-xl transition-all duration-300 shadow-[0_0_20px_rgba(6,182,212,0.3)] hover:shadow-[0_0_30px_rgba(6,182,212,0.5)] text-sm tracking-wider">
              <span>VER PROJETOS</span>
              <ChevronRight className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
            </a>

            <a href="https://github.com" target="_blank" rel="noreferrer" className="group inline-flex items-center space-x-2 bg-slate-900/50 backdrop-blur hover:bg-slate-800/80 text-slate-200 border border-slate-700/80 px-7 py-3.5 rounded-xl transition-all duration-300 text-sm font-mono hover:border-cyan-500/50">
              <Code className="w-4 h-4 text-slate-400 group-hover:text-white transition-colors" />             <span>GITHUB</span>
            </a>
          </motion.div>
        </motion.div>

        {/* Coluna Modelo 3D com flutuação */}
        <motion.div 
          initial={{ opacity: 0, scale: 0.9 }}
          animate={{ opacity: 1, scale: 1 }}
          transition={{ duration: 1, delay: 0.5 }}
          className="lg:col-span-5 h-[400px] sm:h-[500px] w-full"
        >
          <ModelViewer />
        </motion.div>
      </div>

      <motion.div 
        initial={{ opacity: 0 }} 
        animate={{ opacity: 1 }} 
        transition={{ delay: 1.5, duration: 1 }}
        className="absolute bottom-10 left-1/2 -translate-x-1/2 flex justify-center"
      >
        <a href="#projects" className="text-slate-500 hover:text-cyan-400 transition-colors animate-bounce p-3 bg-slate-900/50 rounded-full backdrop-blur-sm border border-slate-800">
          <ArrowDown className="w-5 h-5" />
        </a>
      </motion.div>
    </section>
  );
};