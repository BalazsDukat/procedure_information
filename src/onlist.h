/*Takes a string and a vector of strings, returns true of the string is
already part of the vector, otherwise returns false.*/
bool onlist(string s, vector<string> VS)
{for(int i = 0; i < VS.size(); i++) if(s == VS[i]) return true;
return false;
}
