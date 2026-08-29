import { useState } from 'react';
import { Navbar } from './components/Navbar';
import { Hero } from './components/Hero';
import { ProjectCard } from './components/ProjectCard';
import { ProjectFilter } from './components/ProjectFilter';
import { ProjectDetailModal } from './components/ProjectDetailModal';
import { CadSection } from './components/CadSection';
import { SkillsSection } from './components/SkillsSection';
import { About } from './components/About';
import { Contact } from './components/Contact';
import { projectsData } from './data/projects';
import type { Project, Category } from './data/projects';

export function App() {
  const [selectedCategory, setSelectedCategory] = useState<Category>('All');
  const [activeModalProject, setActiveModalProject] = useState<Project | null>(null);

  const categories: Category[] = ['All', 'Aerospace', 'CAD / 3D', 'Hardware', 'Software', 'IoT'];

  const filteredProjects = selectedCategory === 'All'
    ? projectsData
    : projectsData.filter((p) => p.category === selectedCategory);

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 font-sans selection:bg-cyan-500 selection:text-slate-950">
      <Navbar />
      <Hero />

      {/* Seção Projetos em Destaque */}
      <section id="projects" className="py-24 max-w-7xl mx-auto px-6 border-b border-slate-800/80">
        <div className="text-center max-w-2xl mx-auto mb-8">
          <span className="text-xs font-mono text-cyan-400 tracking-wider">[ PORTFÓLIO DE ENGENHARIA ]</span>
          <h2 className="text-3xl font-extrabold text-white mt-1">PROJETOS EM DESTAQUE</h2>
        </div>

        <ProjectFilter
          categories={categories}
          selectedCategory={selectedCategory}
          onSelectCategory={setSelectedCategory}
        />

        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8 mt-10">
          {filteredProjects.map((project, index) => (
            <ProjectCard
              key={project.id}
              project={project}
              index={index}
              onSelect={(proj: Project) => setActiveModalProject(proj)}
            />
          ))}
        </div>
      </section>

      <CadSection />
      <SkillsSection />
      <About />
      <Contact />

      <ProjectDetailModal
        project={activeModalProject}
        onClose={() => setActiveModalProject(null)}
      />

      <footer className="py-8 bg-slate-950 text-center font-mono text-xs text-slate-600 border-t border-slate-900">
        © {new Date().getFullYear()} Mateus Junio · Engenharia Aeroespacial UFMG
      </footer>
    </div>
  );
}

export default App;