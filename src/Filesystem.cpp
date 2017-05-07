//
// Created by fred on 06/05/17.
//

#include <dirent.h>
#include "../include/Filesystem.h"

bool Filesystem::read_file(const std::string &filepath, std::vector<std::string> &lines)
{
    //Ensure that it's a file, not a folder
    if(Filesystem::get_file_type(filepath) != FSType::File)
        return false;

    //Open the file
    std::ifstream file(filepath);
    if(!file.is_open())
        return false;

    //Read line by line
    lines.emplace_back();
    while(std::getline(file, lines.back()))
        lines.emplace_back();
    lines.pop_back();

    return true;
}

bool Filesystem::read_file(const std::string &filepath, std::string &file_data)
{
    //Ensure that it's a file, not a folder
    if(Filesystem::get_file_type(filepath) != Filesystem::File)
        return false;

    //Open it
    std::string buffer;
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if(!file.is_open())
        return false;

    //Get file size
    file.seekg(0, file.end);
    auto filesz = file.tellg();
    file.seekg(0, file.beg);

    //Error check
    if(!file.good() || filesz < 0)
        return false;

    //Read it
    file_data.resize((unsigned long)filesz);
    file.read(&file_data[0], filesz);

    return true;
}

bool Filesystem::list_files(const std::string &filepath, std::vector<std::string> &results)
{
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(filepath.c_str())) != NULL)
    {
        while((ent = readdir(dir)) != NULL)
        {
            if(std::string(ent->d_name) != ".." && std::string(ent->d_name) != ".")
            {
                results.emplace_back(ent->d_name);
            }
        }
        closedir(dir);
    }
    else
    {
        //Couldn't open directory
        return false;
    }
    return true;
}
