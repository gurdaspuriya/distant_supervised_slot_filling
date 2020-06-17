#include "include.h"
#include "coslutm.h"
using namespace std;

void coslutm::PrintProgress (int done, int total)
{
	real percentage = (real)done/total;
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
}


void coslutm::write_characteristic_dist(ofstream& outfile) {
	int i, j;
	vector<vector<real> > psi = GetCharacteristicWordDist();

	for(i=0; i<psi.size(); i++){
		for(j=0; j<this->vocab_size - 1; j++) {
			outfile << psi[i][j]<<"\t";
		}
		outfile << psi[i][this->vocab_size-1]<<"\n";
	}
}


void coslutm::write_topic_dist(ofstream& outfile) {
	int i, j;
	vector<vector<real> > chi = GetTopicCharacteristicDist();

	for(i=0; i<this->num_topics; i++){
		for(j=0; j<2*this->num_characteristics - 1; j++) {
			outfile << chi[i][j]<<"\t";
		}
		outfile << chi[i][2*this->num_characteristics-1]<<"\n";
	}
}


void coslutm::write_topic_query_dist(ofstream& outfile) {
	int i, j;
	vector<real> phi = GetTopicDist();

	for(i=0; i<this->num_topics - 1; i++){
		outfile << phi[i]<<"\t";
	}
	outfile << phi[this->num_topics-1]<<"\n";
}


void coslutm::read_characteristic_dist(ifstream& infile) {
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

void coslutm::read_topic_dist(ifstream& infile) {
	int i, j;
	this->trained_chi.resize(this->num_topics, vector<real>(2*this->num_characteristics));

	for(i=0; i<this->num_topics; i++){
		for(j=0; j<2*this->num_characteristics; j++) {
			infile >> trained_chi[i][j];
		}
	}
}

void coslutm::read_topic_query_dist(ifstream& infile) {
	int i, j;
	this->trained_phi.resize(this->num_topics);

	for(i=0; i<this->num_topics; i++){
		infile >> trained_phi[i];
	}
}

void coslutm::Inference(int train) {

	int i, j, k, l;
	int index, new_index;
	int current_characteristic, new_characteristic;
	int current_topic, new_topic;
	real max_prob, temp_prob;
	real p1, p2, p3, p4, p5;
	real temp_p4, temp_p5;
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

		//First find the topic of the query
		current_topic = q->my_topic;

		if(train == 1) {
			this->topic_query_count[current_topic]--;
			this->topic_count[current_topic] -= (q->candidate_label_size - this->null_tag);
			for (j=0; j<q->candidate_label_size-this->null_tag; j++) {

				if (q->characteristic_count[j] > 0){
					this->topic_characteristic_count[current_topic][q->labels[j]]--;
				}
				else {
					this->topic_characteristic_count[current_topic][q->labels[j] + this->num_characteristics]--;
				}

			}
		}

		//Initialize temporary counts, will ease the computation later
		if(train == 1) {
			for(l=0; l<this->num_topics; l++) {
				this->temp_query_topic_prob[l] = this->alpha + this->topic_query_count[l];
				this->temp_characteristic_topic_prob[l] = 1.0;
				for (k=0; k<q->candidate_label_size-this->null_tag; k++) {
					if (q->characteristic_count[k] > 0){
						this->temp_characteristic_topic_prob[l] *= ((this->beta + this->topic_characteristic_count[l][q->labels[k]])/(2*this->beta*this->num_characteristics + this->topic_count[l] + k));
					}
					else {
						this->temp_characteristic_topic_prob[l] *= ((this->beta + this->topic_characteristic_count[l][q->labels[k] + this->num_characteristics])/(2*this->beta*this->num_characteristics + this->topic_count[l] + k));
					}
				}
			}
		}

		else {
			for(l=0; l<this->num_topics; l++) {
				this->temp_query_topic_prob[l] = this->trained_phi[l];
				this->temp_characteristic_topic_prob[l] = 1.0;
				for (k=0; k<q->candidate_label_size-this->null_tag; k++) {
					if (q->characteristic_count[k] > 0){
						this->temp_characteristic_topic_prob[l] *= this->trained_chi[l][q->labels[k]];
					}
					else {
						this->temp_characteristic_topic_prob[l] *= this->trained_chi[l][q->labels[k] + this->num_characteristics];
					}
				}
			}
		}
		temp_p5 = this->gamma/(1 - this->gamma);

		//Find the characteristic for every word in the query and the corresponding topic for the query
		for(j=0; j<q->length; j++) {
			word = q->words[j];
			current_characteristic = q->characteristic_assign[j];
			q->characteristic_count[current_characteristic]--;

			if (this->null_tag > 0 && q->characteristic_count[q->candidate_label_size-1] == 0) {
				q->is_null_assigned = 0;
			}

			if(train == 1) {
				this->characteristic_word_count[q->labels[current_characteristic]][word]--;
				this->characteristic_count[q->labels[current_characteristic]]--;
			}

			if((q->characteristic_count[current_characteristic] == 0) && (this->null_tag == 0 || current_characteristic != q->candidate_label_size-1)) {
				q->actual_label_size -= 1;
				if(train == 1) {
			
					for(l=0; l<this->num_topics; l++) {
						this->temp_characteristic_topic_prob[l] /= (this->beta + this->topic_characteristic_count[l][q->labels[current_characteristic]]);
						this->temp_characteristic_topic_prob[l] *= (this->beta + this->topic_characteristic_count[l][q->labels[current_characteristic] + this->num_characteristics]);
					}

				}
				else {

					for(l=0; l<this->num_topics; l++) {
						this->temp_characteristic_topic_prob[l] /= this->trained_chi[l][q->labels[current_characteristic]];
						this->temp_characteristic_topic_prob[l] *= this->trained_chi[l][q->labels[current_characteristic] + this->num_characteristics];
					}
				}
			}

			temp_p4 = (real)(q->actual_label_size + q->is_null_assigned)/(q->actual_label_size+1 + q->is_null_assigned);
			if(temp_p4 < EPSILON) {
				temp_p4 = 1.0;
			}

			for(k=0; k<q->candidate_label_size; k++) {
				for(l=0; l<this->num_topics; l++) {
					if(train == 1) {
						p1 = this->temp_query_topic_prob[l];
						p2 = this->temp_characteristic_topic_prob[l];
						p3 = (this->characteristic_word_count[q->labels[k]][word] + this->delta)/(this->characteristic_count[q->labels[k]] + this->vocab_size * this->delta);
						p4 = 1.0;
						p5 = 1.0;
						if ((q->characteristic_count[k] == 0)&& (this->null_tag == 0 || k != q->candidate_label_size-1)) {
							p2 *= ((this->beta + this->topic_characteristic_count[l][q->labels[k]])/(this->beta + this->topic_characteristic_count[l][q->labels[k] + this->num_characteristics]));
							p4 = temp_p4;
							p5 = temp_p5;
						}
						else if (q->characteristic_count[k] == 0) {
							p4 = temp_p4;
						}
					}
					else {
						p1 = this->temp_query_topic_prob[l];
						p2 = this->temp_characteristic_topic_prob[l];
						p3 = this->trained_psi[q->labels[k]][word];
						p4 = 1.0;
						p5 = 1.0;
						if ((q->characteristic_count[k] == 0)&& (this->null_tag == 0 || k != q->candidate_label_size-1)) {
							p2 *= this->trained_chi[l][q->labels[k]]/this->trained_chi[l][q->labels[k] + this->num_characteristics];
							p4 = temp_p4;
							p5 = temp_p5;
						}
						else if (q->characteristic_count[k] == 0) {
							p4 = temp_p4;
						}
					}
					q->all_weights[q->characteristic_topic_map[k][l]] = p1*p2*p3*p4*p5;
				}
			}

			this->distribution = discrete_distribution<int>(q->all_weights.begin(),q->all_weights.end());
			new_index = this->distribution(this->engine);
			new_characteristic = q->all_characteristic_flag[new_index];
			new_topic = q->all_topic_flag[new_index];



			q->my_topic = new_topic;
			q->characteristic_assign[j] = new_characteristic;
			q->characteristic_count[new_characteristic]++;

			if (this->null_tag > 0 && q->characteristic_count[q->candidate_label_size-1] > 0) {
				q->is_null_assigned = 1;
			}

			if ((q->characteristic_count[new_characteristic] == 1) && (this->null_tag == 0 || new_characteristic != q->candidate_label_size-1)) {

				if(train == 1) {
			
					for(l=0; l<this->num_topics; l++) {
						this->temp_characteristic_topic_prob[l] *= ((this->beta + this->topic_characteristic_count[l][q->labels[new_characteristic]])/(this->beta + this->topic_characteristic_count[l][q->labels[new_characteristic] + this->num_characteristics]));
					}

				}
				else {
			
					for(l=0; l<this->num_topics; l++) {
						this->temp_characteristic_topic_prob[l] *= this->trained_chi[l][q->labels[new_characteristic]]/this->trained_chi[l][q->labels[new_characteristic] + this->num_characteristics];
					}

				}
				q->actual_label_size++;
			}

			if(train == 1){
				this->characteristic_word_count[q->labels[new_characteristic]][word]++;
				this->characteristic_count[q->labels[new_characteristic]]++;
			}

		}


		new_topic = q->my_topic;

		if(train == 1) {
			this->topic_query_count[new_topic]++;
			this->topic_count[new_topic] += (q->candidate_label_size - this->null_tag);

			for (j=0; j<q->candidate_label_size-this->null_tag; j++) {

				if (q->characteristic_count[j] > 0){
					this->topic_characteristic_count[new_topic][q->labels[j]]++;
				}
				else {
					this->topic_characteristic_count[new_topic][q->labels[j] + this->num_characteristics]++;
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

vector<vector<real> > coslutm::GetCharacteristicWordDist(){
	int i,j;
	real denom;
	vector<vector<real> > psi(this->num_characteristics, vector<real>(this->vocab_size));
	if(this->null_tag > 0) {
		psi.resize(this->num_characteristics + 1, vector<real>(this->vocab_size));
	}
	for(i=0; i<this->num_characteristics; i++){
		denom = this->characteristic_count[i] + this->vocab_size * this->delta;
		for(j=0; j<this->vocab_size; j++) {
			psi[i][j] = (this->characteristic_word_count[i][j] + this->delta)/denom;
		}
	}

	if(this->null_tag > 0) {
		denom = this->characteristic_count[this->num_characteristics] + this->vocab_size * this->delta;
		for(j=0; j<this->vocab_size; j++) {
			psi[this->num_characteristics][j] = (this->characteristic_word_count[this->num_characteristics][j] + this->delta)/denom;
		}
	}
	return psi;
}

vector<vector<real> > coslutm::GetTopicCharacteristicDist(){
	int i,j;
	real denom;
	vector<vector<real> > chi(this->num_topics, vector<real>(2*this->num_characteristics));
	for(i=0; i<this->num_topics; i++){
		denom = this->topic_count[i] + 2*this->num_characteristics * this->beta;
		for(j=0; j<2*this->num_characteristics; j++) {
			chi[i][j] = (this->topic_characteristic_count[i][j] + this->beta)/denom;
		}
	}
	return chi;
}

vector<real> coslutm::GetTopicDist(){
	int i;
	real denom;
	vector<real> phi(this->num_topics);
	denom = this->queries.size() + this->num_topics * this->alpha;
	for(i=0; i<this->num_topics; i++){
		phi[i] = (this->topic_query_count[i] + this->alpha)/denom;
	}
	return phi;
}

real coslutm::Perplexity(int train) {
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
			for(k=0; k<this->queries[i].candidate_label_size; k++) {
				dot_product += psi[this->queries[i].labels[k]][word] / queries[i].candidate_label_size;
			}
			perplexity -= log(dot_product);
		}
		cnt += this->queries[i].length;
	}
	return exp(perplexity/cnt);
}

void coslutm::InitializeCounts() {
	int i, j;
	for(i=0; i<this->queries.size(); i++) {
		for(j=0; j<queries[i].characteristic_assign.size(); j++) {
			this->characteristic_word_count[queries[i].labels[queries[i].characteristic_assign[j]]][queries[i].words[j]] += 1;
			this->characteristic_count[queries[i].labels[queries[i].characteristic_assign[j]]] += 1;
		}

		this->topic_query_count[queries[i].my_topic] += 1;
		this->topic_count[queries[i].my_topic] += (queries[i].candidate_label_size - this->null_tag);

		for(j=0; j<queries[i].candidate_label_size - this->null_tag; j++) {

			if (queries[i].characteristic_count[j] > 0){
				this->topic_characteristic_count[queries[i].my_topic][queries[i].labels[j]] += 1;
			}
			else {
				this->topic_characteristic_count[queries[i].my_topic][queries[i].labels[j] + this->num_characteristics] += 1;
			}
		}
	}

	// for(i=0; i<this->num_topics; i++) {
	// 	int temp = 0;
	// 	for (j=0; j<2*this->num_characteristics; j++) {
	// 		temp += this->topic_characteristic_count[i][j];
	// 	}
	// 	cout<<this->topic_count[i]<<" "<<temp<<endl;
	// 	cout<<this->topic_query_count[i]<<endl;
	// }
}


coslutm::coslutm(int vocab_size, int num_characteristics, int num_topics, vector<query> &queries, int num_threads, real alpha, real beta, real gamma, real delta, int null_tag, int seed):vocab_size(vocab_size),num_characteristics(num_characteristics),num_topics(num_topics),queries(queries),num_threads(num_threads),alpha(alpha),beta(beta),gamma(gamma),delta(delta), null_tag(null_tag), seed(seed) {
	int i;
	this->engine.seed(this->seed);

	this->topic_query_count.resize(this->num_topics, 0);
	this->topic_count.resize(this->num_topics, 0);
	this->topic_characteristic_count.resize(this->num_topics);

	for (i=0; i<this->num_topics; i++) {
		this->topic_characteristic_count[i].resize(2*this->num_characteristics, 0);
	}


	this->temp_query_topic_prob.resize(this->num_topics, 0.0);
	this->temp_characteristic_topic_prob.resize(this->num_topics, 0.0);

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

coslutm::~coslutm() {
}
