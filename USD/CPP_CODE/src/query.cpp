#include "include.h"
#include "query.h"
using namespace std;

void query::write_assignment(ofstream& outfile) {
	int i;
	for(i=0; i<this->length-1; i++) {
		outfile << this->labels[this->assign[i]]<<"\t";
	}
	outfile << this->labels[this->assign[this->length - 1]]<<"\n";
}

void query::print_contents() {
	int i;
	for(i=0; i<this->length-1; i++) {
		cout<<this->words[i]<<"\t";
	}
	cout << this->words[this->length - 1]<<"\n";
	for(i=0; i<this->my_label_size-1; i++) {
		cout<<this->labels[i]<<"\t";
	}
	cout << this->labels[this->my_label_size - 1]<<"\n";
	for(i=0; i<this->length-1; i++) {
		cout<<this->assign[i]<<"\t";
	}
	cout << this->assign[this->length - 1]<<"\n";
}


query::query(int total_label_size, std::vector<int> words, std::vector<int> labels, int null_tag):total_label_size(total_label_size), words(words), labels(labels), null_tag(null_tag) {
	int i, index;
	this->length = words.size();

	if(this->null_tag > 0) {
		this->labels.push_back(this->total_label_size);
	}
	
	this->my_label_size = this->labels.size();

	this->assign.resize(this->length, -1);
	this->characteristic_count.resize(this->my_label_size, 0);
	this->characteristic_weights.resize(this->my_label_size);
	for(i=0; i<this->length; i++){
		index = rand()%this->my_label_size;
		this->assign[i] = index;
		this->characteristic_count[index]++;
	}
}

query::query() {
}

query::~query() {
}
