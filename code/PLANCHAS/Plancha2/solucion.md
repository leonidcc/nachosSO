# Práctica 2: Sincronización de hilos

**Respuestas**
1. Modifique la implementaci´on para solucionar o evitar en el caso de los “locks” y variables de condici´on el problema de inversi´on de prioridades.
Explique (en un archivo de texto) por qu´e no puede hacerse lo mismo con los sem´aforos.

* Un proceso A de baja prioridad hace una llamada al sistema, y es interrumpido a la mitad de dicha llamada.

* Un proceso B de prioridad tiempo real hace una segunda llamada al sistema, que requiere de la misma estructura que la que tiene bloqueada el proceso A.

Al presentarse esta situación, B se quedará esperando hasta que A pueda ser nuevamente agendado —esto es, un proceso de alta prioridad no
podrá avanzar hasta que uno de baja prioridad libere su recurso.

 **Solución**
 Todos los procesos que estén accesando (y, por
tanto, bloqueando) recursos requeridos por un proceso de mayor prioridad, serán tratados como si fueran de la prioridad de dicho recurso hasta
que terminen de utilizar el recurso en cuestión, tras lo cual volverán a su prioridad nativa.

__extracto del libro FUNDAMENTOS DE
SISTEMAS OPERATIVOS Gunnar Wolf
Esteban Ruiz
Federico Bergero
Erwin Meza__

En  los  locks  y variables de condición, es posible saber quien  toma y libera un recurso por lo tanto podemos modificar su prioridad, con los semáforos  no podemos saber quien libera el recurso, puesto que puede ser liberado por cualquiera que invoque V(), incluso si este no invoco a P(), por ésa razón no podemos saber a quien tenemos que modificar la prioridad.
