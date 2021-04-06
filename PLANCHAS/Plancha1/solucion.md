# Práctica 1: Introducción a Nachos

**Respuestas**

1. Al ser un proyecto educativo simular el CPU permite simplificar el diseño e implementarlo sin depender del hardware y poder añadirle las funcionalidades o actualizarlos a versiones mas modernas para provecho del estudiante.



2. code/machine/disk.hh line:54

   ```c++
   const unsigned SECTOR_SIZE = 128;       ///< Number of bytes per disk sector.
   ```

   code/machine/mmu.hh line:17

   ```c++
   /// Definitions related to the size, and format of user memory.

   const unsigned PAGE_SIZE = SECTOR_SIZE;  ///< Set the page size equal to the
                                            ///< disk sector size, for
                                            ///< simplicity.
   const unsigned NUM_PHYS_PAGES = 32;
   const unsigned MEMORY_SIZE = NUM_PHYS_PAGES * PAGE_SIZE;
   ```

   Tamaño total de la memoria  es de 4096 bytes

3. Modificar la constante PAGE_SIZE o NUM_PHYS_PAGES  al ser Nachos un SO emulado

4. code/machine/disk.hh line:54

   ```c++
   const unsigned SECTOR_SIZE = 128;       ///< Number of bytes per disk sector.
   const unsigned SECTORS_PER_TRACK = 32;  ///< Number of sectors per disk
                                           ///< track.
   const unsigned NUM_TRACKS = 32;         ///< Number of tracks per disk.
   const unsigned NUM_SECTORS = SECTORS_PER_TRACK * NUM_TRACKS;
     ///< Total # of sectors per disk.
   ```

   code/machine/disk.cc line:26

   ```c++

   static const unsigned MAGIC_NUMBER = 0x456789AB;
   static const unsigned MAGIC_SIZE = sizeof (int);

   static const unsigned DISK_SIZE = MAGIC_SIZE + NUM_SECTORS * SECTOR_SIZE;

   ```

   DISK_SIZE = MAGIC_SIZE + NUM_SECTORS ·  SECTOR_SIZE
                      = sizeof (int) + (SECTORS_PER_TRACK · NUM_TRACKS)  SECTOR_SIZE
                      = sizeof (int) + (32 · 32) 128
                      = sizeof (int) + 131072 bytes

   5. En Machine::ExecInstruction() dentro de machine/mips_sim.cc, puede observarse que NachOS simula 59 instrucciones MIPS.

   6. usando la herramienta `grep` podemos listar donde se definen

   ```bash
   grep "main(" -rI .
   ```
   ```bash
   ./threads/main.cc:main(int argc, char **argv)
   ./bin/main.c:main(int argc, char *argv[])
   ./bin/disasm.c:main(int argc, char *argv[])
   ./bin/fuse/nachosfuse.c:main(int argc, char *argv[])
   ./bin/fuse/nachosfuse.c:    return fuse_main(argc, argv, &OPERATIONS, NULL);
   ./bin/coff2noff.c:main(int argc, char *argv[])
   ./bin/out.c:main(int argc, char *argv[])
   ./bin/readnoff.c:main(int argc, char *argv[])
   ./bin/coff2flat.c:main(int argc, char *argv[])
   ./userland/matmult.c:main(void)
   ./userland/shell.c:main(void)
   ./userland/tiny_shell.c:main(void)
   ./userland/filetest.c:main(void)
   ./userland/sort.c:main(void)
   ./userland/touch.c:main(int argc, char *argv[])
   ./userland/halt.c:main(void)
   ./userland/echo.c:main(int argc, char *argv[])
   ```


   7. A continuación se listan los métodos llamados en la funcion main del direcorio threads, con sus respectivos archivos fuente. Las funciones que se llamen mas de una vez solo aparecen la primera.

        Initialize lib/system.cc
          ASSERT lib/utility.hhf
          strcmp /usr/include/string.h
          RandomInit machine/system_dep.cc
          atoi /usr/include/stdlib.h
          atof /usr/include/stdlib.h
          SetFlags lib/debug.cc
          SetOpts lib/debug.cc
          Statistics machine/statistics.cc
          Interrupt machine/interrupt.cc
          Scheduler threads/scheduler.cc
          Timer machine/timer.cc
          Thread threads/thread.cc
          SetStatus threads.thread.cc
          Enable machine/interupt.cc
          CallOnUserAbort machine/system_dep.cc
          PreemptiveScheduler threads/preemptive.cc
          SetUp theads/preemptive.cc
          Machine machine/machine.cc
          SetExceptionHandlers userprog/exception.cc
          SynchDisk filesys/synch_disk.cc
          FileSystem filesystem/file_system.cc
          PostOffice network/post.cc
        DEBUG lib/utility.hh
        SysInfo threads/sys_info.cc
          printf /usr/include/stdio2.h
        PrintVersion main.cc
        ThreadTest threads/thread_test.cc
          Choose threads/thread_test.cc
          Run threads/thread_test.cc
        Halt machine/interrupt.cc
          Cleanup threads/system.cc
        StartProcess threads/main.cc
        ConsoleTest threads/main.cc
        Copy threads/main.cc
        Print threads/main.cc
        Remove filesys/file_system.cc
          Find filesys/directory.cc
          Bitmap lib/bitmap.cc
          Deallocate filesys/file_header.cc
          Clear lib/bitmap.cc
          Remove filesys/directory.cc
          WriteBack filesys/file_header.cc
        List filesys/file_system.cc
          List filesys/directory.cc
        Print filesys/file_system.cc
          Directory filesys/directory.cc
          FetchFrom filesys/file_header.cc
          Print filesys/file_header.cc
          FetchFrom lib/bitmap.cc
          Print lib/bitmap.cc
          FetchFrom filesys/directory.cc
          Print filesys/directory.cc
        Check filesys/file_system.cc
          GetRaw filesys/file_header.cc
          CheckForError filesys/file_cc.system
          CheckFileHeader filesys/file_cc.system
          CheckDirectory filesys/file_cc.system
          CheckBitmaps filesys/file_cc.system
        PerformanceTest threads/main.cc
        Delay machine/system_dep.cc
          sleep /usr/include/unistd.h
        MailTest threads/main.cc
        Finish threads/thread.cc
          SetLevel machine/interrupt.cc
          Sleep threads/thread.cc

   8. `DEBUG(char flag, const char *format, ...)` imprime un mensaje de depuración si la bandera `flag` que se especificó está activa. Es como `printf` con un argumento de más.
      Por otra parte `ASSERT(condition)` si la condición dada es falsa, imprime un mensaje y efectúa un volcado de memoria (core dump).

   9. Para tener una mejor organización a la hora de depurar el código, y no tener que ver mensajes de depuración que no corresponden a lo buscado, se implementó un sistema de códigos de banderas para filtrar mensajes por clases. Algunas banderas utilizadas son:

      - '+' -- muestra información relacionada a todas las banderas de depuración.
      - n (Network) -- imprime información con relación a redes.
      - d (Disk) -- imprime información de operaciones de disco.
      - i/x (Interrupts) -- muestra cuando se realizan interrupciones
      - m (Machine) -- operaciones relacionadas con la máquina.
      - a (Address translation) -- operaciones relacionadas con espacios de memoria.
      - f (Files) -- operaciones relacionadas con el file system.
      - t (Threads) -- operaciones relacionadas con los hilos de ejecución.
      - p (Preemptive scheduler) -- operaciones asociadas con el scheduler
      - e (User programs arguments) -- argumentos relacionados con programas del usario

   10. Están definidas en los Makefiles correspondientes

   11. Nachos soporta la siguiente linea de comandos:

       ~~~
       nachos [-d <debugflags>] [-do <debugopts>] [-p]
          [-rs <random seed #>] [-z] [-tt]
          [-s] [-x <nachos file>] [-tc <consoleIn> <consoleOut>]
          [-f] [-cp <unix file> <nachos file>] [-pr <nachos file>]
          [-rm <nachos file>] [-ls] [-D] [-c] [-tf]
          [-n <network reliability>] [-id <machine id>]
          [-tn <other machine id>]
       ~~~

       La opcion **-rs** permite realizar yields de forma azarosa pero repetidamente usando como semilla el numero que toma al ejecutarse.

   12. En código

   13. Básicamente una `SynchList` contiene una instancia de una `List` con la diferencia que `SynchList` es thread safe (bloquea a los threads cuando realiza operaciones).

   14.  en thread_test_simple.cc

   15. en thread_test_simple.cc

   16. en thread_test_simple.cc

   17. Se intercambiaron las siguientes lineas de threads/thread_test_garden.cc

       ```c++
           count = temp + 1;
           currentThread->Yield();
       ```

   18. Implentacion del archivo: threads/thread_test_garden_semaphore.cc
