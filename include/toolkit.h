// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#ifndef INCLUDE_TOOLKIT_H_
#define INCLUDE_TOOLKIT_H_

#include <types.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

void extract_element(vector<string> *elements, string bufline, uint start = 0,
                     uint num = MAX_INT, const char seperator = ',') {
  char *charbuf;
  string cut = " \t\r\n";
  cut += seperator;

  uint count = 0;

  try {
    if (NULL != (charbuf = strtok(bufline.data(), cut.data()))) {
      do {
        if (count >= start && count < start + num) {
          elements->push_back(charbuf);
        }
        count++;
      } while (NULL != (charbuf = strtok(NULL, cut.data())));
    }
  } catch (std::exception &e) {
    cout << "extract exception." << endl;
  }
}

#endif  // INCLUDE_TOOLKIT_H_
