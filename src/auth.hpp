#ifndef AUTH_H
#define AUTH_H

#include <string>

/*
 * A function that checks if the file is contained within the whitelist of the
 * server
 * @param filename The file to check
 */
bool access_allowed(const std::string &filename);

/*
 * A function that checks if the file is contained within the deletelist of the
 * server
 * @param filename The file to check
 */
bool allowed_to_delete(const std::string &filename);

/*
 * A function that checks if the file is contained within the post_put_list of
 * the server
 * @param filename The file to check
 */
bool allowed_to_post_put(const std::string &filename);

#endif // !AUTH_H
