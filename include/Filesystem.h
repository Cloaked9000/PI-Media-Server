//
// Created by fred on 06/05/17.
//

#ifndef MEDIASERVER_FILESYSTEM_H
#define MEDIASERVER_FILESYSTEM_H
#include <vector>
#include <string>
#include <fstream>
#include <sys/stat.h>

class Filesystem
{
public:

    /*!
     * For indicating filesystem object types
     */
    enum FSType
    {
        Error = 0,
        Directory = 1,
        File = 2,
    };

    /*!
     * Portable and fast method of checking if a file OR directory exists
     *
     * @param filepath The filepath of the file/directory to check
     * @return True if the file exists, false otherwise.
     */
    static inline bool does_filepath_exist(const std::string &filepath)
    {
        struct stat buffer{};
        return (stat(filepath.c_str(), &buffer) == 0);
    }

    /*!
     * Gets the type of object from a filepath
     *
     * @param filepath The filepath to get the type of
     * @return A FilesystemType enum containing the type. Or 'Unknown' on failure.
     */
    static inline FSType get_file_type(const std::string &filepath)
    {
        struct stat info{};
        if(stat(filepath.c_str(), &info) != 0)
            return FSType::Error;

        if(S_ISDIR(info.st_mode))
            return FSType::Directory;
        return FSType::File;
    }

    /*!
     * Creates a directory at the given filepath
     *
     * @param filepath Where to create the directory
     * @return True if the directory could be created. False otherwise.
     */
    static inline bool create_directory(const std::string &filepath)
    {
        int error = 0;
        #ifdef _WIN32
                error = mkdir(filepath.c_str());
        #else
                error = mkdir(filepath.c_str(), 0777);
        #endif
        return (error == 0);
    }



    /*!
     * Reads a file, line by line, into a vector.
     *
     * @param filepath Filepath of the file to open
     * @param lines Where to store the read data
     * @return True on success, false on failure.
     */
    static bool read_file(const std::string &filepath, std::vector<std::string> &lines);

    /*!
     * Reads a file into a string.
     *
     * @param filepath The filepath of the file to read
     * @param file Where to store the read data
     * @return True on success, false on failure.
     */

    static bool read_file(const std::string &filepath, std::string &file_data);

    /*!
     * Gets a list of files and folders within a given filepath.
     *
     * @param filepath The filepath to get a list of files from
     * @param results Where to store the filename results
     * @return True on success, false on failure.
     */
    static bool list_files(const std::string &filepath, std::vector<std::string> &results);

};


#endif //MEDIASERVER_FILESYSTEM_H
