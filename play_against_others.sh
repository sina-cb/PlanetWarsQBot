map_name=map07
java -jar tools/PlayGame.jar maps/$map_name.txt 1000 1000 log.txt "./QBot.bin 1 $map_name" "java -jar example_bots/BullyBot.jar" | java -jar tools/ShowGame.jar
