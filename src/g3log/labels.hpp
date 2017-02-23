#pragma once

#include <algorithm>
#include <cctype>
#include <set>
#include <string>


namespace g3 {
   namespace internal {

      struct InsensitiveComparator {
         bool operator() (const std::string& a, const std::string& b) const {
            auto result = true;  // by default (a<b)=true

            if (a.length() == b.length()) {

               // using equals function to determine if (a<b)=false
               result = !std::equal(b.begin(), b.end(), a.begin(), [](unsigned char a, unsigned char b) {
                  return std::tolower(a) >= std::tolower(b);
               });
            }

            return result;
         }
      };
   } // internal

   /**
    * This 'Labels' alias can be used for passing labels to log stream
    */
   using Labels = std::set<std::string, internal::InsensitiveComparator>;
} // g3
