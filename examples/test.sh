

for d in `ls .`; do
  if [[ -d ${d} ]]; then
    echo -n "${d} : "
    cd $d && make clean all &> /dev/null
    echo $?
    cd ..
  fi
done
