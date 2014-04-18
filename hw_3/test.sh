RED="\033[01;31m"
YELLOW="\033[01;33m"
GREEN="\033[01;32m"
BLUE="\033[01;34m"
WHITE="\033[00m"
GRAY="\033[01;30m"

num_tests=$(ls benchmarks/*.txt | wc -l)
i=0

for cover in `ls benchmarks/*.txt | sort -V`
do
    i=$((i+1))
    name=`echo $cover | sed 's/benchmarks\///' | sed 's/\.txt//'`
    echo -e $BOLD$(python -c "print '$name'.center(70).replace(' ','=')+' [$i/$num_tests]'") $WHITE

    ./tc $cover 2> /dev/null
    if [[ $? == 0 ]] ; then
        echo -e $GREEN"Is tautology" $WHITE
    else
        echo -e $GRAY"NOT a tuatology, finding complement..."
        ./cc $cover > /tmp/cc.txt
        echo -e $GRAY"Found " `wc -l /tmp/cc.txt` " complements"
        echo -e $GRAY"Adding complement to original cover..."
        head -n1 $cover > /tmp/new_cover.txt
        num_cubes=`cat $cover /tmp/cc.txt | wc -l`
        num_cubes=`python -c "print ${num_cubes}-2"`
        echo $num_cubes >> /tmp/new_cover.txt
        tail -n+3 $cover >> /tmp/new_cover.txt
        cat /tmp/cc.txt >> /tmp/new_cover.txt
        echo -e $GRAY"Running tc against new cover"
        ./tc /tmp/new_cover.txt 2> /dev/null
        if [[ $? == 0 ]] ; then
            echo -e $GREEN"Complement is correct!" $WHITE
        else
            echo -e $RED"ERROR complement is wrong" $WHITE
        fi
    fi
done
