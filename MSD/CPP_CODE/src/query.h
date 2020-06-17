#ifndef QUERY_H_
#define QUERY_H_

#include "include.h"

class query
{
	public:
		int length;
		int my_label_size;
		int total_label_size;
		int null_tag;
		int is_null_assigned;
		std::vector<int> labels;
		std::vector<int> words;
		std::vector<int> assign;
		std::vector<int> characteristic_count;
		std::vector<real> characteristic_weights;
		std::vector<std::vector<real> > viterbi_prob;
		std::vector<std::vector<int> > viterbi_back;
		query(int, std::vector<int>, std::vector<int>, int);
		query();
		void write_assignment(std::ofstream&);
		void print_contents();
		virtual ~query();
};

#endif /* QUERY_H_ */