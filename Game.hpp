#ifndef __GAMERUN_H
#define __GAMERUN_H
//includes
#include "PCQueue.hpp"
#include "Thread.hpp"

/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
struct game_params {
	// All here are derived from ARGV, the program's input parameters. 
	uint n_gen;
	uint n_thread;
	string filename;
	bool interactive_on; 
	bool print_on; 
};

class Job{
public:
	Job(){
		from_row=0;
		to_row=0;
	}
	int from_row;
	int to_row;
	Job(int from,int to){
		from_row=from;
		to_row=to;
	}

};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Game {
public:
	friend class ThreadGameSon;
	Game(game_params);
	~Game();
	void run(); // Runs the game
	const vector<float> gen_hist() const; // Returns the generation timing histogram  
	const vector<float> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)

	PCQueue<Job> *jobs;
	int numOfRows;
	int numOfCols;
	string fileName;
	bool_mat current_mat,next_mat;
	void createCurrMatrix();
	int countCellsDone;
	int JobsEmpty;
	int numThreadFinished;
protected: // All members here are protected, instead of private for testing purposes
	pthread_cond_t notEmpty;
	pthread_mutex_t outMutex;
	// See Game.cpp for details on these three functions
	void _init_game(); 
	void _step(uint curr_gen); 
	void _destroy_game(); 

	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<float> m_tile_hist; 	 // Shared Timing history for tiles: First m_gen_num cells are the calculation durations for tiles in generation 1 and so on. 
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure. 
	vector<float> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool.

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)
	 void print_board(const char* header);
	// TODO: Add in your variables and synchronization primitives





};




//-----------inherited class from thread implementation000000000000000
class ThreadGameSon: public Thread
{

public:
	/** Implement this method in your subclass with the code you want your thread to run. */
	void thread_workload() override {

		while(1){

			Job job=g->jobs->pop();
			pthread_mutex_lock(&g->outMutex);
			//if q is poisoned then end while and finish thread workload
			if(job.to_row<0 || job.from_row<0 ){
				//printf("hi1");
				g->numThreadFinished+=1;

				if((unsigned)g->numThreadFinished==g->m_thread_num){
					pthread_cond_signal(&g->notEmpty);
				}
				pthread_mutex_unlock(&g->outMutex);

				break;
			}
			pthread_mutex_unlock(&g->outMutex);

			int from=job.from_row;
			int to=job.to_row;
			int IsLiveNext;

			pthread_mutex_lock(&g->outMutex);
			auto tile_start = std::chrono::system_clock::now();
			pthread_mutex_unlock(&g->outMutex);


			//pthread_mutex_lock(&g->outMutex);
			for(int i=from;i<=to;++i) {
				for (int j = 0; j < g->numOfCols; j++) {
				//	pthread_mutex_lock(&g->outMutex);

					IsLiveNext = check_cell(i,j);
					//pthread_mutex_lock(&g->outMutex);
					g->next_mat[i][j]=IsLiveNext;
					pthread_mutex_lock(&g->outMutex);
				 	g->countCellsDone+=1;
					pthread_mutex_unlock(&g->outMutex);
				   // pthread_mutex_unlock(&g->outMutex);
				}
			}
			//pthread_mutex_unlock(&g->outMutex);

		   pthread_mutex_lock(&g->outMutex);
			auto tile_end = std::chrono::system_clock::now();
			g->m_tile_hist.push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(tile_end - tile_start).count());
			if(g->countCellsDone==g->numOfCols*g->numOfRows){
				pthread_cond_signal(&g->notEmpty);
			}
			pthread_mutex_unlock(&g->outMutex);

		}





	}
	ThreadGameSon(Game* gm,uint thread_id):Thread(thread_id){
		g=gm;
		//pthread_mutex_init(&mutex, NULL);

	}
	~ThreadGameSon() {

		//pthread_mutex_destroy(&mutex);
		g=NULL;
	}
	//i indicates to rows and j indicates to columns
	int check_cell(int i,int j){

		int neighCountLive=0;
		int  curCellR=0,curCellCol=0;
		uint unSCol;
		uint unSRows;
		for(int row=-1;row<=1;row++) {
			for(int col=-1;col<=1;col++){
				curCellR=row+i;
				curCellCol=col+j;
				unSRows=(unsigned)curCellR;
				unSCol=(unsigned)curCellCol;
				if(curCellR<0 || (unSRows>(g->current_mat.size()-1)) || curCellCol<0 || (unSCol>(g->current_mat[0].size()-1)) )
					continue;

				//if we are dealing the origion cell (it is not the neighbor of himself)
				if(row==0 && col==0)
					continue;

				//deal with DEAD cell
				//count the neighbor live cells  for the dead cell
				if( g->current_mat[curCellR][curCellCol]==1)
					neighCountLive++;

				//deal with LIVE cell
				//count the neighbor live cells  for the Live cell
			/*	if(g->current_mat[i][j]==1 && g->current_mat[curCellR][curCellCol]==1)
					neighCountLive++;*/
			}
		}
		//if the origin cell is dead with 3 live neigh cells becomes alive
		if(g->current_mat[i][j]==0 && neighCountLive==3)
			return 1;
		//if the origin cell is dead with 3 live neigh cells becomes alive
		if((g->current_mat[i][j]==1 && neighCountLive==3)|| (g->current_mat[i][j]==1 && neighCountLive==2))
			return 1;
		//in other cases it becomes dead cell
		return 0;
	}
private:
//	pthread_mutex_t mutex;
	Game* g;


};


#endif
