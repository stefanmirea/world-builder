This was my second homework for a computer graphics course in the 3rd year.

Task: implementing a C++ application for designing a mini-world, using a given
framework. The projection must be isometric and there must be four edit modes:
Terrain (for creating greenspaces or water), Square (for inserting squares that
allow houses to be built around on a certain radius), Houses (to add houses that
you can rotate / translate afterwards) and Roads (to connect houses to squares).

Full statement:
http://cs.curs.pub.ro/2014/pluginfile.php/9405/mod_resource/content/1/Tema%202%20EGC.pdf
(You might not have access to it as a guest.)

Check out the repo wiki for some screenshots.

Original README file:
================================================================================
EGC - Tema 2
    World Builder
    Stefan-Gabriel Mirea, 331CC

Cuprins

1. Cerinta
2. Utilizare
3. Implementare
4. Testare
5. Probleme Aparute
6. Continutul Arhivei
7. Functionalitati

1. Cerinta
--------------------------------------------------------------------------------
Tema cere implementarea unui program care sa permita construirea unei mini-lumi
in proiectie izometrica folosind framework-ul de la laborator, editarea
realizandu-se prin comutarea intre patru moduri: teren, piete, case sau drumuri.
Astfel, modul de editare teren permite adaugarea de spatii verzi sau apa,
pietele ofera zone circulare pe a caror raza pot fi constuite case, casele vor
putea fi rotite si translatate, iar drumurile conecteaza casele la piete.

2. Utilizare
--------------------------------------------------------------------------------
2.1 Fisiere

Programul poate lucra cu fisiere binare pentru salvarea sau incarcarea lumilor
create de utilizator. Numele fisierelor folosite poate fi oricare si se
specifica in interfata programului. Presupunand dimensiunea traditionala a unui
int de 4 octeti, fisierele vor avea urmatoarea structura:
   - 4 octeti: un int ce specifica numarul de linii ale matricei elementelor
               care s-au modificat (fata se spatiul initial, gol);
   - 4 octeti: un int ce specifica numarul de coloane ale aceleiasi matrice;
   - in continuare, pentru fiecare element al matricei (parcurse se sus in jos
     si de la stanga la dreapta):
       - 1 octet: unsigned char ce retine pe biti urmatoarele informatii:
          - bitii 0 si 1 retin tipul de teren, specificat printr-una din
                  constantele simbolice GR_EMPTY, GR_WATER sau GR_GRASS;
          - bitii 2, 3, 4 respectiv 5 specifica inclinarea terenului,
                  determinata de inaltimile colturilor stanga-sus, dreapta-sus,
                  dreapta-jos respectiv stanga-jos: 1 = ridicat, 0 = coborat.
          - bitii 5 si 6 sunt mereu 0 (forward compatibility).
       - 1 octet: char reprezentand elementul situat pe pamant, printr-una din
                  constantele EL_EMPTY, EL_SQUARE, EL_HOUSE sau EL_ROAD;
       - 4 octeti: daca elementul e o casa (EL_HOUSE), un int ce specifica
                   numarul de etaje, altfel sunt nuli;
       - 4 octeti: int reprezentand altitudinea reliefului (care poate fi si
                   negativa);
       - 1 octet: unsigned char pentru flag-uri:
          - bitul 0: spune daca elementul se afla intr-o zona construibila (in
                     raza unei piete);
          - bitul 1: spune daca elementul a fost vizitat in cadrul unei
                     parcurgeri;
          - bitul 2: daca elementul e o casa, spune daca e conectata la o piata,
                     altfel are valoarea 0;
   - 4 octeti: un int reprezentand linia cursorului;
   - 4 octeti: un int reprezentand coloana cursorului;
   - 4 octeti: un float reprezentand dimensiunea laturii unui cub (pentru zoom).

2.2 Consola

Implicit, programul va afisa la deschidere o lume goala, careia nu ii corespune
un fisier. Pentru a specifica un fisier care sa fie deschis automat la pornire,
calea acestuia se poate da ca argument in linia de comanda. Exemplu de apel:
>egc_tema2.exe castel.t2egc
Prin urmare, in Windows se poate realiza asocierea dintre program si o anumita
extensie.

2.3 Input Tastatura

- sageti sau combinatii de sageti: muta selectia in directia dorita (relativ la
     ecran, nu la matrice; de exemplu, sageata in sus muta cursorul prin
     scaderea atat a liniei cat si a coloanei);
- tastele corespunzatoare caracterelor afisabile (mai putin '?') + Backspace:
     editarea comenzii curente din CLI (vezi mai jos);
- '?': apar comenzile / argumentele care pot fi introduse in CLI, sau <cr> daca
     s-a introdus deja o comanda completa (vezi mai jos);
- Enter: executa comanda introdusa in CLI (vezi mai jos);
- Esc: anuleaza selectia multipla (echivalent cu "selection exit", vezi mai
     jos).

2.4 Emulatorul de terminal

Pentru a nu atribui comenzilor un numar mare de taste, am emulat in partea de
jos a ferestrei o interfata in linia de comanda, folosind obiecte 2D si texte.
In acest context, am implementat cateva facilitati asemanatoare cu cele ale
sistemului de operare Cisco IOS:
   - la tastarea caracterului '?' (chiar si in cadrul unui cuvant), apar
     sugestii cu ce se poate introduce mai departe;
   - fiecare comanda / subcomanda / etc. se poate introduce prescurtat,
     printr-un prefix care o identifica in mod unic.
Comenzile acceptate sunt ('[' si ']' semnifica subcomanda implicita):
   - clear: goleste ecranul emulatorului;
   - zoom:
        - [in]: mareste obiectele cu un factor de 1,3;
        - out: micsoreaza obiectele cu un factor de 1,3;
        - reset: reseteaza vizualizarea (latura unui cub devine EDGE_DEFAULT);
   - fragment:
        - [on]: la deplasarea cursorului nu se vor mai actualiza obiectele
                vizibile, ramanand desenate doar cele afisate la introducerea
                comenzii; astfel, va disparea iluzia unei lumi infinite;
        - off: reactiveaza actualizarea elementelor vizibile la dreplasare;
   - selection:
        - [start]: initiaza o selectie dreptunghiulara, modificabila prin
                   deplasarea cursorului;
        - exit (sau tasta ESC): anuleaza selectia multipla;
   - edit:
        - terrain: face trecerea la modul de editare de teren;
        - square: face trecerea la modul de editare de piete;
        - houses: face trecerea la modul de editare de case;
        - roads: face trecerea la modul de editare de drumuri;
   - water (disponibil doar in modul de editare de teren): adauga o celula cu
           apa;
   - grass (disponibil doar in modul de editare de teren): adauga o celula cu
           iarba;
   - delete: incearca stergerea elementelor selectate, in functie de modul de
             editare;
   - up (disponibil doar in modul de editare de teren): incrementeaza
        altitudinea reliefului zonei selectate; pentru mentinerea unei pante de
        cel mult 1, vor fi afectate si celulele din jur;
   - down (disponibil doar in modul de editare de teren): decrementeaza
          altitudinea reliefului zonei selectate, afectand elementele din jur
          datorita aceleiasi restrictii;
   - add (nedisponibil doar in modul de editare de teren): in functie de modul
         de editare, poate adauga o piata, un corp unei case (sau un nou etaj
         corpului deja existent) sau un drum;
   - rotate:
        - [clockwise]: roteste in sensul acelor de ceas casele care au cel putin
                       un corp (o celula) in zona selectata;
        - counterclockwise: roteste in sens trigonometric casele care au cel
                            putin un corp (o celula) in zona selectata;
        - view:
             - [clockwise]: roteste vizualizarea (implementat de fapt prin
                            rotirea lumii) in sensul acelor de ceas;
             - counterclockwise: roteste vizualizarea in sens trigonometric;
   - cut: decupeaza casele care au cel putin un corp in zona selectata;
   - copy: copiaza casele care au cel putin un corp in zona selectata;
   - paste: lipeste ceea ce s-a decupat / copiat, pastrand aceleasi distante
            relativ la cursor;
   - new: creeaza o lume noua;
   - save: salveaza lumea curenta; daca aceasta nu e inca asociata cu un fisier,
           va aparea un dialog care va cere numele fisierului;
        - <numele fisierului>: salveaza lumea curenta in fisierul dat;
   - load / open:
	- <numele fisierului>: incarca lumea din fisierul dat;
   - exit / quit: incheie executia programului dupa asigurarea ca datele au fost
                  salvate daca utilizatorul a dorit acest lucru.

3. Implementare
--------------------------------------------------------------------------------
Platforma:
   IDE: Microsoft Visual Studio Professional 2013
   Compilator: Microsoft C/C++ Optimizing Compiler Version 18.00.21005.1 for x86
   Sistem de operare: Microsoft Windows 7 Ultimate SP1 Version 6.1.7601

Generalitati:
Pentru realizarea proiectiei izometrice, au fost necesare cunostinte minime de
matematica 2 si MN. Am inceput printr-o proiecte pe planul ce trece prin origine
si e paralel cu cel ce contine punctele (1, 0, 0), (0, 1, 0) si (0, 0, 1) (adica
pe planul de ecuatie explicita z = -x - y), a carei matrice se determina simplu,
intrucat coloanele ei sunt punctele in care este proiectata baza canonica. In
continuare, acest plan sufera doua rotiri pentru a se suprapune peste xOy. In
primul rand, e rotit in xOz cu pi / 4, pentru ca intersectia dintre el si xOz
(dreapta 2D x + y = 0) sa se suprapuna peste Ox. Apoi, e rotit in planul yOz cu
un unghi egal cu unghiul care apare intr-un cub intre o muchie laterala si un
segment care uneste varful de jos al acestei muchii cu centrul bazei superioare
a cubului (e usor de vazut ca acesta este unghiul cautat, pentru ca este fix
unghiul dintre axa verticala si planul care trece prin cele 3 varfuri adiacente
varfului de sus al muchiei, plan care are aceeasi inclinatie verticala cu al
nostru). Daca se deseneaza separat o sectiune diagonala a cubului, se va observa
ca acest unghi e arctan(sqrt(2) / 2). Cele doua matrice de rotatie sunt de fapt
doi rotatori Givens. In metoda isometricProjectionMatrix() din Transform3D.cpp,
am scris matricea rezultata din inmultirea acestor trei matrice.

Suprafata pe care se construieste este infinita. Elementele care s-au modificat
(fata de spatiul initial gol) sunt retinute in matricea modified, ce instantiaza
clasa InfiniteMatrix, folosita pentru lucrul cu matrice infinite. In general,
elementele sunt identificate printr-o pereche (linie, coloana) ale carei
elemente pot fi si negative. Diferenta dintre coordonatele in acest sistem
global si cele din matricea membra a clasei InfiniteMatrix este data de perechea
(delta.row, delta.col). De cate ori se adauga un element in afara spatiului
acoperit de matrice (care initial nu contine niciun element), aceasta este
redimensionata pentru a permite insertia, iar campul delta este modificat
corespunzator.

In ceea ce priveste afisarea elementelor pe ecran, obiectele afisate la un
momentdat sunt doar cele care se suprapun cu fereastra din coordonatele logice,
pentru sporirea performantei (de asemenea, in cadrul fiecarui element sunt
desenate doar fetele vizibile). Aceasta zona e determinata de celulele
left_bottom si right_top, care delimiteaza in matrice un romb si care se
incrementeaza / decrementeaza in functie de FRAGMENT_SIZE (practic, se adauga
sau se elimina doar rombulete de latura FRAGMENT_SIZE, numite in cod fragmente).
Pentru a putea tine evidenta pozitiilor din objects3D a elementelor care se
adauga sau se sterg la deplasarea ferestrei, am folosit vectorul drawn_elements,
ce contine structuri care asociaza informatiile (linie, coloana, inaltime, tip
de selectie) cu pozitii din objects3D. Aceste elemente sunt sortate in primul
rand dupa inaltime, apoi dupa "diagonala secundara" pe care se afla (ordine
corespunzatoare afisarii unora in fata / spatele altora pe ecran). Toate
accesarile de elemente in acest vector se fac prin cautare binara. Elementele de
selectie (inclusiv cursorul) vor fi afisate intotdeauna in fata celorlalte
elemente de la acelasi nivel vertical, prioritate implementata prin simularea
unor altitudini fractionare in functia compare_elements().

Emulatorul de terminal foloseste urmatoarele contexte vizuale:
   - current_cmd - incadreaza textul comenzii curente;
   - current_cmd_backgr - fereastra incadreaza dreptunghiul cmd_backgr, poarta e
la fel ca a lui current_cmd;
   - cli - incadreaza textele din linia de comanda;
   - cli_backgr - fereastra incadreaza dreptunghiul cmd_backgr, poarta e la fel
ca a lui cli;
In cazul ambelor perechi, au fost necesare cate doua contexte vizuale pentru a
determina textele sa se afiseze in fata dreptunghiurilor (datorita ordinii de
insertie a contextelor vizuale in visuals2D).
Cursorul din linia de comanda e reprezentat de dreptunghiul cli_cursor, care se
inlocuieste periodic cu cli_cursor_empty.
Toate aceste elemente, la care se adauga dreptunghiul window_backgr (afisat
pentru umplerea spatiului suplimentar la redimensionare), sunt situate in
coordonate logice deasupra spatiului de desenare. Ori de cate ori utilizatorul
se deplaseaza spre ele, acestea sunt mutate si mai sus (de functia
update_hidden_elements()), incat interferenta lor cu spatiul de joc sa nu fie
posibila.

Pentru interpretarea comenzilor din linia de comanda, am folosit vectori de
structuri CmdNameFct, prin care numele comenzilor sau argumentelor sunt asociate
cu functii. Pentru identificarea comenzii dupa prefixul furnizat sau pentru
afisarea sugestiilor la introducerea caracterului '?', se foloseste functia
find_command().

Pentru transmiterea numelui fisierului dat in linia de comanda catre functia
init(), i-am adaugat si acesteia doi parametri, int argc si char** argv.

Calculul elementelor din jurul pietelor (dintr-o raza mai mica sau egala cu
RADIUS) e realizat in update_square_coverage(), care realizeaza o parcurgere in
latime / algoritmul lui Lee pentru identificarea elementelor din jurul pietelor
aflate la distanta Manhattan de cel mult RADIUS. Dintre acestea, la fiecare
extragere din coada sunt marcate numai acelea pentru care distanta euclidiana e
de cel mult RADIUS, pentru ca zona de acoperire sa fie circulara. Rezultatele
sunt retinute in bitul 0 al campului flags al structurilor element (1 = aproape
de piata; 0 = departe).

Calculul elementelor din cadrul caselor conectate la piete se face in
update_square_connectivity(), unde se foloseste tot o parcurgere in latime
pornind de la piete, ce continua apoi pe drumuri. De cate ori e atins un corp
nevizitat al unei case, are loc o parcurgere in latime pentru identificarea si
marcarea casei respective. Rezultatele sunt retinute in bitul 1 al campului
flags al structurilor Element (1 = conectat, 0 = neconectat).
Comenzile de rotire, decupare si copiere se aplica asupra caselor care au cel
putin un corp in zona selectata. Pe post de translatie, se poate folosit 'cut'
urmat de 'paste'.

Singurele elemente care pot fi asezate pe un teren inclinat sunt iarba (care
determina de fapt tipul terenului) si drumurile. Prin urmare, casele pot fi
conectate si de piete aflate la inaltimi diferite.

Intrucat nu am implementat lucru cu mouse-ul, am sters functiile mouseFunction()
si onMouse() din framework.

4. Testare
--------------------------------------------------------------------------------
Tema a fost testata doar pe calculatorul personal (platforma mentionata la 3.).
Am incercat sa aplic cam toate scenariile posibile si am corectat toate
problemele care au aparut, cu exceptia urmatorului bug legat de ordinea
afisarii: elementele de selectie (fie ca e vorba de o selectie multipla sau de
cursor) a unor celule inclinate sunt afisate deasupra tuturor obiectelor aflate
la aceeasi inaltime cu elementul de selectie, motiv pentru care un obiect asezat
in fata unui teren inclinat selectat se va afisa ca si cum ar fi in spatele
selectiei. Acest scenariu este ilustrat in fisierul Teste\bug.t2egc. Am prevazut
aceasta situatie inca de cand am ales ordinea in care vor fi afisate
elementele (ordonate mai intai dupa inaltime, elementele de selectie fiind mereu
mai in fata decat celelalte de la acelasi nivel) dar nu am gasit o metoda simpla
de rezolvare, care sa nu duca la alte probleme. Totodata, elementele de selectie
nu sunt afisate in totalitate daca terenul se continua deasupra (pentru ca
celelalte elemente ale pamantului sunt deja pe un nivel superior, deci se
afiseaza mai in fata). De exemplu, daca se da comanda "down" si se muta cursorul
pe cea mai de jos celula, selectia nu va mai fi vizibila deloc.
Un alt exemplu de test este lab2\castel.t2egc.

5. Probleme aparute
--------------------------------------------------------------------------------
Legat de framework, am avut probleme la distingerea tastelor sageti si a altor
taste speciale (Shift, Alt, Ctrl) de anumte taste alfanumerice. Am rezolvat
acest lucru prin crearea variabilei globale special_key_pressed in
DrawingWindow.cpp, careia ii schimb valoarea in functiile keyboardFunction() si
specialFunction().
De asemenea, framework-ul nu oferea posibilitatea tratarii evenimentului de
eliberare a unei taste, necesar pentru a recunoaste combinatiile de sageti.
Aceasta problema am rezolvat-o prin crearea metodei specialFunctionUp() si
adaugarea liniei:
glutSpecialUpFunc(specialFunctionUp);
in constructorul clasei DrawingWindow.

6. Continutul Arhivei
--------------------------------------------------------------------------------
lab2\castel.t2egc - un exemplu de fisier de intrare
lab2\include\* - definiri de constante simbolice, declaratii de clase si functii
lab2\src\* - sursele adaugate de mine
Teste\bug.t2egc - explicat la 4
Toate celelalte fisiere fac parte din framework-ul din laborator.

7. Functionalitati
--------------------------------------------------------------------------------
7.1 Functionalitati standard (toate cele din enunt)

    - desenare grila si scroll (realizat prin mutarea cursorului);
    - editare teren:
         >edit terrain
         >water
         >grass
    - amplasare piete si desenare zona de acoperire:
         >edit square
         >add
    - amplasare si desenare case (cu rotatie):
         >edit houses
         >add
         >rotate clockwise
         >rotate counterclockwise
    - editare drumuri si indicare case neconectate:
         >edit roads

7.2 Functionalitati Bonus / Suplimentare

    - animatie de deplasare a ferestrei pentru incadrarea cursorului la centru,
      realizata cu o viteza dependenta de distanta dintre pozitia cursorului si
      centrul ferestrei la un momentdat;
    - emularea unui terminal pentru introducerea de comenzi, cu facilitatile
      descrise la 2.4;
    - redimensionarea corecta a ferestrei, prin scalarea uniforma doar a
      spatiului de joc, cu toate ca acesta nu umple fereastra pe niciuna din
      directii si pastrarea latimii liniei de comanda la egalitate cu latimea
      spatiului de joc la orice moment;
    - navigare avansata prin rotirea vizualizarii ('rotate view clockwise /
      counterclockwise') si zoom ('zoom in', 'zoom out', 'zoom reset');
    - salvare si incarcare a lumii create intr-un mod similar cu aplicatiile
      reale care lucreaza cu fisiere, inclusiv folosirea de dialoguri specifice
      unde e cazul (pentru a intreba daca datele trebuie salvate inainte de
      new/open/exit, pentru introducerea numelui fisierului in care se salveaza
      daca nu se cunoaste, precum si pentru suprascriere) si acceptarea in linia
      de comanda a caii fisierului ce va fi deschis la pornire, permitand
      asocierea in Windows a programului cu fisierele de un anumit tip;
    - modele de case oricat de complexe datorita unirii corpurilor, care de
      asemenea pot avea oricate etaje;
    - reprezentarea grafica a drumurilor prin unirea acestora de piete, case si
      de celelalte drumuri adiacente;
    - forme de relief in inaltime si adancime (prin existenta inaltimilor
      negative), cu pastrarea automata a pantei maxime a terenului la valoarea 1
      ('up', 'down' - valabile si pentru o selectie multipla - 'selection
      start');
    - modificarea la orice moment a pamantului (prin comenzile 'water', 'grass'
      si 'delete'), a pietelor ('add' si 'delete'), caselor ('add', 'delete',
      'rotate', 'cut', 'copy', 'paste') si a drumurilor ('add', 'delete') -
      evident, toate astea se pot face folosind si o selectie multipla
      ('selection start');
    - afisarea de mesaje corespunzatoare pentru orice actiune incorecta;
    - simularea luminozitatii fetelor prin crearea de obiecte distincte pentru
      fiecare dintre fete.