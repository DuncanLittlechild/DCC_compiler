//
// Created by duncan on 11/27/25.
//

#ifndef DCC_OVERLOAD_H
#define DCC_OVERLOAD_H

namespace Ol {
   template<class... Ts>
   struct overloaded : Ts... {
      using Ts::operator()...;
   };

   template<class... Ts>
   overloaded(Ts...) -> overloaded<Ts...>;
}

#endif //DCC_OVERLOAD_H