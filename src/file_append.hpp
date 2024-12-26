#ifndef FILE_APPEND_H
#define FILE_APPEND_H

#include <string>

/*
 * Append data to any file on a specific line
 * @param data The data to appended
 * @param filename The name of the file you want to append to
 * @param line The line to append to (-1 for EOF)
 * @param pos The position of the cursor in the line (-1 for EOL)
 * @return True if the operation was successfull
 */
bool append_to_file(const std::string &data, const std::string &filename,
                   int line, int pos);

#endif // !FILE_APPEND_H
