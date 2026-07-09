# Cronos

um simples programa para marcar o tempo enquanto ouve musicas.

compilação:

```bash
cc -o nob nob.c # apenas 1 vez!

./nob # para compilar o cronos (linux por padrão)

./build/cronos # para usar o cronos!
```

flags:

```bash
./nob -help                        # mostra as opções disponiveis
./nob -list                        # lista os targets disponiveis
./nob -target windows              # compila para windows
./nob -target release              # gera os releases (linux + windows)
./nob -target release -only linux  # gera o release só de linux
```
