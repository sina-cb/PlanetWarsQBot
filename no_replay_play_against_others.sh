map_name=map07

for i in {1..20000}
do
  (java -jar tools/PlayGame.jar maps/$map_name.txt 1000 1000 log.txt "./QBot.bin $i $map_name" "java -jar example_bots/BullyBot.jar") > /dev/null
  echo "Episode $i"

#  grep 1 map07-central.q >> ~/Desktop/results/$i.res

  sleep 1
done
