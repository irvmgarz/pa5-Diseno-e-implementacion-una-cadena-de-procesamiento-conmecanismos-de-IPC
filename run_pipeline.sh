#!/bin/bash

# Ejecutar ./ex5 en segundo plano
./ex5 4 &
pid_ex5=$!

# Ejecutar ./sharpen en segundo plano
./sharpen &
pid_sharpen=$!

# Esperar a que ./ex5 y ./sharpen terminen
wait $pid_ex5
wait $pid_sharpen

# Una vez que ambos hayan terminado, ejecutar ./combine
./combine

echo "Proceso completado: Imágenes combinadas."
