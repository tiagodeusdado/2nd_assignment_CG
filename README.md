# UBI - Computação Gráfica (14350) - Assignment 2
## Aircraft Flight Simulator Procedures in $\mathbb{R}^3$

* **Aluno:** Tiago Deusdado (Nº 50123)
* **Docente / Supervisor:** Abel Gomes
* **Procedimento Atribuído:** **Exercício 1 — Simulação do Procedimento de Descolagem (*Aircraft Takeoff Procedure*)** *(último dígito 3: ímpar)*

---

## 1. Visão Geral do Projeto

Este projeto implementa um simulador do procedimento de descolagem de aeronaves em $\mathbb{R}^3$ utilizando **Modern OpenGL (Core Profile 3.3+)**, **GLFW**, **GLEW** e **GLM**.

O sistema carrega e renderiza modelos 3D poligonais em formato Wavefront `.obj` (o caça supersónico **F-104 Starfighter** e a **pista de aterragem/descolagem (*Runway*)**), aplica o modelo de iluminação e sombreamento **Phong Shading** com uma **Point Light** dinâmica na cena, e suporta controlo de câmara orbital/livre/cockpit e pilotagem manual ou automática via rato e teclado.

---

## 2. Requisitos Cumpridos do Enunciado

| Requisito | Estado | Descrição |
|---|---|---|
| **Malha .obj da Pista (*Runway*)** |  Concluído | Carregada de `runway.obj` (120m de largura por 7.32 km de extensão). |
| **Malha .obj do Avião (*Aircraft*)** |  Concluído | Carregada de `f104starfighter.obj` (escala real: 17.8m comprimento, 7.36m envergadura). |
| **Modern OpenGL + GLFW** |  Concluído | Core Profile 3.3 com VAOs, VBOs e shaders GLSL (`#version 330 core`). |
| **Transformações em $\mathbb{R}^3$** |  Concluído | Translações e rotações em tempo real com teclado e rato. |
| **Point Light na cena 3D** |  Concluído | Luz pontual na cena com cálculo de atenuação quadrática e marcador emissivo no espaço. |
| **Phong Shading** |  Concluído | Componentes Ambiente, Difusa (Lambert) e Especular (Phong) em GLSL fragment shader. |
| **Simulação de Descolagem (TOLD)** |  Concluído | Baseado em dados de desempenho aeronáuticos (aceleração, $V_1$, $V_r$, $V_{LOF}$, subida estável). |
| **Extras para Bónus (20 pontos)** |  Concluído | Skybox atmosférico, luzes de pista, torre de controlo, hangares, terreno, HUD/aviónica. |

---

## 3. Elementos Extras Implementados

1. **Skybox Atmosférico Procedural:** Cúpula celeste com gradiente de altitude (zénite azul profundo até ao horizonte claro) e disco solar brilhante com dispersão de luz.
2. **Balizamento e Luzes de Pista (*Runway Lights*):**
   * Luzes verdes de cabeceira (*Threshold*) no início da zona de descolagem.
   * Luzes brancas de bordadura (*Runway Edge Lights*) espaçadas a cada 50m em ambas as margens.
   * Luzes vermelhas de fim de pista (*Runway End Lights*).
3. **Edifícios do Aeroporto:**
   * **Torre de Controlo:** Shaft de betão (40m), cabine panorâmica envidraçada de observação e radar superior.
   * **Hangares Militares:** Hangares com acabamento metálico ondulado ao lado da pista.
   * **Terminal de Passageiros:** Estrutura aeroportuária com iluminação Phong.
4. **Terreno Envolvente:** Base plana de relva e solo circundante com 8 km de extensão.
5. **Painel de Aviónica / HUD no Título:** Exibição em tempo real de Airspeed ($IAS$ em nós), Altitude ($ALT$ em pés), Razão de Subida ($VS$ em pés/minuto), Ângulo de Pitch e Fase de Voo.
6. **Point Light Interativa:** O utilizador pode mover a luz pontual pelas coordenadas da cena com as teclas `I`, `K`, `J`, `L`, `U`, `O` para demonstrar ao vivo ao professor a resposta do Phong Shading na fuselagem do avião e no asfalto.

---

## 4. Controlos e Teclas de Interação

### Câmaras em $\mathbb{R}^3$:
* `1`: **Chase Camera** (Câmara de perseguição em 3ª pessoa — segue o caça suavemente durante a aceleração e descolagem; arrastar com botão direito roda em torno do avião).
* `2`: **Free Fly Camera (Estilo CS2 / FPS)** (O cursor do rato fica capturado no centro da janela, permitindo rotação $360^\circ$ infinita e contínua em qualquer direção sem bater nas bordas do ecrã; premir `TAB` para libertar/recapturar o cursor).
* `3`: **Cockpit Camera** (Câmara em 1ª pessoa no interior da cabine do caça).
* `4`: **Tower Camera** (Câmara fixa na torre de controlo que acompanha (*tracking*) o caça ao longo da pista).
* `SHIFT`: Sprint / Aceleração de deslocamento em modo Free Fly Camera.
* **Scroll do Rato:** Zoom / Ajuste de distância.

### Simulação de Descolagem:
* `ESPAÇO`: Iniciar descolagem / Potência máxima nos motores.
* `P`: Pausar / Continuar simulação.
* `R`: Reiniciar (*Reset*) a simulação de volta à cabeceira da pista.
* `M`: Alternar entre **Piloto Automático (Guiado por TOLD)** e **Pilotagem Manual**.
* `W` / `S` *(em Modo Manual)*: Controlar *Pitch* (puxar o nariz para cima ou empurrar para baixo).
* `A` / `D` *(em Modo Manual)*: Controlar *Yaw* (leme de direção).
* `SETA CIMA` / `SETA BAIXO` *(em Modo Manual)*: Aumentar / Diminuir Throttle.

### Point Light (Phong Shading Demo):
* `I` / `K`: Mover luz para a frente / trás ($Z$).
* `J` / `L`: Mover luz para a esquerda / direita ($X$).
* `U` / `O`: Mover luz para cima / baixo ($Y$).
* `ESC`: Fechar aplicação.

---

## 5. Como Compilar e Executar no VS Code

O projeto está configurado para o **Visual Studio Code**:

1. Abrir a pasta `2nd_cg` no **VS Code**.
2. Pressionar `Ctrl + Shift + B` (ou menu **Terminal -> Run Build Task**).
3. O VS Code compilará o código com `g++` e gerará o executável `bin/flight_sim.exe`.
4. Em alternativa, podes simplesmente executar o script `build.bat` com duplo clique ou no terminal:
   ```cmd
   .\build.bat
   ```
