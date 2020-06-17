#ifndef COSLUTM_H_
#define COSLUTM_H_

#include "include.h"
#include "query.h"

class coslutm
{
	private:
		int vocab_size;
		int num_characteristics;
		int num_classes;
		int num_topics;
		int num_threads;
		int null_tag;
		int seed;
		real alpha, beta, gamma, delta;
		std::vector<query>& queries;

		std::vector<real> trained_phi;
		std::vector<std::vector<real> > trained_psi;
		std::vector<std::vector<real> > trained_chi;
		std::vector<real> temp_query_topic_prob;
		std::vector<real> temp_characteristic_topic_prob;
		std::vector<std::vector<int> > characteristic_word_count;
		std::vector<int> characteristic_count;
		std::vector<std::vector<int> > topic_characteristic_count;
		std::vector<int> topic_query_count;
		std::vector<int> topic_count;
		std::default_random_engine engine;
		std::discrete_distribution<int> distribution;

		void InitializeCounts();
		void PrintProgress (int, int);

	public:
		coslutm(int, int, int, std::vector<query> &, int, real, real, real, real, int, int);
		coslutm();
		void Inference(int);
		std::vector<real> GetTopicDist();
		std::vector<std::vector<real> > GetCharacteristicWordDist();
		std::vector<std::vector<real> > GetTopicCharacteristicDist();
		real Perplexity(int);
		void write_characteristic_dist(std::ofstream&);
		void read_characteristic_dist(std::ifstream&);
		void write_topic_dist(std::ofstream&);
		void write_topic_query_dist(std::ofstream&);
		void read_topic_dist(std::ifstream&);
		void read_topic_query_dist(std::ifstream&);
		virtual ~coslutm();
};

#endif /* COSLUTM_H_ */