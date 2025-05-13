#! /bin/bash
echo "Compilando..."
javac -d target/classes src/main/java/cl/unab/fundamentos/*.java src/main/java/cl/unab/fundamentos/*/*.java
echo "Ejecutando..."
java -cp target/classes cl.unab.fundamentos.Main