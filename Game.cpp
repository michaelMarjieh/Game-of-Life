#include "Game.hpp"
#include "utils.hpp"
#include "Thread.hpp"
/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
//------------------------here Apple and Arrogant begin------------------------//

Game::Game(game_params params){

	//if(params!=NULL){
		m_gen_num=params.n_gen;
		m_thread_num=params.n_thread;
		interactive_on=params.interactive_on;
		print_on=params.print_on;
	//-------
		fileName=params.filename;
		numOfRows=utils::read_lines(params.filename).size();

		vector<string> linesWithSpace=utils::read_lines(fileName);
		vector<string> line=(utils::split(linesWithSpace[0],' '));
		numOfCols=line.size();
	//}

		jobs=new PCQueue<Job>;
		JobsEmpty=0;
		countCellsDone=0;
	numThreadFinished=0;
	pthread_cond_init(&notEmpty, NULL);
	pthread_mutex_init(&outMutex,NULL);

}

uint Game::thread_num()const{
	return m_thread_num;
}

//------------------------here Apple and Arrogant finish------------------------//

void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(NULL);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
	//
	//printf("hi");
}

void Game::_init_game() {
	// Create threads
   for (int i=0; (unsigned)i<m_thread_num; ++i){
	   ThreadGameSon *temp=new ThreadGameSon(this,i);
        m_threadpool.push_back(temp);
    }
	// Create game fields
	this->createCurrMatrix();
	/////create input matrix and put it in current

	// Start the threads
    for (int i=0; (unsigned)i<m_thread_num; ++i){
      m_threadpool[i]->start();

    }
	/*while (this->jobs->consumers_waiting != m_thread_num) {
		printf("start num finished %d \n ", this->jobs->consumers_waiting);
	}*/
/*
    for(int i=1;i<=m_gen_num;++i){
		_step(i);
    }
    //when finished gens destroy and print.
    this->_destroy_game();
	// Testing of your implementation will presume all threads are started here
*/
 }

void Game::_step(uint curr_gen) {

	pthread_mutex_lock(&outMutex);
	if(m_thread_num!=0) {
		int rows_per_job = (unsigned)(numOfRows) / m_thread_num;
		if(m_thread_num>(unsigned)numOfRows){
			rows_per_job=1;
		}
		int rows_for_last_job = rows_per_job +( (m_thread_num>(unsigned)numOfRows)? 0:(numOfRows - (rows_per_job * m_thread_num)));

		// Push jobs to queue
		//first n-1 jobs
		int fornow = 0;
		for (int i = 0; (unsigned) i < ( (m_thread_num>(unsigned)numOfRows)? (unsigned)numOfRows:m_thread_num) - 1; ++i) {
			Job temp;
			temp.from_row = fornow;
			temp.to_row = fornow + rows_per_job - 1; // if i=0 rpj=2 then to row is 1 and from is 0
			fornow = temp.to_row + 1;
			this->jobs->push(temp);

		}
		//last job
		Job temp1;
		temp1.from_row = fornow;
		temp1.to_row = fornow + rows_for_last_job - 1;
		this->jobs->push(temp1);
		//end of last job

		pthread_cond_wait(&notEmpty, &outMutex);

		JobsEmpty = 0;
		countCellsDone = 0;

		//while(jobs->getSize()!=0){}

		bool_mat tempMat = this->current_mat;
		this->current_mat = this->next_mat;
		this->next_mat = tempMat;


	}
	pthread_mutex_unlock(&outMutex);
	//printf("g");
	//print
	//enhe el while(1)
	//if(curr_gen)




	// Wait for the workers to finish calculating 
	// Swap pointers between current and next field 
}

void Game::_destroy_game(){
		//printf("hi");
	//printf("a");
	while ((unsigned)this->jobs->consumers_waiting < m_thread_num) {

		//printf("end num finished %d \n ", this->jobs->consumers_waiting);

	}
	pthread_mutex_lock(&outMutex);
	for (int i = 0; (unsigned) i < m_thread_num; ++i) {
		Job temp;
		temp.from_row = -i-5;
		temp.to_row =  - i-5; // if i=0 rpj=2 then to row is 1 and from is 0

		this->jobs->push(temp);
	//	printf(" entered %d \n",i);

	}
	//printf("hi");
	pthread_cond_wait(&notEmpty,&outMutex);
	//printf("c\n");
	pthread_mutex_unlock(&outMutex);
	//printf("e");

    for (int i=0; (unsigned)i<m_thread_num; ++i){

     	  m_threadpool[i]->join();



    }


	//printf("hi");NN
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// Testing of your implementation will presume all threads are joined here
}

/*--------------------------------------------------------------------------------
//inline static
--------------------------------------------------------------------------------*/
void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != NULL)
			cout << "<------------" << header << "------------>" << endl;
		
		// TODO: Print the board
		cout << u8"╔" << string(u8"═") * numOfCols << u8"╗" << endl;
		for (uint i = 0; i < (unsigned)numOfRows;++i) {
			cout << u8"║";
			for (uint j = 0; j < (unsigned)numOfCols; ++j) {
				cout << (current_mat[i][j] ? u8"█" : u8"░");
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * numOfCols << u8"╝" << endl;
		//

		// Display for GEN_SLEEP_USEC micro-seconds on screen
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}
void Game::createCurrMatrix(){


 vector<string> linesWithSpace=utils::read_lines(fileName);
//initializing matrixes with wanted size
	current_mat= bool_mat(linesWithSpace.size(),
          vector<bool>(utils::split(linesWithSpace[0],' ').size()));
	next_mat= bool_mat(linesWithSpace.size(),
          vector<bool>(utils::split(linesWithSpace[0],' ').size()));
   //initializing matrix with current input
   for(int row=0; (unsigned)row<linesWithSpace.size();row++){
       vector<string> line=(utils::split(linesWithSpace[row],' '));
       for(int col=0; (unsigned)col<line.size();col++){
           if(line[col]=="0"){
			   current_mat[row][col]=0;
           }
           else if(line[col]=="1"){
			   current_mat[row][col]=1;
           }


       }
   }

}
Game::~Game() {
/*	for (int i=0; (unsigned)i<m_thread_num; ++i) {

        ThreadGameSon *d = static_cast<ThreadGameSon *>(m_threadpool[i]);
        delete d;
        m_threadpool[i]=NULL;

	}*/
	for (int i=0; (unsigned)i<m_thread_num; ++i) {

		m_threadpool.pop_back();

	}
//int size= m_threadpool.size();
	pthread_mutex_destroy(&this->outMutex);
	pthread_cond_destroy(&notEmpty);
	delete jobs;

	jobs=NULL;
}
const vector<float> Game::gen_hist() const{
	return m_gen_hist;
}
const vector<float> Game::tile_hist() const{
	return m_tile_hist;
}
/* Function sketch to use for printing the board. You will need to decide its placement and how exactly
to bring in the field's parameters.

cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
for (uint i = 0; i < field_height ++i) {
   cout << u8"║";
   for (uint j = 0; j < field_width; ++j) {
       cout << (field[i][j] ? u8"█" : u8"░");
   }
   cout << u8"║" << endl;
}
cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/ 



