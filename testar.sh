#!/bin/bash

make
rm sis*

# TESTE COM SISTEMA DE ARQUIVOS COM 10MB ------------------------------------------------------------

# apaga 30 niveis de hierarquia, 100 arquivos por diretorio
./ep3 < 30niveisCriaCheio.txt
start=$(date +%s%N)
./ep3 < 30niveisApagaCheio.txt
rm -rf l
rm sis*
end=$(date +%s%N)
echo "scale=3; ($end - $start) / 1000000000" | bc >> respostas.txt

# # apaga 30 niveis de hierarquia, 100 arquivos por diretorio 10MB
# ./ep3 < criaSistema10mb.txt
# ./ep3 < 30niveisCriaCheio.txt
# start=$(date +%s%N)
# ./ep3 < 30niveisApagaCheio.txt
# rm -rf l
# rm sis*
# end=$(date +%s%N)
# echo "scale=3; ($end - $start) / 1000000000" | bc >> respostas.txt

# # apaga 30 niveis de hierarquia, 100 arquivos por diretorio 45MB
# ./ep3 < criaSistema45mb.txt
# ./ep3 < 30niveisCriaCheio.txt
# start=$(date +%s%N)
# ./ep3 < 30niveisApagaCheio.txt
# rm -rf l
# rm sis*
# end=$(date +%s%N)
# echo "scale=3; ($end - $start) / 1000000000" | bc >> respostas.txt