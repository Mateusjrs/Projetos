# Guia Essencial de Git e Convenções de Commit

Este guia rápido apresenta os comandos mais utilizados do Git Bash e as convenções de mensagem de commit (Conventional Commits) para manter o histórico do projeto limpo e organizado.

## 1. Comandos Essenciais do Git Bash

| Comando | Função | Exemplo de Uso |
| :--- | :--- | :--- |
| `git init` | Inicializa um novo repositório Git local no diretório atual. | `git init` |
| `git clone` | Baixa um repositório existente de um servidor remoto (como GitHub) para o seu computador. | `git clone <URL_do_repositorio>` |
| `git status` | Mostra o estado dos arquivos no diretório de trabalho e na área de *staging*. | `git status` |
| `git add` | Adiciona alterações do diretório de trabalho para a área de *staging* (preparando para o commit). | `git add nome_do_arquivo.txt` ou `git add .` |
| `git commit` | Grava as alterações da área de *staging* no histórico do repositório. | `git commit -m "Mensagem do commit"` |
| `git log` | Exibe o histórico de commits do repositório. | `git log` |
| `git branch` | Lista, cria ou deleta *branches* (ramificações). | `git branch nova-feature` |
| `git checkout` | Alterna entre *branches* ou restaura arquivos do diretório de trabalho. | `git checkout nome-da-branch` |
| `git merge` | Combina o histórico de uma *branch* em outra. | `git merge branch-a-combinar` |
| `git pull` | Baixa o conteúdo do repositório remoto e o mescla automaticamente no repositório local. | `git pull origin main` |
| `git push` | Envia os commits locais para o repositório remoto. | `git push origin nome-da-branch` |

## 2. Convenção de Commits (Conventional Commits)

A especificação Conventional Commits é uma convenção leve que fornece um conjunto de regras para criar um histórico de commit explícito. Isso facilita a automação de tarefas como a geração de *changelogs* e o versionamento semântico.

O formato da mensagem de commit deve ser: `<tipo>(<escopo opcional>): <descrição>`

### Tipos Principais

| Tipo | Descrição | Exemplo de Uso |
| :--- | :--- | :--- |
| **`feat`** | Uma nova *feature* (funcionalidade) para o usuário. (Mapeia para `MINOR` no Versionamento Semântico) | `feat: adicionar botão de login com Google` |
| **`fix`** | Uma correção de *bug*. (Mapeia para `PATCH` no Versionamento Semântico) | `fix: corrigir erro de carregamento na página inicial` |
| **`docs`** | Alterações na documentação (README, arquivos de ajuda, etc.). | `docs: atualizar seção de instalação no README` |
| **`style`** | Alterações que não afetam o significado do código (espaços em branco, formatação, ponto e vírgula ausente, etc.). | `style: formatar código com Prettier` |
| **`refactor`** | Uma mudança de código que não corrige um *bug* nem adiciona uma *feature*. | `refactor: extrair lógica de validação para um hook` |
| **`test`** | Adicionar ou corrigir testes. | `test: adicionar testes unitários para o componente de carrinho` |
| **`chore`** | Outras alterações que não modificam o código de produção (ex: atualizações de dependências, configurações de build). | `chore: atualizar versão do React para 18.2.0` |
| **`build`** | Alterações que afetam o sistema de *build* ou dependências externas (ex: npm, yarn, gulp). | `build: configurar CI/CD para deploy automático` |
| **`ci`** | Alterações nos arquivos de configuração de CI (*Continuous Integration*). | `ci: adicionar etapa de lint no pipeline` |

### Quebra de Compatibilidade (Breaking Change)

Para indicar uma **quebra de compatibilidade** (*Breaking Change*), que exige uma mudança `MAJOR` no Versionamento Semântico, adicione um `!` após o tipo/escopo, ou inclua a palavra `BREAKING CHANGE:` no corpo do commit.

**Exemplo:** `feat!: remover suporte para Node.js 14`
