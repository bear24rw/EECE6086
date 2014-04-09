> cheat_vs_original.csv
for f in original/R_*
do
    name=$(basename $f)
    nets=$(echo $name | cut -d"_" -f2)
    new=$(head -n 1 cheat/$name)
    old=$(head -n 1 original/$name)
    if [[ $new != $old ]]
    then
        echo "$nets $old $new" >> cheat_vs_original.csv
    fi
done
