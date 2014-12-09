for i in {1..100}
do
  (java -jar tools/PlayGame.jar maps/map7.txt 1000 1000 log.txt "./QBot.bin $i" "java -jar example_bots/BullyBot.jar") > ./replays/$i.replay
  echo "Iteration $i"
  sleep 0.5
done
