/*
 * Logger.h
 *
 *  Created on: Nov 17, 2014
 *      Author: sina
 */


#ifndef LOGGER_CPP_
#define LOGGER_CPP_

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

class Logger{

public:

	ofstream myfile;
	char buffer [500];

	Logger(){
		myfile.open("log.log");
	}

	Logger(char* filename){
		myfile.open(filename);
	}

	~Logger(){
		myfile.close();
	}

	void log(){
		myfile << buffer << endl;
	}

};

#endif /* LOGGER_CPP_ */
