# Смешанное программирование: `C++` вызывает `FORTRAN`

Положим, вам нужно из приложения, написанного на `C++`, вызвать подпрограммы, написанные на `FORTRAN`’е.
*Да*, такое может легко с вами случиться. Сначала о том, зачем это понадобилось мне. 
Если вам хочется сразу к делу, пропускайте следующий пункт.

## Зачем

В проекте [CATS’ PDEs] (https://github.com/CATSPDEs) часто приходится работать с большими разреженными матрицами — матрицами, большинство элементов которых равны нулю и не хранятся. 
Существует, вообще говоря, довольно много<sup id="fnb1">[1] (#fn1)</sup> форматов хранения для таких матриц. 

CSC–формат (*Comressed Sparse Column*, разреженно–столбцовый) является наиболее гибким и популярным в МКЭ форматом.
На нём же и основан не менее популярный стандарт хранения матриц в виде ASCII–текста **Harwell–Boeing**<sup id="fnb2">[2] (#fn2)</sup> (да–да, те самые боинги).

Матрицы в HB–формате нынешний софт понимает и любит. Например, та же `Mathematica` [умеет] (https://reference.wolfram.com/language/ref/format/HarwellBoeing.html) в импорт/экспорт HB–матриц
в свой [SparseArray] (http://reference.wolfram.com/language/ref/SparseArray.html]), который, кстати, [использует] (http://reference.wolfram.com/language/LibraryLink/tutorial/InteractionWithMathematica.html#918619650) CSR–формат (*Comressed Sparse Row*, разреженно–строчный) андерхуд.

Очевидно<sup id="fnb3">[3](#fn3)</sup>, что хороший тулкит вроде [CATS’ PDEs] (https://github.com/CATSPDEs), в котором предполагается работа с разреженными матрицами, 
должен поддерживать i/o стандартных форматов типа Harwell–Boeing, Matrix Market и [так далее] (http://math.nist.gov/MatrixMarket/):

* **Ввод**. Чтобы тестировать решатели систем на матрицах, полученных из реальных задач моделирования течения жидкостей, акустики, экономики и так далее.
Вот [ссылка] (http://math.nist.gov/MatrixMarket/data/Harwell-Boeing/) на коллекцию Harwell–Boeing.
* **Вывод**. Чтобы с собранными в вашем приложении матрицами мог работать другой софт — например, чтобы тот же [MatrixPlot](http://reference.wolfram.com/language/ref/MatrixPlot.html)
в Математике нарисовать. Конечно, вы можете передать вашу матрицу и в плотном виде. Если она поместится в оперативную память и у вас есть желание подождать 1000 лет.

[CATS’ PDEs] (https://github.com/CATSPDEs) написан преимущественно на `C++11`. 
Существуют готовые библиотеки ввода/вывода для Harwell–Boeing, написанные на `C`/`C++`:

* [раз] (http://people.sc.fsu.edu/~jburkardt/cpp_src/hb_io/hb_io.html), `С++` (John Burkardt),
* [два] (http://math.nist.gov/~KRemington/harwell_io/harwell_io.html), `С` (Karin A. Remington).

Однако ввод/вывод делать я решил на `FORTRAN`’е и сейчас объясню, наконец, **зачем**. 
Так уж вышло, что Harwell–Boeing придумали в далёком 1992-м; если вы посмотрите в [[2] (#fn2), с. 9], как он устроен, то довольно быстро поймёте,
что формат очень `FORTRAN`–ориентирован — уже по первому предложению:

> Our collection is held in an **80–column**, fixed–length format for portability…

В отличие от `C`/`C++`, `FORTRAN` [очень богат] (http://www.cs.mtu.edu/~shene/COURSES/cs201/NOTES/format.html) на форматы ввода/вывода.
Поэтому чтобы написать i/o на `C`/`C++`… придётся писать свой `FORTRAN`–like форматный i/o! 
Что, кстати, обе вышеуказанные библиотеки и делают: первая — 4 226 строк, вторая — 1 604. И это [против элегантных 129] (http://math.nist.gov/MatrixMarket/src/hbcode2.f), написанных на `FORTRAN`’е   

Впрочем, свой `FORTRAN` на `C`/`C++` — это ещё половина беды. Например, [вот эту] (http://math.nist.gov/MatrixMarket/data/Harwell-Boeing/lsq/illc1033.html)
матрицу первое решение вообще не прочитает, потому что поддержку [дескриптора P] (https://docs.oracle.com/cd/E19957-01/805-4939/z4000743a6e2/index.html)
автор не сделал (упс, не дописали свой `FORTRAN`).

А ещё в этом же решении мной был найден баг (смотрите 1232-ю строчку [здесь] (http://people.sc.fsu.edu/~jburkardt/cpp_src/hb_io/hb_io.cpp) или просто поищите “Alexander” — зачем мне врать).
Короче, сами понимаете…

Надеюсь, на вопрос **зачем** я ответил.

## С чем будем работать

* платформа: Windows 10 x64,
* компилятор `C++`: MSVC (Visual Studio 2015),
* компилятор `FORTRAN`: ifort.

Вероятно, Visual Studio* у вас уже есть. Intel Fortran суть часть Intel Parallel Studio, купить её можно [здесь] (https://software.intel.com/en-us/fortran-compilers/try-buy). Если вы студент, ресёрчер и т.п., можете [забрать бесплатно] (https://software.intel.com/en-us/qualify-for-free-software) — достаточно иметь вузовскую почту.

Вероятно, вам не захочется устанавливать все модули Intel Parallel Studio — оставьте чекбоксы только на ifort. Крайне советую установить их расширение для работы с `FORTRAN`’ом для Visual Studio* (нажать чекбокс во время установки). Ниже объясню, почему это удобно.
В Intel Parallel Studio нет GUI. **Неочевидная вещь**: если вы забыли установить какой-то модуль (или установили лишний), можно это исправить… через удаление. После запуска деинстоллера выбираете опцию “Modify” и вперёд.  

## План™

Вызывать будем подпрограммы `FORTRAN`’а из `C++`–приложения. Точка входа в [`sln/C++ Sources/user.cpp`](https://github.com/56th/CPP-Calls-FORTRAN-lib/blob/master/sln/C++ Sources/user.cpp), подпрограммы — в [`sln/FORTRAN Static Lib/add.f90`](https://github.com/56th/CPP-Calls-FORTRAN-lib/blob/master/sln/FORTRAN Static Lib/add.f90 ) и [`″/square.f90`](https://github.com/56th/CPP-Calls-FORTRAN-lib/blob/master/sln/FORTRAN Static Lib/square.f90 ).

Есть два пути: создать статическую (в Windows обычно имеет расширение **.lib**) или динамическую (shared) библиотеку (″ **.dll**). По своей сути статическая библиотека суть объектный файл (**.o**, **.obj**) и подключается к приложению во время линковки, динамическая — исполняемый файл (**.exe**), подключается в рантайме.

Для наших целей достаточно написать (возможно, много) подпрограмм на `FORTRAN`’е, скормить их ifort’рану и собрать статическую библиотеку, которую затем успешно линковать к `C++`. На времени компиляции основного приложения это не отразится и все будут счастливы.

### Игрушечный пример: компиляция из терминала

Запускаем терминал ifort: `[Start] — All Apps — Intel Parallel Studio* — Compiler*`. [Соберём и прилинкуем] (https://software.intel.com/en-us/node/525274) статическую библиотеку.

1. Получаем объектные файлы:

   `> ifort /c add.f90 square.f90`

2. Собираем библиотеку:

   `> lib /out:f.lib add.obj square.obj`

3. Компилируем приложение с MSVC и запускаем:

   `> cl /EHsc /Ox user.cpp f.lib`
   `> user.exe`
   
![Терминал ifort][ifortTerminal]

Ту же штуку вне среды ifort у вас так просто провернуть не получится, потому что MSVC понятия не имеет, где искать `FORTRAN`’онские библиотеки (типа ifconsol.lib и т.д., как [предупреждает] (https://software.intel.com/en-us/node/525352) документация):

![Терминал MSVC][MSVCTerminal]

Проблему можно решить, прилинковав вручную недостающие библиотеки. Дефолтно искать их стоит где-то в `C:\Program Files*\IntelSWTools\compilers_and_libraries*\windows\compiler\lib`.

Легко догадаться, что то же веселье вас ждёт и при работе из Visual Studio* (а бог не зря вам её послал — едва ли вы будете компилировать и дебажить большой проект вручную из терминала). Ну и ещё добавится возня с разными платформами — Win32 / x64, разными конфигурациями — Release / Debug и [конфликтами C Run–Time Library] (https://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx) — добавленные вручную библиотеки могут быть скомпилированы с разными ключами CRT.

Чтобы сего веселья избежать, удобно использовать интеловское расширение для Visual Studio*, которое в предыдущем пункте я советовал установить.

### Настройка IDE

Положим, у нас есть в решении два проекта: пустой проект Visual C++ для user.cpp (**C++ Sources**) и статическая библиотека Visual Fortran для add.f90 и square.f90 (**FORTRAN Static Lib**).

Второй создать очень просто (я предполагаю, что расширение установлено) — `File — New Project — Intel(R) Visual Fortran — Library — Static Library`.

Головной модуль расположен в **C++ Sources**, поэтому необходимо, чтобы компилировался именно этот проект: `Right Click — Set as SrartUp Project`.

Для настройки (почти) достаточно прочитать [эту] (https://software.intel.com/en-us/node/525352) и [эту] (https://software.intel.com/en-us/articles/configuring-visual-studio-for-mixed-language-applications) странички документации и посмотреть примеры проектов, дефолтно расположенных примерно в `C:\Program Files*\IntelSWTools\samples*\en\compiler_f\psxe\MixedLanguage`. Однако имеют место небольшие опечатки и недосказанности, поэтому я лучше здесь об этом напишу.

По какой-то причине Visual Fortran–проекты не переключаются на x64–платформу автоматически (естественно, если вы хотите делать 64-битные приложения, вы скачали эту версию ifort). Чтобы это исправить, нажимаем на селектбокс платформ Win32 / x64: `Configuration Manager… — Platform — (напротив FORTRAN Static Lib) <New…> — x64 — Copy settings from: <Empty>`.

Теперь попробуйте билдить **FORTRAN Static Lib**, переключая платформы. Если всё верно, в режиме x64 в окошке с логами вы заметите что-то вроде `Compiling with Intel(R) Visual Fortran Compiler* [Intel(R) 64]...`, а в режиме Win32 — `Compiling with Intel(R) Visual Fortran Compiler* [IA-32]...`.

Осталось научить **C++ Sources** искать `FORTRAN`’онские библиотеки (см. предыдущий пункт) в нужных местах. Настройка выполняется один раз для всех Visual C++ проектов.

Кликаем `View — Other Windows — Property Manager — C++ Sources — Debug | Win32 — Microsoft.Cpp.Win32.user`. Для настройки x64 выбираем `″ — Debug | x64 — ″` соответственно (Release–конфигурацию настраивать вручную не придётся).

Далее идём в `VC++ Directories`. В `Include Directories` добавляем `$(IFORT_COMPILER[VERSION])compiler\include` (юху, Intel и макросы сделали для пути!). Вместо **[VERSION]** поставьте версию, у меня на момент написания стоит **16**. 
Проверьте, что в `Evaluated value:` стоит что-то вменяемое типа `C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2016.3.207\windows\compiler\include` — скопируйте в проводник и откройте эту папку.

В `Library Directories` добавляем `$(IFORT_COMPILER[VERSION])compiler\lib\ia32` и `$(IFORT_COMPILER[VERSION])compiler\lib\intel64` для Win32 и x64 соответственно. 

Сбилдите **FORTRAN Static Lib**–проект и добавьте FORTRAN Static Lib.lib в **C++ Sources** (я предпочитаю вручную, но можно и в Project Properties подключить). Готово, можно компилировать.

Удобно, что исходники теперь аккуратно расфасованы по разным проектам и код можно редактировать в любимой IDE. Более того, дебаг работает привычным образом:

![Дебаг][VSDebug]

## Сноски

1. <span id="fn1">Yousef Saad, [*SPARSKIT: A Basic Toolkit for Sparse Matrix Computations*] (http://www-users.cs.umn.edu/~saad/software/SPARSKIT/index.html) [↩](#fnb1)</span>
2. <span id="fn2">Iain Duff, Roger Grimes, and John Lewis, [*User’s Guide for the Harwell–Boeing Sparse Matrix Collection*] (http://math.nist.gov/MatrixMarket/formats.html#hb) [↩](#fnb2)</span>
3. <span id="fn3">Почему-то в университетских курсах для инженеров вообще не рассказывают о том, что стандартные форматы для хранения матриц уже придумали.
В России это как-то не принято (по крайней мере, у нас в НГТУ) — быть может, из-за не любви к истории (в одном только названии «боинг» столько романтики!). 
В иностранных курсах такое я [встречал](http://ta.twi.tudelft.nl/nw/users/gijzen/CURSUS_DTU/HOMEPAGE/PhD_Course.html). Как следствие, студенты пишут свои велосипеды с нулём переносимости.
Но вас я от этого уберёг — добавьте [Matrix Market](http://math.nist.gov/MatrixMarket/) в закладки и живите спокойно. [↩](#fnb3)</span>

[ifortTerminal]: https://github.com/56th/CPP-Calls-FORTRAN-lib/blob/master/img/1.png "Терминал ifort"
[MSVCTerminal]: https://github.com/56th/CPP-Calls-FORTRAN-lib/blob/master/img/2.png "Терминал MSVC"
[VSDebug]: https://github.com/56th/CPP-Calls-FORTRAN-lib/blob/master/img/3.gif "Дебаг"
