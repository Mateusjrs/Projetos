import React, { Suspense, useRef } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, useGLTF, Stage, Float, Html } from '@react-three/drei';
import * as THREE from 'three';

interface ModelViewerProps {
  modelPath?: string;
  autoRotate?: boolean;
}

function DefaultFallbackGeometry() {
  const meshRef = useRef<THREE.Group>(null);

  useFrame((_state, delta) => {
    if (meshRef.current) {
      meshRef.current.rotation.y += delta * 0.4;
      meshRef.current.rotation.x += delta * 0.15;
    }
  });

  return (
    <group ref={meshRef}>
      {/* Geometria TÉCNICA estilizada (Satélite/Estrutura Aeronáutica) */}
      <mesh>
        <octahedronGeometry args={[1.4, 0]} />
        <meshStandardMaterial color="#0284c7" wireframe roughness={0.2} metalness={0.8} />
      </mesh>
      <mesh>
        <boxGeometry args={[0.8, 0.8, 0.8]} />
        <meshStandardMaterial color="#38bdf8" roughness={0.3} metalness={0.9} />
      </mesh>
      {/* Painéis Solares SIMULADOS */}
      <mesh position={[1.8, 0, 0]}>
        <boxGeometry args={[1.8, 0.05, 0.8]} />
        <meshStandardMaterial color="#0ea5e9" metalness={0.9} roughness={0.1} />
      </mesh>
      <mesh position={[-1.8, 0, 0]}>
        <boxGeometry args={[1.8, 0.05, 0.8]} />
        <meshStandardMaterial color="#0ea5e9" metalness={0.9} roughness={0.1} />
      </mesh>
    </group>
  );
}

function LoadedModel({ path }: { path: string }) {
  try {
    const { scene } = useGLTF(path);
    return <primitive object={scene} scale={1.2} />;
  } catch (error) {
    return <DefaultFallbackGeometry />;
  }
}

function Loader() {
  return (
    <Html center>
      <div className="flex flex-col items-center justify-center text-cyan-400 font-mono text-xs tracking-wider">
        <div className="w-8 h-8 border-2 border-cyan-500 border-t-transparent rounded-full animate-spin mb-2" />
        CARREGANDO MODELO 3D...
      </div>
    </Html>
  );
}

export const ModelViewer: React.FC<ModelViewerProps> = ({ modelPath, autoRotate = true }) => {
  return (
    <div className="w-full h-full min-h-[300px] relative bg-slate-950/80 rounded-xl overflow-hidden border border-slate-800/80 shadow-2xl">
      <div className="absolute top-3 left-3 z-10 flex items-center space-x-2 bg-slate-900/90 backdrop-blur-md px-3 py-1.5 rounded-md border border-slate-700/50 text-[11px] font-mono text-cyan-400">
        <span className="w-2 h-2 rounded-full bg-cyan-400 animate-pulse" />
        <span>INTERATIVO 3D [ ARRASTE / ZOOM ]</span>
      </div>

      <Canvas camera={{ position: [0, 0, 5], fov: 45 }}>
        <ambientLight intensity={0.7} />
        <directionalLight position={[10, 10, 5]} intensity={1.5} color="#38bdf8" />
        <directionalLight position={[-10, -10, -5]} intensity={0.5} color="#0284c7" />

        <Suspense fallback={<Loader />}>
          <Float speed={1.5} rotationIntensity={0.2} floatIntensity={0.3}>
            <Stage environment="city" intensity={0.5} adjustCamera={true}>
              {modelPath ? <LoadedModel path={modelPath} /> : <DefaultFallbackGeometry />}
            </Stage>
          </Float>
        </Suspense>

        <OrbitControls
          enableZoom={true}
          autoRotate={autoRotate}
          autoRotateSpeed={1.2}
          maxPolarAngle={Math.PI / 1.5}
          minPolarAngle={Math.PI / 6}
        />
      </Canvas>
    </div>
  );
};