for i in {1..2}
do
  java -jar tools/PlayGame.jar maps/map7.txt 1000 1000 log.txt "./QBot.bin $i" "java -jar example_bots/BullyBot.jar"
done
