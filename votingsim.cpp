#include<iostream>
#include<stdio.h>
#include<random>
#include<time.h>
#include <vector>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator

using namespace std;

std::normal_distribution<double> normal(0.0, 1.0); //first argument is mean, second sttdev

#define nVoter 9 //no: of voters
#define nCand 5 //no: of candidates

class Voter
{
	/*	
	Every voter has a vector of utilities. The length of the vector = no: of candidates in the election.
	Utilities are in standard normal distribution (mean = 0, stddev = 1) 	
	*/
	
	public:
	vector<double> utils;	
	vector<double> dummy_utils; //it is just used to play around so that original one is preserved in "utils"
	int strength;
	
	vector<int> ballot; //this is the ballot a voter casts at the time of elections 
		
	Voter()
	{
		//srand(time(NULL));
		//strength = rand()%10 + 1;
		ballot.assign(nCand, 0);
	}
	
	void generateUtil()
	{	
		utils.clear();
		unsigned seed = rand();
		std::default_random_engine generator (seed);
	
		for(int i=0; i < nCand; i++) //generate as many values as no: of candidates
		{
			double value = normal(generator);
			utils.push_back(value);
		
		}
		
		////////////////////////
		dummy_utils = utils;
		///////////////////////
		
		strength = rand()%10 + 1;
		
		//return utils;
		
	}
	
	void printUtils()
	{
		copy(utils.begin(), utils.end(), ostream_iterator<double>(cout, "   "));
		cout<<endl;
		//copy(ballot.begin(), ballot.end(), ostream_iterator<double>(cout, "   "));
		//cout<<endl;
	}
	
	int maxutilCand(vector<double> u)
	{
		double max = -100.00;
		int j = 0;
		
		for(int i=0; i<nCand; i++)
		{
			if(u[i] >= max)
			{
				max = u[i];		
				
				if(u[i] == max)
					if(rand()%2)
						j = i; //i.e if there are multiple candidates with same max util then pick one of them ramdomly
				else
					j = i;
			}					
			
		}
		
		return j;
	
	}	



}v[nVoter];

//Voter v[nVoter]; //each object v[i] is a voter. (global because it will be used everywhere)	



class Electorate: public Voter
{
	/*	
	The voters together make up an electorate. Key things to consider: 
		How do you build the electorate? This in turn asks how the voters are correlated. 
	
	Functions:
		1) Calculate social utility: Given the utilities for each voter in an electorate, aggregate them and arrive at the social value for each candidate.
	
		2) Create Electorate: How do you create an electorate?
			- one way is to have a completely random model with no correlation among the voters.
				 	
	*/
	
	public: 
	
	void influencing(int nV, int nInf = 1)
	{
		//nInf - number of influencers
		//nV - number of voters to take into account 
		
		int influencer;				
		
		for(int i=0; i<nV; i++)
		{
			//cout<<"Voter "<<i<<endl;
			
			for(int j=0; j<nInf; j++)
			{
				influencer = rand()%nV;  //pick a random influencer	
				
				if(influencer == i) //just to take care of edge conditions
				{
					if ( (influencer+1) == nV)
					 influencer--;
					 else
					 influencer++;
					 
				}			
				
				//cout<<"Inf = "<<influencer<<endl;
								
				double w = (double)v[influencer].strength/v[i].strength;				
				//cout<<"w ="<<w<<endl;
				
				for(int k=0; k<nCand; k++)	
					v[i].utils[k] = (v[i].utils[k] + v[influencer].utils[k]*w*w)/(1+w*w); //influence affect 
					
				
				//extra --to account v1's influence on v2 (does this make sense??)
				w = 1/w;
				for(int k=0; k<nCand; k++)	
					v[influencer].utils[k] = (v[influencer].utils[k] + v[i].utils[k]*w*w)/(1+w*w); //influence affect 		
					
			}
			
		}			
	
	
	}
	
	
	
	
	void randomElectorate()
	{
		/*
		Here there is no correlation among the voters. Very Very unrealistic!
		*/
		
		for(int i=0; i<nVoter; i++)			
			v[i].generateUtil();	
			
		
		if(1) //to print 
		{
			cout<<"Random Electorate\n";
			for(int i=0; i<nVoter; i++)
    				v[i].printUtils();
    				
    			vector<double> a = this->socUtil();
    			cout<<"\nAfter electorate social utilities\n";
    			copy(a.begin(), a.end(), ostream_iterator<double>(cout, "  "));	
    			cout<<endl;
    		}			
    		
	}
	
	
	void randomInfluence()
	{
		/* 
		#Similar to the the impartial culture that is seen in randomElectorate() with the only difference being that here there is an attempt to model some social influence. 
		
		#Every voter here has a parameter strength that is associated with them. 'strength' indicates how firm or otherwise they're in their opinion (strength (1-10) 10 => loyalists [extreme views, won't budge]; 1 => fence sitters [easily influenced]). 
		
		#Each voter interacts with a random set of influencers (say 10) and as a result his opinion changes. 
		*/
		
		for(int i=0; i<nVoter; i++)			
			v[i].generateUtil();  //initially generate random utilities for each voter (same as in randomElectorate)
	
		
		int nInf = 3; //number of influencers
		
		influencing(nVoter, nInf);
		
		if(1) //to print 
		{
			cout<<"\nRandom-influence Electorate\n";
			for(int i=0; i<nVoter; i++)
    				v[i].printUtils();
    				
    			vector<double> a = this->socUtil();
    			cout<<"\nAfter electorate social utilities\n";
    			copy(a.begin(), a.end(), ostream_iterator<double>(cout, "  "));	
    			cout<<endl;	
    		}
				
		
	}
	
	void chineseRestaurant()
	{
		/*
		#Using the Polya-Urn model to create elecotrates
		#Start with alpha voters and beta wildcards
		#if you draw a voter, clone it. Else make a new random voter. Do this until the number of voters required
		#Works like the Chinese Restaurant Process
		
		alpha = 2; beta = 1
		
		#Here I'm creating similar voters with each one influencing the other as in randomInfluence();
		
		*/
		
		int alpha = 2;
		int beta = 1;
		
		//create alpha number of similar voters
		for(int i=0; i<alpha; i++)			
			v[i].generateUtil();  //initially generate random utilities for each voter (same as in randomElectorate)
		
		influencing(alpha, 1); //influencing done to make them somewhat similar 
		
		int nCreated = alpha; //nCreated denotes the number of voters already created 
		
		//just for printing
		if(0)
		{
			for(int i =0; i<alpha; i++)
			{
				copy(v[i].utils.begin(), v[i].utils.end(), ostream_iterator<double>(cout, "  "));
				cout<<endl;
			}
		
		}
		
		
		
		//Urn functioning 
		
		while(nCreated < nVoter)
		{
			int pick = random()%(nCreated + beta); //pick a number between 0 and nVoter - 1
			
			if(pick < nCreated) //implies that we have picked up a voter
			{
				//clone voter - same as mutantChild in vse.py
						
				double mWeight = 0.2;
				v[nCreated].generateUtil();
				
				for(int i=0; i<nCand; i++)				
					v[nCreated].utils[i] = (v[nCreated].utils[i] + mWeight*v[pick].utils[i])/sqrt(1+ mWeight*mWeight);
			
			
			}
			
			else
				v[nCreated].generateUtil(); //creates a new random voter							
			
			
			
			nCreated++;
			
		}
		
		if(1) //to print 
		{
			cout<<"\nChinese Restaurant Electorate\n";
			for(int i=0; i<nVoter; i++)
    				v[i].printUtils();
    				
    			vector<double> a = this->socUtil();
    			cout<<"\nAfter electorate social utilities\n";
    			copy(a.begin(), a.end(), ostream_iterator<double>(cout, "  "));	
    			cout<<endl;
    				
    		}
			
	
	} 
	
	
	void createElectorate(int model = 0)
	{
		/*		
		how do you create an electorate? 
		After this function we will have nVoter voters each with a utility vector associated with them
		"int model" represents the kind of model we use for the electorate
		*/
		
		switch(model)
		{
			case 0: 
				this->randomElectorate();
				break;
			case 1: 
				this->randomInfluence();
				break;
				
			case 2:
				this->chineseRestaurant();
				break;
			
			default: 
				this->randomElectorate();
			
		}
			
		
		
	}
	
	
	
	vector<double> socUtil()
	{
		/*
		Calculates the social utility.
		We define social utility of candidate c as the mean of the utilities of c by each voter v. 
		*/
		
		vector<double> socialUtils;
		
		for(int j=0; j<nCand; j++)
		{
			double u = 0.00;
			
			for(int i=0; i<nVoter; i++)
				u += v[i].utils[j];
			
			u = u/nVoter;
			socialUtils.push_back(u);
			
		}
		
		return socialUtils;
		
	}

};


//Need to create functions so as to run elections

class electionMethods
{
	public:	
	
	vector<int> candScore;
	
	void calculate_candScores()
	{
		candScore.clear();
		candScore.assign(nCand, 0);
		
		for(int i=0; i<nCand; i++)
		{
			int s = 0;
			
			for(int j=0; j<nVoter; j++)
				s = s + v[j].ballot[i];
			
			candScore[i] = s;
			
		}			
	
	}
	
	int returnWinner()
	{
		int maxscore = 0;
		int winner = 0;
		
		calculate_candScores();
		
		for(int i=0; i<nCand; i++)
		{	
			if(candScore[i] >= maxscore)
			{
				if(candScore[i] == maxscore)
				{
					if(rand()%2)
						winner = i; //i.e if there are multiple candidates with same max util then pick one of them
				}
				
				else
					winner = i;
					
				maxscore = candScore[i];
			}			
		}
				
		copy(candScore.begin(), candScore.end(), ostream_iterator<double>(cout, "  "));	
    		cout<<endl;
    		
    		return winner;
		
	}
	
};


class plurality : public electionMethods 
{	
	public:
	
	int honestBallot()
	{
		for(int i=0; i<nVoter; i++)
		{		
			int j = v[i].maxutilCand(v[i].utils); //j is the candidate with max uitlity for v[i]
			v[i].ballot[j]	= 1;			
		}
		
		int winner = returnWinner();
		return winner;
		
	}
	
	//Have to make strategic voting

};

class pluralityRunoff : public plurality 
{
	/*
	Just like plurality. 
	But here if the plurality winner does not get a majority then there is a 2nd round where the top 2 candidates participate.
	
	= got to think about strategic voting here
	*/
	
	public:
	
	int honestBallot()
	{
		int winner = plurality::honestBallot(); //find the plurality winner  
		
		if(2*candScore[winner] < nVoter) //implies that the winner does not have an absolute majority 
		{
			cout<<"The plurality winner did not get a majority"<<endl;
			
			int firstCand = winner;
			int secondCand = -1;
			
			vector<int> temp_scores = candScore;
			vector<int>::iterator it = max_element(temp_scores.begin(), temp_scores.end());
			*it = -1; //removes the top ranked candidate
			
			it = max_element(temp_scores.begin(), temp_scores.end()); //returns the position of the 2nd best
			//it++;
			
			for(int i=0; i<nCand; i++)
			{
				if(candScore[i] == *it && i != firstCand)
				{
					if(secondCand == -1)
						secondCand = i;
					else
						if(random()%2)
							secondCand = i; //i.e if there is a tie for the 2nd place then pick randomly
				}		
			}
			
			cout<<"The two candidates are "<<firstCand + 1<<" and "<<secondCand + 1<<endl;
			//now conduct the elections with just 2 candidates
			int nCand1=0, nCand2 =0; //nCand1/2 #votes for candiate 1/2				
			
			for(int i=0; i<nVoter; i++)
				if(v[i].utils[firstCand] > v[i].utils[secondCand])
					nCand1++;
				else if(v[i].utils[firstCand] < v[i].utils[secondCand]) //I'm ignoring people who have a tie between the two
					nCand2++; 
			
			if(nCand1 >= nCand2) //in case there is a tie nCand1 wins (just to give him an incentive for winning the first election)
				winner = firstCand;
			else
				winner = secondCand;				
		
		}
		
		return winner;
	
	}

};

class instantRunoff : public plurality
{
	public:
	
	int honestBallot()
	{
		int winner = plurality::honestBallot(); //find the plurality winner  
		
		while(2*candScore[winner] < nVoter)
		{		
			//let c be the candidate with the least number of first votes
			int min = nVoter;
			int c = 0;
			
			for(int i=0; i<nCand; i++)
			{				
				if(candScore[i]	< min && candScore[i] > 0)
				{
					c = i;
					min = candScore[i];
				}
				
				if(candScore[i] == 0) //we needn't consider any candidate which already has only zero first votes 
					v[i].dummy_utils[i] = -100.00; 								
			
			}
			
			
			
			//to mimick the removal of c just make the utilitites of c for each voter as -100.00
			
			for(int i=0; i<nVoter; i++)
			{
				v[i].ballot.clear();
				v[i].ballot.assign(nCand, 0);
								
				v[i].dummy_utils[c] = -100.00; 
				
				int j = v[i].maxutilCand(v[i].dummy_utils); //j is the candidate with max uitlity for v[i]
				v[i].ballot[j]	= 1;				
			
			}			
			
			winner = returnWinner();			
		}
		
		return winner;	
	}
};


class scoreVoting : public electionMethods
{
	public:
		
	int honestBallot()
	{
		for(int i=0; i<nVoter; i++)
		{
			//score voting range 0..9
			
			double n = *min_element(v[i].utils.begin(), v[i].utils.end());
			double m = *max_element(v[i].utils.begin(), v[i].utils.end()) - n;
			
			for(int j=0; j<nCand; j++)
				v[i].ballot[j] = 9.0*(v[i].utils[j] - n)/m;
				
		}
		
		int winner = returnWinner();
		return winner;
	
	}

};


class borda : public electionMethods
{
	public:
		
	int honestBallot()
	{
		for(int i=0; i<nVoter; i++)
		{
			vector<double> temp_util = v[i].utils;
			int c = 0;
						
			while(!temp_util.empty())		
			{
				 vector<double>::iterator it = min_element(temp_util.begin(), temp_util.end());				 
				 for(int j=0; j<nCand; j++)
		 			if(v[i].utils[j] == *it)
		 				v[i].ballot[j] = c++;
							 
				 temp_util.erase(it);				 
			}
			
			//v[i].printUtils();
		}
				
		int winner = returnWinner();
		return winner;
	
	}
};


class approvalVoting : public electionMethods
{
	/*
	Each Voter approves all the candidate above a threshold value.
	Two ways:
	1) Using the avg utility as the threshold 
	2) calculate the range score for each candidate. approve all of them with a rating >=5 (range used 0..9)		
	*/
	
	public:
	
	void usingAvg()
	{
		for(int i=0; i<nVoter; i++)
		{
			v[i].ballot.clear();
			v[i].ballot.assign(nCand, 0);
			
			double avg = 0.00;
			for(int j=0; j<nCand; j++)
				avg = avg + v[i].utils[j];
			
			avg = avg/nCand;
			
			//cout<<"\n Average = "<<avg<<endl;
			
			for(int j=0; j<nCand; j++)
				if(v[i].utils[j] >= avg)
				{ cout<<j+1<<"\t"; v[i].ballot[j] = 1;} //approves candidate j
				
			cout<<endl;	
		}
		
		int winner = returnWinner();
		cout<<"Winner using average approval voting is = Candidate "<<winner+1<<endl;
	
	}
	
	void usingRange()
	{
		for(int i=0; i<nVoter; i++)
		{
			//score voting range 0..9
			v[i].ballot.clear();
			v[i].ballot.assign(nCand, 0);
			
			double n = *min_element(v[i].utils.begin(), v[i].utils.end());
			double m = *max_element(v[i].utils.begin(), v[i].utils.end()) - n;
			
			for(int j=0; j<nCand; j++)
				if(9.0*(v[i].utils[j] - n)/m >= 5.0)  //i.e. if the range score >= 5 then approve the candidate
					 {cout<<j+1<<"\t"; v[i].ballot[j] = 1;}
					 
			cout<<endl;
		}
						
		int winner = returnWinner();
		cout<<"Winner using range scores is = Candidate "<<winner+1<<endl;
	
	}
		
	void honestBallot()
	{	
		usingAvg();
		usingRange();		
	}	


};



int main()
{

	srand(time(NULL));
	 
 	
 	Electorate e; 
 	    	
    	e.createElectorate(0);  
    	
    	instantRunoff pl;
    	
    	cout<<"Winner is = Candidate "<<pl.honestBallot() + 1<<endl;  	
    	
    	//e.createElectorate(1);
     	
    	//e.createElectorate(2);    	
    	

 
	return 0;
	
	
}


