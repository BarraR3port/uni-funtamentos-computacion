#! /bin/bash
javac -d target/classes src/main/java/cl/unab/fundamentos/*.java src/main/java/cl/unab/fundamentos/*/*.java
java -cp target/classes cl.unab.fundamentos.Main