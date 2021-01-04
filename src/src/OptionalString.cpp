#include <OptionalString.h>
#include <stdexcept>

OptionalString::OptionalString(const OptionalString& o) {
  *this = o;
}

OptionalString& OptionalString::operator=(const OptionalString& o){
  this->content.reset();
  if(nullptr != o.content){
    this->content = std::make_unique<std::string>(*o.content.get());
  }
  return *this;
}

OptionalString& OptionalString::operator=(OptionalString&& o){
  this->content = std::move(o.content);
  return *this;
}

bool operator==(std::nullptr_t, const OptionalString& s){
  return (nullptr == s.content);
}

bool operator!=(std::nullptr_t, const OptionalString& s){
  return !(nullptr == s);
}

void OptionalString::set(const std::string& value){
  if(nullptr == this->content){
    this->content = std::make_unique<std::string>(value);
  }
  else {
    *this->content = value;
  }
}

const std::string& OptionalString::operator*() const{
  if(nullptr == this->content){
    throw std::runtime_error("empty OptionalString cannot be dereferencied");
  }
  return *this->content.get();
}