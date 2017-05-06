//
// Created by fred on 06/05/17.
//

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
    std::ifstream file(filepath);
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
