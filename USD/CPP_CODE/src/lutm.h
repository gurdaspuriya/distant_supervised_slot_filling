#ifndef LUTM_H_
#define LUTM_H_

#include "include.h"
#include "query.h"

class lutm
{
	private:
		int vocab_size;
		int num_characteristics;
		int num_threads;
		real delta;
		int null_tag;
		int seed;
		std::vector<query>& queries;

		std::vector<std::vector<int> > characteristic_word_count;
		std::vector<std::vector<real> > trained_psi;
		std::vector<int> characteristic_count;
		std::default_random_engine engine;
		std::discrete_distribution<int> distribution;

		void InitializeCounts();
		void PrintProgress (int, int);

	public:
		lutm(int, int, std::vector<query> &, int, real, int, int);
		lutm();
		void Inference(int);
		std::vector<std::vector<real> > GetCharacteristicWordDist();
		real Perplexity(int);
		void write_dist(std::ofstream&);
		void read_dist(std::ifstream&);
		virtual ~lutm();
};

#endif /* LUTM_H_ */