#include "JsonArrayStream.h"

JSONArrayStream::JSONArrayStream() 
    : isFirstElement(true) { 
        this->stream << "["; 
};

JSONArrayStream& operator<<(JSONArrayStream& s, const std::string& element) {
  if(s.isFirstElement) {
    s.isFirstElement = false;
  }
  else {
    s.stream << ",";
  }
  s.stream << element;
  return s;
}

std::string JSONArrayStream::reset() { 
    this->stream << "]"; 
    std::string result = this->stream.str();
    this->stream.clear();
    this->isFirstElement = true;
    this->stream << "[";
    return result;
};