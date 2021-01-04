#ifndef OPTIONAL_STRING_H
#define OPTIONAL_STRING_H

#include <memory>
#include <string>

class OptionalString {
public:
    OptionalString() = default;    
    OptionalString(const OptionalString& o);
    OptionalString& operator=(const OptionalString& o);
    OptionalString(OptionalString&& o) = default;
    OptionalString& operator=(OptionalString&& o);

    friend bool operator==(std::nullptr_t, const OptionalString&);
    friend bool operator!=(std::nullptr_t, const OptionalString&);

    void set(const std::string& value);
    const std::string& operator*() const;
private:
    std::unique_ptr<std::string> content;
};

#endif