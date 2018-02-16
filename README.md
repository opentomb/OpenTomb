[![Build Status](https://travis-ci.org/opentomb/OpenTomb.svg?branch=master)](https://travis-ci.org/opentomb/OpenTomb)

OpenTomb — an open-source Tomb Raider 1-5 engine remake
-------------------------------------------------------

### Talaan ng mga Nilalaman ###

- [Ano ito?](#what-is-this)
- [Bakit lilikha ng isang bagong engine?](#why-create-a-new-engine)
- [Mga Tampok](#features)
- [Mga suportadong platform](#supported-platforms)
- [Setup](#setup)
- [Pag-ipon](#compiling)
- [Running and Configuration](#running-and-configuration)
- [Paglilisensya](#licensing)
- [Mga Kredito](#credits)


### Ano ito? ###
OpenTomb ay isang open-source na muling pagpapatupad ng klasikong Tomb Raider engine,
nilayon upang i-play ang mga antas mula sa lahat ng mga klasikong laro ng Tomb Raider (1-5), pati na rin
custom na mga antas ng TRLE. Ang proyekto ay hindi gumagamit ng anuman sa orihinal na Tomb Raider
code, dahil ang lahat ng mga pagtatangka upang makuha ang mga source file mula sa Eidos / Core ay walang kabuluhan.

Sa halip, ang lahat ay ganap na muling binuo mula sa simula. Dapat ay
nabanggit,Gayunpaman, ang OpenTomb ay gumagamit ng ilang mga gawain ng lega mula sa hindi natapos
mga proyektong open-source, tulad ng  [OpenRaider](http://openraider.sourceforge.net/)
at VT project (found at [icculus.org](https://icculus.org/)), kasama ang ilang mga code
mula sa Quake Tenebrae.

Sinusubukan ng OpenTomb na muling lumikha ng orihinal na karanasan ng serye ng Tomb Raider, bagaman
na may mga kontemporaryong update, mga tampok at mga karagdagan, nakakakuha ng ganap na kapakinabangan
mula sa pagtakbo sa modernong mga PC na may malakas na CPU at graphic card.

Mga link sa mga forum at impormasyon:
* TR forum link: http://www.tombraiderforums.com/showthread.php?t=197508
* Discord channel: https://discord.gg/d8mQgdc

### Bakit lilikha ng isang bagong engine? ###
Ito ay totoo na kami ay ganap na nagtatrabaho sa Windows builds ng TR2-5, at TR1 na perpektong gumagana
sa pamamagitan ng [DosBox](https://www.dosbox.com/). Gayunpaman, kapag itoy pinagpatuloy
ang pag-unlad ang sitwasyon ay lalala lamang, na may mas bagong Operating Systems na
nagiging pataas lalong hindi malamang na suportahan ang mga laro. Ang OpenTomb ay laging
ma-port sa anumang platform na gusto mo.
be able to be ported to any platform you wish.

Ito ay totoo na mayroong mga patchers para sa orihinal na engine, na naglalayong
mapabuti at i-update ito: TREP, TRNG, atbp. Ang kalamangan sa OpenTomb ay ay kami
ay hindi limitado sa pamamagitan ng orihinal na Binary, isang malaking limitasyon sa pagdating ng bagong
mga tampok, mga graphical na pagpapahusay, pagbabago ng code at higit pa. Isang open-source
Inalis ng engine ang mga limitasyon na ito.

### Mga Tampok ###
* Ang OpenTomb ay may ganap na magkakaibang diskarte sa banggaan ng orihinal na engine,
pag-iwas sa marami sa mga limitasyon na naroroon. Ginagamit namin ang terrain generator sa
paggawa ng isang optimized mesh ng banggaan para sa bawat kuwarto mula sa tinatawag na"floordata".
* Ang OpenTomb ay may kakayahang isang variable frame rate, hindi limitado sa 30fps tulad ng
orihinal na engine.
* Ang OpenTomb ay gumagamit ng mga karaniwang at flexible na mga aklatan, tulad ng OpenGL, OpenAL, SDL at
Bullet Physics.
* Ang OpenTomb ay nagpapatupad ng isang engine ng Lua script upang tukuyin ang lahat ng functionality ng entity.
 Nangangahulugan ito na, muli, hindi katulad ng orihinal, mas mababa sa hardcoded na
 engine mismo, kaya ang pag-andar ay maaaring pinalawak o binago nang walang havng para
 baguhin at i-recompile ang engine mismo.
* Maraming mga inabandunang at hindi nagamit na mga tampok mula sa orihinal na engine ang pinagana
sa OpenTomb. Bagong animation, hindi ginagamit na mga item, mga nakatagong PSX na istraktura sa loob
mga antas ng file, at iba pa!

### Mga suportadong platform ###
Ang OpenTomb ay isang cross-platform engine: sa kasalukuyan maaari itong tumakbo sa Windows, Mac o
Linux. Wala pang mga pagpapatupad sa mobile ng paunlad, ngunit ang mga ito ay sa katunayan ay
maaari.

### Setup ###
Upang patakbuhin ang alinman sa mga antas mula sa orihinal na mga laro, kakailanganin mo ang mga asset mula
ang kani-kanilang laro. Ang mga mapagkukunan na ito ay kadalasang may posibilidad na maging sa cryptic format, na may
pagkakaiba-iba sa mga laro. Dahil dito, kakailanganin mong i-convert ang ilang laro
na mapagkukunan upang magamit ang iyong mga format, o makuha ang mga ito mula sa isang lugar sa Net.

Narito ang listahan ng lahat ng kinakailangang ari-arian at kung saan makakakuha ng mga ito:

 * Mga folder ng data mula sa bawat laro. Kunin ang mga ito mula sa iyong mga retail games CD o Steam / GOG na
 mga bundle. Kunin lamang ang folder ng data mula sa folder ng bawat laro, at ilagay ito sa
 katumbas na  `/data/tr*/` na folder. Halimbawa, para sa TR3, ang landas ay magiging
 `OpenTomb/data/tr3/data/`

 * CD audio track. Sinusuportahan lamang ng OpenTomb OGG ang mga audiotrack nang ilang sandali, kaya dapat mo itong
 i-convert ang orihinal na mga soundtrack sa iyong sarili, o i-download lamang ang buong TR1-5
 pakete ng musika dito:  https://opentomb.earvillage.net
 PAKITANDAAN: Maaaring kailanganin ng mga file na palitan ang pangalan para magtrabaho ito, pakitingnan
  https://github.com/opentomb/OpenTomb/issues/447

 * Loading screens para sa TR1-3 at TR5. Para sa TR3, kunin sila mula sa direktoryo ng pix ng
 ang iyong naka-install na opisyal na laro. Ilagay lamang ang direktang pix sa `/ data / tr3 /`
 folder. Tulad ng sa iba pang mga laro, ito ay medyo nakakalito upang makakuha ng mga screen ng pag-load, tulad ng doon
 ay walang mga screen sa paglo-load para sa mga bersyon ng PC TR1-2, TR4 na ginamit na mga antas ng screenshot bilang
 nag-load ng mga screen, at ginamit ng TR5 ang naka-encrypt na format upang iimbak ang lahat ng naglo-load
 graphics. Kaya, upang mabawasan ang iyong buhay, maaari mong i-download lamang ang pag-load ng package ng screen
  dito: http://trep.trlevel.de/temp/loading_screens.zip
 Basta ilagay ito mismo sa direktoryo ng OpenTomb, at dapat na gawin ang bilis ng kamay. Tandaan:
 Sinusuportahan ng engine ang png at pcx na format ng mga larawan sa screen.

### Pag-ipon ###
May isang CMakeLists.txt na file na ibinigay kasama ang source code, kaya maaari kang sumulat ng libro
OpenTomb gamit ang CMake. Sa Windows, maaari mo ring itala ito mula sa Code :: Blocks IDE
(ibinigay din ang project file). Bilang kahalili, maaari mong manwal na itala ito
Code:: Blocks sa pamamagitan ng recursively pagdaragdag ng lahat ng mga file ng source mula sa / src directory, at
pagdaragdag ng mga aklatang ito sa Mga Setting ng Linker sa ilalim ng mga pagpipilian sa Proyekto ng Build:

* libmingw32.a
* libSDL2main.a
* libSDL2.dll.a
* liblua.a
* libpng.a
* libz.a
* libpthread.a

Sa Linux, i-download lamang ang source code at tumakbo sa terminal:

    cmake . && make

Ang mga kinakailangang dependency ay ang mga header ng pag-unlad para sa SDL2, png, LUA 5.2,
ZLIB. Maaari mong i-install ang mga ito sa isang distro na batay sa Ubuntu gamit ang command na ito:

    sudo apt-get install libopenal-dev libsdl2-dev libpng12-dev liblua5.2-dev libglu1-mesa-dev zlib1g-dev

Sa Mac, gamitin ang proyektong XCode, na magagamit din sa source code.

NB: Mangyaring tandaan ang OpenTomb ito'y nangangailangan ng C + + 11 (`-std = c + + 11`) na bandila upang makompile
maayos! Maaari kang gumamit ng mga bandila ng pag-optimize na partikular sa CPU (`-march = prescott`,
`-march = i486`,` -march = core2`), pati na rin ang pangkalahatang mga flag ng pag-optimize (`-O1` at` -O2`),
 ngunit HUWAG GAMITIN ang `-O3` na bandila, dahil ang Bullet ay may pag-crash sa antas ng pag-optimize na ito
 (GCC 5.1 + ay maaaring itala ito nang walang mga error
 
### Running and Configuration ###
Upang patakbuhin ang OpenTomb, patakbuhin lang ang mga maipapatupad na binuo ng build. Bilang default,
walang mga pagpipilian sa command line ang kinakailangan. I-access ang console sa pamamagitan ng pagpindot sa \`. TIto
nagpapahintulot sa iyo na magpasok ng mga command upang piliin ang mga antas, baguhin ang mga setting, at higit pa. Ipasok ang
'help' upang makakuha ng isang listahan ng mga utos. Ipasok ang 'exit' upang umalis sa engine.

Sa kasalukuyan, ang lahat ng mga setting sa OpenTomb ay pinamamahalaan sa pamamagitan ng config.lua at
autoexec.lua. Ang config.lua ay naglalaman ng mga persistent na engine at mga setting ng laro, habang
autoexec.lua ay naglalaman ng anumang mga utos na dapat isagawa sa engine start-up.

Ang Config.lua ay nahahati sa iba't ibang mga seksyon: screen, audio, render, kontrol,
console at system. Sa bawat isa sa mga seksyong ito, maaari mong baguhin ang maraming
mga parameter, ang mga pangalan ng kung saan ay karaniwang medyo madaling maunawaan.
Autoexec.lua is a simple list of commands which are ran at startup. Modifying
existing commands may cause the engine to function incorrectly.

Upang pumili ng isang antas, ipasok ang 'setgamef (laro, antas) sa alinman sa autoexec.lua o sa
ang console, kung saan ang laro ay 1-5. Ang mga antas ng palapag ay karaniwang 0, at mga laro na
walang simula ng mansyon mula sa antas 1. Halimbawa, upang i-load ang antas 2 ng TR3,
ipapasok mo ang `setgamef (3, 2)`.

### Paglilisensya ###
Ang OpenTomb ay isang open-source engine na ipinamamahagi sa ilalim ng LGPLv3 license, na nangangahulugang
na ang ANUMANG bahagi ng source code ay dapat ding open-source rin. Samakatuwid, lahat ay ginagamit
ng mga library at bundle resources ay dapat open-source sa GPL-compatible
mga lisensya. Narito ang listahan ng mga gamit na aklatan at mga mapagkukunan at ang kanilang mga lisensya:

* OpenGL — Hindi kailangan ng paglilisensya (https://www.opengl.org/about/#11)
* OpenAL Soft — LGPL
* SDL / SDL Image — zlib
* Bullet — zlib
* Freetype2 — GPL
* Lua — MIT
* ffmpeg rpl format at codecs (http://git.videolan.org/)

* Droid Sans Mono, Roboto Condensed Regular and Roboto Regular fonts — Apache

### Mga Kredito ###
NB: Mangyaring tandaan na ang mga may-akda at listahan ng mga kontribyutor ay patuloy na nagpapalawak, bilang
mayroong higit at higit pang mga tao na kasangkot sa pag-unlad ng proyekto, kaya ang isang tao ay maaaring
 nawawala mula sa listahang ito!

* [TeslaRus](https://github.com/TeslaRus): pangunahing developer.
* [Cochrane](https://github.com/Cochrane): renderer rewrites at optimizing, Mac OS X support.
* [Gh0stBlade](https://github.com/Gh0stBlade): renderer add-ons, shader port, gameflow implementation, state
control fix-ups, camera at AI programming.
* [Lwmte](https://github.com/Lwmte): state at scripting fix-ups, controls, GUI and audio modules, trigger
at entity system rewrites.
* Nickotte: interface programming, ring inventory implementation,
camera fix-ups.
* [pmatulka](https://github.com/pmatulka): Linux port at testing.
* [richardba](https://github.com/richardba): Github migration, Github repo maintenance, website design.
* [Saracen](https://github.com/Saracen): room and static mesh lighting.
* [T4Larson](https://github.com/T4Larson): general stability patches and bugfixing.
* [vobject](https://github.com/vobject): nightly builds, maintaining general compiler compatibility.
* [vvs-](https://github.com/vvs-): testing, feedback, bug report.
* [xproger](https://github.com/xproger): documentation updates.
* [Banderi](https://github.com/Banderi): documentation, bugfixing.
* [gabrielmtzcarrillo](https://github.com/gabrielmtzcarrillo): entity shader work.
* [filfreire](https://github.com/filfreire): documentation.


Additional contributions from: Ado Croft (extensive testing),
E. Popov (TRN caustics shader port), [godmodder](https://github.com/godmodder) (general help),
[jack9267](https://github.com/jack9267) (vt loader optimization), meta2tr (testing and bugtracking),
shabtronic (renderer fix-ups), [Tonttu](https://github.com/Tonttu) (console patch) and
[xythobuz](https://github.com/xythobuz) (additional Mac compatibility patches).

Translations by: [Joey79100](https://github.com/Joey79100) (French), Nickotte (Italian), [Lwmte](https://github.com/Lwmte) (Russian),
[SuiKaze Raider](https://twitter.com/suikazeraider) (Spanish), [filfreire](https://github.com/filfreire) (Portuguese - Portugal).
