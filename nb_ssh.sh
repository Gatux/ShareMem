while true
do
  echo "Nombre de ssh :" $(($(ps auxf | grep "ssh " --color | wc -l) - 1))
  sleep 1
done
