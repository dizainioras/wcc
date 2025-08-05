using namespace std;
#include <string>

#pragma once
// void getPage(const string& urlLink, const string& outputFile);

// void cleanHTML(const string& inputFile, const string& outputFile, int& langCode);

void cleanHTML(const string& inputFile, string& cleanedText, int& langCode);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

int fetchURL(const string& url, const string& outputFile);