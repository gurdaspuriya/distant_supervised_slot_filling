#ifndef LUTM_H_
#define LUTM_H_

#include "include.h"
#include "query.h"

class hmm
{
	private:
		int vocab_size;
		int num_characteristics;
		int num_threads;
		real delta;
		real zeta;
		int null_tag;
		int seed;
		std::vector<query>& queries;

		std::vector<std::vector<int> > characteristic_word_count;
		std::vector<std::vector<int> > transition_count;
		std::vector<int> transition_denominator_count;
		std::vector<std::vector<real> > trained_psi;
		std::vector<std::vector<real> > trained_upsilon;
		std::vector<int> characteristic_count;
		std::default_random_engine engine;
		std::discrete_distribution<int> distribution;

		void InitializeCounts();
		void PrintProgress (int, int);

	public:
		hmm(int, int, std::vector<query> &, int, real, real, int, int);
		hmm();
		void Inference();
		void Viterbi();
		std::vector<std::vector<real> > GetCharacteristicWordDist();
		std::vector<std::vector<real> > GetTransitionDist();
		real Perplexity(int);
		void write_characteristic_dist(std::ofstream&);
		void read_characteristic_dist(std::ifstream&);
		void write_transition_dist(std::ofstream&);
		void read_transition_dist(std::ifstream&);
		virtual ~hmm();
};

#endif /* LUTM_H_ */