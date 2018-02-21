/*The general logic is to look for keywords, get the character(s) or string(s)
until a specified delimiter (which may be just the end of the line), save the
found parts, possibly run a comparison/check on it to validate it, then
recall it when writing output, adding the specific markdown-code around it.*/
/*----------------------------------------------------------------------------*/
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <conio.h>
#include <stdlib.h>

using namespace std;

#include "onlist.h"
/*This is a newer version of getname, eof() restriction removed.*/
string getname(istringstream& inSS, ifstream& inF)
{char c;
string line, temp = "";
while(temp == "" /*&& !inSS.eof()*/)
/*let's eat up leading spaces 1st:*/
{while(inSS >> noskipws >> c && isspace(c));
// just keep going until non-space found or no more characters on the line.
if(!isspace(c) && c != '\n' && c != '\0' && c != '#' && c != '-' && c != '/')
//if we arrived here because we found non-space:
{inSS.unget();
while(inSS >> noskipws >> c && !isspace(c) && c != ';' && c != '#' && c != '\0'
&& c != '(')
if(isalnum(c) || c == '_') temp = temp + c; //else ; // nothing
}
if(temp == "")
{if(getline(inF,line))
{inSS.clear();
inSS.str(line);
}
else break; //couldn't get new line, so just get out of here.
}
}
return temp;
}

string get_error_number(istringstream& inSS)
//this looks for the error number on the same line only.
{char c;
int counter = 0; // I could possibly reuse this for the output.
while(inSS >> noskipws >> c && counter < 2) if(c == ',') counter++;
//2 because it is the 3rd parameter we want, that's the error code.
string temp;
while(inSS >> c && c != ',') temp = temp + c; //cout << temp;
if(atoi(temp.c_str())) return temp;
//If it can be converted into a number, then return it, else return empty.
return "";/*this is why the output cannot be int; we have to have the option to
output an empty output.*/
}

string get_rest_of_line(istringstream& inSS)
{char c;
string temp;
while(inSS >> noskipws >> c) temp = temp + c;
return temp;
}

string get_all(istringstream& inSS, ifstream& inF, const char end)
{char c;
string line, temp = "";
while(c != end)
{while(inSS >> noskipws >> c && c != end) temp += c;
if(inSS.eof())
	if(getline(inF,line))
	{inSS.clear();
	inSS.str(line);
	temp += '\n';
	}//else c = end; //this is a signal to finish
}return temp;
}

int main()
{DIR *pDIR;
struct dirent *entry;
string line, temp, input_filename, output_filename, name, in_out, type;
const string folder = "./analyse/", outfolder = "./md/";
char c;
/*The output will be a different .md file for every single .sql file.*/
if(pDIR = opendir(folder.c_str()))
{cout << "Looking for files in folder" << endl;
while(entry = readdir(pDIR))
{if(strcmp(entry -> d_name, ".") != 0 && strcmp(entry -> d_name,"..") != 0)
input_filename = folder + entry -> d_name;
temp = outfolder + entry -> d_name;
output_filename = temp.substr(0, temp.length()-3) + "md";
ofstream output(output_filename.c_str()); //filename generated from input
ifstream inF;
inF.open(input_filename.c_str());
temp = "";
string procedure, category, description, additional_part;
vector< vector<string> > parameters;
vector<string> calls, reads, updates, error_codes;
bool parameters_done = false;
while(getline(inF,line)) //take a line
{istringstream inSS(line); //convert it into input for strings
	while(inSS >> temp) //take this in single characters? using getname?
	{if(procedure == "") //look for the procedure name first
		{if(description == "" && temp == "#") description = get_rest_of_line(inSS);
		else if(category == "" && temp == "##")	category = get_rest_of_line(inSS);
		else if(additional_part == "" && temp == "/*")
		additional_part = get_all(inSS, inF, '*');
		else if(temp == "PROCEDURE" || temp == "FUNCTION" || temp == "procedure" ||
		temp == "function")
		procedure = getname(inSS, inF);
		}
	else if(temp == "CALL")
	{temp = getname(inSS, inF);
	if(temp != "" && !onlist(temp,calls)) calls.push_back(temp);
	if(temp == "__p_error_handler")
	{temp = get_error_number(inSS);
	if(temp != "") error_codes.push_back(temp);
	}
	}
	// onlist() checks if already on list.
	else if(temp == "FROM" || temp == "JOIN" || temp == "from" || temp == "join")
	{temp = getname(inSS, inF);
 	if(temp != "" && !onlist(temp,reads)) reads.push_back(temp);
	}
	else if(temp == "UPDATE" || temp == "INTO" || temp == "update" ||
	temp == "into")
	{temp = getname(inSS, inF);
	if(temp != "" && !onlist(temp,updates) && temp.substr(0,2) != "v_")
	/*Filtering out variable names*/
	updates.push_back(temp);
	}
	else if(temp == "BEGIN" || temp == "begin") parameters_done = true;
	else if(temp.substr(0,1) == "#" || temp.substr(0,1) == "-" ||
	temp.substr(0,1) == "/") break;
	//skip the rest of the line by escaping the while loop that reads it.
	else //parameters here
		{if(!parameters_done && (temp == "IN" || temp == "OUT" || temp == "in" ||
		temp == "out"))
		 	{in_out = temp;
		 	//names are a special case, '`' has to be filtered out with this:
		 	while(inSS >> noskipws >> c && isspace(c));
		 	inSS.unget();
		 	while(inSS >> noskipws >> c && !isspace(c))
				if(isalnum(c) || c == '_') name = name + c;
		  while(inSS >> noskipws >> c && isspace(c)); //eat up whitespaces
		  /*For some reason this is needed to tackle double spaces.*/
		  inSS.unget();
		 	inSS >> type;
			vector<string> p_list_entry;
		 	p_list_entry.push_back(name);
		 	p_list_entry.push_back(in_out);
		 	p_list_entry.push_back(type);
		 	parameters.push_back(p_list_entry);
		 	name = ""; //reset, because this is taken character by character
			}
		}
	}
temp = "";
}
inF.close();
/*Writing output from here:*/
if(category != "") output << "#### " << category << endl << endl;
if(description != "") output << "#### " << description << endl << endl;

output << "##### **" + procedure + "**(*";
for(int i = 0; i < parameters.size(); i++)
{output << parameters[i][0];
if(i < parameters.size() - 1) output << ", ";
/*the compiler decides that i is increased only at the end, no matter what.*/
}
output << "*)" << endl << endl;

output << "#### Depends on:" << endl;
for(int i = 0; i < calls.size(); i++)
{output << "- [" << calls[i] << "](" << calls[i] << ".md)" << endl;
}

output << endl << endl << "Parameter | In/Out | Type | Description" << endl
<< "--- |--- |--- |---" << endl;
for(int i = 0; i < parameters.size(); i++)
output << "```" << parameters[i][0] << "```|" << parameters[i][1] << "|"
<< parameters[i][2] << "|" << endl;

output << endl << endl << "~~~sql" << endl << "CALL " << procedure << "(";

for(int i = 0; i < parameters.size(); i++)
{output << parameters[i][0];
if(i < parameters.size() - 1) output << ", ";
/*the compiler decides that i is increased only at the end of the loop.*/
}
output << ")" << endl << "~~~" << endl;

output << additional_part << endl << endl;

if(reads.size() > 0)
{output << "#### Reads from:" << endl;
for(int i = 0; i < reads.size(); i++) output << "- " << reads[i] << endl;
output << endl << endl;
}

if(updates.size() > 0)
{output << "#### Inserts into or updates:" << endl;
for(int i = 0; i < updates.size(); i++) output << "- " << updates[i] << endl;
output << endl << endl;
}

if(error_codes.size() > 0)
{output << "#### Error codes that can be thrown by this:" << endl;
for(int i = 0; i < error_codes.size(); i++)
	{output << error_codes[i];
	if(i < error_codes.size() - 1) output << ", ";
	}
}

output.close();
}
closedir(pDIR);
}else cerr << "Couldn't open the folder." << endl;
cout << endl << "Extraction finished!" << endl << "Press any key to quit.";
c = getch();
}
