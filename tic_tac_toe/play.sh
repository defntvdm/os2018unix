#!/bin/bash

clear

fifo1=/tmp/my_pipe_1
fifo2=/tmp/my_pipe_2
my_pid=$$

if [ ! -e $fifo1 ] || [ ! -e $fifo2 ]; then
    rm $fifo1 $fifo2 >/dev/null 2>&1
    mknod $fifo1 p
    mknod $fifo2 p
    ME=1
    my_step=1
    r=$fifo1
    w=$fifo2
    echo You are tic \(X\)
else
    ME=2
    my_step=0
    r=$fifo2
    w=$fifo1
    echo You are tac \(O\)
fi

trap "echo quit > $w && rm -f $w >/dev/null 2>&1" EXIT
echo Use your Num to play \(the same location\)
echo "Connected" > $w &
cat < $r

MAP=( "." "." "." "." "." "." "." "." "." )

find_looser ()
{
    if [ "$1" == "X" ]; then
        if [ $ME == 1 ]; then
            echo -e "\033[1;32mYOU WIN :-)\033[0m"
        else
            echo -e "\033[1;31mYOU LOSE :-(\033[0m"
        fi
    else
        if [ $ME == 1 ]; then
            echo -e "\033[1;31mYOU LOSE :-(\033[0m"
        else
            echo -e "\033[1;32mYOU WIN :-)\033[0m"
        fi
    fi
    rm $r $w >/dev/null 2>&1
    exit 0
}

draw_field ()
{

    echo -e " ${MAP[6]} ┃ ${MAP[7]} ┃ ${MAP[8]}"
    echo -e "━━━╋━━━╋━━━"
    echo -e " ${MAP[3]} ┃ ${MAP[4]} ┃ ${MAP[5]}"
    echo -e "━━━╋━━━╋━━━"
    echo -e " ${MAP[0]} ┃ ${MAP[1]} ┃ ${MAP[2]}"
    # проверочка на проигрыш)
    if [ ${MAP[0]} == ${MAP[3]} ] && [ ${MAP[3]} == ${MAP[6]} ] && [ ! "${MAP[0]}" == "." ]; then
        find_looser ${MAP[0]}
    fi
    if [ ${MAP[0]} == ${MAP[1]} ] && [ ${MAP[1]} == ${MAP[2]} ] && [ ! "${MAP[0]}" == "." ]; then
        find_looser ${MAP[0]}
    fi
    if [ ${MAP[0]} == ${MAP[4]} ] && [ ${MAP[4]} == ${MAP[8]} ] && [ ! "${MAP[0]}" == "." ]; then
        find_looser ${MAP[0]}
    fi
    if [ ${MAP[6]} == ${MAP[7]} ] && [ ${MAP[7]} == ${MAP[8]} ] && [ ! "${MAP[6]}" == "." ]; then
        find_looser ${MAP[6]}
    fi
    if [ ${MAP[6]} == ${MAP[4]} ] && [ ${MAP[4]} == ${MAP[2]} ] && [ ! "${MAP[2]}" == "." ]; then
        find_looser ${MAP[2]}
    fi
    if [ ${MAP[1]} == ${MAP[4]} ] && [ ${MAP[4]} == ${MAP[7]} ] && [ ! "${MAP[1]}" == "." ]; then
        find_looser ${MAP[1]}
    fi
    if [ ${MAP[2]} == ${MAP[5]} ] && [ ${MAP[5]} == ${MAP[8]} ] && [ ! "${MAP[2]}" == "." ]; then
        find_looser ${MAP[2]}
    fi
    if [ ${MAP[3]} == ${MAP[4]} ] && [ ${MAP[3]} == ${MAP[5]} ] && [ ! "${MAP[3]}" == "." ]; then
        find_looser ${MAP[3]}
    fi
    for cell in ${MAP[*]}; do
        if [ "$cell" == "." ]; then
            return 0
        fi
    done
    echo -e "\033[1;34mНичья :-)\033[0m"
    rm -f $w >/dev/null 2>&1
    exit 0
}

read_map ()
{
    inp=$(cat $r)
    if [ "$inp" == "quit" ]; then
        echo -e "\033[1;34mПротивник отключился\033[0m"
        rm -f $w >/dev/null 2>&1
        exit 0
    fi
    i=0
    for word in $inp
    do
        MAP[$i]=$word
        i=$(($i+1))
    done
}

not_correct_input ()
{
    if [[ $inp =~ ^[1-9]$ ]]; then
        if [ "${MAP[$inp-1]}" == "." ]; then
            return 1
        fi
    fi
    return 0
}

while [ 1 ]; do
    if [ $my_step == 1 ]; then
        draw_field
        read -p "Ваш ход (от 1 до 9): " inp
        if not_correct_input; then
            clear
            echo Bad step
        else
            if [ $ME == 1 ]; then
                MAP[$inp-1]="X"
            else
                MAP[$inp-1]="O"
            fi
            echo ${MAP[*]} >$w &
            my_step=0
            clear
        fi
    else
        draw_field
        echo "Ждём ход противника"
        read_map
        my_step=1
        clear
    fi
done

