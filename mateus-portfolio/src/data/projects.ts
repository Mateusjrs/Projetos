export type Category = 'All' | 'Aerospace' | 'CAD / 3D' | 'Hardware' | 'Software' | 'IoT';

export interface Project {
  id: string;
  title: string;
  category: Category;
  shortDescription: string;
  technologies: string[];
  gallery?: string[]; 
  imageUrl: string;
  modelPath?: string;
  edrawingsUrl?: string;
  featured: boolean;
  summary: string;
  problem: string;
  contribution: string;
  modification?: string;
    regulatory?: string;
    equipment?: string;
    equipmentList?: string[];
    electricalSystem?: string;
    loadAnalysis?: string;
    generator?: string;
    electricalMargin?: string;
    wiring?: string;
    protection?: string;
    connections?: string;
    antenna?: string;
    documentation?: string;
    engineeringSkills?: string[];
  engineeringDetails: {
    schematic?: string;
    controller?: string;
    power?: string;
    temperature?: string;
    adc?: string;
    communication?: string;
    rtc?: string;
    display?: string;
    inputs?: string;
    connectors?: string;
    hardwareComponents?: string[];
    bom?: string;
    calculations?: string;
    decisions?: string;
    dimensions?: string;
    components?: string[];
    dataStructure?: string; 
    antenna?: string;
    documentation?: string;
    engineeringSkills?: string[];
    // === Novas do projeto NACA ===
    baseline?: string;
    postProcessing?: string;
    parameters?: string;
    thickness?: string;
    camber?: string;
    leadingEdge?: string;
    trailingEdge?: string;
    aerodynamicCurves?: string[];
    pressureAnalysis?: string;
    boundaryLayer?: string;
    centerOfPressure?: string;
    analysis?: string;
    mainFindings?: string[];
    systemArchitecture?: string;
    temperatureControl?: string;
    thermostat?: string;
    cooler?: string;
    temperatureSafety?: string;
    feedingControl?: string;
    feedingSchedule?: string;
    feedingMemory?: string;
    lightingControl?: string;
    lightingIntensity?: string;
    lightingSchedule?: string;
    lightingRoutines?: string;
    realTimeClock?: string;
    interface?: string;
    wifi?: string;
    mqtt?: string;
    memory?: string;
    multitasking?: string;
    mainFunctions?: string[];
  };
  media?: {
    [key: string]: {
      title: string;
      description: string;
      url: string;
    };
  };

  codeSnippet?: {
    language: string;
    code: string;
  };
  githubUrl?: string;
  result: string;
}

export const projectsData: Project[] = [
 {
    id: 'aero-data-acquisition',
    title: 'Sistema de Aquisição de Dados para Ensaios Aerodinâmicos',
    category: 'Aerospace',
    shortDescription: 'Sistema automatizado de aquisição de dados para investigação do stall na esteira de uma asa por meio da análise de vibração e carregamento.',
    technologies: [
      'C/C++',
      'Microcontroladores',
      'HX711',
      'Célula de carga',
      'Aquisição de dados',
      'Túnel de vento'
    ],
    imageUrl: 'https://images.unsplash.com/photo-1581092160607-ee22621dd758?auto=format&fit=crop&w=1200&q=80',
    featured: true,
    summary: 'Desenvolvimento de um sistema de aquisição de dados para ensaios em túnel de vento, com o objetivo de investigar a ocorrência de stall na esteira de uma asa por meio da análise das variações e picos de vibração durante a mudança do ângulo de ataque. O sistema utiliza uma célula de carga integrada a um módulo HX711 para realizar a medição dos esforços, permitindo registrar tanto os valores individuais das amostras quanto suas médias para posterior análise e caracterização do fenômeno aerodinâmico.',
    problem: 'Durante os ensaios, era necessário realizar medições em diferentes ângulos de ataque e preservar as amostras individuais para identificar variações associadas ao início do stall. A aquisição contínua poderia gerar um volume elevado de dados e dificultar a organização das medições. Dessa forma, foi necessário desenvolver uma estratégia de aquisição capaz de armazenar os dados de forma estruturada, realizar o processamento básico em tempo real e permitir que o operador controlasse o momento exato da coleta após a estabilização das condições do túnel de vento.',
    contribution: 'Desenvolvi o firmware de aquisição de dados em C/C++, implementando a comunicação com o módulo HX711 e a leitura da célula de carga. Também desenvolvi uma interface interativa via comunicação Serial, permitindo ao operador informar o ângulo de ataque e iniciar cada etapa de aquisição. O sistema organiza automaticamente as amostras por ângulo, calcula a força média e mantém os dados brutos disponíveis para posterior processamento e análise. O desenvolvimento também envolveu a definição da estrutura de armazenamento dos dados e da lógica de controle da aquisição.',
    engineeringDetails: {
      calculations: 'Implementação de uma estrutura matricial para armazenar até 25 amostras brutas para cada ângulo de ataque, com capacidade de registrar até 20 condições diferentes. Para cada condição, o firmware calcula a média das amostras adquiridas, permitindo comparar o comportamento do carregamento entre diferentes ângulos de ataque.',
      decisions: 'Foi adotado um sistema de aquisição controlado pelo operador, em vez de uma medição contínua. Essa abordagem permite que o túnel de vento seja estabilizado antes de cada coleta e reduz a quantidade de dados desnecessários armazenados. A estratégia também facilita a associação entre cada conjunto de amostras e seu respectivo ângulo de ataque.',
      components: [
        'Microcontrolador',
        'Módulo amplificador HX711',
        'Célula de carga de 5 kg/10 kg',
        'Túnel de vento'
      ],
      dataStructure: 'Os dados são organizados em matrizes bidimensionais, nas quais cada linha representa uma condição de ensaio associada a um determinado ângulo de ataque e cada coluna representa uma amostra individual da célula de carga.'
    },
    
    // === NOVO BLOCO DE MÍDIAS AQUI ===
    media: {
      image1: {
        title: 'Modelo 3D da Bancada',
        description: 'Visualização da montagem mecânica e posicionamento da célula de carga para o ensaio aerodinâmico.',
        url: '/images/asa-celula.png' // <-- Coloque o nome real da sua imagem
      },
      image2: {
        title: 'Calibração da Célula de Carga',
        description: 'Testes práticos de aquisição e calibração do módulo HX711 para garantir a precisão da leitura de vibração.',
        url: '/images/Modelo_Tunel.jpeg' // <-- Coloque o nome real da sua imagem
      },
      gif1: {
        title: 'Aquisição de Dados (Interface)',
        description: 'Demonstração do ensaio em túnel de vento.',
        url: '/images/Modelo_Stall.gif' // <-- Coloque o nome real do seu GIF
      }
    },
    // =================================

    codeSnippet: {
      language: 'cpp',
      code: `// Rotina de Aquisição de Dados
#include "HX711.h"

#define calibration_factor -800000
#define LOADCELL_DOUT_PIN 32
#define LOADCELL_SCK_PIN 30

HX711 scale;

// Configuração de memória
const int MAX_LEITURAS = 20;
const int NUM_AMOSTRAS = 25;

float angulos[MAX_LEITURAS];
float medias[MAX_LEITURAS];
float dados_brutos[MAX_LEITURAS][NUM_AMOSTRAS];

int indiceLeitura = 0;

void realizarMedicao() {

  if (indiceLeitura >= MAX_LEITURAS)
    return;

  Serial.println("Digite o angulo de ataque (em graus): ");

  while (Serial.available() == 0) {}

  float angulo = Serial.parseFloat();

  while (Serial.available() > 0)
    Serial.read();

  float soma = 0;

  for (int i = 0; i < NUM_AMOSTRAS; i++) {

    float leitura = scale.get_units(1);

    dados_brutos[indiceLeitura][i] = leitura;

    soma += leitura;
  }

  angulos[indiceLeitura] = angulo;

  medias[indiceLeitura] =
    soma / NUM_AMOSTRAS;

  indiceLeitura++;
}`
    },
    result: 'O sistema possibilitou a realização de ensaios de forma organizada e repetível, mantendo as amostras individuais e os valores médios de cada condição de teste. A estrutura de aquisição permitiu correlacionar as variações de carregamento e vibração com o ângulo de ataque, fornecendo uma base de dados para a identificação e caracterização do início do stall na esteira da asa. O código-fonte completo deste firmware está disponível no GitHub.',
    githubUrl: 'https://github.com/Mateusjrs/Projetos/tree/main/aero-data-system'
  },
  {
    id: 'airbus-wing-structure',
    title: 'Projeto Estrutural de Conjunto Aeronáutico (Airbus Style)',
    category: 'CAD / 3D',
    shortDescription: 'Modelagem 3D avançada de montagem aeronáutica com perfis aerodinâmicos e longarinas estruturais.',
    technologies: ['SolidWorks', 'CAD', 'Análise Estrutural', 'Aerodinâmica'],
    imageUrl: 'https://images.unsplash.com/photo-1541185933-ef5d8ed016c2?auto=format&fit=crop&w=1200&q=80',
    modelPath: '/models/Airbus.glb',
    edrawingsUrl: '/models/Airbus.easm',
    featured: true,
    summary: 'Desenvolvimento de modelo CAD complexo focado em montagem mecânica, tolerâncias de encaixe e distribuição de esforços em superfícies sustentadoras.',
    problem: 'Necessidade de projetar uma junção estrutural rígida entre o perfil aerodinâmico e a longarina principal, minimizando peso sem comprometer a resistência mecânica a cargas transversais.',
    contribution: 'Modelagem paramétrica completa no SolidWorks, aplicação de nervuras internas de reforço estrutural, furações de montagem e otimização geométrica para exportação otimizada em ambientes web.',
    engineeringDetails: {
      calculations: 'Dimensionamento de parafusos de fixação sob cisalhamento e verificação de rigidez torcional na raiz da asa.',
      decisions: 'Uso de cavidades aliviadas nas nervuras internas para redução de massa estrutural (mass reduction) mantendo o momento de inércia.',
      dimensions: 'Comprimento de corda de 450mm com longarina central de perfil tubular.',
      components: ['Perfil aerodinâmico customizado', 'Longarina estrutural de seção retangular', 'Flanges de acoplamento e parafusos estruturais M4']
    },
    result: 'Modelo 100% parametrizado, validado para visualização em tempo real na web e pronto para inspeção técnica via eDrawings.'
  },
  {
    id: 'quilli-hardware-design',
    title: 'Projeto Eletrônico e Hardware — Quilli',
    category: 'Hardware',
    shortDescription: 'Desenvolvimento do esquemático e arquitetura de hardware para automação de aquários, focando em isolamento óptico, eletrônica de potência e controle de cargas.',
    technologies: [
      'Design de PCB / Esquemáticos',
      'Eletrônica de Potência',
      'Isolamento Óptico',
      'MOSFETs / Relés',
      'Comunicação RS-485',
      'Barramentos I2C e SPI'
    ],
    // A IMAGEM DE CAPA AGORA É O SEU ESQUEMÁTICO:
    imageUrl: '/images/esquematicoAquario.png', 
    featured: false,
    summary: 'O Quilli foi desenvolvido como um sistema embarcado completo, tendo um ESP32 como controlador central e uma arquitetura eletrônica dedicada para integrar sensores, atuadores, interfaces de usuário, comunicação e alimentação. O hardware foi organizado em blocos funcionais, separando os circuitos de processamento e sinais dos circuitos responsáveis pelo acionamento de potência.',
    problem: 'Controlar aquecedores, coolers de 12V e fitas de LED de alta potência através de um microcontrolador que opera em 3.3V (ESP32) apresenta riscos graves, como transientes, ruídos elétricos e até a queima do controlador. Era necessário desenvolver um projeto de hardware robusto, que isolasse a parte lógica da parte de potência, e que coubesse num layout enxuto.',
    contribution: 'Fui responsável pela engenharia e projeto elétrico do sistema. Especifiquei os estágios de acionamento utilizando optoacopladores, transistores BC817 e MOSFETs. Dimensionei a rede de distribuição de energia (HLK-5V e linhas de 12V) e estruturei a comunicação (SPI para TFT, I2C para RTC, e módulo RS-485 via MAX485).',
    engineeringDetails: {
      schematic: 'O projeto eletrônico foi desenvolvido a partir de um esquemático completo, onde estão representados o ESP32, fontes de alimentação, entradas, sensores, circuitos de acionamento, interfaces de comunicação, display, RTC e conectores. A organização em blocos facilita a identificação e montagem do sistema.',
      controller: 'O ESP32-DEVKIT-V1 atua como unidade central de processamento. Ele é responsável pela aquisição dos sinais analógicos, processamento da lógica, controle de atuadores via PWM, display gráfico e conectividade sem fio.',
      power: 'A alimentação utiliza uma fonte isolada HLK-5V para a lógica e possui uma linha de 12V destinada aos atuadores (cooler/luzes). A distribuição foi organizada para que o ESP32 comande cargas de maior potência sem submetê-lo diretamente às correntes dessas cargas.',
      thermostat: 'O aquecedor é acionado por um estágio dedicado com transistor BC817 e relé de 5V. Essa configuração permite que o pino do ESP32 atue de maneira isolada. Um diodo 1N4007 flyback é utilizado no circuito para proteção contra transientes da bobina do relé.',
      cooler: 'O cooler é acionado por um estágio eletrônico baseado em optoacoplador PC817 e MOSFET de potência, garantindo separação galvânica entre o controle (3.3V) e a potência (12V).',
      lighting: 'O sistema possui múltiplos canais independentes (lâmpadas e fitas LED) controlados via MOSFETs e optoacopladores PC817, habilitando ajuste de intensidade da iluminação via PWM sem sobrecarregar a fonte primária.',
      adc: 'O projeto incorpora um conversor analógico-digital ADS1115 (16 bits) via I2C para aquisição de sinais analógicos com maior resolução do que os ADCs internos do ESP32 permitem.',
      communication: 'Foi utilizado um transceptor MAX485 (Half-Duplex) para interface física RS-485. Sinais de controle do ESP32 alternam entre transmissão e recepção, permitindo integração futura com sensores ou atuadores industriais em rede Modbus.',
      rtc: 'O controle temporal utiliza um módulo RTC DS3231 via barramento I2C, mantido por uma bateria de lítio CR2032 (RTC Backup) para não perder as agendas programadas do aquário em caso de queda de energia.',
      display: 'O hardware disponibiliza barramento SPI de alta velocidade dedicado ao Display TFT, e botões físicos nas entradas do ESP (pull-up/pull-down) para interface de usuário (IHM).',
      protection: 'A arquitetura é focada na proteção do microcontrolador: uso intenso de optoacopladores (PC817), MOSFETs de nível lógico (Logic Level) com resistores de gate, e diodos de proteção. As cargas nunca encontram diretamente os pinos do ESP32.',
      bom: 'A lista de materiais (BOM) foi estruturada englobando toda a arquitetura de CIs, transistores, reguladores HLK, relés, capacitores de filtro, resistores limitadores e terminais de conexão padronizados tipo JST-XH.',
      hardwareComponents: [
        'ESP32-DEVKIT-V1',
        'Fonte Hi-Link HLK-5M05',
        'ADC de 16 bits ADS1115',
        'Transceptor RS-485 (MAX485)',
        'RTC DS3231 + Bateria CR2032',
        'Relé SRD-05VDC-SL-C',
        'Optoacopladores PC817',
        'Power MOSFETs (IRF)',
        'Transistores NPN BC817',
        'Diodo Flyback 1N4007',
        'Conectores tipo JST-XH'
      ],
      engineeringSkills: [
        'Projeto de Circuitos Eletrônicos (Schematic Design)',
        'Isolamento Galvânico e Eletrônica de Potência',
        'Dimensionamento de Relés e Flyback',
        'Acionamento de MOSFETs e Optoacopladores',
        'Barramentos industriais: RS-485 / Modbus',
        'Barramentos embarcados: I2C e SPI',
        'Interfaceamento ADC de alta resolução',
        'Estruturação de BOM'
      ]
    },
    
    // A Galeria foi removida e usamos apenas a seção Media:
    media: {
      schematicImg: {
        title: 'Esquemático Eletrônico do Quilli',
        description: 'Desenho unifilar completo separando os blocos de sensores, lógica e potência.',
        url: '/images/esquematicoAquario.png' // <-- Certifique-se de salvar sua foto do esquemático com este nome
      },
      pcb3d: {
        title: 'Modelo 3D da PCB / Engenharia de Layout',
        description: 'Visualização 3D da placa de circuito impresso projetada para abrigar a fonte HLK, o ESP32, os blocos de relés, MOSFETs e os conectores JST-XH.',
        url: '/images/pcbAquario.png' // <-- Certifique-se de salvar sua foto 3D com este nome
      }
    },
    result: 'O desenvolvimento arquitetural resultou em uma placa (hardware) centralizada e extremamente segura. A divisão física entre as seções lógica, medição e potência protege o processador principal de ruídos elétricos. O emprego de MOSFETs de potência junto a optoacopladores permitiu acionamento contínuo das rotinas térmicas e de PWM sem sobreaquecimento ou danos elétricos à unidade central.',
    githubUrl: 'https://github.com/mateusjrs/quilli-hardware'
  },
  {
    id: 'afrodite-autonomous-sumo',
    title: 'Afrodite — Robô Sumô Autônomo (3kg)',
    category: 'Hardware',
    shortDescription: 'Projeto de hardware e integração eletrônica para um robô de sumô autônomo da categoria 3kg, projetado para suportar até 100kg de peso virtual.',
    technologies: [
      'ESP32',
      'Design de PCB / Esquemáticos',
      'Sensores VL53L0X (ToF)',
      'Acelerômetro MPU6050',
      'Leitura PPM',
      'Controle de Motores',
      'Barramento I2C'
    ],
    imageUrl: 'https://images.unsplash.com/photo-1589254065878-42c9da997008?auto=format&fit=crop&w=1200&q=80', // Imagem ilustrativa de robótica
    featured: true,
    summary: 'A Afrodite é um projeto de robótica competitiva desenvolvido para a categoria Sumô Autônomo (3kg). O grande desafio mecânico e eletrônico deste projeto foi projetar um sistema capaz de gerar um "peso virtual" superior a 100kg (através de aerodinâmica/sucção) e, ao mesmo tempo, possuir um tempo de resposta extremamente rápido para localizar o oponente e desviar de bordas utilizando um conjunto avançado de sensores ToF e inerciais.',
    problem: 'Na categoria de sumô autônomo de 3kg, a tração e a velocidade de resposta são os fatores decisivos para a vitória. O desafio eletrônico consistia em integrar múltiplos sensores de distância (VL53L0X) via barramento I2C sem conflito de endereços, ler comandos de segurança (rádio PPM), monitorar a orientação (MPU6050) e acionar a lógica de motores sem que o ruído elétrico da alta corrente dos motores afetasse a estabilidade do ESP32.',
    contribution: 'Fui responsável pela arquitetura do hardware e elaboração do esquemático completo da placa principal. Desenvolvi a lógica de integração dos 5 sensores de distância VL53L0X gerenciando os pinos XSHUT, a interface do módulo inercial MPU6050 e a rede de distribuição de energia (Step Down 5V e reguladores 3V3). Também estruturei a leitura de sinais PPM do receptor de segurança e os barramentos para os sensores de borda.',
    engineeringDetails: {
      schematic: 'O esquemático foi desenhado separando claramente os blocos vitais do robô: alimentação, sensores de linha/borda, sensores de distância I2C, interface de rádio (PPM) e feedback visual (Display OLED e LEDs).',
      controller: 'O cérebro do robô é um ESP32-DEVKIT-V1, escolhido por sua alta capacidade de processamento (Dual Core), permitindo rodar a malha de controle dos motores em um núcleo enquanto processa a fusão de dados dos sensores I2C no outro.',
      power: 'Para suportar os picos de corrente e isolar a lógica, o projeto conta com um módulo Step Down dedicado para baixar a tensão da bateria para 5V, e reguladores g108433 para fornecer 3.3V limpo aos sensores e ao ESP32, com ampla filtragem por capacitores.',
      communication: 'O projeto faz uso intensivo do barramento I2C. Para utilizar 5 sensores VL53L0X simultaneamente, o ESP32 controla os pinos XSHUT de cada um (XSHUT1 a XSHUT5), inicializando-os um por vez e trocando seus endereços I2C dinamicamente no boot.',
      inputs: 'As entradas incluem 5 sensores ToF, sensores de borda (g1113), sensor inercial (MPU6050), e um receptor Nano R616XN para leitura do sinal PPM, que atua como chave de segurança/start remoto exigida pelas regras da competição.',
      display: 'O robô possui um Display OLED 128x64 e LEDs de debug para feedback visual instantâneo do estado da bateria e da leitura dos sensores durante os testes no dojô.',
      bom: 'A lista de materiais foi rigorosamente mapeada, totalizando 40 resistores (100Ω, 10kΩ, 20kΩ), 19 capacitores de filtro, 12 LEDs de status, receptor IR TSOP1838, além de conectores padronizados para módulos PPM, Display e Motores.',
      hardwareComponents: [
        'ESP32-DEVKIT-V1',
        'Sensores ToF (VL53L0X / I2c)',
        'Módulo Inercial (MPU6050)',
        'Sensores de Borda (g1113)',
        'Receptor PPM (Orange R616XN Nano)',
        'Módulo Step Down 5V',
        'Regulador 3.3V (g108433)',
        'Display OLED 128x64',
        'Receptor IR TSOP1838',
        'Rede de Capacitores (100nF, 10uF) e Resistores'
      ],
      engineeringSkills: [
        'Arquitetura de Hardware para Robótica',
        'Gerenciamento de Barramento I2C Multi-Device',
        'Condicionamento de Energia (Step-Down e LDOs)',
        'Design de Esquemático Eletrônico',
        'Integração de Sensores Inerciais e Ópticos',
        'Leitura de Sinais de Rádio (PPM)'
      ]
    },
    gallery: [
      // Você precisará salvar essas imagens na pasta public/images/ com os nomes abaixo
      '/images/afrodite-schematic-1.png', 
      '/images/afrodite-schematic-2.png'
    ],
    result: 'O projeto de hardware garantiu que o ESP32 operasse com energia limpa e estável, essencial para o funcionamento sem travamentos da malha de controle de alta velocidade. A estratégia de usar os pinos XSHUT permitiu a leitura simultânea de 5 sensores ToF, cobrindo um amplo campo de visão frontal e lateral. A placa se tornou o núcleo confiável que permitiu ao robô focar exclusivamente em sua estratégia de combate e no acionamento do sistema de vácuo (peso virtual).',
    githubUrl: 'https://github.com/mateusjrs/afrodite-sumo-robot' // Substitua pelo link correto do repositório, se houver
  },
  {
    id: 'mini-sumo-autonomous',
    title: 'Mini Sumô Autônomo — Alta Densidade e Vácuo',
    category: 'Hardware',
    shortDescription: 'Miniaturização extrema da arquitetura do robô Afrodite, mantendo o sistema de peso virtual e múltiplos sensores ToF em uma PCB de alta densidade.',
    technologies: [
      'ESP32',
      'Design de PCB SMD',
      'Alta Densidade (High-Density Routing)',
      'Sensores VL53L0X',
      'Controle Inercial',
      'Eletrônica de Potência Compacta'
    ],
    imageUrl: 'https://images.unsplash.com/photo-1517077304055-6e89abbf09b0?auto=format&fit=crop&w=1200&q=80', // Imagem sugerida (tecnologia/microeletrônica)
    featured: false,
    summary: 'Adaptação da arquitetura eletrônica e de controle do robô Afrodite (3kg) para a categoria Mini Sumô (500g). O desafio principal consistiu em compactar todo o processamento dual-core do ESP32, a leitura paralela de 5 sensores de distância (VL53L0X), o controle inercial e a eletrônica de potência da turbina de vácuo (peso virtual) em uma área rigorosamente restrita a 10x10 cm.',
    problem: 'A redução drástica de volume e massa agrava severamente os problemas de interferência eletromagnética (EMI) e dissipação térmica. Posicionar trilhas de alta corrente (para os motores e a turbina de sucção) milímetros ao lado de barramentos de comunicação sensíveis (I2C) exigia um roteamento de placa de circuito impresso (PCB) extremamente rigoroso para evitar ruídos e travamentos do microcontrolador.',
    contribution: 'Realizei a transição do projeto de módulos comerciais para componentes discretos totalmente SMD (Surface-Mount Device). Refatorei o esquemático e executei o roteamento da placa garantindo o isolamento entre a área lógica e a de potência. Mantive a topologia XSHUT para o I2C e implementei reguladores monolíticos de alta frequência para reduzir o tamanho físico dos indutores da fonte.',
    engineeringDetails: {
      schematic: 'Transição completa para componentes SMD (pacotes 0603 e 0402). Eliminação de módulos "prontos" (breakout boards) em favor da integração dos circuitos integrados (como o MPU6050 e os próprios sensores ToF) diretamente na placa-mãe do robô.',
      controller: 'Uso otimizado do ESP32 soldado diretamente na placa, mantendo o processamento paralelo (FreeRTOS) que separa a malha de navegação (leitura I2C) da malha de controle PID dos motores.',
      power: 'Redesenho radical da fonte de alimentação. Substituição dos módulos Step-Down grandes por CIs reguladores buck monolíticos de alta frequência, que requerem capacitores e indutores drasticamente menores e reduzem o peso global.',
      communication: 'Roteamento focado do barramento I2C, utilizando trilhas mais curtas e vias de blindagem (ground shielding) para comunicação limpa com o MPU6050 e os 5 sensores VL53L0X multiplexados.',
      dimensions: 'Design estrutural confinado às regras da categoria Mini Sumô (máximo de 10x10 cm). A própria PCB precisou ser tratada mecânicamente para acomodar a estrutura do sistema de vácuo sem exceder o limite.',
      protection: 'Implementação de planos de terra (GND planes) reforçados e vias térmicas (thermal vias) sob os controladores de motor (ponte H) para dissipar o calor gerado durante as disputas travadas no dojô, onde a corrente sobe drasticamente.',
      hardwareComponents: [
        'ESP32 (SMD)',
        'Sensores ToF VL53L0X (Integrados)',
        'Acelerômetro MPU6050',
        'Receptor PPM Nano',
        'Ponte H SMD (Alta Corrente)',
        'Reguladores Step-Down Monolíticos',
        'Componentes Passivos SMD 0603/0402'
      ],
      engineeringSkills: [
        'Roteamento de PCB de Alta Densidade (HDI)',
        'Miniaturização de Hardware',
        'Controle de EMI/EMC em Espaços Reduzidos',
        'Gestão Térmica em PCB (Thermal Vias)',
        'Soldagem e Integração de Componentes SMD Discretos'
      ]
    },
    result: 'A miniaturização foi bem-sucedida, resultando em uma PCB extremamente compacta que suporta todo o sistema de sucção (vácuo) e processamento avançado do irmão maior de 3kg. O layout blindou os sinais lógicos contra o ruído da ponte H, garantindo um robô com tempo de reação impecável e força de tração completamente desproporcional à sua categoria.',
    githubUrl: 'https://github.com/mateusjrs/mini-sumo-robot' // Atualize com o link real, se possuir
  },
 {
    id: 'can-bus-obd2-hacking',
    title: 'Scanner Automotivo e Engenharia Reversa via CAN-BUS',
    category: 'Hardware',
    
    shortDescription: 'Desenvolvimento de um scanner OBD-II sem fio com ESP32, implementando stack ISO-TP customizada para decodificação de telemetria, DTCs e Freeze Frames.',
    
    technologies: [
      'C/C++',
      'ESP32',
      'CAN-BUS (MCP2515)',
      'Protocolo ISO-TP',
      'OBD-II (SAE J1979)',
      'Bluetooth Serial',
      'Sistemas Embarcados'
    ],
    
    imageUrl: 'https://images.unsplash.com/photo-1492144534655-ae79c964c9d7?auto=format&fit=crop&w=1200&q=80',
    
    featured: true,
    
    summary: 'Projeto completo de um datalogger e scanner automotivo sem fio. O sistema utiliza um ESP32 e um controlador MCP2515 para acessar a rede CAN veicular. Além de ler dados brutos (RAW), o firmware foi projetado para atuar como um scanner OBD-II ativo, solicitando o chassi (VIN), varrendo Códigos de Falha (DTCs), extraindo o contexto do erro (Freeze Frame) e transmitindo a telemetria em tempo real via Bluetooth.',
    
    problem: 'Ferramentas de diagnóstico comerciais costumam atuar como "caixas pretas", ocultando os dados RAW e limitando a análise de engenharia. O desafio era construir um hardware de baixo custo e desenvolver, do zero, uma stack de comunicação capaz não apenas de ler mensagens simples de 8 bytes, mas de remontar pacotes longos e gerenciar o fluxo de dados com a ECU em diferentes taxas de transmissão.',
    
    contribution: 'Desenvolvi o firmware em C++ implementando uma arquitetura baseada em Máquina de Estados para orquestrar as requisições OBD-II de forma não-bloqueante. Programei a detecção automática de baud rate (500k, 250k, 125k), a rotina de PID Discovery e, principalmente, uma camada ISO-TP customizada para gerenciar a fragmentação de mensagens, permitindo o tráfego de pacotes extensos como a leitura do VIN (Modo 09) e falhas ativas (Modo 03).',
    
    engineeringDetails: {
      firmwareArchitecture: 'Implementação de uma Máquina de Estados (OBD_IDLE, REQUEST_VIN, REQUEST_DTC, REQUEST_FF) para evitar o bloqueio do processador e garantir a prioridade máxima para a leitura do barramento CAN através de interrupções (emuladas por polling rápido).',
      
      isoTpStack: 'Desenvolvimento manual do protocolo de transporte ISO 15765-2 (ISO-TP). O firmware é capaz de processar Single Frames, identificar First Frames, responder com mensagens de controle de fluxo (Flow Control / CTS) respeitando os tempos de STmin, e remontar Consecutive Frames em um buffer local.',
      
      diagnostics: 'Suporte a múltiplos modos OBD-II: Mode 01 (Live Data), Mode 02 (Freeze Frame), Mode 03 (Leitura de DTCs), Mode 04 (Limpeza de falhas com trava de segurança de RPM) e Mode 09 (VIN).',
      
      hardware: 'Utilização do microcontrolador ESP32 integrado ao controlador CAN MCP2515 (via barramento SPI a 8MHz) em modo promíscuo (Filtros e Máscaras em 0x00) para capturar tanto mensagens de diagnóstico (0x7DF-0x7EF) quanto tráfego proprietário das montadoras.',
      
      connectivity: 'Integração com a stack Bluetooth clássica do ESP32 (BluetoothSerial) para streaming bidirecional da telemetria e recepção de comandos do usuário (ex: limpar falhas) de forma remota.',
      
      safety: 'Implementação de fallback de endereçamento (0x7DF, 0x7E0, 0x7E1) para aumentar a compatibilidade entre diferentes módulos do motor e rotina de segurança que bloqueia o comando de limpeza de falhas caso o motor esteja em funcionamento (RPM > 0).'
    },
    
    codeSnippet: {
      language: 'cpp',
      code: `// Implementação customizada de Flow Control (ISO-TP) para CAN-BUS
if (pciType == 0x1) { // FIRST FRAME detectado
  isotp.rxId = rxId;
  isotp.txId = rxId - 0x8; // Define o ID de resposta para a ECU

  isotp.length = ((data[0] & 0x0F) << 8) | data[1];
  isotp.offset = len - 2;
  memcpy(isotp.buffer, &data[2], isotp.offset);

  isotp.expected_sn  = 1;
  isotp.active       = true;

  // Monta e envia o FLOW CONTROL - Clear To Send (CTS)
  uint8_t fc[8] = {
    0x30,  // PCI 3 (Flow Control) | Status 0 (CTS)
    0x00,  // Block Size = 0 (Ilimitado)
    0x02,  // STmin = 2 ms entre frames
    0, 0, 0, 0, 0
  };

  CAN.sendMsgBuf(isotp.txId, 0, 8, fc); // Autoriza a ECU a continuar o envio
  return false;
}`
    },
    
    result: 'O hardware e o firmware operaram com sucesso, estabelecendo comunicação estável, identificando os parâmetros suportados pelo veículo (PID Discovery) e processando telemetria e códigos de falha em tempo real através da interface Bluetooth, validando a robustez da stack ISO-TP desenvolvida.',
    
    githubUrl: 'https://github.com/mateusjrs/esp32-can-scanner' // <-- Ajuste o link do seu repositório aqui
  },
 {
    id: 'ir-universal-cloner',
    title: 'Clonador Universal Infravermelho para Automação',
    category: 'IoT',
    shortDescription: 'Sistema embarcado IoT com termostato preditivo e interface universal baseada em macros C para abstração de 19 protocolos de climatização.',
    technologies: [
      'ESP32', 
      'C/C++', 
      'IoT', 
      'MQTT', 
      'SmartConfig',
      'Controle Preditivo (EMA)',
      'IR Protocols'
    ],
    imageUrl: 'https://images.unsplash.com/photo-1558346490-a72e53ae2d4f?auto=format&fit=crop&w=1200&q=80',
    featured: false,
    summary: 'Projeto de um nó IoT (ITW-600) para automação de centrais de ar-condicionado. O sistema realiza engenharia reversa e reprodução de sinais infravermelhos (IR), e integra um sensor de temperatura (DS18B20) para operar como um termostato preditivo autônomo conectado à rede via MQTT. O projeto foi projetado para ambientes corporativos, suportando provisionamento via SmartConfig e atualização contínua do banco de dados na EEPROM.',
    problem: 'O ecossistema de climatização corporativa possui dezenas de marcas (Samsung, Carrier, Midea, Hitachi, etc.), cada uma com sua própria estrutura de codificação IR. Era necessário criar um dispositivo universal, capaz de autodetectar o modelo do equipamento, controlar ativamente a temperatura do ambiente sem oscilações bruscas (liga/desliga frequente) e comunicar-se de forma segura com um broker MQTT, sem travar o processador durante as tentativas de reconexão de rede.',
    contribution: 'Desenvolvi um firmware no padrão RTOS, onde a comunicação de rede e o provisionamento (ESP-Touch/SmartConfig) operam em background, enquanto o controle térmico roda continuamente. A principal contribuição arquitetural foi a criação de uma interface genérica de controle (ACInterface) estruturada via Macros C e ponteiros de função, permitindo integrar 19 bibliotecas de fabricantes diferentes em uma única lista gerenciável, reduzindo drasticamente o consumo de memória RAM do ESP32.',
    engineeringDetails: {
      firmwareArchitecture: 'Implementação de arquitetura baseada em abstração de ponteiros de função (`ACInterface`) para polimorfismo em C, instanciando drivers apenas sob demanda através de macros de pré-compilação (`DEF_DRIVER_...`).',
      thermostatControl: 'O controle de temperatura não usa apenas limite superior/inferior. Implementei um Filtro de Média Móvel Exponencial (EMA) sobre o sinal RAW do sensor e o cálculo da Taxa de Variação (Derivativa dT/dt). O algoritmo prediz a inércia térmica da sala e ajusta o setpoint do AC de forma suave, respeitando margens de overshoot e tempos de acomodação (Settling Time).',
      connectivity: 'Provisionamento de rede WiFi sem contato físico (headless) utilizando o protocolo SmartConfig/ESP-Touch. O dispositivo entra na rede e se cadastra automaticamente no Broker MQTT (iotew.com.br), gerenciando payloads dinâmicos (JSON) e tolerância a falhas (reconexões automáticas e salvamento de estado de pânico na EEPROM).',
      memoryManagement: 'Estruturação avançada de gravação na memória Flash/EEPROM, salvando as credenciais de rede, perfil do driver AC autodetectado e configurações do termostato, garantindo a recuperação integral do estado do sistema após quedas de energia e proteção contra loops infinitos de reset de Watchdog.',
      components: [
        'Microcontrolador ESP32', 
        'Sensor de Temperatura 1-Wire DS18B20', 
        'Receptor IR e Emissor IR de Alta Potência', 
        'LED RGB de Status de Conectividade'
      ],
      engineeringSkills: [
        'Polimorfismo e abstração de hardware em C/C++',
        'Teoria de Controle Aplicada (Derivativa e Filtro Exponencial)',
        'FreeRTOS (Task Management e Multiplexing)',
        'Stack ESP-IDF e SmartConfig WiFi',
        'Protocolo MQTT IoT'
      ]
    },
    codeSnippet: {
      language: 'cpp',
      code: `// Termostato Preditivo (Cálculo Derivativo e Filtro EMA)
void runThermostat() {
  if (activeAC == nullptr || !thermostatEnabled) return;
  
  if (millis() - last_read_time > 5000) {
    sensors.requestTemperatures();
    T_raw = sensors.getTempCByIndex(0);

    // Filtro Exponencial (EMA) para suavizar ruído térmico
    T_filt = alpha * T_raw + (1.0 - alpha) * T_filt;

    // Cálculo da Taxa de Variação (Derivativa dT/dt em graus/min)
    float dt = (millis() - last_read_time) / 1000.0;
    if (dt > 0) dTdt = (T_filt - T_prev) / dt * 60.0;

    T_prev = T_filt;
    last_read_time = millis();

    float error = T_filt - targetTemp;

    // Lógica Preditiva de Ambiente Quente
    if (error > hysteresis) {
      // Se a temperatura já está caindo rápido (-0.2 C/min), não envia sinal para baixar mais
      if (dTdt < -0.2) return; 
      
      if (millis() - last_action_time >= settling_time) {
        AC_setpoint = constrain(--AC_setpoint, AC_MIN_TEMP, AC_MAX_TEMP);
        activeAC->setTemp(activeAC->obj, AC_setpoint);
        activeAC->send(activeAC->obj);
        last_action_time = millis();
      }
    }
  }
}`
    },
    result: 'Dispositivo IoT altamente estável, capaz de controlar ativamente o conforto térmico e minimizar gastos energéticos evitando oscilações excessivas. A abstração universal (`ACInterface`) reduziu o código repetitivo, permitindo suporte "plug-and-play" para novas marcas de AC no futuro através de atualizações OTA (Over-The-Air).',
    githubUrl: 'https://github.com/mateusjrs/ir-universal-cloner' // <-- Coloque o seu link
  },
  {
    id: 'naca-5312-aerodynamic-analysis',
    title: 'Análise Aerodinâmica e Otimização Paramétrica do NACA 5312',
    category: 'Aerospace',
    shortDescription: 'Estudo paramétrico do perfil NACA 5312 utilizando XFLR5 e Python para investigar a influência da geometria sobre sustentação, arrasto, eficiência, pressão e camada limite.',
    technologies: [
      'XFLR5',
      'Python',
      'Aerodinâmica',
      'Método dos Painéis',
      'Camada Limite',
      'Análise Paramétrica',
      'Processamento de Dados'
    ],
    imageUrl: 'https://images.unsplash.com/photo-1559297434-fae8a1916a79?auto=format&fit=crop&w=1200&q=80',
    
    // === BASTA ADICIONAR ESTE BLOCO AQUI ===
    gallery: [
      '/images/naca-cl-alpha.png',
      '/images/naca-efficiency.png',
      '/images/naca-cd-alpha.png',
      '/images/naca-boundary-layer.png'
    ],
    // =======================================

    featured: true,
    summary: 'Estudo computacional do perfil aerodinâmico NACA 5312 com o objetivo de investigar como alterações individuais em sua geometria modificam o desempenho aerodinâmico...',
    problem: 'O desempenho de um perfil aerodinâmico depende diretamente de sua geometria e das condições do escoamento. O desafio do projeto foi separar a influência de diferentes parâmetros geométricos sobre o comportamento do NACA 5312, evitando analisar apenas o perfil como uma configuração única. Para isso, foram desenvolvidas variações controladas de espessura, posição da espessura máxima, camber, posição do camber máximo, raio do bordo de ataque e geometria do bordo de fuga, permitindo comparar sistematicamente os efeitos de cada modificação.',
    contribution: 'Participei do desenvolvimento da análise aerodinâmica do NACA 5312, estruturando as comparações entre diferentes geometrias e interpretando os resultados das simulações. O estudo envolveu a configuração das análises no XFLR5, avaliação das curvas aerodinâmicas e processamento dos resultados em Python. Foram analisadas as relações entre geometria e coeficientes aerodinâmicos, distribuição de pressão, camada limite e centro de pressão, permitindo identificar quais modificações apresentavam maior impacto sobre o desempenho do perfil.',
    engineeringDetails: {
      baseline: 'O perfil de referência utilizado foi o NACA 5312. Sua nomenclatura indica 5% de camber máximo, localizado a 30% da corda, e espessura máxima correspondente a 12% da corda. O perfil foi utilizado como referência para todas as modificações geométricas realizadas ao longo do estudo.',
      simulation: 'As simulações foram realizadas utilizando o XFLR5, baseado em métodos de painel combinados a modelos de camada limite. As condições analisadas abrangeram números de Reynolds entre 4 × 10^6 e 6 × 10^6, números de Mach entre 0,15 e 0,22 e ângulos de ataque entre -2° e 14°.',
      postProcessing: 'Os resultados das simulações foram exportados e posteriormente tratados em Python. O processamento permitiu organizar os dados, gerar gráficos comparativos e avaliar de maneira consistente as diferenças entre as configurações geométricas analisadas.',
      parameters: 'O estudo foi estruturado de forma paramétrica, alterando individualmente características geométricas do perfil enquanto os demais parâmetros eram mantidos constantes. Essa abordagem permitiu associar diretamente cada alteração geométrica às mudanças observadas no comportamento aerodinâmico.',
      thickness: 'Foi analisada a influência da espessura máxima e de sua posição ao longo da corda. Foram comparadas configurações com espessura de 8% e 16%, além do deslocamento da posição da espessura máxima de 30% para 40% da corda. Os resultados mostraram que a posição da espessura pode ser tão relevante quanto seu valor absoluto, especialmente em relação ao arrasto, separação do escoamento e comportamento em elevados ângulos de ataque.',
      camber: 'A influência do camber também foi investigada por meio de configurações com 3% e 6% de curvatura máxima, além do deslocamento da posição do camber máximo de 30% para 40% da corda. O aumento do camber apresentou forte influência sobre a sustentação e sobre a eficiência aerodinâmica, enquanto o deslocamento da curvatura alterou a distribuição de pressão e a posição do centro de pressão.',
      leadingEdge: 'O raio do bordo de ataque foi analisado por meio de três configurações relativas ao valor original: 0,80R, 1,12R e 1,15R. A análise mostrou que alterações no raio modificam principalmente características associadas à região frontal e à transição do escoamento, apresentando influência relativamente limitada sobre os coeficientes aerodinâmicos globais.',
      trailingEdge: 'Também foi investigada a influência da forma e da espessura do bordo de fuga. As modificações permitiram avaliar o efeito da distribuição de carga na região posterior do perfil, incluindo alterações no momento aerodinâmico, no centro de pressão e no comportamento da esteira.',
      aerodynamicCurves: [
        'CL × α (Sustentação)',
        'CD × α (Arrasto)',
        'CM × α (Momento)',
        'CL/CD × α (Eficiência)',
        'Identificação de CL,max',
        'Identificação do ângulo de estol',
        'Análise do momento de arfagem'
      ],
      pressureAnalysis: 'Foram analisadas distribuições do coeficiente de pressão Cp ao longo da corda para diferentes ângulos de ataque. A análise permitiu identificar picos de sucção, alterações na distribuição de pressão e mudanças no carregamento aerodinâmico provocadas pelas modificações geométricas.',
      boundaryLayer: 'A camada limite foi analisada por meio da espessura de deslocamento ao longo da corda. Essa análise permitiu avaliar como diferentes geometrias modificam o crescimento da camada limite, os gradientes de pressão e a tendência à separação do escoamento.',
      centerOfPressure: 'O comportamento do centro de pressão foi avaliado em função do ângulo de ataque. A posição do centro de pressão foi utilizada para relacionar alterações na distribuição de pressão com mudanças no momento aerodinâmico e nas características de estabilidade longitudinal do perfil.',
      analysis: 'Ao todo, foram gerados mais de 40 gráficos para comparar os diferentes cenários. A análise conjunta dos resultados permitiu avaliar os efeitos da geometria não apenas sobre sustentação e arrasto, mas também sobre eficiência aerodinâmica, momento, pressão, camada limite e centro de pressão.',
      mainFindings: [
        'Aumento do CL associado ao aumento do Reynolds, camber e modificações no bordo de fuga.',
        'Aumento do arrasto associado à redução do Reynolds, aumento da espessura e camber.',
        'Aumento do camber apresentou forte influência positiva sobre a sustentação e a eficiência.',
        'O raio do bordo de ataque apresentou influência pequena nos coeficientes aerodinâmicos globais.',
        'O deslocamento da espessura máxima e do camber modificou significativamente a distribuição de pressão.',
        'As alterações apresentaram efeitos diferentes dependendo do ângulo de ataque analisado.'
      ],
      engineeringSkills: [
        'Modelagem de perfis aerodinâmicos',
        'Análise paramétrica',
        'Aerodinâmica computacional (CFD Básico)',
        'Método dos Painéis',
        'Análise de camada limite',
        'Distribuição de pressão',
        'Processamento de dados (Python)',
        'Geração e interpretação de curvas',
        'Análise de estabilidade longitudinal'
      ]
    },
    result: 'A análise demonstrou que pequenas alterações geométricas podem produzir mudanças significativas no comportamento aerodinâmico do perfil, embora a influência de cada parâmetro dependa da faixa de ângulo de ataque e das condições de escoamento. O aumento do camber e determinadas alterações na geometria do bordo de fuga apresentaram forte influência sobre a sustentação e a eficiência. O estudo evidenciou a importância da distribuição de pressão e do comportamento da camada limite para compreender as diferenças observadas nas curvas. A metodologia permitiu relacionar diretamente modificações geométricas com desempenho aerodinâmico.',
    githubUrl: 'https://github.com/mateusjrs/naca-5312-analysis', // <-- Altere para o seu repositório real
    report: 'Relatório técnico completo: Análise do Perfil Aerodinâmico NACA 5312.'
  },
  {
    id: 'electrical-load-analysis-cessna',
    title: 'Análise de Carga Elétrica (ELA) - Cessna 182Q',
    category: 'Aerospace',
    shortDescription: 'Estudo normativo e cálculo de balanceamento elétrico aeronáutico para projeto de modificação de aeronave.',
    technologies: ['Sistemas Aeronáuticos', 'FAR/RBAC', 'Excel / MATLAB'],
    imageUrl: 'https://images.unsplash.com/photo-1540962351504-03099e0a754b?auto=format&fit=crop&w=1200&q=80',
    featured: false,
    summary: 'Elaboração de um documento técnico formal de Análise de Carga Elétrica (Electrical Load Analysis) para garantir a segurança operacional e regulamentar do sistema elétrico de um Cessna 182Q.',
    problem: 'Ao adicionar ou modificar aviônicos em uma aeronave, é mandatório comprovar junto às autoridades aeronáuticas que os barramentos e a geração (alternador/bateria) suportam as novas demandas em todas as fases do voo.',
    contribution: 'Levantamento completo do consumo de todos os equipamentos (rádios, luzes, aviônicos, atuadores) e estruturação do balanço energético considerando fases de táxi, decolagem, cruzeiro e pouso.',
    engineeringDetails: {
      calculations: 'Somatório de correntes por barramento, cálculo de capacidade residual do alternador e tempo de autonomia da bateria em cenário de emergência (falha de geração dupla).',
      decisions: 'Metodologia de cálculo rigorosamente baseada na ASTM F2490 e requisitos do RBAC/FAR Part 23 para garantir conformidade de certificação.',
      components: ['Sistemas de Geração (Alternador 60A)', 'Bateria (Chumbo-Ácido)', 'Barramentos Primário e de Aviônicos']
    },
    result: 'Relatório técnico consistente demonstrando margem de segurança operacional positiva, aplicável a processos reais de certificação e engenharia.'
  },
  {
    id: 'quilli-smart-aquarium',
    title: 'Aquário Inteligente',
    category: 'IoT',
    shortDescription: 'Sistema embarcado para automação de aquário, integrando controle de temperatura, alimentação programada, iluminação com intensidade ajustável e conectividade Wi-Fi.',
    technologies: [
      'C/C++',
      'ESP32',
      'IoT',
      'Wi-Fi',
      'MQTT',
      'TFT',
      'Sensores de temperatura',
      'RTC',
      'EEPROM',
      'PWM',
      'Sistemas Embarcados'
    ],
    imageUrl: '/images/Aquario.jpeg', // Imagem principal provisória
    featured: true,
    summary: 'Desenvolvimento do Quilli, um aquário inteligente baseado em ESP32 desenvolvido para automatizar as principais tarefas de manutenção e controle do aquário. O sistema integra controle automático de temperatura, alimentação programada, iluminação com ajuste de intensidade e horários, interface gráfica TFT e conectividade Wi-Fi. A arquitetura foi desenvolvida para permitir que as diferentes funções operem de forma independente e coordenada, reduzindo a necessidade de intervenção manual do usuário.',
    problem: 'A manutenção de um aquário envolve tarefas que precisam ser realizadas de maneira periódica e consistente, como controlar a temperatura da água, alimentar os peixes em horários determinados e reproduzir ciclos adequados de iluminação. O objetivo do projeto foi desenvolver um sistema capaz de automatizar essas funções e centralizar seu controle em uma única plataforma, permitindo configurar horários, intensidades e parâmetros diretamente pela interface do equipamento ou remotamente.',
    contribution: 'Atuei no desenvolvimento do firmware e na integração entre hardware e software do Quilli. Implementei as rotinas de controle de temperatura, acionamento do termostato e cooler, programação dos horários de alimentação, controle da intensidade e dos horários da iluminação, além da interface gráfica e comunicação Wi-Fi/MQTT. Também trabalhei na organização do firmware em diferentes tarefas e temporizadores para que as funções de monitoramento, controle e comunicação pudessem operar simultaneamente.',
    engineeringDetails: {
      systemArchitecture: 'O Quilli utiliza um ESP32 como controlador principal, responsável pelo processamento das informações dos sensores, execução das rotinas de automação, controle dos atuadores, atualização da interface e comunicação com o servidor. O firmware foi dividido em módulos específicos para gerenciamento de memória, comunicação, interface, sensores e funções de controle.',
      temperatureControl: 'O sistema realiza o monitoramento contínuo da temperatura da água e permite ao usuário definir uma temperatura desejada e uma faixa de controle. A partir da temperatura medida, o firmware determina automaticamente qual atuador deve ser acionado para aproximar a água da temperatura configurada.',
      thermostat: 'O controle de aquecimento é realizado por meio de um termostato acionado pelo ESP32. Quando a temperatura está abaixo da condição desejada, o sistema pode ativar o aquecedor e mantê-lo funcionando até que a temperatura alcance a faixa estabelecida. O firmware também possui limites de segurança para evitar o acionamento do controle quando a temperatura medida estiver fora de uma faixa válida.',
      cooler: 'Quando a temperatura da água ultrapassa a faixa configurada, o sistema pode acionar um cooler para realizar o resfriamento. Dessa forma, o Quilli consegue atuar tanto no aquecimento quanto no resfriamento, mantendo a temperatura próxima ao valor definido pelo usuário.',
      temperatureSafety: 'Foram definidos limites mínimo e máximo para a temperatura considerada válida. Caso a leitura ultrapasse esses limites, o sistema interrompe o acionamento do aquecedor e do cooler, desabilitando temporariamente o controle automático para evitar que uma leitura inválida provoque um acionamento inadequado.',
      feedingControl: 'O sistema possui um alimentador automático capaz de executar rotinas de alimentação previamente programadas. O usuário pode definir os horários em que a alimentação deve ocorrer e a quantidade de acionamentos do mecanismo, permitindo automatizar completamente essa tarefa.',
      feedingSchedule: 'As rotinas de alimentação são associadas ao relógio de tempo real do sistema. O ESP32 compara continuamente o horário atual com os horários programados e executa automaticamente o alimentador quando uma rotina é atingida. O sistema também mantém informações sobre as rotinas configuradas e os próximos horários de alimentação.',
      feedingMemory: 'As configurações das rotinas são armazenadas em memória não volátil, permitindo preservar os horários e parâmetros configurados mesmo após uma reinicialização do equipamento.',
      lightingControl: 'O Quilli possui controle independente de diferentes canais de iluminação. Cada canal pode ter sua intensidade configurada, permitindo adaptar a iluminação às necessidades do aquário e criar diferentes cenários de funcionamento.',
      lightingIntensity: 'A intensidade das lâmpadas pode ser ajustada pelo sistema, permitindo controlar a quantidade de luz fornecida ao aquário. O controle da intensidade é realizado de forma independente para os diferentes canais de iluminação, possibilitando combinações distintas entre eles.',
      lightingSchedule: 'Além da intensidade, o usuário pode programar os horários de funcionamento da iluminação. O sistema utiliza o relógio de tempo real para determinar quando cada rotina deve ser iniciada e encerrada, automatizando o ciclo diário de iluminação.',
      lightingRoutines: 'O firmware permite armazenar diferentes rotinas de iluminação, associando horários a intensidades específicas para os canais disponíveis. Dessa maneira, é possível criar diferentes períodos de iluminação ao longo do dia em vez de utilizar apenas um estado ligado/desligado.',
      realTimeClock: 'O sistema utiliza um RTC (Real Time Clock) para manter o horário utilizado pelas funções de automação. O relógio é utilizado principalmente pelas rotinas de alimentação e iluminação, garantindo que os eventos programados sejam executados nos horários definidos pelo usuário.',
      interface: 'Foi desenvolvida uma interface gráfica utilizando um display TFT para permitir a interação direta com o aquário. A interface apresenta informações de temperatura, estado dos atuadores, iluminação, alimentação e conexão Wi-Fi, além de disponibilizar menus para configuração das principais funções.',
      wifi: 'O ESP32 possui conectividade Wi-Fi para permitir a configuração e comunicação do sistema com uma infraestrutura externa. O firmware também possui mecanismos para gerenciamento da conexão e reconexão da rede.',
      mqtt: 'A comunicação MQTT permite que parâmetros e comandos relacionados ao aquário sejam enviados e recebidos remotamente. Essa arquitetura possibilita integrar o Quilli a uma plataforma IoT e controlar funções do equipamento sem depender exclusivamente da interface física.',
      memory: 'A EEPROM é utilizada para armazenar configurações que precisam permanecer disponíveis após o desligamento do equipamento. Dessa forma, parâmetros de funcionamento, rotinas de iluminação e alimentação podem ser recuperados durante a inicialização do sistema.',
      multitasking: 'A arquitetura do firmware foi desenvolvida para executar diferentes funções de maneira coordenada. Temporizadores e tarefas independentes são utilizados para lidar com leitura de temperatura, atualização do relógio, comunicação com o servidor, atualização da interface, controle da iluminação, alimentação e controle térmico.',
      mainFunctions: [
        'Monitoramento da temperatura da água',
        'Controle automático do termostato',
        'Controle automático do cooler para resfriamento',
        'Configuração da temperatura desejada',
        'Programação dos horários de alimentação',
        'Controle automático do alimentador',
        'Configuração da intensidade da iluminação',
        'Controle independente dos canais de iluminação',
        'Programação dos horários de iluminação',
        'Criação de rotinas de iluminação',
        'Interface gráfica TFT',
        'Conectividade Wi-Fi',
        'Comunicação MQTT',
        'Armazenamento em EEPROM',
        'Execução multitarefa (Timers/Tasks)'
      ],
      engineeringSkills: [
        'Desenvolvimento de firmware C/C++',
        'Programação ESP32',
        'Controle de temperatura',
        'Automação e controle de atuadores',
        'Sinais PWM',
        'Comunicação MQTT e Wi-Fi',
        'Desenvolvimento de interface gráfica',
        'Integração com RTC e EEPROM',
        'Gerenciamento de tarefas e multithreading'
      ]
    },
    media: {
      gif1: {
        title: 'Interface e controle do Quilli',
        description: 'Demonstração da interface TFT e da navegação entre as funções de controle do aquário.',
        url: '/images/AquarioFuncionamento2.gif' // Troque pelo caminho real do seu GIF
      },
      gif2: {
        title: 'Automação do aquário',
        description: 'Demonstração do funcionamento automático da rotina de alimentação e iluminação.',
        url: '/images/AquarioFuncionamento.gif' // Troque pelo caminho real do seu GIF
      },
      image1: {
        title: 'Protótipo do Quilli',
        description: 'Imagem do protótipo, mostrando o hardware e os principais componentes (ESP32, TFT, Sensores).',
        url: '/images/Aquario.jpeg' // Troque pelo caminho real da sua Imagem
      },

    },
    result: 'O Quilli resultou em uma plataforma embarcada capaz de automatizar as principais funções de manutenção de um aquário. O sistema controla a temperatura por meio do acionamento de termostato e cooler, automatiza a alimentação de acordo com horários programados e permite controlar tanto a intensidade quanto os horários de funcionamento da iluminação. A integração entre ESP32, sensores, atuadores, RTC, memória, interface TFT e comunicação Wi-Fi/MQTT transformou diferentes tarefas independentes em um único sistema de automação avançado.',
    githubUrl: 'https://github.com/mateusjrs/quilli-smart-aquarium' // Substitua pelo link correto
  }
  ,
  {
    id: 'emb-110-flight-dynamics',
    title: 'Análise do Comportamento de Voo do EMB-110 Bandeirante',
    category: 'Aerospace',
    shortDescription: 'Modelagem aerodinâmica, análise de estabilidade e simulação da dinâmica de voo do EMB-110 Bandeirante utilizando AVL, AutoCAD e MATLAB.',
    technologies: [
      'MATLAB',
      'AVL',
      'AutoCAD',
      'Vortex Lattice Method (VLM)',
      'Dinâmica de Voo',
      'Mecânica do Voo'
    ],
imageUrl: '/images/bandeirante-real.jpg', // <--- Foto principal do card
    gallery: [
      '/images/bandeirante-avl.png', // <--- Imagem do AVL
      '/images/bandeirante-plot.png' // <--- Gráfico 3D do MATLAB
    ],
        featured: true,
    summary: 'Projeto desenvolvido na disciplina de Mecânica do Voo com foco na análise do comportamento aerodinâmico, estabilidade e dinâmica de voo do EMB-110 Bandeirante (C-95). O trabalho envolveu o levantamento e reconstrução da geometria da aeronave, modelagem aerodinâmica utilizando o método Vortex Lattice no AVL, determinação de derivadas aerodinâmicas, análise de trimagem e desenvolvimento de um simulador não linear de seis graus de liberdade (6-DOF) em MATLAB.',
    problem: 'O projeto exigia a construção de um modelo representativo do EMB-110 a partir de informações disponíveis em manuais técnicos, dados operacionais e documentação de certificação. Algumas dimensões necessárias para a modelagem não estavam disponíveis diretamente, exigindo um processo de reconstrução geométrica. Além disso, era necessário avaliar não apenas as condições de equilíbrio da aeronave, mas também sua resposta dinâmica a diferentes comandos de controle e condições de massa e centro de gravidade.',
    contribution: 'Realizei o levantamento geométrico da aeronave a partir de suas vistas ortográficas utilizando Autodesk AutoCAD, escalonando as dimensões para obter uma representação compatível com os dados de referência. Desenvolvi a modelagem aerodinâmica no AVL, defini os arquivos de geometria e massa e extraí as derivadas de estabilidade e controle. Em MATLAB, implementei um simulador não linear de seis graus de liberdade utilizando ode45, integrando as forças e momentos aerodinâmicos às equações de movimento e realizando análises das respostas da aeronave a diferentes comandos de controle.',
    engineeringDetails: {
      aircraft: 'O objeto de estudo foi o EMB-110 Bandeirante, aeronave bimotora turboélice de asa baixa, empenagem convencional e estrutura semimonocoque metálica. O estudo foi fundamentado em informações provenientes de manuais operacionais e de manutenção, além do Type Certificate Data Sheet (TCDS) EASA.IM.A.229.',
      geometry: 'A geometria da aeronave foi reconstruída utilizando três vistas ortográficas — lateral, superior e frontal — importadas para o Autodesk AutoCAD. As imagens foram escalonadas para dimensões reais e utilizadas para obter parâmetros necessários à modelagem aerodinâmica. O modelo considerado possui envergadura de 15,33 m, área alar de 29,10 m², alongamento de 8,08, corda média aerodinâmica de 1,98 m, diedro de 7° e ângulo de incidência da asa de 3°. O comprimento da fuselagem considerado foi de 15,10 m.',
      cad: 'O uso do AutoCAD permitiu realizar um processo de engenharia reversa da geometria da aeronave. As dimensões obtidas das vistas ortográficas foram comparadas com os valores disponíveis na documentação técnica, buscando manter a coerência entre a geometria reconstruída e os parâmetros oficiais da aeronave. Esse modelo serviu como base para a construção da geometria utilizada posteriormente no AVL.',
      massAndCG: 'Foram analisadas duas condições de carregamento da aeronave. A primeira considerou massa de 5000 kg e centro de gravidade de aproximadamente 6,534 m. A segunda considerou massa de 3380 kg e centro de gravidade de aproximadamente 6,645 m. A posição do CG foi determinada a partir dos limites de centragem e da condição de massa considerada. A variação de massa e CG foi posteriormente utilizada para avaliar a influência dessas características sobre a estabilidade e a resposta dinâmica da aeronave.',
      inertia: 'Os momentos de inércia da aeronave foram estimados por um método semiempírico baseado na geometria global, massa e raios de giração adimensionais. Para as condições de 3380 kg e 5000 kg foram obtidos, respectivamente, diferentes valores de Ixx, Iyy e Izz, permitindo representar a alteração das propriedades inerciais do modelo entre as condições de carregamento.',
      aerodynamicModel: 'A modelagem aerodinâmica foi realizada no Athena Vortex Lattice (AVL), utilizando o Vortex Lattice Method (VLM). A geometria das superfícies sustentadoras foi representada por uma malha de vórtices. A fuselagem foi simplificada no modelo devido às limitações do VLM para representar corpos rombudos, enquanto a asa principal foi prolongada até o plano de simetria para manter a representação aerodinâmica utilizada na análise.',
      derivatives: 'Foram extraídas do AVL as principais derivadas aerodinâmicas de estabilidade e controle. Esses coeficientes representam a influência de variáveis como ângulo de ataque, ângulo de derrapagem, taxas de rotação e deflexões das superfícies de controle sobre as forças e momentos aerodinâmicos. As derivadas foram utilizadas tanto na análise de trimagem quanto na construção do modelo matemático do simulador de voo.',
      trim: 'Foi realizada uma análise de trimagem longitudinal para determinar as condições necessárias para manter a aeronave em voo reto e nivelado. O ângulo de ataque e a deflexão do profundor foram calculados analiticamente a partir das derivadas aerodinâmicas e comparados com os resultados obtidos diretamente no AVL. A análise considerou velocidades entre 70 e 130 m/s.',
      trimValidation: 'A comparação entre os resultados analíticos e a solução numérica do AVL apresentou boa concordância. As maiores diferenças ocorreram nas menores velocidades analisadas, com discrepâncias inferiores a 0,2° no caso mais extremo. Esse resultado forneceu uma validação do modelo analítico de trimagem utilizado no estudo.',
      simulation: 'Foi desenvolvido em MATLAB um simulador não linear de dinâmica de voo com seis graus de liberdade (6-DOF), utilizando o integrador numérico ode45. O modelo representa os movimentos translacionais e rotacionais da aeronave nos eixos do corpo, as taxas angulares p, q e r, os ângulos de Euler e a posição da aeronave no sistema de referência inercial.',
      equations: 'O simulador utiliza as equações de movimento para determinar as acelerações lineares e angulares da aeronave. As forças e momentos aerodinâmicos são calculados a partir dos coeficientes obtidos no AVL, sendo posteriormente dimensionalizados utilizando a pressão dinâmica, área de referência, corda média aerodinâmica e envergadura.',
      aerodynamicForces: 'O modelo aerodinâmico considera os coeficientes de sustentação, arrasto e força lateral, além dos coeficientes de momento de rolagem, arfagem e guinada. As contribuições das derivadas aerodinâmicas são calculadas em função de α, β, p, q, r e das deflexões de aileron, profundor e leme. Os coeficientes são posteriormente transformados dos eixos aerodinâmicos para os eixos do corpo da aeronave.',
      dynamicTests: 'Para avaliar a dinâmica da aeronave foram aplicadas perturbações do tipo pulso e doublet nas superfícies de controle. Foram analisados comandos no aileron, profundor e leme, permitindo observar as respostas longitudinais e laterodirecionais da aeronave.',
      pulse: 'Nos ensaios com pulso foram aplicadas deflexões de 2°, iniciadas em t = 1 s e mantidas durante 1 s. As respostas foram avaliadas por meio da evolução temporal das velocidades, taxas angulares, ângulos de atitude, altitude e posições Norte e Leste.',
      doublet: 'Os ensaios do tipo doublet utilizaram uma deflexão seguida por um comando de sentido oposto. Essa configuração permitiu investigar a resposta transitória da aeronave e identificar o comportamento dos diferentes modos dinâmicos sem manter uma perturbação contínua no sistema.',
      dynamicModes: 'As simulações permitiram investigar diferentes modos característicos da dinâmica de voo. As perturbações no profundor evidenciaram principalmente os modos de curto período e fugóide. Os comandos no leme apresentaram respostas associadas ao Dutch roll e ao modo espiral. Os comandos laterais também demonstraram o acoplamento entre os movimentos laterodirecionais e longitudinais.',
      massComparison: 'A influência da massa e do centro de gravidade foi analisada comparando diretamente as condições de 5000 kg e 3380 kg. A configuração de menor massa e CG mais recuado apresentou respostas dinâmicas mais intensas em diversas perturbações, com maiores amplitudes de velocidade, atitude e taxas angulares.',
      stabilityAnalysis: 'A comparação das respostas mostrou que a redução da massa combinada ao deslocamento do CG modificou significativamente o comportamento dinâmico da aeronave. Em determinadas perturbações, a condição mais leve apresentou menor amortecimento, maiores oscilações e maior perda de altitude, evidenciando a influência das propriedades de massa e centragem sobre a estabilidade e a qualidade de voo.',
      analyses: [
        'Levantamento geométrico (AutoCAD)',
        'Modelagem aerodinâmica (AVL)',
        'Extração de derivadas aerodinâmicas',
        'Análise de trimagem longitudinal e validação',
        'Desenvolvimento de simulador 6-DOF (MATLAB)',
        'Pulso e Doublet (aileron, profundor, leme)',
        'Análise de modos: fugóide, curto período, Dutch roll e espiral',
        'Influência de massa e posição de CG nas trajetórias 3D'
      ]
    },
    result: 'O projeto resultou em um modelo integrado da aeronave, combinando reconstrução geométrica, modelagem aerodinâmica, análise de estabilidade e simulação de dinâmica de voo. A comparação entre a trimagem analítica e os resultados do AVL apresentou boa concordância. O simulador 6-DOF em MATLAB permitiu avaliar a resposta a perturbações e identificar os modos dinâmicos, provando que a configuração de 3380 kg e CG recuado apresenta respostas mais sensíveis. O código completo (scripts MATLAB e modelos AVL) está disponível no repositório.',
    githubUrl: 'https://github.com/mateusjrs/emb-110-flight-dynamics', // <--- COLOQUE SEU LINK AQUI
    report: 'Relatório Técnico Completo: Análise do Comportamento de Voo do EMB-110 Bandeirante'
  },
  {
    id: 'cessna-182-avionics-modification',
    title: 'Projeto de Modificação de Sistema Aviônico — Cessna 182',
    category: 'Aerospace',
    shortDescription: 'Análise de engenharia para substituição de um sistema de navegação e comunicação por um Garmin GNS 430W, incluindo balanço de carga elétrica, especificação de componentes e integração elétrica.',
    technologies: [
      'Engenharia Aeronáutica',
      'Sistemas Elétricos de Aeronaves',
      'Aviónica',
      'Balanço de Carga Elétrica',
      'Wiring',
      'Documentação Técnica',
      'Regulamentação Aeronáutica'
    ],
    imageUrl: 'https://upload.wikimedia.org/wikipedia/commons/thumb/9/9e/Cessna182t_skylane_n2231f_cotswoldairshow_2010_arp.jpg/1280px-Cessna182t_skylane_n2231f_cotswoldairshow_2010_arp.jpg?utm_source=en.wikipedia.org&utm_campaign=index&utm_content=thumbnail',
    featured: false,
    summary: 'Resolução de um estudo de modificação de uma aeronave Cessna 182, envolvendo a substituição do sistema RT-385 pelo Garmin GNS 430W. O trabalho foi desenvolvido com foco na análise dos requisitos técnicos da modificação, especificação dos equipamentos necessários, avaliação do balanço de carga elétrica da aeronave e elaboração da integração elétrica do novo sistema. A atividade também envolveu a interpretação de documentação técnica e requisitos aplicáveis à instalação de sistemas aviônicos.',
    problem: 'A modificação de um sistema aviônico exige avaliar não apenas a compatibilidade do novo equipamento, mas também seu impacto sobre o sistema elétrico da aeronave e sua integração com a infraestrutura existente. O desafio proposto consistia em analisar a substituição do equipamento original por um Garmin GNS 430W, determinar os componentes necessários para a instalação, verificar o consumo elétrico adicional e estruturar uma solução de alimentação e conexão compatível com a aeronave.',
    contribution: 'Desenvolvi a resolução técnica da modificação, analisando os requisitos apresentados na Instrução Suplementar, consultando a documentação de instalação do equipamento e estruturando a solução de integração elétrica. Realizei o levantamento dos equipamentos removidos e instalados, analisei o consumo elétrico do sistema, determinei o impacto da modificação no balanço de carga e organizei as principais conexões elétricas, incluindo alimentação, aterramento e proteção por circuit breakers.',
    engineeringDetails: {
      aircraft: 'A aeronave analisada foi um Cessna 182 do ano de 1979, utilizado como plataforma para o estudo da modificação do sistema aviônico. O projeto considerou a arquitetura elétrica de 28 V da aeronave e a infraestrutura existente do sistema de aviônicos.',
      modification: 'A modificação proposta consiste na substituição do sistema RT-385 por um Garmin GNS 430W. O novo equipamento integra funções relacionadas à navegação e comunicação, exigindo a análise dos requisitos de alimentação, proteção elétrica, conectores, montagem e antena GPS/WAAS.',
      regulatory: 'A análise considerou a Instrução Suplementar IS Nº 21-021, especificamente a seção 5.6.1 indicada no teste, relacionada à aprovação de modificações em aeronaves. O estudo também considerou a necessidade de utilização de dados técnicos aprovados e a verificação do impacto da instalação sobre o sistema elétrico da aeronave.',
      equipment: 'Foram identificados os principais componentes necessários para a instalação do Garmin GNS 430W: unidade principal GNS 430W, mounting rack, connector kit, back plate e antena GA 35 GPS/WAAS. A especificação foi organizada considerando modelo, part number, quantidade e função de cada componente.',
      equipmentList: [
        '01 × Garmin GNS 430W',
        '01 × Mounting Rack',
        '01 × Connector Kit',
        '01 × Back Plate',
        '01 × GA 35 GPS/WAAS Antenna'
      ],
      electricalSystem: 'O sistema elétrico considerado possui arquitetura de 28 V. A análise levou em consideração a capacidade de geração da aeronave, o consumo dos equipamentos removidos e o consumo dos novos equipamentos instalados, permitindo determinar a variação líquida de carga provocada pela modificação.',
      loadAnalysis: 'O balanço de carga elétrica comparou o consumo do equipamento RT-385A removido com o consumo do Garmin GNS 430W instalado. O RT-385A foi considerado com consumo típico de 2,0 A, enquanto o GNS 430W apresenta consumo de aproximadamente 0,8 A em operação típica e até 3,0 A durante a transmissão. Dessa forma, a modificação representa um aumento líquido de aproximadamente 1,8 A no pico de transmissão.',
      generator: 'Para a análise da capacidade de geração foi considerado um alternador de 28 V e 60 A. O estudo também considerou uma bateria de 24 V e 14 Ah como referência para a aeronave. A capacidade do sistema de geração foi comparada com o limite de carga contínua considerado na análise.',
      electricalMargin: 'Considerando o critério de utilização de até 80% da capacidade do alternador para operação contínua, o limite de referência calculado foi de 48 A. O acréscimo de aproximadamente 1,8 A associado à modificação representa uma parcela pequena da capacidade disponível, indicando margem elétrica suficiente para a instalação proposta dentro das premissas adotadas no estudo.',
      wiring: 'Foi estruturada uma proposta de integração elétrica simplificada para o novo equipamento, considerando alimentação, aterramento, proteção por circuit breaker e conexão ao barramento de aviônicos. A solução diferencia os novos circuitos instalados dos elementos existentes que permanecem na aeronave.',
      protection: 'A proteção elétrica foi considerada por meio de circuit breakers dedicados. O estudo especificou um disjuntor de 5 A para a alimentação principal e um disjuntor de 10 A associado à alimentação do circuito de comunicação (COM), conforme a solução apresentada na documentação da atividade.',
      connections: 'As principais conexões foram organizadas de acordo com os conectores do Garmin. No P4001 (Main), os pinos 19 e 20 foram destinados à entrada de alimentação Aircraft Power 1 e os pinos 77 e 78 ao Aircraft Ground. No P4002 (COM), os pinos 11 e 12 foram associados à alimentação Aircraft Power COM e os pinos 21 e 22 ao aterramento. No P4006 (NAV), o pino 44 foi associado à alimentação NAV Power.',
      antenna: 'A integração também contempla a instalação da antena GA 35, utilizada para o sistema GPS/WAAS. O componente foi incluído no levantamento de materiais e considerado parte da arquitetura necessária para a implementação do novo sistema aviônico.',
      documentation: 'A solução foi organizada como uma documentação de engenharia, contemplando identificação dos equipamentos, part numbers, análise de carga elétrica, capacidade de geração, lista de materiais e conexões principais. Essa estrutura permite rastrear as decisões de engenharia e facilitar a análise da modificação.',
      engineeringSkills: [
        'Interpretação de documentação técnica',
        'Análise de requisitos de instalação',
        'Sistemas elétricos aeronáuticos',
        'Balanço de carga elétrica',
        'Dimensionamento de proteção elétrica',
        'Integração de equipamentos aviônicos',
        'Análise de wiring',
        'Identificação de componentes e part numbers',
        'Interpretação de requisitos de aeronavegabilidade',
        'Elaboração de documentação técnica'
      ]
    },
    result: 'A análise resultou em uma proposta estruturada para a substituição do sistema RT-385 pelo Garmin GNS 430W, contemplando os principais componentes necessários, a arquitetura de alimentação, proteção elétrica, conexões e integração com o sistema aviônico existente. O balanço de carga indicou um acréscimo líquido de aproximadamente 1,8 A no pico de transmissão e, considerando o limite de 80% da capacidade do alternador, uma margem suficiente dentro das premissas adotadas. O projeto demonstrou a aplicação integrada de conhecimentos de sistemas elétricos aeronáuticos, aviônicos, regulamentação, análise de requisitos e documentação de engenharia. O material desenvolvido para a resolução técnica e os arquivos utilizados no estudo estão disponibilizados no repositório.',
    githubUrl: 'https://github.com/mateusjrs/cessna-avionics-mod' // Lembre-se de colocar o link correto
  },
  
];
