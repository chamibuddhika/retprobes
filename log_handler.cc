
#include <iostream>

#include <thread>

#include "log_handler.h"

void log_return(void* addr) {
  std::cout << "Logging return " << addr << " at thread " <<
    std::this_thread::get_id() << "\n" << std::flush;
}
