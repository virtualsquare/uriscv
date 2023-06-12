### Exceptions

ABI
gestione da emulatore
LEXCHanldler
Gestione interna



LDST
-> riscrive tutte i registri
-> riscrivere quindi pure `ra`
-> ovvero la funzioni dopo non sanno dove ritornare ??? va bene ???

Da modificare exec.S 
- aggiungere controlli di status e modificarlo


Gestire EWFI


https://stackoverflow.com/questions/61913210/risc-v-interrupt-handling-flow


CAUSE diventa MIP -> lo schema utilizzato per MIP al momento e' diverso
dal manuale originale per semplicita' mantengo quella di uMIPS


riguardare mtvec -> sembra abbia il primo bit per decidere se le eccezioni
sono gestite tutte insieme o vettorizzate


gestire meglio pc,nextpc,succpc

capire privilege mode register chi e', per ripristinare 

non uso 34-bit come da manuale ma solo 32-bit (no zero-extended)

TLB-refill -> chiamato quando non si trova matching in TLB


Quello che succede

- si lancia il test classico
- viene creato il processo 1
- esegue qualche istruzione
- viene creato il processo 2
- esegue qualche istruzione
- processo 1 ha il controllo
- memoria virtuale KUSEG
- Refill -> pager -> etc ...
- Ritorna a lui, ora puo' proseguire



il problema e' che per qualche motivo quando arrivo un nuovo proc il vecchio non riceve
l'interrupt con ack e quindi non viene sbloccato dalla sua coda, probabile sia 
dovuto al fatto che il MIE sia impostato male da qualche parte
