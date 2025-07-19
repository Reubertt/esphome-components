#!/bin/bash

# Verifica se ao menos um arquivo foi passado
if [ "$#" -lt 1 ]; then
    echo "Uso: $0 arquivo1 [arquivo2 ...]"
    exit 1
fi

# Arquivo de saída
saida="saida_combinada.txt"
>"$saida" # Limpa o conteúdo anterior

# Loop pelos arquivos passados como argumento
for arquivo in "$@"; do
    if [ -f "$arquivo" ]; then
        echo "// $arquivo" >>"$saida"
        cat "$arquivo" >>"$saida"
        echo -e "\n" >>"$saida"
    else
        echo "Aviso: '$arquivo' não é um arquivo válido e será ignorado."
    fi
done

echo "Arquivos combinados em: $saida"
