//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

// Framework includes
#include <DD4hep/IOV.h>
#include <DD4hep/Printout.h>
#include <DD4hep/Primitives.h>

// C/C++ include files
#include <climits>
#include <cstring>
#include <ctime>

using namespace dd4hep;

#if __cplusplus == 201402
const IOV::Key_value_type IOV::MIN_KEY;
const IOV::Key_value_type IOV::MAX_KEY;
#endif

#if 0
/// Assignment operator
IOVType& IOVType::operator=(const IOVType& copy)  {
  if ( &copy != this )  {
    name = copy.name;
    type = copy.type;
  }
  return *this;
}
#endif

/// Conversion to string
std::string IOVType::str()  const   {
  char text[256];
  ::snprintf(text,sizeof(text),"%s(%d)",name.c_str(),int(type));
  return text;
}

/// Initializing constructor
IOV::IOV(const IOVType* t) : iovType(t)  {
  if ( t ) type = t->type;
}

/// Specialized copy constructor for discrete IOVs
IOV::IOV(const IOVType* t, Key_value_type iov_value)
  : iovType(t), keyData(iov_value,iov_value)
{
  if ( t ) type = t->type;
}

/// Copy constructor
IOV::IOV(const IOVType* t, const Key& k)
  : iovType(t), keyData(k)
{
  if ( t ) type = t->type;
}

/// Set discrete IOV value
void IOV::set(const Key& value)  {
  keyData = value;
}

/// Set discrete IOV value
void IOV::set(Key::first_type value)  {
  keyData.first = keyData.second = value;
}

/// Set range IOV value
void IOV::set(Key::first_type val_1, Key::second_type val_2)  {
  keyData.first  = val_1;
  keyData.second = val_2;
}

/// Set keys to unphysical values (LONG_MAX, LONG_MIN)
IOV& IOV::reset()  {
  keyData.first  = LONG_MAX;
  keyData.second = LONG_MIN;
  return *this;
}

/// Set keys to unphysical values (LONG_MAX, LONG_MIN)
IOV& IOV::invert()  {
  Key::first_type tmp = keyData.first;
  keyData.first  = keyData.second;
  keyData.second = tmp;
  return *this;
}

void IOV::iov_intersection(const IOV& validity)   {
  if ( !iovType )
    *this = validity;
  else if ( validity.keyData.first > keyData.first ) 
    keyData.first = validity.keyData.first;
  if ( validity.keyData.second < keyData.second )
    keyData.second = validity.keyData.second;
}

void IOV::iov_intersection(const IOV::Key& validity)   {
  if ( validity.first > keyData.first ) 
    keyData.first = validity.first;
  if ( validity.second < keyData.second )
    keyData.second = validity.second;
}

void IOV::iov_union(const IOV& validity)   {
  if ( !iovType )
    *this = validity;
  else if ( validity.keyData.first < keyData.first ) 
    keyData.first = validity.keyData.first;
  if ( validity.keyData.second > keyData.second )
    keyData.second = validity.keyData.second;
}

void IOV::iov_union(const IOV::Key& validity)   {
  if ( validity.first < keyData.first ) 
    keyData.first = validity.first;
  if ( validity.second > keyData.second )
    keyData.second = validity.second;
}

/// Move the data content: 'from' will be reset to NULL
void IOV::move(IOV& from)   {
  //::memcpy(this,&from,sizeof(IOV));
  *this = from;
  from.keyData.first = from.keyData.second = from.optData = 0;
  from.type = int(IOVType::UNKNOWN_IOV);
  from.iovType = 0;
}

/// Create string representation of the IOV
std::string IOV::str()  const  {
  char text[256];
  if ( iovType )  {
    /// Need the long(x) casts for compatibility with Apple MAC
    if ( iovType->name[0] != 'e' )   {
      ::snprintf(text,sizeof(text),"%s(%u):[%ld-%ld]",
                 iovType->name.c_str(), iovType->type, long(keyData.first), long(keyData.second));
    }
    else if ( iovType->name == "epoch" )  {
      struct tm  time_buff;
      char c_since[64], c_until[64];
      static constexpr const Key_value_type nil = 0;
      static const Key_value_type max_time = detail::makeTime(2099,12,31,24,59,59);
      std::time_t since = std::min(std::max(keyData.first, nil), max_time);
      std::time_t until = std::min(std::max(keyData.second,nil), max_time);
      struct tm* tm_since = ::gmtime_r(&since,&time_buff);
      struct tm* tm_until = ::gmtime_r(&until,&time_buff);
      if ( nullptr == tm_since || nullptr == tm_until )    {
        except("IOV::str"," Invalid epoch time stamp: %d:[%ld-%ld]",
               type, long(keyData.first), long(keyData.second));
      }
      ::strftime(c_since,sizeof(c_since),"%d-%m-%Y %H:%M:%S", tm_since);
      ::strftime(c_until,sizeof(c_until),"%d-%m-%Y %H:%M:%S", tm_until);
      ::snprintf(text,sizeof(text),"%s(%u):[%s - %s]",
                 iovType->name.c_str(), iovType->type,
                 c_since, c_until);
    }
    else   {
      ::snprintf(text,sizeof(text),"%s(%u):[%ld-%ld]",
                 iovType->name.c_str(), iovType->type, long(keyData.first), long(keyData.second));
    }
  }
  else  {
    ::snprintf(text,sizeof(text),"%u:[%ld-%ld]", type, long(keyData.first), long(keyData.second));
  }
  return text;
}

/// Check for validity containment
bool IOV::contains(const IOV& iov)  const   {
  if ( key_is_contained(iov.keyData,keyData) )  {
    unsigned int typ1 = iov.iovType ? iov.iovType->type : iov.type;
    unsigned int typ2 = iovType ? iovType->type : type;
    return typ1 == typ2;
  }
  return false;
}
