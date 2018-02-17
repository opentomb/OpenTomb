OpenTomb -listahan ng GAGAWIN para sa mataas na priyoridad na mga bug / gawain
---------------------------------------------------

### Talaan ng mga Nilalaman ###

1. Ang pangunahing plano
2. Buhuin ang pagsasaayos
3. Git/GitHub workflow
4. Pangunahing Code
5. Sistema ng kolisyon
6. Pagkontrol ng karakter
7.Pagkontrol ng animation
8. Pagkontrol ng camera
9. Pag-iiskrip
10. Audio


1. Ang pangunahing plano
----------------
Una kailangan nating ipatupad ay ang TR1 gameplay, para ang TR1 / 2/3 na function ng gawain ay may mas mataas na prayoridad sa mga function ng TR4 / TR5. Ang unang layunin ay ang gumawa ng isang simple, ngunit matatag at nagtatrabaho na bersyon, pagkatapos ay palawakin  ang pag-andar sa mga ito nang sunud-sunod.

2. Buhuin ng Pagsasaayos
----------------------
* Gagawin:
	* I-update ang panloob na lib ng larawan
	* Bawasan ang bilang ng mga dependecies
	* Gumawa ng magandang autobuild system
	  
3. Git/GitHub workflow
---------------
* Angpagbigay sa `master` ay maaring:
	* Pagsamahin kasama ang tampok na sangay (sangay na naglalaman ng mga bagong tampok, o malaking pag-aayos ng bug) - Dapat na magagamit ang tampok na sangay sa GitHub; pagkatapos ng pagsasama ito ay dapat tanggalin
	* Maliit at malinis na mga pag-aayos ng bug
	* Mga update sa dokumentasyon
	* Gumawa ng mga update sa pag-script (maaaring gawin sa isang sangay ng tampok, sa pamamagitan ng pagpili ng may-akda)
	* Malayang mapagkukunang pag-uupdate

* Ang paglikha ng mga sangay ng tampok at paggawa sa kanila ay libre
* Ang mga espesyal na kahilingan sa pagsasama, hindi para sa pagsasama o sa naantala na oras para sa pagsasama ay dapat maglaman ng  `[NOT_FOR_MERGING]` prefix; ang mga naturang kahilingan ay maaaring pagsasama lamang pagkatapos magsulat ng may-akda ng komento ay nagsasabi na `[CAN_BE_MERGED_NOW]`
* 
* Paano magsumite ng isang tampok na branch sa `master`:
	1. I-install ang GIT bash at ilunsad ang terminal (o gamitin ang iyong sariling GUI)
	2. Lumikha ng lokal na sangay ng tampok sa pamamagitan ng issuing:  `git checkout -b feature_branch_name` (o lumikha ng isa sa repo sa web ng GitHub
	3. Ibigay ang lokal na branch na may `git add -u`, `git commit` (kung ikaw ay gagamit ng Vim, pindutin ang 'i', ipasok ang mensahe, pindutin ang escape, at pagkatapos ay ipasok)
	4. Pagsamahin ang tampok na branch kasama ang local master branch na may `git merge master` at lutasin ang anumang hindi pagkakaunawaan (gawing muli ang proyekto)
	5. Ipagpatuloy ang mga pagbabago at itulak ang local feature branch sa remote sa GitHub sa: `git push origin feature_branch_name`
	6.Lumikha ng isang pagsasanib (hilahin) ng kahilingan sa GitHub web UI
	7. Pagkatapos tanggapin ang pagsasama, tanggalin ang tampok na sangay

* _extern_ Hindi dapat baguhin ang folder - ito ay isang lugar para sa mga panlabas na mga pinagkukunan ng library / mga header
* _src/core_ folder: tanging sa pamamagitan ng TeslaRus, hilingin kung gusto mong baguhin ang isang bagay
* Other folders: sa pamamagitan ng pagsasama ng mga kahilingan sa TeslaRus o, pagkatapos ng pag-review ng code (sa pamamagitan ng kahilingan ng pagsasanib) sa pamamagitan ng command (mas maraming mga detalye ang tatalakayin); Magsasagawa ako ng ilan pagkatapos ng kahilingan sa pagsasama (ang bilang ng mga pagkakamali ay mabawasan nang malaki sa kaso ng pagsusuri)

4. Pangunahing Code
---------------
* Gagawin:
	* `game.cpp`: maraming iba't ibang mga logika sa isang lugar, kailangang refactored
	* Lumikha ng ilang mga module (hindi lahat!) interface mas abstract (itago panloob na pagsasakatuparan, tulad ng `physics.h`/`physics_bullet.cpp`)

5. Engine
-------------------
* Kasalukuyang sitwasyon:
	* Ipinatupad na pagbubuo ng entity at pagtanggal ng kaligtasan, projectiles, paglipat ng manlalaro ...

* Gagawin:
	* Magdagdag ng mga pag-aaktibong ng mga callback para sa mga aytem sa imbentaryo (walang higit na `read-only` para sa imbentaryo)
	* Bawasan ang paggamit ng globals (ibinahagi sa pagitan ng mga globe ng module)
	* Ilipat ang console.c rendering code 
	* Gumawa ng savable state ng alt-room sa kasong ito kung saan mayroon hanay ng 3 o higit pang mga alt-room.
	

6.Sistema ng kolisyon
-------------------
* Kasalukuyang sitwasyon:
	* Nakapirmi na back / front-facing polygons orientation para sa physics geometry, ngayon ang engine ay may working _Filtered Ray Test_ (skips back-faced polygons)
	* Ang kolisyon ng margin ay zero, kung hindi man ay magiging makinis ang mga normal na mga gilid at si Lara ay lilipat o titigil sa mga lugar na hindi niya dapat
	* Sinusupil ang pagpapatupad ng mga pag-crash ng banggaan na nagpapahintulot upang magrehistro ng pinsala sa hit at anumang iba pang mga banggaan
  
* Gagawin:
	* Ayusin ang paglipat pagkatapos ng landing sa sloped surface:
		* Maghanap ng mga bahagi ng katawan na pipigil kay Lara
		* Mag-form ng kolisyon, o huwag paganahin ang pag-check ng kolisyon para sa mga ito
		* Bind sa 3
	* Gumawa ng mga mahigpit na bahagi ng katawan na mahuhusay sa pamamagitan ng config
	* Para sa pag-optimize sa hinaharap, magdagdag ng switchable single ghost object para sa karakter
	* Add _Long Ray Test_ (pierces rooms portals and builds room list for collisional checking) - kailangan para sa mahabang hanay ng pagbaril at AI
	* Muling ipatupad ang  Character_FixPosByFloorInfoUnderLegs(...) it has been deleted
	* Tingnan ang room tween butterfly na mga normal

7. Pag-kontrol ng karakter
-----------------------
* Gagawin:
	* Base AI, paghahanap ng landas, mga kahon...
	* Ang sistema ng kontrol ng sandata ay kailangang refactored / fixed (2-kamay na modelo ng switch ng armas sa maling frame)
	* Magdagdag ng auto weapon hiding sa kapaligiran ng tubig e.t.c. (simpleng gawain)
	* Ayusin ang paggamit ng mga armas habang yumuyuko

8. Pag-kontrol sa Animation
--------------------
* Todo:
	* I-update ang dokumentasyon tungkol sa  `ss_animation` na istraktura at mga function
	* Ayusin ang hindi tama smoothing kung mayroong _move_ or _rotate_ anim commands
	* Ayusin ang mga dive-roll:
		* Ang mga Roll ay hindi nagsisimula kaagad habang ito'y sumusulong
		* Napakalaki ng distansya ng roll (hal. Bumabagsak na 1x1m ledges kapag nasa tapat na gilid)
	* Ayusin ang pasulong at pabalik na magkakasunod na mga roll ng jump (mid-air roll) na hindi concatenating nang tama sa keypress (TR2 +)
	* Ayusin ang swan dive hindi maaaring gawin kapag tumatalon ng irregular (diamond shaped <>) na mga slope
	* Ayusin ang pag-akyat sa gilid:
		* Ayusin ang dive-Climbability na distansya ng threshold masyadong mataas kapag tumatalon (ibig sabihin maabot ang mga taas na hindi dapat maabot) roll
		* Bawasan ang pagwawasto ng taas kapag hinahayaan ni Lara na mahawakan ang isang gilid (hawakan pindutan nang matagal)
9. Pag-kontrol sa Camera
-----------------
* Gagawin:
	* Ayusin ang pag-target ng camera upang itama ang bahagi ng katawan o sentro ng OBB (magdagdag ng mga espesyal na pag-andar upang makakuha ng pag-target ng pos sa id ng target na entidad)
	* Ipatupad ang mga flags ng camera at ang kanilang function (hal. "Flyby", "isang beses")
	* Magdagdag ng mga espesyal na  `camera_entity`, iimbak ito sa module ng mundo, pag-access ng `entity_p World_GetCameraEntity();` - kinakailangan para sa mabigat na pag-trigger

10. Pag-iiskrip
------------
* Kasalukuyang sitwasyon:
	* SEE TRIGGERS_tasks.md
	* I-update ang iskrip sa BAWAT frame ng laro! Gamitin ang oras ng global engine frame sa loob ng time depended script!
* Gagawin:
	* Magdagdag ng pag-andar tulad ng `lua_SaveTable(...)` na recursively na naka-print sa file / buffer / clay tablet lua code sa table content (i.e. `table_name = { red = 1; green = 0; blue = 0; name = "name"; is_u = true; in_tbl = { p1 = "inner"; val = 32.45 } }`)
	* Sa lahat ng mga iskrip na maaaring magbago ang estado ng laro, ang data ay dapat na naka-imbak sa espesyal na pandaigdigang talahanayan (na mai-save sa i-save ang laro) - kailangan para sa pag-save / pag-andar ng mga pag-andar ng laro upang magtrabaho nang tama

11. Audio
---------
* kasalukuyang sitwasyon:
	* Hindi gumagana ang mga sound track playing
	* Ang AL build-in library ay gumagana sa Windows at MacOS, ngunit sa ilalim ng Linux ng katutubong AL library ay kinakailangan
* Gagawin:
	* Sa `audio.cpp` ipatupad ang klase para sa pagmamanipula ng data ng track ng tunog (hal. `result GetBufferData(track_id, buffer, size, offset, flag)`)
	* Ipatupad ang sariling audio routine thread (API tulad ng (APIs like `Audio_Send(...)` payagan ito)
	* Gumamit ng ibang bagay sa halip na Vorbis (hindi ito makabasa ng _OGG_ mula sa memorya, at gumagamit ng mga default na pag-andar para sa mga pagbubukas ng mga file, kaya ang engine ay hindi maaaring ma-precache ang mga track sa memory o gumamit ng  `SDL_rwops`)
