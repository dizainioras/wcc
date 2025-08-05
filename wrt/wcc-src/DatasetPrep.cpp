using namespace std;
#define CURL_STATICLIB
#define _HAS_STD_BYTE 0

#include "DatasetPrep.h"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <algorithm>
#include <curl/curl.h>

#include <unordered_set>
#include <sstream>



// Callback function to write data to a file
size_t WriteCallback0(void* contents, size_t size, size_t nmemb, void* userp) {
	ofstream* file = static_cast<ofstream*>(userp);
	size_t totalSize = size * nmemb;
	file->write(static_cast<char*>(contents), totalSize);
	return totalSize;
}


long fetchURL0(const string& url, const string& outputFile) {
	CURL* curl;
	CURLcode res;
	long RCode = 999;
	
	ofstream file(outputFile);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback0);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (res == CURLE_OK) {
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &RCode);
		}
		else {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
	}
	curl_global_cleanup();
	file.close();

	
	return RCode; // Return the response code
}



void cleanHTML(const string& inputFile, string& cleanedText, int& langCode) {
	
	ifstream inFile(inputFile);
	
	string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
	regex scriptTag("<script.*?>[\\s\\S]*?</script>", regex::icase);
	regex styleTag("<style.*?>[\\s\\S]*?</style>", regex::icase);
	regex htmlTag("<[^>]*>");
	regex inlineScript("on\\w+\\s*=\\s*\"[^\"]*\"", regex::icase);
	regex inlineStyle("style\\s*=\\s*\"[^\"]*\"", regex::icase);
	regex cssClass("#[\\w-]+\\s*\\{[\\s\\S]*?\\}", regex::icase);
	regex cssId("\\.[\\w-]+\\s*\\{[\\s\\S]*?\\}", regex::icase);

	regex numberRegex(R"(\b\w*\d\w*\b)"); // Matches words containing numbers
	regex ampersandRegex(R"(\b\w*&\w*\b)"); // Matches words containing '&'
	regex punctuationRegex(R"([.,;:?!()\"'])"); // Matches punctuation symbols
	regex digitRegex(R"(\d+)"); // Matches standalone digits
	regex whitespaceRegex(R"(\s+)"); // Matches multiple whitespace characters
	string result;

	/* regex Lang_regex(R"(lang="en-US"| lang="en"| lang="en-AU"| lang="en-UK"| lang="en-GB"| lang="en | lang=en-US| lang=en| lang=en-AU| lang=en-UK| lang=en-GB| lang=en)");
	if (!regex_search(content, Lang_regex)) {
		
		langCode = 1;
	}
	else {
		langCode = 0;
	}
	*/
	langCode = 1;
		// Remove script and style blocks
	content = regex_replace(content, scriptTag, " ");
	content = regex_replace(content, styleTag, " ");

		// Remove inline scripts and styles
	content = regex_replace(content, inlineScript, " ");
	content = regex_replace(content, inlineStyle, " ");

		// Remove CSS classes and IDs
	content = regex_replace(content, cssClass, " ");
	content = regex_replace(content, cssId, " ");

		// Remove remaining HTML tags
	content = regex_replace(content, htmlTag, " ");


		// Remove punctuation symbols
	content = regex_replace(content, punctuationRegex, " ");

		// Remove empty lines
	content = regex_replace(content, std::regex("(^\\s*\\n|\\n\\s*$|\\n\\s*\\n)"), " ");

		// Remove newlines
	content.erase(remove(content.begin(), content.end(), '\n'), content.end());
	content.erase(remove(content.begin(), content.end(), '\r'), content.end());

		// Replace multiple spaces with a single space
	content = regex_replace(content, std::regex("\\s+"), " ");

		// Trim leading and trailing whitespace
	content = regex_replace(content, std::regex(R"(^\s+|\s+$)"), " ");


		// Regex pattern to match words containing at least one letter
	regex wordRegex(R"(\b[a-zA-Z]+\b)");
	
		// Result string to hold the cleaned words
		
	sregex_iterator iter(content.begin(), content.end(), wordRegex);
	sregex_iterator end;

			// Iterate through all matches and append them to the result
	while (iter != end) {
		if (!result.empty()) {
			result += " "; // Add space between words
		}
		result += iter->str(); // Append the matched word
		++iter;
		}
	
		
	cleanedText = result;

	inFile.close();

}
