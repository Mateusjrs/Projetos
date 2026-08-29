import React, { useEffect } from 'react';
import type { Project } from '../data/projects';
import { ModelViewer } from './ModelViewer';
import {CheckCircle2, Cpu, Wrench, Code2, ZoomIn, X } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { useState } from 'react'; // Garanta que o useState está importado

interface Project {
  title: string;
  description?: string;
  image?: string;
  images?: string[];
}



export const ProjectDetailModal: React.FC<ProjectDetailModalProps> = ({
  project,
  onClose,
}) => {
  const [expandedImage, setExpandedImage] = useState<string | null>(null);

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'Escape') onClose();
    };

    if (project) {
      document.body.style.overflow = 'hidden';
      window.addEventListener('keydown', handleKeyDown);
    }

    return () => {
      document.body.style.overflow = 'auto';
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, [project, onClose]);


  return (
    <AnimatePresence>
      {project && (
        <motion.div 
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          exit={{ opacity: 0 }}
          className="fixed inset-0 z-50 flex items-center justify-center p-4 sm:p-6 overflow-y-auto bg-slate-950/80 backdrop-blur-xl"
          onClick={onClose}
        >
          <motion.div 
            initial={{ scale: 0.95, opacity: 0, y: 20 }}
            animate={{ scale: 1, opacity: 1, y: 0 }}
            exit={{ scale: 0.95, opacity: 0, y: 20 }}
            transition={{ type: "spring", bounce: 0, duration: 0.4 }}
            onClick={(e) => e.stopPropagation()}
            className="bg-slate-900/90 border border-slate-700/50 rounded-2xl max-w-4xl w-full max-h-[90vh] overflow-y-auto shadow-2xl shadow-cyan-900/10 relative text-white my-8 custom-scrollbar"
          >
            {/* Cabeçalho */}
            <div className="sticky top-0 z-20 bg-slate-900/80 backdrop-blur-md border-b border-slate-800 px-6 py-4 flex justify-between items-center">
              <div className="space-y-1">
                <span className="text-xs font-mono text-cyan-400 font-semibold tracking-wider">[ {project.category} ]</span>
                <h2 className="text-2xl font-bold tracking-tight">{project.title}</h2>
              </div>
              <button onClick={onClose} className="p-2 rounded-xl bg-slate-800/50 hover:bg-slate-700 hover:text-cyan-400 transition-colors border border-slate-700/50">
                <X className="w-5 h-5" />
              </button>
            </div>
            
            <div className="p-6 sm:p-8 space-y-8 font-sans">
              
              {/* Resumo & Badges */}
              <div>
                <p className="text-lg text-slate-300 font-light leading-relaxed">{project.summary}</p>
                <div className="flex flex-wrap gap-2 mt-4">
                  {project.technologies.map((tech) => (
                    <span key={tech} className="bg-cyan-950/60 text-cyan-300 border border-cyan-800/50 text-xs font-mono px-3 py-1 rounded-md">
                      {tech}
                    </span>
                  ))}
                </div>
              </div>

              {/* O Problema & Minha Contribuição */}
              <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                <div className="bg-slate-950/60 p-5 rounded-xl border border-slate-800">
                  <h4 className="text-sm font-mono text-cyan-400 mb-2 flex items-center gap-2">
                    <Wrench className="w-4 h-4" /> O PROBLEMA
                  </h4>
                  <p className="text-slate-300 text-sm leading-relaxed">{project.problem}</p>
                </div>

                <div className="bg-slate-950/60 p-5 rounded-xl border border-slate-800">
                  <h4 className="text-sm font-mono text-cyan-400 mb-2 flex items-center gap-2">
                    <Cpu className="w-4 h-4" /> MINHA CONTRIBUIÇÃO
                  </h4>
                  <p className="text-slate-300 text-sm leading-relaxed">{project.contribution}</p>
                </div>
              </div>

             {/* Visualizador 3D Dedicado - SÓ APARECE SE TIVER MODELPATH */}
              {project.modelPath && (
                <div className="space-y-3">
                  <div className="flex items-center justify-between">
                    <h3 className="text-sm font-mono text-cyan-400 tracking-wider">MODELO 3D INTERATIVO</h3>
                    
                    {/* Botão de Download eDrawings */}
                    {project.edrawingsUrl && (
                      <a
                        href={project.edrawingsUrl}
                        download
                        className="inline-flex items-center space-x-1.5 bg-slate-800 hover:bg-slate-700 text-cyan-400 text-[10px] font-mono px-3 py-1 rounded border border-slate-700 transition-colors"
                      >
                        <span>Baixar p/ eDrawings (.easm)</span>
                      </a>
                    )}
                  </div>
                  
                  <div className="h-[350px] w-full border border-slate-800 rounded-xl overflow-hidden">
                    <ModelViewer modelPath={project.modelPath} />
                  </div>
                </div>
              )}
             {/* Detalhes de Engenharia Dinâmico */}
              <div className="space-y-4 bg-slate-950/80 p-6 rounded-xl border border-slate-800">
                <h3 className="text-base font-bold text-white flex items-center gap-2 border-b border-slate-800 pb-3">
                  <Cpu className="w-5 h-5 text-cyan-400" /> Engenharia & Detalhes Técnicos
                </h3>
                
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                  {Object.entries(project.engineeringDetails).map(([key, value]) => {
                    // Ignora campos vazios ou arrays (tratamos o array embaixo)
                    if (!value || Array.isArray(value)) return null;

                    // Traduz as chaves para títulos bonitos na tela
                    const titleMap: Record<string, string> = {
                      calculations: 'Cálculos/Análise',
                      decisions: 'Decisões',
                      dimensions: 'Dimensões',
                      dataStructure: 'Estrutura de Dados',
                      aircraft: 'Aeronave',
                      geometry: 'Geometria',
                      cad: 'Engenharia Reversa CAD',
                      massAndCG: 'Massa e CG',
                      inertia: 'Inércia',
                      aerodynamicModel: 'Modelo Aerodinâmico',
                      derivatives: 'Derivadas',
                      trim: 'Trimagem',
                      trimValidation: 'Validação',
                      simulation: 'Simulação 6-DOF',
                      equations: 'Equações de Movimento',
                      aerodynamicForces: 'Forças Aerodinâmicas',
                      dynamicTests: 'Ensaios Dinâmicos',
                      pulse: 'Resposta a Pulso',
                      doublet: 'Resposta a Doublet',
                      dynamicModes: 'Modos Dinâmicos',
                      massComparison: 'Comparação de Massas',
                      stabilityAnalysis: 'Análise de Estabilidade',
                      firmwareArchitecture: 'Arquitetura de Firmware',
                      isoTpStack: 'Stack ISO-TP Customizada',
                      diagnostics: 'Rotinas de Diagnóstico',
                      hardware: 'Hardware & Baixo Nível',
                      connectivity: 'Conectividade (IoT)',
                      safety: 'Segurança Lógica',
                      firmwareArchitecture: 'Arquitetura de Firmware',
                      thermostatControl: 'Controle Térmico Preditivo',
                      connectivity: 'Conectividade e Provisionamento',
                      memoryManagement: 'Gestão de Memória Flash',
                      massComparison: 'Comparação de Massas',
                      stabilityAnalysis: 'Análise de Estabilidade',
                      modification: 'Modificação',
                      regulatory: 'Regulamentação',
                      equipment: 'Equipamentos',
                      electricalSystem: 'Sistema Elétrico',
                      loadAnalysis: 'Balanço de Carga',
                      generator: 'Geração',
                      electricalMargin: 'Margem Elétrica',
                      wiring: 'Wiring (Fiação)',
                      protection: 'Proteção Elétrica',
                      connections: 'Conexões',
                      antenna: 'Antena',
                      documentation: 'Documentação',
                      // === Novos campos para o projeto NACA ===
                      baseline: 'Perfil de Referência',
                      postProcessing: 'Pós-processamento (Python)',
                      parameters: 'Metodologia Paramétrica',
                      thickness: 'Análise de Espessura',
                      camber: 'Análise de Camber',
                      leadingEdge: 'Bordo de Ataque (LE)',
                      trailingEdge: 'Bordo de Fuga (TE)',
                      pressureAnalysis: 'Distribuição de Pressão (Cp)',
                      boundaryLayer: 'Camada Limite',
                      centerOfPressure: 'Centro de Pressão',
                      analysis: 'Escopo da Análise',
                  schematic: 'Projeto de Esquemático',
                      controller: 'Microcontrolador (MCU)',
                      power: 'Fonte e Distribuição de Energia',
                      adc: 'Conversor A/D (ADS1115)',
                      communication: 'Transceptor e Barramento',
                      rtc: 'Real Time Clock',
                      display: 'Barramento de Display',
                      inputs: 'Entradas (I/O)',
                      connectors: 'Terminais e Conexões',
                      protection: 'Isolamento e Proteção (Flyback)',
                      bom: 'Lista de Materiais (BOM)'
                    };

                    return (
                      <div key={key} className="bg-slate-900/50 p-4 rounded-lg border border-slate-800/50">
                        <strong className="text-cyan-400 font-mono text-xs block mb-1.5 uppercase tracking-wider">
                          {titleMap[key] || key}
                        </strong> 
                        <p className="text-sm text-slate-300 leading-relaxed">{value}</p>
                      </div>
                    );
                  })}
                </div>

            {/* Renderização Segura de Múltiplas Listas */}
                {((project.engineeringDetails.components?.length || 0) > 0 || 
                  (project.engineeringDetails.analyses?.length || 0) > 0 ||
                  (project.engineeringDetails.equipmentList?.length || 0) > 0 ||
                  (project.engineeringDetails.aerodynamicCurves?.length || 0) > 0 ||
                  (project.engineeringDetails.mainFindings?.length || 0) > 0 ||
                  (project.engineeringDetails.hardwareComponents?.length || 0) > 0 ||
                  (project.engineeringDetails.engineeringSkills?.length || 0) > 0) && (
                  
                  <div className="pt-4 mt-2 border-t border-slate-800/50">
                    <span className="text-xs font-mono text-slate-400 block mb-3">TÓPICOS, CURVAS, COMPONENTES (BOM) E SKILLS:</span>
                    <div className="flex flex-wrap gap-2">
                      {/* Junta TODAS as listas possíveis em um único array para imprimir na tela */}
                      {[
                        ...(project.engineeringDetails.components || []),
                        ...(project.engineeringDetails.analyses || []),
                        ...(project.engineeringDetails.equipmentList || []),
                        ...(project.engineeringDetails.aerodynamicCurves || []),
                        ...(project.engineeringDetails.mainFindings || []),
                        ...(project.engineeringDetails.hardwareComponents || []),
                        ...(project.engineeringDetails.engineeringSkills || [])
                      ].map((item, idx) => (
                        <span key={idx} className="text-xs bg-slate-900 text-slate-300 px-3 py-1.5 rounded-md border border-slate-700 shadow-sm">
                          • {item}
                        </span>
                      ))}
                    </div>
                  </div>
                )}
              </div>
              {/* Bloco de Código */}
              {project.codeSnippet && (
                <div className="space-y-3">
                  <div className="flex justify-between items-center">
                    <h3 className="text-sm font-mono text-cyan-400 flex items-center gap-2">
                      <Code2 className="w-4 h-4" /> IMPLEMENTAÇÃO DE FIRMWARE / SOFTWARE
                    </h3>
                    <span className="text-xs font-mono text-slate-500 uppercase">{project.codeSnippet.language}</span>
                  </div>
                  <pre className="bg-slate-950 p-4 rounded-xl border border-slate-800 font-mono text-xs text-slate-300 overflow-x-auto leading-relaxed">
                    <code>{project.codeSnippet.code}</code>
                  </pre>
                </div>
              )}

              {/* Resultado Final */}
              <div className="bg-cyan-950/30 border border-cyan-800/50 p-4 rounded-xl flex items-start gap-3">
                <CheckCircle2 className="w-5 h-5 text-cyan-400 shrink-0 mt-0.5" />
                <div>
                  <h4 className="text-sm font-bold text-cyan-300">RESULTADO ALCANÇADO</h4>
                  <p className="text-xs text-slate-300 mt-1">{project.result}</p>
                </div>
              </div>

{/* Galeria de Imagens Condicional */}
              {project.gallery && project.gallery.length > 0 && (
                <div className="space-y-4 bg-slate-950/80 p-6 rounded-xl border border-slate-800">
                  <h3 className="text-base font-bold text-white flex items-center gap-2 border-b border-slate-800 pb-3">
                    Galeria do Projeto
                  </h3>
                  <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 pt-2">
                    {project.gallery.map((imgSrc, idx) => (
                      <div key={idx} className="rounded-lg overflow-hidden border border-slate-700/50 hover:border-cyan-500/50 transition-colors">
                        <img 
                          src={imgSrc} 
                          alt={`Imagem técnica ${idx + 1}`} 
                          className="w-full h-auto object-cover hover:scale-105 transition-transform duration-500"
                        />
                      </div>
                    ))}
                  </div>
                </div>
              )}
              {/* Seção de Mídia (GIFs e Telas com Descrição) */}
              {/* Seção de Mídia (GIFs e Telas com Descrição) */}
              {project.media && Object.keys(project.media).length > 0 && (
                <div className="space-y-4 bg-slate-950/80 p-6 rounded-xl border border-slate-800 mt-4">
                  <h3 className="text-base font-bold text-white flex items-center gap-2 border-b border-slate-800 pb-3">
                     Mídias & Demonstração
                  </h3>
                  <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 pt-2">
                    {Object.values(project.media).map((item, idx) => {
                      if (!item || !item.url) return null;
                      return (
                        <div key={idx} className="bg-slate-900 rounded-lg overflow-hidden border border-slate-700/50 hover:border-cyan-500/50 transition-colors group flex flex-col">
                          
                          {/* CAIXA DA IMAGEM ATUALIZADA (Clicável) */}
                          <div 
                            className="h-48 relative overflow-hidden bg-black flex-shrink-0 cursor-pointer"
                            onClick={() => setExpandedImage(item.url)}
                          >
                            <img 
                              src={item.url} 
                              alt={item.title} 
                              className="w-full h-full object-cover group-hover:scale-110 transition-transform duration-700" 
                            />
                            {/* Máscara de Hover com Lupa */}
                            <div className="absolute inset-0 bg-slate-900/40 opacity-0 group-hover:opacity-100 transition-opacity duration-300 flex items-center justify-center">
                              <div className="bg-cyan-500/20 p-3 rounded-full backdrop-blur-sm border border-cyan-400/30">
                                <ZoomIn className="w-6 h-6 text-cyan-300" />
                              </div>
                            </div>
                          </div>

                          <div className="p-4 flex-grow flex flex-col">
                            <h4 className="text-cyan-400 font-bold text-sm mb-1">{item.title}</h4>
                            <p className="text-slate-400 text-xs leading-relaxed">{item.description}</p>
                          </div>
                        </div>
                      );
                    })}
                  </div>
                </div>
              )}
              {/* Footer do Modal com Github */}
              {project.githubUrl && (
                <div className="pt-2 flex justify-end">
                  <a
                    href={project.githubUrl}
                    target="_blank"
                    rel="noreferrer"
                    className="inline-flex items-center space-x-2 bg-slate-800 hover:bg-slate-700 text-white font-mono text-xs px-5 py-2.5 rounded-lg border border-slate-700 transition-colors"
                  >
                  <Code2 className="w-4 h-4"/>
                    <span>VER CÓDIGO NO GITHUB</span>
                  </a>
                </div>
              )}
            </div>
          </motion.div>
        </motion.div>
      )}
      {expandedImage && (
        <div 
          className="fixed inset-0 z-[99999] flex items-center justify-center bg-black/90 p-4 sm:p-8 backdrop-blur-sm"
          onClick={() => setExpandedImage(null)}
        >
          <button 
            className="absolute top-6 right-6 text-slate-400 hover:text-cyan-400 transition-colors p-2 bg-slate-900/50 rounded-full border border-slate-700 hover:border-cyan-500"
            onClick={() => setExpandedImage(null)}
          >
            <X className="w-6 h-6" />
          </button>
          
          <img 
            src={expandedImage} 
            alt="Expandida" 
            className="max-w-full max-h-full object-contain rounded-lg shadow-2xl border border-slate-800"
            onClick={(e) => e.stopPropagation()}
          />
        </div>
      )}
    </AnimatePresence>
  );
};