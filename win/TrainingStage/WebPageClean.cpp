#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <algorithm>

void cleanHTML2(const std::string& inputFile, const std::string& outputFile) {
	std::ifstream inFile(inputFile);
	std::ofstream outFile(outputFile);
	std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
	std::regex scriptTag("<script.*?>[\\s\\S]*?</script>", std::regex::icase);
	std::regex styleTag("<style.*?>[\\s\\S]*?</style>", std::regex::icase);
	std::regex htmlTag("<[^>]*>");
	std::regex inlineScript("on\\w+\\s*=\\s*\"[^\"]*\"", std::regex::icase);
	std::regex inlineStyle("style\\s*=\\s*\"[^\"]*\"", std::regex::icase);
	std::regex cssClass("#[\\w-]+\\s*\\{[\\s\\S]*?\\}", std::regex::icase);
	std::regex cssId("\\.[\\w-]+\\s*\\{[\\s\\S]*?\\}", std::regex::icase);

	if (!inFile.is_open() || !outFile.is_open()) {
		std::cerr << "Error opening file!" << std::endl;
		return;
	}

	// Remove script and style blocks
	content = std::regex_replace(content, scriptTag, "");
	content = std::regex_replace(content, styleTag, "");
	// Remove inline scripts and styles
	content = std::regex_replace(content, inlineScript, "");
	content = std::regex_replace(content, inlineStyle, "");
	// Remove CSS classes and IDs
	content = std::regex_replace(content, cssClass, "");
	content = std::regex_replace(content, cssId, "");
	// Remove remaining HTML tags
	content = std::regex_replace(content, htmlTag, "");

	// Remove empty lines
	content = std::regex_replace(content, std::regex("(^\\s*\\n|\\n\\s*$|\\n\\s*\\n)"), "");

	// Remove newlines
	content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
	content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

	// Replace multiple spaces with a single space
	content = std::regex_replace(content, std::regex("\\s+"), " ");

	outFile << content;

	inFile.close();
	outFile.close();
}

