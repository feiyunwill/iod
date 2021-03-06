#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

std::string symbol_definition(std::string s);

// Iod symbols generator.
//
//    For each variable name starting with underscore, generates a symbol
//    definition.
//
int main(int argc, char* argv[])
{
  using namespace std;

  if (argc < 3)
  {
    cout << "Usage: " << argv[0] << " input_cpp_file1, ..., input_cpp_fileN, output_cpp_header_file" << endl;
    return 1;
  }
  
  std::set<string> symbols;
  std::regex symbol_regex(".?_([[:alnum:]_]+)");
  std::set<std::string> keywords = {"alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class", "compl", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"};
  
  auto parse_file = [&] (std::string filename) {
    
    ifstream f(filename);
    if (!f)
    {
      std::cerr << "Cannot open file " << argv[1] << " for reading." << std::endl;
    }
  
    std::string line;
    bool in_raw_string = false;
    while (!f.eof())
    {
      getline(f, line);

      std::vector<int> dbl_quotes_pos;
      bool escaped = false;
      for (int i = 0; i < line.size(); i++)
      {
        if (line[i] == '"' and !escaped) dbl_quotes_pos.push_back(i);
        else if (line[i] == '\\') escaped = !escaped;
        else escaped = false;
      }

      auto is_in_string = [&] (int p) {
        int i = 0;
        while (i < dbl_quotes_pos.size() and dbl_quotes_pos[i] <= p) i++;
        return i % 2;
      };

      std::string::const_iterator start, end;
      start = line.begin();
      end = line.end();
      std::match_results<std::string::const_iterator> what;
      std::regex_constants::match_flag_type flags = std::regex_constants::match_default;
      while(regex_search(start, end, what, symbol_regex, flags))
      {
        std::string m = what[0];
        std::string s = what[1];

        bool is_type = s.size() >= 2 and s[s.size() - 2] == '_' and s[s.size() - 1] == 't';
        
        if (!std::isalnum(m[0]) and !is_in_string(static_cast<int>(what.position())) and
            !is_type and keywords.find(s) == keywords.end())
          symbols.insert(what[1]);
        start = what[0].second;      
      }

    }
  };

  for (int i = 1; i < argc - 1; i++)
    parse_file(argv[i]);
  
  std::ofstream os(argv[argc - 1]);
  if (!os)
  {
    std::cerr << "Cannot open file " << argv[2] << " for writing." << std::endl;
    return 2;
  }

  os << "// Generated by iod_generate_symbols."  << endl;
  std::stringstream symbols_content;
  os << "#include <iod/symbol.hh>" << endl;
  for (string s : symbols)
  {
    os << symbol_definition(s) << endl;
  }
}

std::string symbol_definition(std::string s)
{
  std::string body;
  if (std::isdigit(s[0]))
  {
    body = R"cpp(#ifndef IOD_SYMBOL___S__
#define IOD_SYMBOL___S__
    iod_define_number_symbol(__S__)
#endif
)cpp";
    // Check the string is actually a number.
    for (int i = 0; i < s.size(); i++)
      if (!std::isdigit(s[i])) return "";
  }
  else
    body = R"cpp(#ifndef IOD_SYMBOL___S__
#define IOD_SYMBOL___S__
    iod_define_symbol(__S__)
#endif
)cpp";

  std::regex s_regex("__S__");
  body = std::regex_replace(body, s_regex, s);
  return body;
}
