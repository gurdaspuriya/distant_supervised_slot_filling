#include "include.h"
#include "hmm.h"
using namespace std;

void hmm::PrintProgress (int done, int total)
{
	real percentage = (real)done/total;
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
}


void hmm::write_characteristic_dist(ofstream& outfile) {
	int i, j;
	vector<vector<real> > psi = GetCharacteristicWordDist();

	for(i=0; i<psi.size(); i++){
		for(j=0; j<this->vocab_size - 1; j++) {
			outfile << psi[i][j]<<"\t";
		}
		outfile << psi[i][this->vocab_size-1]<<"\n";
	}
}


void hmm::write_transition_dist(ofstream& outfile) {
	int i, j;
	vector<vector<real> > upsilon = GetTransitionDist();

	for(i=0; i<upsilon.size(); i++){
		for(j=0; j<this->num_characteristics+this->null_tag - 1; j++) {
			outfile << upsilon[i][j]<<"\t";
		}
		outfile << upsilon[i][this->num_characteristics+this->null_tag-1]<<"\n";
	}
}


void hmm::read_characteristic_dist(ifstream& infile) {
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


void hmm::read_transition_dist(ifstream& infile) {
	int i, j;

	this->trained_upsilon.resize(this->num_characteristics+this->null_tag, vector<real>(this->num_characteristics+this->null_tag));

	for(i=0; i<this->trained_upsilon.size(); i++){
		for(j=0; j<this->num_characteristics+this->null_tag; j++) {
			infile >> trained_upsilon[i][j];
		}
	}
}

void hmm::Viterbi() {

	int i, j, k, l;
	int index;
	int current_characteristic, new_characteristic;
	real max_prob, temp_prob;
	int max_back, temp_back;
	real char_prob = 1.0;
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
		word = q->words[0];


		current_characteristic = q->assign[0];
		q->characteristic_count[current_characteristic]--;
		char_prob = 1.0/(float)q->length;

		for (k=0; k<q->my_label_size; k++) {
			q->viterbi_prob[0][k] = char_prob*this->trained_psi[q->labels[k]][word];
			q->viterbi_back[0][k] = 0;
		}

		for(j=1; j<q->length; j++) {
			word = q->words[j];
			current_characteristic = q->assign[j];
			q->characteristic_count[current_characteristic]--;

			for (k=0; k<q->my_label_size; k++) {
				max_prob = 0.0;
				temp_prob = 1.0;
				max_back = 0;
				for (l=0; l<q->my_label_size; l++) {
					temp_prob = q->viterbi_prob[j-1][l]*this->trained_upsilon[q->labels[l]][q->labels[k]]*this->trained_psi[q->labels[k]][word];
					if (temp_prob > max_prob) {
						max_prob = temp_prob;
						max_back = l;
					}
				}
				q->viterbi_prob[j][k] = max_prob;
				q->viterbi_back[j][k] = max_back;
			}
		}

		max_prob = q->viterbi_prob[q->length-1][0];
		q->assign[q->length - 1] = 0;
		max_back = q->viterbi_back[q->length-1][0];

		for (k=1; k<q->my_label_size; k++) {
			if (q->viterbi_prob[q->length-1][k] > max_prob) {
				max_prob = q->viterbi_prob[q->length-1][k];
				max_back = q->viterbi_back[q->length-1][k];
				q->assign[q->length - 1] = k;
			}
		}

		new_characteristic = q->assign[q->length - 1];
		q->characteristic_count[new_characteristic]++;

		for (j = q->length-2; j>=0; j--) {

			new_characteristic = max_back;
			q->characteristic_count[new_characteristic]++;
			q->assign[j] = max_back;
			temp_back = q->viterbi_back[j][max_back];
			max_back = temp_back;
		}
		if(i%1000 == 0){
			PrintProgress(i+1, indices.size());
		}
	}
	PrintProgress(indices.size(), indices.size());
	cout<<endl;
}

void hmm::Inference() {

	int i, j, k;
	int index;
	int current_characteristic, new_characteristic;
	real max_prob, temp_prob;
	real p1, p2;
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

			word = q->words[j];
			current_characteristic = q->assign[j];
			q->characteristic_count[current_characteristic]--;

			this->characteristic_word_count[q->labels[current_characteristic]][word] -= 1;
			this->characteristic_count[q->labels[current_characteristic]] -= 1;
			if (j < (q->length - 1)) {			
				this->transition_count[q->labels[current_characteristic]][q->labels[q->assign[j+1]]] -= 1;
				this->transition_denominator_count[q->labels[current_characteristic]] -= 1;
			}
			if (j > 0) {			
				this->transition_count[q->labels[q->assign[j-1]]][q->labels[current_characteristic]] -= 1;
				this->transition_denominator_count[q->labels[q->assign[j-1]]] -= 1;
			}


			for(k=0; k<q->my_label_size; k++) {
				if (j == 0) {
					p1 = 1.0/(float)q->length;
				}
				else{
					p1 = (this->transition_count[q->labels[q->assign[j-1]]][q->labels[q->assign[j]]] + this->zeta)/(this->transition_denominator_count[q->labels[q->assign[j-1]]] + (this->num_characteristics+this->null_tag) * this->zeta);
				}
				p2 = (this->characteristic_word_count[q->labels[k]][word] + this->delta)/(this->characteristic_count[q->labels[k]] + this->vocab_size * this->delta);
				temp_prob = p1*p2;
				q->characteristic_weights[k] = temp_prob;
			}

			this->distribution = discrete_distribution<int>(q->characteristic_weights.begin(),q->characteristic_weights.end());
			new_characteristic = this->distribution(this->engine);

			q->assign[j] = new_characteristic;
			q->characteristic_count[new_characteristic]++;

			this->characteristic_word_count[q->labels[new_characteristic]][word]++;
			this->characteristic_count[q->labels[new_characteristic]]++;


			if (j < (q->length - 1)) {			
				this->transition_count[q->labels[new_characteristic]][q->labels[q->assign[j+1]]] += 1;
				this->transition_denominator_count[q->labels[new_characteristic]] += 1;
			}
			if (j > 0) {			
				this->transition_count[q->labels[q->assign[j-1]]][q->labels[new_characteristic]] += 1;
				this->transition_denominator_count[q->labels[q->assign[j-1]]] += 1;
			}

		}
		if(i%1000 == 0){
			PrintProgress(i+1, indices.size());
		}
	}
	PrintProgress(indices.size(), indices.size());
	cout<<endl;
}

vector<vector<real> > hmm::GetCharacteristicWordDist(){
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

vector<vector<real> > hmm::GetTransitionDist(){
	int i,j;
	real denom;
	vector<vector<real> > upsilon(this->num_characteristics+this->null_tag, vector<real>(this->num_characteristics+this->null_tag));
	for(i=0; i<upsilon.size(); i++){
		denom = this->transition_denominator_count[i] + (this->num_characteristics+this->null_tag) * this->zeta;
		for(j=0; j<this->num_characteristics+this->null_tag; j++) {
			upsilon[i][j] = (this->transition_count[i][j] + this->zeta)/denom;
		}
	}
	return upsilon;
}

real hmm::Perplexity(int train) {
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

void hmm::InitializeCounts() {
	int i, j;
	real denom;
	for(i=0; i<this->queries.size(); i++) {
		for(j=0; j<queries[i].assign.size(); j++) {
			this->characteristic_word_count[queries[i].labels[queries[i].assign[j]]][queries[i].words[j]] += 1;
			this->characteristic_count[queries[i].labels[queries[i].assign[j]]] += 1;
		}
		for(j=0; j<queries[i].assign.size()-1; j++) {
			this->transition_count[queries[i].labels[queries[i].assign[j]]][queries[i].labels[queries[i].assign[j+1]]] += 1;
			this->transition_denominator_count[queries[i].labels[queries[i].assign[j]]] += 1;
		}
	}
}


hmm::hmm(int vocab_size, int num_characteristics, vector<query> &queries, int num_threads, real delta, real zeta, int null_tag, int seed):vocab_size(vocab_size),num_characteristics(num_characteristics),queries(queries),num_threads(num_threads),delta(delta),zeta(zeta), null_tag(null_tag), seed(seed) {
	int i;
	this->engine.seed(this->seed);

	this->characteristic_count.resize(this->num_characteristics+this->null_tag, 0);
	this->characteristic_word_count.resize(this->num_characteristics+this->null_tag);
	this->transition_denominator_count.resize(this->num_characteristics+this->null_tag, 0);
	this->transition_count.resize(this->num_characteristics+this->null_tag);

	for (i=0; i<this->num_characteristics+this->null_tag; i++) {
		this->characteristic_word_count[i].resize(this->vocab_size, 0);
		this->transition_count[i].resize(this->num_characteristics+this->null_tag, 0);
	}

	InitializeCounts();
}

hmm::~hmm() {
}
