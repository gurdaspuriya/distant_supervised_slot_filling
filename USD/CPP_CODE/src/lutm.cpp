#include "include.h"
#include "lutm.h"
using namespace std;

void lutm::PrintProgress (int done, int total)
{
	real percentage = (real)done/total;
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
}


void lutm::write_dist(ofstream& outfile) {
	int i, j;
	vector<vector<real> > psi = GetCharacteristicWordDist();

	for(i=0; i<psi.size(); i++){
		for(j=0; j<this->vocab_size - 1; j++) {
			outfile << psi[i][j]<<"\t";
		}
		outfile << psi[i][this->vocab_size-1]<<"\n";
	}
}


void lutm::read_dist(ifstream& infile) {
	int i, j;


	if(this->null_tag == 0) {
		this->trained_psi.resize(this->num_characteristics, vector<real>(this->vocab_size));
	}

	else {
		this->trained_psi.resize(this->num_characteristics+1, vector<real>(this->vocab_size));
	}

	for(i=0; i<this->trained_psi.size(); i++){
		for(j=0; j<this->vocab_size; j++) {
			infile >> trained_psi[i][j];
		}
	}
}

void lutm::Inference(int train) {

	int i, j, k;
	int index;
	int current_characteristic, new_characteristic;
	real max_prob, temp_prob;
	query *q;
	int word;
	vector<int> indices(this->queries.size());

	for(i=0; i<this->queries.size(); i++) {
		indices[i] = i;
	}

	random_shuffle(indices.begin(), indices.end());


	for(i=0; i<indices.size(); i++) {
		index = indices[i];
		q = &(this->queries[index]);

		for(j=0; j<q->length; j++) {
			current_characteristic = q->assign[j];
			word = q->words[j];
			if(train == 1) {
				temp_prob = (this->characteristic_word_count[q->labels[current_characteristic]][word] - 1 + this->delta)/(this->characteristic_count[q->labels[current_characteristic]] - 1 + this->vocab_size * this->delta);
			}
			else {
				temp_prob = this->trained_psi[q->labels[current_characteristic]][word];
			}
			q->characteristic_weights[current_characteristic] = temp_prob;

			for(k=0; k<q->my_label_size; k++) {
				if(k == current_characteristic){
					continue;
				}

				if(train == 1) {
					temp_prob = (this->characteristic_word_count[q->labels[k]][word] + this->delta)/(this->characteristic_count[q->labels[k]] + this->vocab_size * this->delta);
				}
				else {
					temp_prob = this->trained_psi[q->labels[k]][word];
				}
				q->characteristic_weights[k] = temp_prob;
			}
			this->distribution = discrete_distribution<int>(q->characteristic_weights.begin(),q->characteristic_weights.end());
			new_characteristic = this->distribution(this->engine);

			if(current_characteristic != new_characteristic) {
				q->assign[j] = new_characteristic;
				q->characteristic_count[current_characteristic]--;
				q->characteristic_count[new_characteristic]++;

				if(train == 1){
					this->characteristic_word_count[q->labels[current_characteristic]][word]--;
					this->characteristic_count[q->labels[current_characteristic]]--;

					this->characteristic_word_count[q->labels[new_characteristic]][word]++;
					this->characteristic_count[q->labels[new_characteristic]]++;
				}

			}

		}
		if(i%1000 == 0){
			PrintProgress(i+1, indices.size());
		}
	}
	PrintProgress(indices.size(), indices.size());
	cout<<endl;
}

vector<vector<real> > lutm::GetCharacteristicWordDist(){
	int i,j;
	real denom;
	vector<vector<real> > psi(this->num_characteristics+this->null_tag, vector<real>(this->vocab_size));
	for(i=0; i<psi.size(); i++){
		denom = this->characteristic_count[i] + this->vocab_size * this->delta;
		for(j=0; j<this->vocab_size; j++) {
			psi[i][j] = (this->characteristic_word_count[i][j] + this->delta)/denom;
		}
	}
	return psi;
}

real lutm::Perplexity(int train) {
	int i, j, k;
	int cnt = 0;
	int word = 0;
	real perplexity = 0.0;
	real dot_product = 0.0;
	vector<vector<real> > psi;
	if (train == 1){
		psi = GetCharacteristicWordDist();
	}
	else {
		psi = this->trained_psi;
	}
	
	for(i=0; i<this->queries.size(); i++) {
		for(j=0; j<this->queries[i].length; j++) {
			word = this->queries[i].words[j];
			dot_product = 0.0;
			for(k=0; k<this->queries[i].my_label_size; k++) {
				dot_product += psi[this->queries[i].labels[k]][word] / queries[i].my_label_size;
			}
			perplexity -= log(dot_product);
		}
		cnt += this->queries[i].length;
	}
	return exp(perplexity/cnt);
}

void lutm::InitializeCounts() {
	int i, j;
	for(i=0; i<this->queries.size(); i++) {
		for(j=0; j<queries[i].assign.size(); j++) {
			this->characteristic_word_count[queries[i].labels[queries[i].assign[j]]][queries[i].words[j]] += 1;
			this->characteristic_count[queries[i].labels[queries[i].assign[j]]] += 1;
		}
	}
}


lutm::lutm(int vocab_size, int num_characteristics, vector<query> &queries, int num_threads, real delta, int null_tag, int seed):vocab_size(vocab_size),num_characteristics(num_characteristics),queries(queries),num_threads(num_threads),delta(delta), null_tag(null_tag), seed(seed) {
	int i;
	this->engine.seed(this->seed);


	if(this->null_tag == 0) {
		this->characteristic_count.resize(this->num_characteristics, 0);
		this->characteristic_word_count.resize(this->num_characteristics);

		for (i=0; i<this->num_characteristics; i++) {
			this->characteristic_word_count[i].resize(this->vocab_size, 0);
		}
	}

	else {
		this->characteristic_count.resize(this->num_characteristics+1, 0);
		this->characteristic_word_count.resize(this->num_characteristics+1);

		for (i=0; i<this->num_characteristics+1; i++) {
			this->characteristic_word_count[i].resize(this->vocab_size, 0);
		}
	}

	InitializeCounts();
}

lutm::~lutm() {
}
