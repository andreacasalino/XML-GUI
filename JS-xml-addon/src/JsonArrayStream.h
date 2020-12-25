#ifndef JSON_ARRAY_STREAM_H
#define JSON_ARRAY_STREAM_H

#include <sstream>
#include <string>

class JSONArrayStream {
public:
    JSONArrayStream();

    friend JSONArrayStream& operator<<(JSONArrayStream&, const std::string&);

    std::string reset();
private:
    bool isFirstElement;
    std::stringstream stream;
};

#endif