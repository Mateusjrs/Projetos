export type Category = 'All' | 'Aerospace' | 'CAD / 3D' | 'Hardware' | 'Software' | 'IoT';

export interface Project {
  id: string;
  title: string;
  category: Category;
  shortDescription: string;
  technologies: string[];
  imageUrl: string;
  modelPath?: string;
  featured: boolean;
  summary: string;
  problem: string;
  contribution: string;
  engineeringDetails: {
    calculations?: string;
    decisions?: string;
    dimensions?: string;
    components?: string[];
  };
  codeSnippet?: {
    language: string;
    code: string;
  };
  githubUrl?: string;
  result: string;
}