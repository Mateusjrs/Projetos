import React from 'react';
import type { Project } from '../data/projects';
import { ExternalLink, Layers } from 'lucide-react';
import { motion } from 'framer-motion';

interface ProjectCardProps {
  project: Project;
  onSelect: (project: Project) => void;
  index?: number;
}

export const ProjectCard: React.FC<ProjectCardProps> = ({ project, onSelect }) => {
  return (
    <motion.div
      initial={{ opacity: 0, y: 20 }}
      whileInView={{ opacity: 1, y: 0 }}
      viewport={{ once: true }}
      transition={{ duration: 0.5 }}
      onClick={() => onSelect(project)}
      className="group relative bg-slate-900 rounded-2xl border border-slate-800 hover:border-cyan-500/50 transition-all duration-300 overflow-hidden cursor-pointer flex flex-col shadow-xl hover:shadow-cyan-500/10"
    >
      {/* Imagem do Projeto */}
      <div className="relative h-48 w-full overflow-hidden bg-slate-950">
        <img
          src={project.imageUrl}
          alt={project.title}
          className="w-full h-full object-cover group-hover:scale-105 transition-transform duration-500 opacity-80 group-hover:opacity-100"
        />
        <div className="absolute inset-0 bg-gradient-to-t from-slate-900 via-transparent to-transparent" />
        
        {/* Tag de Categoria */}
        <span className="absolute top-3 right-3 bg-slate-950/80 backdrop-blur-md text-cyan-400 text-xs font-mono px-3 py-1 rounded-full border border-slate-800">
          {project.category}
        </span>
      </div>

      {/* Conteúdo do Card */}
      <div className="p-6 flex flex-col flex-grow justify-between space-y-4">
        <div className="space-y-2">
          <h3 className="text-xl font-bold text-white group-hover:text-cyan-400 transition-colors">
            {project.title}
          </h3>
          <p className="text-slate-400 text-sm line-clamp-2 font-sans font-light">
            {project.shortDescription}
          </p>
        </div>

        {/* Tecnologias */}
        <div className="flex flex-wrap gap-1.5 pt-2">
          {project.technologies.slice(0, 3).map((tech, techIndex) => (
            <span
              key={techIndex}
              className="text-xs font-mono bg-slate-950 text-slate-300 px-2.5 py-1 rounded-lg border border-slate-800/80"
            >
              {tech}
            </span>
          ))}
          {project.technologies.length > 3 && (
            <span className="text-xs font-mono bg-slate-950 text-slate-500 px-2 py-1 rounded-lg border border-slate-800/80">
              +{project.technologies.length - 3}
            </span>
          )}
        </div>

        {/* Rodapé do Card */}
        <div className="pt-4 border-t border-slate-800/60 flex items-center justify-between text-xs font-mono text-cyan-400">
          <span className="inline-flex items-center space-x-1">
            <Layers className="w-3.5 h-3.5" />
            <span>Detalhes Técnicos</span>
          </span>
          <span className="inline-flex items-center space-x-1 group-hover:translate-x-1 transition-transform">
            <span>Ver projeto</span>
            <ExternalLink className="w-3.5 h-3.5" />
          </span>
        </div>
      </div>
    </motion.div>
  );
};