[![Build Status](https://travis-ci.org/opentomb/OpenTomb.svg?branch=master)](https://travis-ci.org/opentomb/OpenTomb)

OpenTomb — uma versão de códio aberto do motor Tomb Raider 1-5
-------------------------------------------------------

### Tabela de conteúdos ###

- [O que é isso?](#o-que-é-ist)
- [Por que criar um novo motor?](#por-que-criar-um-novo-motor)
- [Características](#características)
- [Plataformas suportadas](#plataformas-suportadas)
- [Configuração](#configuração)
- [Compilando](#compilando)
- [Corrida e Configuração](#corrida-e-configuração)
- [Licenciamento](#licenciamento)
- [Créditos](#créditos)


### O que é isso? ###
OpenTomb é uma re-implementação de código aberto do clássico motor Tomb Raider,
pretendia jogar níveis de todos os jogos Tomb Raider da época clássica (1-5), bem como
níveis TRLE personalizados. O projeto não usa nenhum dos códigos Tomb Raider originais,
pois todas as tentativas de recuperar arquivos de origem do Eidos/Core foram em vão.

Em vez disso, tudo está a ser reestruturado completamente do zero. Deveria ser
observado, no entanto, que o OpenTomb usa certas rotinas legadas de projetos
open-source inacabados, como [OpenRaider](http://openraider.sourceforge.net/)
e VT projeto (encontrado em [icculus.org](https://icculus.org/)), além de algum código
do Quake Tenebrae.

O OpenTomb tenta recriar a experiência original da série Tomb Raider, embora
com atualizações, características e complementos contemporâneos, podendo beneficiar plenamente
de funcionar com computadores modernos com poderosos CPUs e placas gráficas.

Links para fóruns e informações:
* TR link do fórum: http://www.tombraiderforums.com/showthread.php?t=197508
* Canal da discórdia: https://discord.gg/d8mQgdc

### Por que criar um novo motor? ###
É verdade que temos versões completas do Windows TR2-5, e o TR1 funciona perfeitamente através de [DosBox]
(https://www.dosbox.com/). No entanto, como o tempo prossegue, a situação só piorará, com Sistemas Operacionais mais novos
tornando-se cada vez mais improvável apoiar os jogos. OpenTomb sempre pode ser portado para qualquer plataforma que desejar.

Também é verdade que existem remendos para o motor original, visando o
melhorar e atualiza-lo: TREP, TRNG, etc. A vantagem com o OpenTomb é que nós
não estamos limitados pelo binário original, uma enorme limitação quando se trata de novos
recursos, melhorias gráficas, modificação de código e muito mais. Um motor de código aberto
elimina estas limitações.

### Características ###
* OpenTomb tem uma abordagem de colisão completamente diferente para o motor original,
contornando muitas das limitações presentes. Usamos um gerador de terreno para
fazer uma malha de colisão otimizada para cada quarto do chamado "floordata".
* O OpenTomb é capaz de uma taxa de quadros variável, não limitada a 30fps, como o
motor original.
* OpenTomb usa bibliotecas comuns e flexíveis, como OpenGL, OpenAL, SDL e
Física da Bala.
* OpenTomb implementa um mecanismo de script Lua para definir toda a funcionalidade da entidade.
  Isso significa que, novamente, ao contrário do original, muito menos é codificado no
  motor próprio, de modo que a funcionalidade pode ser estendida ou modificada sem
  modificar e recompilar o próprio motor.
* Muitos recursos abandonados e não utilizados do mecanismo original foram habilitados
em OpenTomb. Nova animação, itens não utilizados, estruturas escondidas específicas do PSX no interior
arquivos de nível, e assim por diante!

### Plataformas suportadas ###
O OpenTomb é um mecanismo multi-plataforma: atualmente ele pode ser executado no Windows, Mac ou
Linux. Nenhuma implementação móvel encontra-se ainda em desenvolvimento , mas eles são de fato
possíveis.

### Configuração ###
Para executar qualquer um dos níveis dos jogos originais, precisarás dos ativos de
este respectivo jogo. Estes recursos muitas vezes tendem a estar em formatos crípticos, com
variações em todos os jogos. Por isso, precisarás de converter alguns recursos dejogo
para formatos utilizáveis, ou obte-los em algum lugar na Net.

Aqui está a lista de todos os recursos necessários e onde obtê-los:

  * Pastas de dados de cada jogo. Obtenha-as dos teus CDs de jogos ou Steam/GOG
  pacotes. Basta pegar na pasta de dados da pasta de cada jogo e colocá-la na
  correspondente `/data/tr*/` pasta. Por exemplo, para TR3, o caminho seria
  `OpenTomb/data/tr3/data/`

  * Faixas de áudio de CD. O OpenTomb só suporta neste momento audiotracks OGG, então tu
  deves converter faixas sonoras originais por si mesmo, ou simplesmente descarregar todo o TR1-5
  pacote de música aqui: https://opentomb.earvillage.net
  ATENÇÃO: os arquivos podem precisar de ser renomeados para que isto funcione, consulta
   https://github.com/opentomb/OpenTomb/issues/447

* Carregando ecrãs para TR1-3 e TR5. Para TR3, obtenhem-os no diretório pix do
  teu jogo oficial instalado. Basta colocar este diretório pix em `/data/tr3/`
  pasta. Quanto a outros jogos, é um pouco complicado obter ecrãs de carregamento, pois lá
  não havia ecrãs de carregamento para versões de PC TR1-2, capturas de ecrã de nível TR4 usadas como
  ecrãs carregados e TR5 usou um formato criptografado para armazenar todo os gráficos carregados.
  Então, para facilitar a tua vida, podes simplesmente descarrear o pacote de ecrã de carregamento
   aqui: http://trep.trlevel.de/temp/loading_screens.zip
  Basta colocá-lo diretamente no diretório OpenTomb, e isto deve fazer o truque. Nota:
  O motor suporta o formato png e pcx das imagens do ecrã.

### Compilando ###
Há um arquivo CMakeLists.txt fornecido com o código-fonte, para que possas compilar
OpenTomb usando CMake. No Windows, também podes compilá-lo a partir do Code :: Blocks IDE
(o arquivo do projeto também é fornecido). Alternativamente, podes compilá-lo manualmente em
Código :: Blocos ao adicionar recursivamente todos os arquivos de origem do  / diretório src e
adicionando estas bibliotecas nas Configurações do Linker em Opções de Projeto:

* libmingw32.a
* libSDL2main.a
* libSDL2.dll.a
* liblua.a
* libpng.a
* libz.a
* libpthread.a

No Linux, basta descarrear o código-fonte e executar no terminal:

     cmake. && faz

As dependências necessárias são os cabeçalhos de desenvolvimento para SDL2, png, LUA 5.2,
ZLIB. Tu podes instalá-los numa distribuição baseada em Ubuntu com este comando:

     sudo apt-get install libopenal-dev libsdl2-dev libpng12-dev liblua5.2-dev libglu1-mesa-dev zlib1g-dev

No Mac, usa o projeto XCode, que também está disponível no código-fonte.

NB: Observa que o OpenTomb requer o sinalizador C++11 (`-std=c++11`) para o compilar
devidamente! Podes usar sinalizadores de otimização específicos da CPU (`-march=prescott`,
`-march=i486`,`-march=core2`), bem como bandeiras de otimização geral (`-O1` e` -O2`),
  mas NÃO USES bandeira `-O3`, pois a Bullet tende a falhar com este nível de otimização
  (GCC 5.1+ pode compilar sem erros).

### Executando e Configuração ###
Para executar OpenTomb, simplesmente executa o executável gerado pela compilação. Por padrão,
não são necessárias opções de linha de comando. Acede o console pressionando \`. Este
permite que insiras os comandos para selecionar os níveis, alterar configurações e muito mais. Entrar
em 'ajuda' para obter uma lista de comandos. Digita 'exit' para sair do motor.

Atualmente, todas as configurações no OpenTomb são geridas através do config.lua e
autoexec.lua. Config.lua contém configurações persistentes do mecanismo e do jogo, enquanto
autoexec.lua contém todos os comandos que devem ser executados no arranque do motor.

Config.lua é dividido em diferentes seções: ecrã, áudio, renderização, controles,
console e sistema. Em cada uma destas seções, podes mudar numerosos
parâmetros, cujos nomes geralmente são bastante intuitivos.

Autoexec.lua é uma simples lista de comandos que são executados na inicialização. Modificando
os comandos existentes podem fazer com que o motor funcione incorretamente.

Para selecionar um nível, digita 'setgamef(jogo, nível) em autoexec.lua ou em
o console, onde o jogo é 1-5. Os níveis de mansão são geralmente 0, e jogos que
não tenha uma mansão a partir do nível 1. Por exemplo, para carregar o nível 2 do TR3,
entrarias `setgamef (3, 2)`

### Licenciamento ###
O OpenTomb é um mecanismo de código aberto distribuído sob a licença LGPLv3, o que significa
que qualquer parte do código-fonte também deve ser de código aberto. Por isso, todos usam
bibliotecas e recursos agrupados devem ser de código aberto compatível com licenças GPL.
Aqui está a lista de bibliotecas e recursos usados e suas licenças:

* OpenGL - não precisa de licenciamento (https://www.opengl.org/about/#11)
* OpenAL Soft - LGPL
* Imagem SDL / SDL - zlib
* Bullet - zlib
* Freetype2 - GPL
* Lua - MIT
* formato ffmpeg rpl e codecs (http://git.videolan.org/)

* Droid Sans Mono, Roboto Condensado Regular e Roboto Fontes regulares - Apache

### Créditos ###
NB: Observa que a lista de autores e de contribuidores está constantemente a estender-se, como
há cada vez mais e mais pessoas envolvidas no desenvolvimento de projetos, por isso alguém pode estar
  a faltar nesta lista!

* [TeslaRus](https://github.com/TeslaRus): desenvolvedor principal.
* [Cochrane](https://github.com/Cochrane): o renderizador reescreve e otimiza, Mac OS X suporte.
* [Gh0stBlade](https://github.com/Gh0stBlade): complementos de renderização, porta de sombreador, implementação de fluxo de jogo, ajustes
de controle de estado, câmara e programação de AI.
* [Lwmte](https://github.com/Lwmte): correcções de estado e scripts, controles, GUI e módulos de áudio, acionar e reescrever o sistema da entidade.
* Nickotte: programação de interface, implementação de inventário de toque,
arranjos de câmera.
* [pmatulka](https://github.com/pmatulka): Porta Linux e testes.
* [richardba](https://github.com/richardba): Migração do Github, manutenção do repo da Github, design do site.
* [Saracen](https://github.com/Saracen): iluminação em malha de sala e estática.
* [T4Larson](https://github.com/T4Larson): patches de estabilidade geral e correção de erros.
* [vobject](https://github.com/vobject): construções noturnas, mantendo a compatibilidade geral do compilador.
* [vvs-](https://github.com/vvs-): teste, feedback, relatório de erros.
* [xproger](https://github.com/xproger): atualizações de documentação.
* [Banderi](https://github.com/Banderi): documentação, correcção de erros.
* [gabrielmtzcarrillo](https://github.com/gabrielmtzcarrillo): trabalho de sombreamento de entidade.
* [filfreire](https://github.com/filfreire): documentação

Contribuições adicionais de: Ado Croft (teste extensivo),
E. Popov (TRN porto de sombreamento caustico), [godmodder](https://github.com/godmodder) (ajuda geral),
[jack9267](https://github.com/jack9267) (vt otimização do carregador), meta2tr (teste e rastreamento de erros),
shabtronic (arranjos de renderização), [Tonttu](https://github.com/Tonttu) (patch de console) e
[xythobuz](https://github.com/xythobuz) (patches de compatibilidade Mac adicionais).

Traduções por: [Joey79100](https://github.com/Joey79100) (Francês), Nickotte (Italiano), [Lwmte](https://github.com/Lwmte) (Russo),
[SuiKaze Raider](https://twitter.com/suikazeraider) (Espanhol), [filfreire](https://github.com/filfreire) (Portuguese - Portugal)
