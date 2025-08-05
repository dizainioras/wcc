#define _HAS_STD_BYTE 0
#define CURL_STATICLIB
using namespace std;

#include <iostream>
#include <fstream>
#include <curl\curl.h>
#include "DatasetPrep.h"
#include "TextStemming.h"

#include <algorithm>
#include <cctype>

#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <set>

#include <cmath>

#define MLPACK_PRINT_INFO
#define MLPACK_PRINT_WARN
#define ARMA_32BIT_WORD 1

#include <mlpack.hpp>
#include <armadillo>
#include <ensmallen.hpp>

#include <iomanip>

#include <chrono>

using namespace arma;
using namespace mlpack;
using namespace mlpack::tree;
using namespace std;

using namespace std::chrono;

struct Record {
	int id;
	string url;
	string label;
	int responseCode;
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
		
		records.push_back({ id, url, label});
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

static string removeStopWords(const string& text, const unordered_set<string>& stopWords) {
	istringstream stream(text);
	ostringstream cleanedText;
	string word;

	while (stream >> word) {
		// Remove punctuation from the word
		word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());

		// Convert to lowercase for comparison
		transform(word.begin(), word.end(), word.begin(), ::tolower);

		
		// If the word is not a stop word, add it to the cleaned text
		if (stopWords.find(word) == stopWords.end()) {
			cleanedText << word << " ";
		}
	}

	return cleanedText.str();
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
static void calculateTFIDF(const vector<string>& documents, vector<map<string, double>>& tfidf, map<string, int>& df) {

	vector<map<string, int>> tf(documents.size());

	// Calculate term frequencies and document frequencies
	
	cout << "Number of documents: " << documents.size() << "\n";
	for (size_t i = 0; i < documents.size(); ++i) {
		vector<string> words = split(documents[i]);
		map<string, int>& tf_doc = tf[i];
		for (const string& word : words) {
			tf_doc[word]++;
		}
		for (const auto& pair : tf_doc) {
			df[pair.first]++;
		}
		cout << "TF and DF --- " << i << "\n";
	}

	ofstream outfile("df.csv");
	outfile << "word,df\n";

	for (const auto& dfItem : df) {
		outfile << dfItem.first << "," << dfItem.second << "\n";
	}
	outfile.close();

	cout << "DF values have been calculated and stored in df.csv" << "\n";

	// Calculate TF-IDF
	
	cout << "----Calculation TF-IDF---- \n";
	for (size_t i = 0; i < documents.size(); ++i) {
		map<string, double> tfidf_doc;
		for (const auto& pair : tf[i]) {
			const string& word = pair.first;
			double tf_value = static_cast<double>(pair.second) / split(documents[i]).size();
			double idf_value = log(static_cast<double>(documents.size()) / df[word]);
			tfidf_doc[word] = tf_value * idf_value;
		}
		for (const auto& pair : df) {
			if (!tfidf_doc.count(pair.first)) {
				tfidf_doc.insert({ pair.first,0 });
			}

		}
		tfidf.push_back(tfidf_doc);
		cout << "calculated TF-IDF --- " << i << "\n";
		
	}
	
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
			
	
		
	cout << "TF and DF --- " << "\n";
	

	// Calculate TF-IDF
	int documents_number = 2393;

	cout << "----Calculation TF-IDF---- \n";
	
		
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
		
		cout << "calculated TF-IDF --- " <<  "\n";

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
		transform(word.begin(), word.end(), word.begin(),::tolower);
		words.push_back(word);
	}
	return words;
}

// Function to filter words based on stop-list
static string filterWords(const vector<string>& words, const unordered_set<string>& dictionary, const unordered_set<string>& stopList, const int& maxWords) {
	vector<string> filteredWords;
	int iW = 0;
	for (const auto& word : words) {
		if (isEnglishWord(word, dictionary) && stopList.find(word) == stopList.end() && iW < maxWords) {
			filteredWords.push_back(word);
			++iW;
		}
	}
	ostringstream result;
	for (size_t i = 0; i < filteredWords.size(); ++i) {
		if (i > 0) result << " ";
		result << filteredWords[i];
	}
	return result.str();
}


int main() {

	int scenario = 0;
	cout << "Chose scenario:" << "\n";
	cout << "1 -  Get webpages by URL and store for dataset preparation" << "\n";
	cout << "2 - Clean webpages - remove stopwords, stem words and store for further calculations" << "\n";
	cout << "3 - Calculate TFIDF values and store as dataset csv file. Prepare data for training and testing" << "\n";
	cout << "4 - Create ML models: Linear SVM, Softmax Regresion, Decision Tree and Random Forest" << "\n";
	
	
	cin >> scenario;

	switch (scenario) {
	case 0:
		cout << "Scenario is not chosen" << "\n";
		break;

	case 1:
	{
		vector<Record> records;
		unordered_set<string> stopWords;

		string inputCSV = "URL_list.csv";
		string outputCSV = "URL_webpages.csv";

		string text = "";

		readCSV(inputCSV, records);

		ofstream file(outputCSV);
		file << "id,url,label,responseCode,text\n";

		for (auto& record : records) {

			string webPage = "webpage.html";
			cout << "Web page URL: " << record.url << "\n";

			record.responseCode = fetchURL(record.url, webPage);
			cout << "Response code  " << record.responseCode << "\n";

			string cleanedHTML = "";
			int langCode = 0;
			if (record.responseCode == 200) {

				cleanHTML(webPage, cleanedHTML, langCode);
				if (langCode == 1) {
					record.responseCode = 999;
				}
			}
			else {
				cout << record.url << " Unreachable " << "\n";
				cleanedHTML = "";

			}

			file << record.id << "," << record.url << "," << record.label << "," << record.responseCode << "," << cleanedHTML << "\n";

		}

		file.close();

		cout << "CSV file updated successfully!" << "\n";
		break;
	}
	case 2:
	{
		unordered_set<string> stopWords;
		// Load stop words from file

		loadStopWords("stopwords_extended.txt", stopWords);

		unordered_set<string> dictionary;
		// Load english words from file
		loadStopWords("words.txt", dictionary);

		string inputCSV = "URL_webpages.csv";

		ifstream file(inputCSV);
		string line, url, label, text;
		string id, responseCode;

		struct RecordCl {
			string id;
			string url;
			string label;
			string responseCode;
			string text;
		};

		vector<RecordCl> recordsCl;

		set<string> tokens;

		int maxWords = 50; // Number of first words

		if (!file.is_open()) {
			cerr << "Error opening file: " << inputCSV << "\n";
		}

		// Skip the header
		getline(file, line);

		// Read each line from the file
		while (getline(file, line)) {
			stringstream ss(line);

			getline(ss, id, ',');
			getline(ss, url, ',');
			getline(ss, label, ',');
			getline(ss, responseCode, ',');

			getline(ss, text);

			recordsCl.push_back({ id, url, label, responseCode, text });
		}
		file.close();

		for (auto& record : recordsCl) {
			if ((record.responseCode == "200")) {

				vector<string> words = splitToWords(record.text);

				// Remove stop words, check if words is in dictionary

				string cleanedHTMLString = filterWords(words, dictionary, stopWords, maxWords);

				vector<string> cleanWords = splitToWords(cleanedHTMLString);

				for (auto& word : cleanWords) {

					tokens.insert(word);
				}

				string stemText = stemfile(cleanedHTMLString);

				record.text = stemText;


				if (stemText.empty()) {
					record.responseCode = "777";
				}
			}
		}

		ofstream fileTokens("tokens.txt");
		for (auto& token : tokens) {
			fileTokens << token << "\n";
		}
		fileTokens.close();

		ofstream fileCl("URL_webpages_tokens.csv");

		fileCl << "id,url,label,responseCode,text\n";

		for (auto& record : recordsCl) {
			fileCl << record.id << "," << record.url << "," << record.label << "," << record.responseCode << "," << record.text << "\n";
		}
		cout << "Webpges cleaned and prepared for datasets \n";

		break;
	}

	case 3:
	{
		string line, id, url, label, responseCode, text;
		vector<string> documents;
		vector<string> ids;
		vector<string> words;
		vector<string> labels;

		string inputFile = "URL_webpages_tokens.csv";

		string outputFile = "URL_webpages_tfidf.csv";
		ifstream infile(inputFile);

		if (!infile.is_open()) {
			cerr << "Error opening file: " << inputFile << "\n";
		}

		// Skip the header
		getline(infile, line);
		// Read the input CSV file

		cout << inputFile << endl;
		// Read the input CSV file
		while (getline(infile, line)) {
			stringstream ss(line);
			getline(ss, id, ',');
			getline(ss, url, ',');
			getline(ss, label, ',');
			getline(ss, responseCode, ',');
			getline(ss, text, ',');
			if (responseCode == "200" || (responseCode == "999")) {
				documents.push_back(text);
				ids.push_back(id);
				labels.push_back(label);
				cout << url << endl;
			}
		}

		vector<map<string, double>> tfidf;
		map<string, int> df;
		calculateTFIDF(documents, tfidf, df);


		ofstream outfile2(outputFile);
		outfile2 << "id,label";
		for (const auto& dfItem : df) {
			outfile2 << "," << dfItem.first;
		}
		outfile2 << endl;

		for (size_t i = 0; i < tfidf.size(); ++i) {
			outfile2 << ids[i] << "," << labels[i];
			for (const auto& pair : tfidf[i]) {
				outfile2 << "," << pair.second;
			}
			outfile2 << "\n";
		}
		outfile2.close();
		cout << "TF_IDF vectors  have been stored in tf-idf_vector.csv" << "\n";

		// prepare datasets for train and test
		int amount_train = tfidf.size() - 500;
		int t_amount = df.size();

		ofstream data_train("URL_train_data.csv");
		for (size_t i = 0; i < amount_train; ++i) {
			int j = 1;
			for (const auto& pair : tfidf[i]) {
				data_train << pair.second;
				if (j < t_amount) {
					data_train << ",";
					++j;
				}

			}
			data_train << "\n";
		}
		data_train.close();

		ofstream label_train("URL_train_label.csv");
		for (size_t i = 0; i < amount_train; ++i) {
			label_train << labels[i] << "\n";
		}
		label_train.close();

		ofstream data_test("URL_test_data.csv");
		for (size_t i = amount_train; i < tfidf.size(); ++i) {
			int j = 1;
			for (const auto& pair : tfidf[i]) {
				data_test << pair.second;
				if (j < t_amount) {
					data_test << ",";
					++j;
				}
			}
			data_test << "\n";

		}
		data_test.close();

		ofstream label_test("URL_test_label.csv");
		for (size_t i = amount_train; i < tfidf.size(); ++i) {
			label_test << labels[i] << "\n";
		}
		label_test.close();


		cout << "TF_IDF vectors  have been split and stored" << "\n";

		break;
	}

	case 4:
	{
		// Load Dataset for training and testing 
		arma::mat dataset;
		mlpack::data::Load("URL_train_data.csv", dataset, true);

		arma::Row<size_t> labels;
		mlpack::data::Load("URL_train_label.csv", labels, true);

		arma::Row<size_t> predictions;

		arma::mat testDataset;
		mlpack::data::Load("URL_test_data.csv", testDataset, true);

		arma::Row<size_t> testLabels;
		mlpack::data::Load("URL_test_label.csv", testLabels, true);

		arma::Row<size_t> testPredictions;

		arma::mat testProbabilities;

		double trainAccuracy = 0.0, testAccuracy = 0.0;

		std::cout << "\n";

		const size_t labelNumber = 7;
		// Create SVM model and test


		std::cout << "---- LinearSVM model ---- " << "\n";


		mlpack::LinearSVM svm; // Create model

		svm.Lambda() = 0.00005;
		svm.Delta() = 0.01;

		svm.Train(dataset, labels, labelNumber); // Train model

		mlpack::data::Save("model-svm_50.bin", "svm", svm, true);
		mlpack::data::Save("model-svm_50.json", "svm", svm, true);
		mlpack::data::Save("model-svm_50.xml", "svm", svm, true);


		svm.Classify(dataset, predictions); // Check training

		trainAccuracy = 100.0 *
			((double)arma::accu(predictions == labels)) / labels.n_elem;
		std::cout << "Accuracy of model on training data: " << trainAccuracy << "\%."
			<< std::endl;

		svm.Classify(testDataset, testPredictions, testProbabilities); // Classify points



		testAccuracy = 100.0 *
			((double)arma::accu(testPredictions == testLabels)) / testLabels.n_elem;
		std::cout << "Accuracy of model on test data:     " << testAccuracy << "\%."
			<< std::endl;
		std::cout << "\n";



		// Create SoftMax model and test

		std::cout << "---- SoftmaxRegression model ---- " << "\n";

		mlpack::SoftmaxRegression sr;          // Step 1: create model.
		sr.Train(dataset, labels, labelNumber);          // Step 2: train model.

		// Save the model to disk.
		mlpack::data::Save("model-sr_50.bin", "sr", sr, true);
		mlpack::data::Save("model-sr_50.xml", "sr", sr, true);
		mlpack::data::Save("model-sr_50.json", "sr", sr, true);

		sr.Classify(dataset, predictions);		// Check training
		trainAccuracy = 100.0 *
			((double)arma::accu(predictions == labels)) / labels.n_elem;
		std::cout << "Accuracy of model on training data: " << trainAccuracy << "\%."
			<< std::endl;

		sr.Classify(testDataset, testPredictions, testProbabilities); // Testing model

		testAccuracy = 100.0 *
			((double)arma::accu(testPredictions == testLabels)) / testLabels.n_elem;
		std::cout << "Accuracy of model on test data:     " << testAccuracy << "\%."
			<< std::endl;
		std::cout << "\n";




		// Create DecisionTree model and test

		std::cout << "---- Decision Tree model ---- " << "\n";


		mlpack::DecisionTree tree; // Create model

		mlpack::data::DatasetInfo info;

		//tree.Train(dataset, labels, labelNumber, 10 /* minimum leaf size */); // Model 200 tokens
		//tree.Train(dataset, labels, labelNumber, 7 /* minimum leaf size */); // Model 150 tokens
		//tree.Train(dataset, labels, labelNumber, 6 /* minimum leaf size */); // Model 100 tokens
		tree.Train(dataset, labels, labelNumber, 5 /* minimum leaf size */); // Model 50 tokens

		mlpack::data::Save("model-tree_50.bin", "tree", tree, true);

		mlpack::data::Save("model-tree_50.json", "tree", tree, true);


		tree.Classify(dataset, predictions); // Check training

		trainAccuracy = 100.0 *
			((double)arma::accu(predictions == labels)) / labels.n_elem;
		std::cout << "Accuracy of model on training data: " << trainAccuracy << "\%."
			<< std::endl;

		tree.Classify(testDataset, testPredictions, testProbabilities); // Classify points


		testAccuracy = 100.0 *
			((double)arma::accu(testPredictions == testLabels)) / testLabels.n_elem;
		std::cout << "Accuracy of model on test data:     " << testAccuracy << "\%."
			<< std::endl;
		std::cout << "\n";




		// Create Random Forest  model and test

		std::cout << "---- 		Random Forest  model ---- " << "\n";

		mlpack::RandomForest rf;		// Create model

		//rf.Train(dataset, labels, labelNumber, 25, 10);			// Train model - tokens 200
		//rf.Train(dataset, labels, labelNumber, 20, 10);			// Train model - tokens 150
		//rf.Train(dataset, labels, labelNumber, 20, 5);			// Train model - tokens 100
		rf.Train(dataset, labels, labelNumber, 20, 5);			// Train model - tokens 50


		// Save the model to disk with the name "rf".
		mlpack::data::Save("model-rf_50.bin", "rf", rf, true);

		mlpack::data::Save("model-rf_50.json", "rf", rf, true);




		rf.Classify(dataset, predictions);		// Check training
		trainAccuracy = 100.0 *
			((double)arma::accu(predictions == labels)) / labels.n_elem;
		std::cout << "Accuracy of model on training data: " << trainAccuracy << "\%."
			<< std::endl;

		rf.Classify(testDataset, testPredictions, testProbabilities); // Check testing

		testAccuracy = 100.0 *
			((double)arma::accu(testPredictions == testLabels)) / testLabels.n_elem;
		std::cout << "Accuracy of model on test data:     " << testAccuracy << "\%."
			<< std::endl;
		std::cout << "\n";


		break;
	}


	}
	
	return 0;
}


