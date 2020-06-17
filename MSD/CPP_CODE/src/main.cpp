//============================================================================
// Name        : hmm/main.cpp
// Author      : Saurav Manchanda
// Version     : 1.0
// Description : hmm implementation
//============================================================================

/*
./hmm -queries /export/scratch2/Walmart/second_project/new_preprocessed_data/train/queries.txt -labels /export/scratch2/Walmart/second_project/new_preprocessed_data/train/attribute_labels.txt -vocab /export/scratch2/Walmart/second_project/new_preprocessed_data/vocab.txt -label_map /export/scratch2/Walmart/second_project/new_preprocessed_data/attribute_indices.txt -characteristic_word_dist /export/scratch2/Walmart/second_project/hmm/attribute/characteristic_word_dist.txt -transition_dist /export/scratch2/Walmart/second_project/hmm/attribute/transition_dist.txt -characteristic_assignment /export/scratch2/Walmart/second_project/hmm/attribute/train_characteristic_assignment.txt -delta 0.1 -zeta 0.001 -null 1 -seed 1 -iter 100 -threads 8 -train 1
./hmm -queries /export/scratch2/Walmart/second_project/new_preprocessed_data/test/queries.txt -labels /export/scratch2/Walmart/second_project/new_preprocessed_data/test/attribute_labels.txt -vocab /export/scratch2/Walmart/second_project/new_preprocessed_data/vocab.txt -label_map /export/scratch2/Walmart/second_project/new_preprocessed_data/attribute_indices.txt -characteristic_word_dist /export/scratch2/Walmart/second_project/hmm/attribute/characteristic_word_dist.txt -transition_dist /export/scratch2/Walmart/second_project/hmm/attribute/transition_dist.txt -characteristic_assignment /export/scratch2/Walmart/second_project/hmm/attribute/test_characteristic_assignment.txt -delta 0.1 -zeta 0.001 -null 1 -seed 1 -iter 10 -threads 8 -train 0
python /home/manch043/code/walmart/p2/most_probable_test_generator.py --datapath /export/scratch2/Walmart/second_project/raw_data/mossaab/sequence-tagger-annotations/src/main/resources/traffic-dev-20180813-new-format.txt --training /export/scratch2/Walmart/second_project/new_preprocessed_data/train/queries.txt --tagdict /home/manch043/code/walmart/p2/tag_mapping.txt --vocab /export/scratch2/Walmart/second_project/new_preprocessed_data/vocab.txt --distribution /export/scratch2/Walmart/second_project/hmm/attribute/characteristic_word_dist.txt --mapping /export/scratch2/Walmart/second_project/new_preprocessed_data/attribute_indices.txt --output /export/scratch2/Walmart/second_project/hmm/temp/queries.txt --label /export/scratch2/Walmart/second_project/hmm/temp/labels.txt --candidate /export/scratch2/Walmart/second_project/hmm/temp/candidates.txt --tags /export/scratch2/Walmart/second_project/hmm/temp/tags.txt --topk 1
./hmm -queries /export/scratch2/Walmart/second_project/hmm/temp/queries.txt -labels /export/scratch2/Walmart/second_project/hmm/temp/labels.txt -vocab /export/scratch2/Walmart/second_project/new_preprocessed_data/vocab.txt -label_map /export/scratch2/Walmart/second_project/new_preprocessed_data/attribute_indices.txt -characteristic_word_dist /export/scratch2/Walmart/second_project/hmm/attribute/characteristic_word_dist.txt -transition_dist /export/scratch2/Walmart/second_project/hmm/attribute/transition_dist.txt -characteristic_assignment /export/scratch2/Walmart/second_project/hmm/temp/test_characteristic_assignment.txt -delta 0.1 -zeta 0.001 -null 1 -seed 1 -iter 10 -threads 8 -train 0


*/
#include "include.h"
#include "hmm.h"
#include "query.h"
using namespace std;

void show_help()
{
  cout<<"Usage:    [-option] [argument]"<<endl;
  cout<<"option:  "<<"-queries\tUse queries from <queries> file to annotate, default is queries.txt"<<endl;
  cout<<"         "<<"-labels\tUse labels from <labels> file to annotate the queries, default is labels.txt"<<endl;
  cout<<"         "<<"-vocab\tRead vocab from <vocab> file, default is vocab.txt"<<endl;
  cout<<"         "<<"-label_map\tRead label map from <label_map> file, default is label_indices.txt"<<endl;
  cout<<"         "<<"-characteristic_word_dist\tSave characteristic word distribution to the <characteristic_word_dist> file, default is characteristic_word_dist.txt"<<endl;
  cout<<"         "<<"-transition_dist\tSave characteristic transition prob to the <transition_dist> file, default is transition_dist.txt"<<endl;
  cout<<"         "<<"-characteristic_assignment\tSave characteristic assignments to the <characteristic_assignment> file, default is characteristic_assignment.txt"<<endl;
  cout<<"         "<<"-delta\tPrior for characteristic word distribution, default is 0.1"<<endl;
  cout<<"         "<<"-zeta\tPrior for transition probability, default is 0.1"<<endl;
  cout<<"         "<<"-iter\tNumber of iterations, default is 20"<<endl;
  cout<<"         "<<"-null\tUse a null tag, default is 1"<<endl;
  cout<<"         "<<"-seed\tRandom seed, default is 1"<<endl;
  cout<<"         "<<"-threads\tNumber of threads, default is 1"<<endl;
  cout<<"         "<<"-train\tWhether to train (1) or test (0), default is 1"<<endl;
}

int ArgPos(char *str, int argc, char **argv) {
  int a;
  for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
    if (a == argc - 1) {
      printf("Argument missing for %s\n", str);
      exit(1);
    }
    return a;
  }
  return -1;
}

int main(int argc,char *argv[])
{
	int i, cnt, num;
	char query_file[MAX_STRING], label_file[MAX_STRING], characteristic_word_file[MAX_STRING], transition_file[MAX_STRING], characteristic_assignment_file[MAX_STRING], vocab_file[MAX_STRING], label_map_file[MAX_STRING];
	int num_threads = 1, num_iter = 20, train = 1, null_tag = 1, seed = 1;
	real delta = 0.1, zeta = 0.1;

	int num_classes = 0;
	int vocab_size = 0;
	real perplexity;

	strcpy(query_file, (char *)"queries.txt");
	strcpy(label_file, (char *)"labels.txt");
	strcpy(characteristic_word_file, (char *)"characteristic_word_dist.txt");
	strcpy(transition_file, (char *)"transition_dist.txt");
	strcpy(characteristic_assignment_file, (char *)"characteristic_assignment_dist.txt");
	strcpy(vocab_file, (char *)"vocab.txt");
	strcpy(label_map_file, (char *)"label_indices.txt");

	if(argc == 2) {
		show_help();
		exit(EXIT_FAILURE);
	}

	//Take command line arguments
	if ((i = ArgPos((char *)"-queries", argc, argv)) > 0) strcpy(query_file, argv[i + 1]);
	if ((i = ArgPos((char *)"-labels", argc, argv)) > 0) strcpy(label_file, argv[i + 1]);
	if ((i = ArgPos((char *)"-vocab", argc, argv)) > 0) strcpy(vocab_file, argv[i + 1]);
	if ((i = ArgPos((char *)"-label_map", argc, argv)) > 0) strcpy(label_map_file, argv[i + 1]);
	if ((i = ArgPos((char *)"-characteristic_word_dist", argc, argv)) > 0) strcpy(characteristic_word_file, argv[i + 1]);
	if ((i = ArgPos((char *)"-transition_dist", argc, argv)) > 0) strcpy(transition_file, argv[i + 1]);
	if ((i = ArgPos((char *)"-characteristic_assignment", argc, argv)) > 0) strcpy(characteristic_assignment_file, argv[i + 1]);
	if ((i = ArgPos((char *)"-delta", argc, argv)) > 0) delta = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-zeta", argc, argv)) > 0) zeta = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-null", argc, argv)) > 0) null_tag = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-seed", argc, argv)) > 0) seed = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-iter", argc, argv)) > 0) num_iter = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-train", argc, argv)) > 0) train = atoi(argv[i + 1]);

	cout<<"The model parameters are as follows"<<endl;
	cout<<"\tqueries "<<query_file<<endl;
	cout<<"\tlabels "<<label_file<<endl;
	cout<<"\tvocab "<<vocab_file<<endl;
	cout<<"\tlabel_map "<<label_map_file<<endl;
	cout<<"\tcharacteristic_word_dist "<<characteristic_word_file<<endl;
	cout<<"\ttransition_dist "<<transition_file<<endl;
	cout<<"\tcharacteristic_assignment "<<characteristic_assignment_file<<endl;
	cout<<"\tdelta "<<delta<<endl;
	cout<<"\tzeta "<<zeta<<endl;
	cout<<"\tnull "<<null_tag<<endl;
	cout<<"\tseed "<<seed<<endl;
	cout<<"\titer "<<num_iter<<endl;
	cout<<"\tthreads "<<num_threads<<endl;
	cout<<"\ttrain "<<train<<endl;


	srand (seed);
	vector<string> vocab;
	vector<string> label_map;
	vector<int> temp_vector_1, temp_vector_2;
	vector<query> queries;


	string str;
	stringstream iss;
	ifstream file_handle_1, file_handle_2;
	ofstream out_file_handle;

	file_handle_1.open(vocab_file);

	if (!file_handle_1) {
		cout<<"Cannot open vocab file!!!"<<endl;
		exit(EXIT_FAILURE);
	}

	cnt = 0;
	while (getline(file_handle_1, str)) {
		cnt++;
		vocab.push_back(str);
	}

	file_handle_1.close();


	file_handle_1.open(label_map_file);

	if (!file_handle_1) {
		cout<<"Cannot open label mapping file!!!"<<endl;
		exit(EXIT_FAILURE);
	}

	cnt = 0;
	while (getline(file_handle_1, str)) {
		cnt++;
		label_map.push_back(str);
	}

	file_handle_1.close();

	vocab_size = vocab.size();
	num_classes = label_map.size();

	file_handle_1.open(query_file);

	if (!file_handle_1) {
		cout<<"Cannot open query file!!!"<<endl;
		exit(EXIT_FAILURE);
	}

	file_handle_2.open(label_file);

	if (!file_handle_2) {
		cout<<"Cannot open label file!!!"<<endl;
		exit(EXIT_FAILURE);
	}

	cnt = 0;
	while (getline(file_handle_1, str)) {
		cnt++;

		temp_vector_1.clear();
		temp_vector_2.clear();

		iss.str("");
		iss.clear();
		iss << str;
		while (iss >> num) {
			temp_vector_1.push_back(num);
		}

		getline(file_handle_2, str);
		iss.str("");
		iss.clear();
		iss << str;
		
		while (iss >> num) {
			temp_vector_2.push_back(num);
		}
		queries.push_back(query(num_classes, temp_vector_1, temp_vector_2, null_tag));
	}

	file_handle_1.close();
	file_handle_2.close();

	// for(i = 0; i < queries.size(); i++) {
	// 	queries[i].print_contents();
	// }

	hmm topic_model(vocab_size, num_classes, queries, num_threads, delta, zeta, null_tag, seed);

	if(train == 0) {
		file_handle_1.open(characteristic_word_file);

		if (!file_handle_1) {
			cout<<"Cannot open characteristic word distribution file!!!"<<endl;
			exit(EXIT_FAILURE);
		}
		topic_model.read_characteristic_dist(file_handle_1);

		file_handle_1.close();
		file_handle_1.open(transition_file);

		if (!file_handle_1) {
			cout<<"Cannot open transition distribution file!!!"<<endl;
			exit(EXIT_FAILURE);
		}
		topic_model.read_transition_dist(file_handle_1);

		file_handle_1.close();
	}

	cout<<"Random initialization"<<endl;
	perplexity = topic_model.Perplexity(train);
	cout<<"Perplexity is "<<perplexity<<endl;

	for(i=0; i<num_iter; i++) {
		cout<<"Iteration "<<i+1<<endl;
		if (train == 1) {
			topic_model.Inference();
		}
		else {
			topic_model.Viterbi();
		}
		perplexity = topic_model.Perplexity(train);
		cout<<"Perplexity is "<<perplexity<<endl;
	}


	out_file_handle.open(characteristic_assignment_file);
	for(i = 0; i < queries.size(); i++) {
		queries[i].write_assignment(out_file_handle);
	}
	out_file_handle.close();

	if(train == 1){
		out_file_handle.open(characteristic_word_file);
		topic_model.write_characteristic_dist(out_file_handle);
		out_file_handle.close();

		out_file_handle.open(transition_file);
		topic_model.write_transition_dist(out_file_handle);
		out_file_handle.close();
	}

  	return 0;
}
