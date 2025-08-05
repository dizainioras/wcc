#define _HAS_STD_BYTE 0
#define CURL_STATICLIB
#define MLPACK_PRINT_INFO
#define MLPACK_PRINT_WARN
#define ARMA_32BIT_WORD 1
//#define ARMA_64BIT_WORD 1 

#include <iostream>
#include <fstream>

#include <curl/curl.h>
#include "DatasetPrep.h"
#include "TextStemming.h"
#include "WebPageClean.h"

#include <algorithm>
#include <cctype>

#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_set>

#include <cmath>

#include <mlpack.hpp>
#include <armadillo>
#include <ensmallen.hpp>

#include <iomanip>

#include <chrono>

#include <stdio.h> 

#define DEBUG
#include "dbg.h"

using namespace arma;
using namespace mlpack;

using namespace std;
using namespace std::chrono;

struct Record {
	int id;
	string url;
	string label;
};


static void readCSV(const string& filename, vector<Record>& records) {
	ifstream file(filename);
	string line, url, label;
	string idStr;
	int id;

	if (!file.is_open()) {
		cerr << "Error opening file: " << filename << "\n";
	}

	// Skip the header
	getline(file, line);

	// Read each line from the file
	while (getline(file, line)) {
		stringstream ss(line);

		getline(ss, idStr, ',');
		id = stoi(idStr);
		getline(ss, url, ',');
		getline(ss, label, ',');

		records.push_back({ id, url, label });
	}
}


static void loadStopWords(const string& filename, unordered_set<string>& stopWords) {
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Could not open stop words file: " << filename << "\n";
		return;
	}

	string word;
	while (file >> word) {
		stopWords.insert(word);
	}
}


// Function to split a string into words
static vector<string> split(const string& text) {
	stringstream ss(text);
	string word;
	vector<string> words;
	while (ss >> word) {
		transform(word.begin(), word.end(), word.begin(), ::tolower);
		words.push_back(word);
	}
	return words;
}


// Function to calculate TF-IDF
static void calculateTFIDFclassify(const string& document, map<string, double>& tfidf_doc, map<string, int>& df) {

	// Calculate term frequencies and document frequencies

	vector<string> words = split(document);

	map<string, int> tf_doc;

	for (const string& word : words) {
		if (tf_doc.count(word))
		{
			// add tf++
			tf_doc[word]++;
		}
		else
			if (df.count(word)) {
				tf_doc.insert({ word,1 });
			}

	}



	// Calculate TF-IDF
	int documents_number = 2393;

	
	for (const auto& pair : tf_doc) {

		const string& word = pair.first;
		double tf_value = static_cast<double>(pair.second) / split(document).size();
		double idf_value = log(static_cast<double>(documents_number) / df[word]);
		tfidf_doc[word] = tf_value * idf_value;
	}
	for (const auto& pair : df) {
		if (!tfidf_doc.count(pair.first)) {
			tfidf_doc.insert({ pair.first,0 });
		}

	}

}


static bool isEnglishWord(const string& word, const unordered_set<string>& dictionary) {
	return dictionary.find(word) != dictionary.end();
}

// Function to split a string into words
static vector<string> splitToWords(const string& text) {
	vector<string> words;
	istringstream stream(text);
	string word;
	while (stream >> word) {
		transform(word.begin(), word.end(), word.begin(), ::tolower);
		words.push_back(word);
	}
	return words;
}


static void filterWords3(const vector<string>& words, const unordered_set<string>& dictionary, const int& maxWords, string& cleanedWords, int& amountWords) {
	vector<string> filteredWords;
	int iW = 0;
	for (const auto& word : words) {
		if (isEnglishWord(word, dictionary) && iW < maxWords) {
			filteredWords.push_back(word);
			++iW;
		}
	}
	ostringstream result;
	for (size_t i = 0; i < filteredWords.size(); ++i) {
		if (i > 0) result << " ";
		result << filteredWords[i];
	}
	cleanedWords = result.str();
	amountWords = iW;
}



int main(int argc, char* argv[]) {
	
	string modelFileName = "model-sr.xml";	// modelis, gali būti naudojami .bin, .xml ir .json formatas
	string urlFileName = "URL.csv";			// testuojamu puslapių sąrašas. Laukai->  id,url,label
	string resultFileName = "results.csv";  // surašomi testavimo rezultatai. Tokie patys kaip ir išvedami į ekraną. 
											// Laukai -> URL,label,predictedlabel,totalTime,getTime,cleanTime,tfidfCalcTime,classifyTime,predictedlabel,0,1,2,3,4,5,6,tokensAmount
	string dfFileName = "df.csv";			// DF vektorius, pagal kurį skaičiuojamas TFIDF vektorius klasifikavimui. 
											// Keičiamas priklausomai nuo duomenų kiekio ir naudojamų puslapio žodžių kiekio.
	string tokensFileName = "tokens.txt";	// žodžių sąrašas, kuris buvo naudojamas skaičiuojan DF vektorius. Žodžiai pilni, netrumpinti.
	string model = "sr";					// modelio tipas: sr, nbc, svm, tree, rf

	dbg("Debugas ijungtas."); 

	dbg("Argumentu nuskaitymas.");

	dbg("Argumentu kiekis %d", argc);
	switch (argc) {
	case 3: {
		model = argv[1];
		modelFileName = argv[2]; 
		dbg("Modelio tipas, modelio pavadinimas");
		break;
	}
	case 4: 
	{
		model = argv[1];
		modelFileName = argv[2];
		urlFileName = argv[3];
		dbg("Modelio tipas, modelio pavadinimas, URL failo pavadinimas");
		break;
	}
	case 5:
	{
		model = argv[1];
		modelFileName = argv[2];
		urlFileName = argv[3];
		resultFileName = argv[4];
		dbg("Modelio tipas, modelio pavadinimas, URL failo pavadinimas, Rezultatu failo pavadinimas");
		break;
	}
	case 6:
	{
		model = argv[1];
		modelFileName = argv[2];
		urlFileName = argv[3];
		resultFileName = argv[4];
		dfFileName = argv[5];
		dbg("Modelio tipas, modelio pavadinimas, URL failo pavadinimas, Rezultatu failo pavadinimas, DF filo pavadinimas");
		break;
	}
	case 7:
	{
		model = argv[1];
		modelFileName = argv[2];
		urlFileName = argv[3];
		resultFileName = argv[4];
		dfFileName = argv[5];
		tokensFileName = argv[6];
		dbg("Modelio tipas, modelio pavadinimas, URL failo pavadinimas, Rezultatu failo pavadinimas, DF filo pavadinimas, Zodziu failu pavadinimas");
		break;
	}
	}

	// Þodyno uþkrovimas
	dbg("Loading tokens.");

	unordered_set<string> dictionary;
	loadStopWords(tokensFileName, dictionary);
	dbg("Tokens are loaded - %s.", tokensFileName.c_str());
	
	// DF vektoriaus uþkrovimas

	dbg("Loading DF.");

	map<string, int> df;
	ifstream dfFile(dfFileName);

	if (!dfFile.is_open()) {
		cerr << "Error opening file: " << dfFileName << "\n";
	}

	string line;
	getline(dfFile, line);
	string dfWord, dfValue;

	while (getline(dfFile, line)) {
		stringstream ss(line);
		getline(ss, dfWord, ',');
		getline(ss, dfValue, ',');
		df.insert(pair<string, int>(dfWord, stoi(dfValue)));

	}

	dbg("DF are loaded - %s.", dfFileName.c_str());

	// Modelio uþkrovimas

	dbg("Loading model");
	mlpack::SoftmaxRegression sr;
	mlpack::LinearSVM svm; // Create model
	mlpack::NaiveBayesClassifier nbc;		// Create model
	mlpack::DecisionTree tree; // Create model
	mlpack::RandomForest rf;		// Create model

	if (model == "sr") {
		
		mlpack::data::Load(modelFileName, "sr", sr, true);
			
		}
	else if (model == "svm") {
		
		svm.Lambda() = 0.00005;
		svm.Delta() = 0.01;
		mlpack::data::Load(modelFileName, "svm", svm, true);
		
	}
	else if (model == "tree") {
				
		mlpack::data::Load(modelFileName, "tree", tree, true);
	}

	else if (model == "rf") {

		mlpack::data::Load(modelFileName, "rf", rf, true);
	}

	else {

		mlpack::data::Load(modelFileName, "nbc", nbc, true);
	}

	dbg("Model is loaded - %s.", modelFileName.c_str());

	// Þodþiø kiekio nustatymai
	int maxWords = 200;
	int minWords = 5;

	// URL sàraðo nuskaitymas
	vector<Record> records;
	
	readCSV(urlFileName, records);
	dbg("URLs are loaded - %s.", urlFileName.c_str());


	ofstream resultfile(resultFileName);
	resultfile << "URL,label,predictedlabel,totalTime,getTime,cleanTime,tfidfCalcTime,classifyTime,predictedlabel,0,1,2,3,4,5,6,tokensAmount\n";

	dbg("Results file - %s.", resultFileName.c_str());


	for (auto& record : records) {

		auto start = high_resolution_clock::now();

		string web_url, web_fullText;
		long web_responseCode = 0;
		string web_text;
		string url_label;
		bool classify = true;

		web_url = record.url;
		url_label = record.label;

		dbg("--- Start ---");
		dbg("Web page URL is %s", web_url.c_str());
		cout << "-------- " << web_url << " ---------- \n";

		string webPage = "webpage.html";
		string cleanedText = "";


		dbg("Fetching URL. ");
		web_responseCode = fetchURL(web_url, webPage);

		auto stop_get = high_resolution_clock::now();

		dbg("Checking responce code - %d", web_responseCode);

		if (web_responseCode == 200) {

			dbg("Cleaning webpage. Step 1");
			readFile3(webPage, cleanedText);

			dbg("Cleaning webpage. Step 2");
			cleanText(cleanedText);

			dbg("Cleaning webpage. Step 3");
			remove_extra_whitespaces(cleanedText);

			dbg("Webpage is cleaned");
		}
		else {

			dbg("Webpage is unreachable");
			cout << web_url << " Unreachable " << "\n";
			classify = false;
		}


		int amountWords = 0;

		if (web_responseCode == 200) {

			vector<string> words = splitToWords(cleanedText);

			dbg("Filtering words. ");

			filterWords3(words, dictionary, maxWords, cleanedText, amountWords);

			dbg("Webpages contais %d words for classification.", amountWords);

			if (amountWords >= minWords) {

				dbg("Steming words.");
				web_text = stemfile(cleanedText);


			}
			else {
				dbg("Too little words for classification.");
				web_text = stemfile(cleanedText);
				cout << "Too litle words \n";
				classify = false;
			}

			if (web_text.empty()) {
				web_responseCode = 777;
				classify = false;
			}

			words.clear();
		}

		cleanedText = "";

		auto stop_webpage = high_resolution_clock::now();

		if (classify) {
			
			cout << "Webpges cleaned and prepared for classify dataset \n";
			map<string, double> tfidfClassify;

			dbg("Calculating TFIDF vector.");

			calculateTFIDFclassify(web_text, tfidfClassify, df);
			
			auto stop_tfidf = high_resolution_clock::now();


			dbg("Preaparing data for classification.");

						
			arma::mat testDataset(tfidfClassify.size(), 1);

			dbg("Testing dataset is prepared.");
			


			int k = 0;
			for (const auto& pair : tfidfClassify) {
				testDataset(k, 0) = pair.second;
				++k;
			}
			
			dbg ("Testing Dataset 2");
			tfidfClassify.clear();

			dbg("Testing Predictions");
			arma::Row<size_t> testPredictions;

			arma::mat testProbabilities;

			dbg("Clasifying. ");

			if (model == "sr") {
				sr.Classify(testDataset, testPredictions, testProbabilities); // Testing model

			}
			else if (model == "svm") {

				svm.Classify(testDataset, testPredictions, testProbabilities); // Testing model

			}
			else if (model == "tree") {

				tree.Classify(testDataset, testPredictions, testProbabilities); // Testing model

			}
			else if (model == "rf") {

				rf.Classify(testDataset, testPredictions, testProbabilities); // Testing model

			}
			else {

				nbc.Classify(testDataset, testPredictions, testProbabilities); // Testing model

			}



			

			dbg("Printing. ");



			/* To print probabilities */

			string label = "";
			switch (testPredictions(0)) {
			case 0: label = "Business";  break;
			case 1: label = "Education"; break;
			case 2: label = "Adult"; break;
			case 3: label = "Games"; break;
			case 4: label = "Health"; break;
			case 5: label = "Sports"; break;
			case 6: label = "Travels"; break;
			}

			cout << " --------- \n";
			cout << web_url << "    - label " << url_label << "\n";

			cout << " Predicted label : " << testPredictions(0) << " " << label << "\n";
			cout << " Probabilities : \n";
			for (uword j = 0; j < testProbabilities.n_rows; j++)
			{
				cout << setprecision(3) << j << " - " << testProbabilities(j, 0) << " ; ";
			}
			cout << "\n";


			auto stop = high_resolution_clock::now();


			auto duration = duration_cast<milliseconds>(stop - start);
			auto duration_get = duration_cast<milliseconds>(stop_get - start);
			auto duration_clean = duration_cast<milliseconds>(stop_webpage - stop_get);
			auto duration_tfidf = duration_cast<milliseconds>(stop_tfidf - stop_webpage);
			auto duration_classify = duration_cast<milliseconds>(stop - stop_tfidf);

			cout << "Time taken :     " << duration.count() << " ms" << endl;
			cout << "Get webpage:     "
				<< duration_get.count() << " ms" << endl;
			cout << "Clean webpage:   "
				<< duration_clean.count() << " ms" << endl;
			cout << "Calculate TFIDF: "
				<< duration_tfidf.count() << " ms" << endl;
			cout << "Classify:        "
				<< duration_classify.count() << " ms" << endl;
			cout << "\n";

			dbg("Writting results to file. ");
			resultfile << web_url << "," << url_label << "," << testPredictions(0) << ","
				<< duration.count() << "," << duration_get.count() << ","
				<< duration_clean.count() << "," << duration_tfidf.count() << ","
				<< duration_classify.count() << ","
				 << testPredictions(0) << "," << setprecision(3) << testProbabilities(0, 0) << "," << testProbabilities(1, 0) << "," << testProbabilities(2, 0) << ","
				<< testProbabilities(3, 0) << "," << testProbabilities(4, 0) << "," << testProbabilities(5, 0) << "," << testProbabilities(6, 0) << ","
				<< amountWords <<"\n";
			
			

			testPredictions.reset();
			testProbabilities.reset();
			testDataset.reset();
		}

		else
		{
			dbg("Cannot classify webpage.");
			cout << "Cannot classify webpage \n";
			dbg("Writting results to file. ");
			resultfile << web_url << "," << url_label << "," << "99" << "," 
				<< duration_cast<milliseconds>(stop_get - start).count() << "," << duration_cast<milliseconds>(stop_get - start).count() << ","
				<< "0" << "," << "0" << ","
				<< "0" << ","
				<< "99" << "," << "0" << "," << "0" << "," << "0" << ","
				<< "0" << "," <<"0" << "," << "0" << "," << "0" << ","
				<< amountWords << "\n";
		}

		dbg("Go to next URL.");
		
	}

	resultfile.close();

	return 0;
}

