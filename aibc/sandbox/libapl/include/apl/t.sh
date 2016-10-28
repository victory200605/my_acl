
FILENAME=`echo $1|tr [a-z]. [A-Z]_`

touch $1

echo "#ifndef APL__"$FILENAME >> $1
echo "#define APL__"$FILENAME >> $1
echo >> $1
echo >> $1
echo >> $1
echo "#endif // APL__"$FILENAME >> $1
echo >> $1

