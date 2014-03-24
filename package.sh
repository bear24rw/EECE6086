F=thrun_silbak_hw_2
rm -fr $F $F.zip
mkdir $F
cp *.{cpp,h} Makefile README report/*.pdf $F
mkdir $F/benchmarks
cp benchmarks/{1..10}.{log,mag} $F/benchmarks
7z a $F.zip $F
