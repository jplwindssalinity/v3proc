#include "NC2Options.h"

using namespace std;

void Options2NC(NcFile& ncFile, Options& opt) {
  
  // Get an iterator through the options keywords

  list<string>::iterator key;
  for ( key = opt.keys.begin() ; key != opt.keys.end(); key++ ) {
    string value = opt[*key];
    ncFile.add_att(key->c_str(),value.c_str());
  }

}


void Options2NC(NcFile& ncFile, Options& opt, const char* var_name) {

  // Get the variable

  NcVar *var = ncFile.get_var(var_name);
  
  // Get an iterator through the options keywords

  list<string>::iterator key;
  for ( key = opt.keys.begin() ; key != opt.keys.end(); key++ ) {
    string value = opt[*key];
    var->add_att(key->c_str(),value.c_str());
  }

}

Options NC2Options(NcFile& ncFile) {
  Options opt;
  
  for(int i = 0; i < ncFile.num_atts(); i++) {
    NcAtt *att = ncFile.get_att(i);
    NcType type = att->type();
    
    string key(att->name());
    string value;
    int nvalues = att->num_vals();
    for(int j = 0; j < nvalues; j++) {
      value += string( att->as_string(j) );
      if( type != ncChar ) value += string(","); // CSV format
    }
    opt.add(key,value);
  }

  return opt;
}
