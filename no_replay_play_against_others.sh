for i in {1..200}
do
  (java -jar tools/PlayGame.jar maps/map7.txt 1000 1000 log.txt "./QBot.bin $i" "java -jar example_bots/BullyBot.jar") > ./replays/$i.replay
  echo "Iteration $i"

  grep 1 map07-central.q >> ~/Desktop/results/$i.res

  sleep 1
done
