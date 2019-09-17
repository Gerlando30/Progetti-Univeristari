#file chatty.c  
 #    author Gramaglia Gerlando 530269
 #     Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
 #     originale dell'autore  
 
#!/bin/bash

if [ $# != 2 -o "$1" == "-help" ]; then
    echo "passa due argomenti: 
			file di configurazione
			time"
    exit 1
fi

if [ ! -f $1 ]; then
    echo "impossibile aprire il file $1"
    exit 1
fi

if [ ! -s $1 ]; then
    echo "il file $1 e' vuoto"
    exit 1
fi

exec 3<$1   # apro il file il lettura

#leggo ogni linea del file
while read -u 3 linea; do
	read -r -a elem <<< "$linea"
	if [ "${elem[0]}" == "DirName" ]; then
		dirName=${elem[2]}
	fi
done

cd $dirName
#se ho indicato come tempo 0 stampo in output i file presenti all'interno della cartella
if [ $2 == 0 ]; then
    ls
    exit 1
fi

#archivio i file  
tar -zcvf chatty.tar.gz $(find $dirName -mmin +$2)

#elimino il tar
rm -i -R chatty.tar.gz



