# Prova finale di Algoritmi e Principi dell'Informatica - A.A. 2022/2023

- Scadenza: 2023-09-09.
- Voto: 30/30.

## Obbiettivo

L'obbiettivo di questo progetto è l'implementazione di un programma che, data un'autostrada con delle stazioni di servizio, dotate di veicoli elettrici a noleggio, sia in grado di pianificare il percorso con il minor numero di tappe tra due stazioni date, seguendo dei criteri di ottimo.

## Specifiche

L'autostrada è descritta come una sequenza di stazioni di servizio, ognuna identificata dalla sua distanza dall'inizio dell'autostrada, espressa in chilometri (da un numero intero positivo o nullo).

Ogni stazione di servizio ha un parco veicoli elettrici a noleggio, ognuno contraddistinto dall'autonomia, espressa in chilometri da un numero intero positivo. Il parco veicoli di una singola stazione comprende al più 512 veicoli. Presa a noleggio un'auto da una stazione $s$, è possibile raggiungere tutte quelle la cui distanza da $s$ è minore o uguale all'autonomia dell'automobile.

Un viaggio è identificato da una sequenza di stazioni di servizio in cui il conducente fa tappa. Ha quindi inizio in una stazione di servizio e termina in un'altra, passando per zero o più stazioni intermedie. Si assume che il conducente non possa tornare indietro durante il viaggio, e noleggi un'auto nuova ogniqualvolta faccia tappa in una stazione di servizio. Quindi, date due tappe consecutive $s$ e $t$, $t$ deve essere sempre più lontana dalla partenza rispetto a $s$, e $t$ deve essere raggiungibile usando uno dei veicoli disponibili in $s$. Il programma deve essere in grado di pianificare il percorso con il minor numero di tappe tra due stazioni date. Nel caso siano presenti più percorsi con lo stesso numero minimo di tappe (cioè a pari merito), deve essere scelto il percorso che predilige le tappe con distanza più breve dall'inizio dell'autostrada.

## I/O

L'ingresso, fornito mediante `stdin`, contiene una sequenza di comandi, uno per riga, con il seguente formato:

- `aggiungi-stazione distanza numero-auto autonomia-auto-1 ... autonomia-auto-n` Aggiunge una stazione, posta alla distanza indicata, dotata di numero-auto, dall'autonomia indicata. Se esiste già una stazione alla distanza indicata, il comando non fa nulla.
- `demolisci-stazione distanza` Rimuove la stazione posta alla distanza indicata, se essa esiste.
- `aggiungi-auto distanza-stazione autonomia-auto-da-aggiungere` Se la stazione esiste, aggiunge un'auto alla stessa. È possibile avere più auto con la stessa autonomia.
- `rottama-auto distanza-stazione autonomia-auto-da-rottamare` Rimuove un'automobile dalla stazione indicata, se la stazione esiste ed è dotata di almeno un'automobile con l'autonomia indicata.
- `pianifica-percorso distanza-stazione-partenza distanza-stazione-arrivo` Richiede di pianificare il percorso con i vincoli sopra indicati. Nell'output devono essere incluse partenza e arrivo; se coincidono la stazione viene stampata una sola volta. Se il percorso non esiste, stampa nessun percorso. L'azione di pianificazione non altera le stazioni o il loro parco veicoli.

### Esempio di esecuzione

Dato il seguente caso di test:

```text
aggiungi-stazione 91 9 5 4 5 5 4 6 5 4 5
aggiungi-stazione 24 6 5 4 5 4 4 5
pianifica-percorso 24 91
aggiungi-stazione 92 6 5 4 4 5 5 5
demolisci-stazione 59
aggiungi-stazione 52 9 13 14 13 17 17 15 18 15 16
aggiungi-auto 91 39
aggiungi-auto 52 39
aggiungi-auto 24 28
pianifica-percorso 24 92
```

L'output atteso è il seguente:

```text
aggiunta
aggiunta
nessun percorso
aggiunta
non demolita
aggiunta
aggiunta
aggiunta
aggiunta
24 52 91 92
```

