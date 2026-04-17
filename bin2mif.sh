#!/bin/bash

echo 'WIDTH = 128;' > init.mif
echo 'DEPTH = 4096;' >> init.mif
echo 'ADDRESS_RADIX = HEX;' >> init.mif
echo 'DATA_RADIX = HEX;' >> init.mif
echo 'CONTENT BEGIN' >> init.mif

hexdump -C init.bin>hexdump_output
count=0
while read -r line
do
    let count++
    cutline=${line:0:60}
    readline=`echo "$cutline"|sed -e 's/  / /g'`
    read1=`echo "$readline"|awk '{print $1}'`
    read2=`echo "$readline"|awk '{print $2}'`
    #echo $readline
    if [[ "x""$read1""x" != "x*x" && "x""$read1""x" != "xx" && "x""$read2""x" != "xx" ]] then
        i=2
        j=15
        for str in 0 1 2 3 4 5 6 7 8 9 a b c d e f
        do
            read_byte=`echo "$readline"|awk -v col=$i '{print $col}'`
            if [[ "x""$read_byte""x" == "xx" ]] then
                read_byte="00"
            fi
            # 从左开始，从0开始，切割7个字符
            # echo "${read1:0:7}${str} : ${read_byte};" >> init.mif

            array_[j]=${read_byte}

            let j--
            let i++
        done
        tmp_str=""
        for x in {0..15}; do
            tmp_str=${tmp_str}${array_[$x]}
        done
        #echo $tmp_str
        echo "${read1:0:7} : ${tmp_str};" >> init.mif
    elif [[ "x""$read1""x" == "x*x" ]] then
        line_m1=`expr $count - 1`
        # get previous line addr
        line_m1=`sed -n "${line_m1}p" hexdump_output|awk '{print $1}'`
        # 16->10
        let line_m1=16#$line_m1
        line_m1=`expr $line_m1 + 16`
        # 10->16
        line_m1=$(echo "obase=16;$line_m1"|bc)
        # 去掉结尾的0 
        line_m1=${line_m1%?}

        line_p1=`expr $count + 1`
        line_p1=`sed -n "${line_p1}p" hexdump_output|awk '{print $1}'`
        let line_p1=16#$line_p1
        line_p1=`expr $line_p1 - 16`
        line_p1=$(echo "obase=16;$line_p1"|bc)
        line_p1=${line_p1%?}

        if [[ $line_m1 == $line_p1 ]] then
            echo "$line_m1 : 0;" >> init.mif
        else
            echo "[$line_m1..$line_p1] : 0;" >> init.mif
        fi

    elif [[ "x""$read1""x" != "xx" && "x""$read2""x" == "xx" ]] then
        line_m1=`expr $count - 1`
        line_m1=`sed -n "${line_m1}p" hexdump_output|awk '{print $1}'`
        let line_m1=16#$line_m1
        line_m1=`expr $line_m1 + 16`
        line_m1=$(echo "obase=16;$line_m1"|bc)
        line_m1=${line_m1%?}

        echo "[$line_m1..FFF] : 0;" >> init.mif
    fi
done < hexdump_output
echo 'END;' >> init.mif
exit 0
