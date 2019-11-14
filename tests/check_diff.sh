diff $1 $2
if [ $? -ne 0 ]; then
    echo "Files do not match";
fi
