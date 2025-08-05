#define CURL_STATICLIB
#define _HAS_STD_BYTE 0


using namespace std;


#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <algorithm>

#include <curl/curl.h>
#include <unordered_set>
#include <sstream>


// Callback function to write data to a file
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	ofstream* file = static_cast<ofstream*>(userp);
	size_t totalSize = size * nmemb;
	file->write(static_cast<char*>(contents), totalSize);
	return totalSize;
}


long fetchURL(const string& url, const string& outputFile) {
	CURL* curl;
	CURLcode res;
	long RCode = 999;


	ofstream file(outputFile);
	

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
		
		res = curl_easy_perform(curl);
		

		if (res == CURLE_OK) {
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &RCode);
			curl_easy_cleanup(curl);
		}
		else {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
	}
	curl_global_cleanup();

	file.close();


	return RCode; // Return the response code
}



void readFile3(const std::string& filePath, std::string& cleanedText) {

	std::ifstream inputFile(filePath);
	std::string line, storedText;
	bool inScript = false, inStyle = false;

	if (!inputFile.is_open()) {
		std::cerr << "Error opening file!" << std::endl;
		return;
	}
	

	while (std::getline(inputFile, line)) {
		if (!inScript) {
			size_t scriptPos = line.find("<script");
			if (scriptPos != std::string::npos) {
				
				size_t endScriptPos = line.find("</script>");
								
				if (endScriptPos != std::string::npos) {

					std::string draftLine = line.erase(scriptPos, endScriptPos + 9 - scriptPos);

					inScript = false;
					bool reCheck = true;
				
					while ((!draftLine.empty()) && reCheck)
					{
						size_t scriptPos = draftLine.find("<script");
						size_t endScriptPos = draftLine.find("</script>");
						if (scriptPos != std::string::npos)
						{
							
							if (endScriptPos != std::string::npos)
							{
								reCheck = true;
								draftLine = draftLine.erase(scriptPos, endScriptPos + 9 - scriptPos);

								inScript = false;
								
							}
							else
							{
								
								reCheck = false;
								draftLine = draftLine.substr(0, scriptPos);
								inScript = true;
							}
						}
						else
						{
							
							reCheck = false;
							inScript = false;
						}
					}
					storedText += draftLine;
				}
				else
				{
					storedText += line.substr(0, scriptPos);

					inScript = true;
				}
			}
			else {
				storedText += line;
			}
		}
		else {
			size_t endScriptPos = line.find("</script>");
			if (endScriptPos != std::string::npos) {

				std::string afterScript = line.substr(endScriptPos + 9); // "script/>".length() is 8
				inScript = false;
				bool reCheck = true;
				
				while ((!afterScript.empty()) && reCheck)
					{
						size_t scriptPos = afterScript.find("<script");
						size_t endScriptPos = afterScript.find("</script>");
						if (scriptPos != std::string::npos)
						{
							
							if (endScriptPos != std::string::npos)
							{
								reCheck = true;
								afterScript = afterScript.erase(scriptPos, endScriptPos + 9 - scriptPos);
								inScript = false;
								
							}
							else
							{
							reCheck = false;
								afterScript = afterScript.substr(0, scriptPos);
								inScript = true;

							}
						}

						else
						{
							reCheck = false;
							inScript = false;
						}
				}

				storedText += afterScript;
			}
			else {
				storedText += line;
			}
		}
		size_t scriptPos = line.find("<style");
	}
	
	bool findStyle = true;

	while (findStyle) {
		size_t stylePos = storedText.find("<style");

		if (stylePos != std::string::npos) {
		
			size_t endStylePos = storedText.find("</style>");
			if (endStylePos != std::string::npos)
			{
				storedText = storedText.erase(stylePos, endStylePos + 8 - stylePos);
		
			}
			else { findStyle = false;
	
			}
		}
		else 
		{
			findStyle = false;
		
		}
	}
	cleanedText = storedText;
	inputFile.close();

	
	// std::cout << "\n Stored Text: " << storedText << std::endl;
}



void cleanText(std::string& cleanedText) {

	bool intag = false;
	
	std::string outStr;

	
	for (size_t i = 0; i < cleanedText.size(); ++i)
	{
		char ch = cleanedText[i];
		if (ch == '<')
			intag = true;
		else
			if (ch == '>')
			{
				intag = false;
				outStr += ' ';
			}
		else
				if (!intag) {
					if ((ch == '\t') || (ch == '\n' ))
					{
						outStr += ' ';
					}
					
					else {
						outStr += ch;
					}
				}
	}
	cleanedText = outStr;
	outStr =' ';
	intag = false;
	for (size_t i = 0; i < cleanedText.size(); ++i)
	{
		char ch = cleanedText[i];
		if (ch == '{')
			intag = true;
		else
			if (ch == '}')
			{
				intag = false;
				outStr += ' ';
			}
			else
				if (!intag) {
					outStr += ch;
				}
	}
	cleanedText = outStr;
	intag = false;
	outStr = ' ';

	for (size_t i = 0; i < cleanedText.size(); ++i)
	{
		char ch = cleanedText[i];
		if (ch == '[')
			intag = true;
		else
			if (ch == ']')
			{
				intag = false;
				outStr += ' ';
			}
			else
				if (!intag) {
					outStr += ch;
				}
	}

	cleanedText = outStr;

	
}

void remove_extra_whitespaces(std::string& cleanedText)
{
	bool wasSpace = false;

	std::string outStr;

	for (size_t i = 0; i < cleanedText.size(); ++i)
	{
		char ch = cleanedText[i];
		if (ch == ' ')
		{
			if (!wasSpace)
			{
				wasSpace = true;
				outStr += ch;
			}
			else
			{
				wasSpace = true;
			}
		}
		else
		{
			wasSpace = false;
			outStr += ch;
		}
	}
	cleanedText = outStr;
		
}

