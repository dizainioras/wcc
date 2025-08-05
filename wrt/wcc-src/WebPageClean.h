#pragma once


void readFile3(const std::string& filePath, std::string& cleanedText);

void cleanText(std::string& cleanedText);

void remove_extra_whitespaces(std::string& cleanedText);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

long fetchURL(const string& url, const string& outputFile);