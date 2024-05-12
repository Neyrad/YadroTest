#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <set>

#define POISON_DESK_NUMBER -1

enum EventID
{
	NONE			  = -1,
	
	IN_CLIENT_COMES   = 1,
	IN_CLIENT_SITS	  = 2,
	IN_CLIENT_WAITS   = 3,
	IN_CLIENT_LEAVES  = 4,
	
	OUT_CLIENT_LEAVES = 11,
	OUT_CLIENT_SITS   = 12,
	OUT_ERROR		  = 13
};

void AssertForInput(bool condition, const std::string& currentLine);

class Time final
{
public:
	int hours;
	int minutes;
	
	Time(int hours = 0, int minutes = 0) : hours(hours), minutes(minutes) {}
	
	std::string Print() const
	{
		std::string out;	
		if (this->hours < 10)
			out += '0';
		out += std::to_string(this->hours);
	
		out += ':';
	
		if (this->minutes < 10)
			out += '0';
		out += std::to_string(this->minutes);
		return out;
	}
	
	bool operator<(const Time& other) const
	{
        if (this->hours < other.hours)
			return true;
		if (this->hours == other.hours)
			return this->minutes < other.minutes;
		return false;
    }
};
bool ValidOpenCloseTime(const std::string& currentLine);
bool ValidEventTimeAndID(const std::string& currentLine);

class Desk final
{
public:
	Time startedUsingTime;
	Time allUsageTime;
	std::string client;
	bool taken;
	int money;
	
	Desk(const Time& startedUsingTime = Time(), const Time& allUsageTime = Time(),
		 const std::string& client = "Empty", bool taken = false, int money = 0) :
			startedUsingTime(startedUsingTime), allUsageTime(allUsageTime),
			client(client), taken(taken), money(money) {}
	
	void AddTime(const Time& currentSessionTime, int hourCost)
	{	
		int intAllUsageTime 	   =     this->allUsageTime.hours * 60
									   + this->allUsageTime.minutes;
		int intCurrentSessionTime  =     currentSessionTime.hours * 60
									   + currentSessionTime.minutes;
		int intStartedUsingTime    = this->startedUsingTime.hours * 60
								   + this->startedUsingTime.minutes;
		
		this->money += ((intCurrentSessionTime - intStartedUsingTime) / 60 + 1) * hourCost;
		intAllUsageTime += intCurrentSessionTime - intStartedUsingTime;
		
		this->allUsageTime.hours   = intAllUsageTime / 60;
		this->allUsageTime.minutes = intAllUsageTime % 60;
	}
};
bool AllDesksTaken(const std::vector<Desk>& desks);

class Event final
{
public:
	Time time;
	EventID ID;
	std::string name;
	int deskNumber;	
	
	Event(const Time& time = Time(), EventID ID = NONE, const std::string& name = "Empty",
		  int deskNumber = POISON_DESK_NUMBER) : 
			time(time), ID(ID), name(name), deskNumber(deskNumber) {}
	
	Event(const std::string& currentLine, int nDesks)
	{
		AssertForInput(ValidEventTimeAndID(currentLine), currentLine);
		
		std::istringstream iss(currentLine);
		char delimiter;
		iss >> this->time.hours >> delimiter >> this->time.minutes;
		
		int IntID;
		iss >> IntID;
		this->ID = (EventID)IntID;
		
		iss >> this->name;
		AssertForInput(!this->name.empty(), currentLine);
		
		for (char c: this->name)
			AssertForInput(std::islower(c) || std::isdigit(c) ||
						   std::isspace(c) || c == '-' || c == '_', currentLine);
	
		std::string DeskNumberString;
		iss >> DeskNumberString;
		for (char c: DeskNumberString)
			AssertForInput(std::isdigit(c) || std::isspace(c), currentLine);
		
		if (this->ID == IN_CLIENT_SITS)
		{
			AssertForInput(!DeskNumberString.empty(), currentLine);
			this->deskNumber = std::stoi(DeskNumberString);
		}
		else
			this->deskNumber = POISON_DESK_NUMBER;
		
		AssertForInput(this->deskNumber == POISON_DESK_NUMBER || 
					   this->deskNumber > 0 && this->deskNumber <= nDesks, currentLine);
		
		std::string Remainder;
		iss >> Remainder;
		for (char c: Remainder)
			AssertForInput(std::isspace(c), currentLine);
	}
	
	std::string Print() const
	{
		std::string out;	
		out += this->time.Print();
		out += ' ';
		out += std::to_string(this->ID);
		out += ' ';
		out += this->name;
		if (this->ID == IN_CLIENT_SITS || this->ID == OUT_CLIENT_SITS)
		{
			out += ' ';
			out += std::to_string(this->deskNumber);
		}
		return out;
	}
};

struct ComputerClub final
{
	std::queue<std::string> waitingClients;
	std::set<std::string> 	clients;
};

struct WorkingHours final
{
	Time openTime;
	Time closeTime;
};

std::ifstream GetInputFile(int argc, char* path);

void AddOuputEvents(std::ifstream& inFile, int nDesks, const WorkingHours& workingHours, int hourCost);

WorkingHours GetWorkingHours(std::ifstream& inFile);

int GetNumber(std::ifstream& inFile);

void HandleClientComes(std::vector<Event>& events, const Event& currentEvent,
					   ComputerClub& computerClub, const WorkingHours& workingHours,
					   bool& error);
					   
void HandleClientSits(std::vector<Event>& events, const Event& currentEvent,
					  ComputerClub& computerClub, std::vector<Desk>& desks,
					  bool& error);
					  
void HandleClientWaits(std::vector<Event>& events, const Event& currentEvent,
					   ComputerClub& computerClub, std::vector<Desk>& desks,
					   bool& error, int nDesks);

void HandleClientLeaves(std::vector<Event>& events, const Event& currentEvent,
					    ComputerClub& computerClub, std::vector<Desk>& desks,
						int hourCost, bool& error);

void KickOutRemainingClients(std::vector<Event>& events,
							 std::vector<Desk>& desks,
							 const WorkingHours& workingHours, int hourCost);

void PrintOutputToConsole(const std::vector<Event>& events,
						  const std::vector<Desk>& desks,
						  const WorkingHours& workingHours);

int main(int argc, char* argv[])
{
	std::ifstream InFile = GetInputFile(argc, argv[1]);
	
	int NDesks				  = GetNumber(InFile);
	WorkingHours WorkingHours = GetWorkingHours(InFile);
	int HourCost 			  = GetNumber(InFile);
	
	AddOuputEvents(InFile, NDesks, WorkingHours, HourCost);
	InFile.close();
}

std::ifstream GetInputFile(int argc, char* path)
{
	if (argc < 2)
	{
		std::cout << "Usage: ./task test_file.txt" << std::endl;
		exit(1);
	}
	std::ifstream InFile(path);
	if (InFile.fail())
	{
		std::cout << "Error opening file." << std::endl;
		exit(1);
	}
	return InFile;
}

void AddOuputEvents(std::ifstream& inFile, int nDesks, const WorkingHours& workingHours, int hourCost)
{
	ComputerClub ComputerClub;
	std::vector<Event> Events;
	std::vector<Desk> Desks(nDesks);
	Time LastEventTime = workingHours.openTime;
	while(true)
	{
		std::string CurrentLine;
		std::getline(inFile, CurrentLine);
		if (CurrentLine.empty())
			break;
		
		Event CurrentEvent(CurrentLine, nDesks);
		Events.push_back(CurrentEvent);
		bool error = false;
		switch (CurrentEvent.ID)
		{
			case IN_CLIENT_COMES:
				HandleClientComes(Events, CurrentEvent, ComputerClub, workingHours, error);
				break;
			case IN_CLIENT_SITS:
				HandleClientSits(Events, CurrentEvent, ComputerClub, Desks, error);
				break;
			case IN_CLIENT_WAITS:
				HandleClientWaits(Events, CurrentEvent, ComputerClub, Desks, error, nDesks);
				break;
			case IN_CLIENT_LEAVES:
				HandleClientLeaves(Events, CurrentEvent, ComputerClub, Desks, hourCost, error);
				break;
			default:
				std::cout << "Error! Invalid Event ID" << std::endl;
				exit(1);
		}
		
		if (!error)
		{
			AssertForInput(!(CurrentEvent.time < LastEventTime), CurrentLine);
			LastEventTime = CurrentEvent.time;
		}
	}
	KickOutRemainingClients(Events, Desks, workingHours, hourCost);
	PrintOutputToConsole(Events, Desks, workingHours);
}

bool AllDesksTaken(const std::vector<Desk>& desks)
{
	for (int i = 0; i < desks.size(); ++i)
		if (!desks[i].taken)
			return false; 
	return true;
}

WorkingHours GetWorkingHours(std::ifstream& inFile)
{
	std::string CurrentLine;
	std::getline(inFile, CurrentLine);
	AssertForInput(ValidOpenCloseTime(CurrentLine), CurrentLine);
	WorkingHours Temp;
	Temp.openTime.hours    = (CurrentLine[0] - '0') * 10 + (CurrentLine[1] - '0');
	Temp.openTime.minutes  = (CurrentLine[3] - '0') * 10 + (CurrentLine[4] - '0');
	Temp.closeTime.hours   = (CurrentLine[6] - '0') * 10 + (CurrentLine[7] - '0');
	Temp.closeTime.minutes = (CurrentLine[9] - '0') * 10 + (CurrentLine[10] - '0');
	AssertForInput(Temp.openTime < Temp.closeTime, CurrentLine);
	return Temp;
}

void AssertForInput(bool condition, const std::string& currentLine)
{
	if (!condition)
	{
		std::cout << currentLine << std::endl;
		exit(1);
	}
}

int GetNumber(std::ifstream& inFile)
{
	std::string CurrentLine;
	std::getline(inFile, CurrentLine);
	for (char c: CurrentLine)
		AssertForInput(std::isdigit(c) || std::isspace(c), CurrentLine);
	int Number = std::stoi(CurrentLine);
	AssertForInput(Number > 0, CurrentLine);
	return Number;
}

void HandleClientComes(std::vector<Event>& events, const Event& currentEvent,
					   ComputerClub& computerClub, const WorkingHours& workingHours,
					   bool& error)
{
	auto Search = computerClub.clients.find(currentEvent.name);
	if (Search != computerClub.clients.end())
	{
		Event ErrorEvent(currentEvent.time, OUT_ERROR, "YouShallNotPass");
		events.push_back(ErrorEvent);
		error = true;	
	}
	if (currentEvent.time < workingHours.openTime)
	{
		Event ErrorEvent(currentEvent.time, OUT_ERROR, "NotOpenYet");
		events.push_back(ErrorEvent);
		error = true;
	}		
	if (workingHours.closeTime < currentEvent.time)
	{
		Event ErrorEvent(currentEvent.time, OUT_ERROR, "AlreadyClosed");
		events.push_back(ErrorEvent);
		error = true;
	}
	
	if (!error)
		computerClub.clients.insert(currentEvent.name);
}

void HandleClientSits(std::vector<Event>& events, const Event& currentEvent,
					  ComputerClub& computerClub, std::vector<Desk>& desks,
					  bool& error)
{
	if (desks[currentEvent.deskNumber - 1].taken)
	{
		Event ErrorEvent(currentEvent.time, OUT_ERROR, "PlaceIsBusy");
		events.push_back(ErrorEvent);
		error = true;
	}
				
	auto search = computerClub.clients.find(currentEvent.name);
	if (search == computerClub.clients.end())
	{					
		Event ErrorEvent(currentEvent.time, OUT_ERROR, "ClientUnknown");
		events.push_back(ErrorEvent);
		error = true;
	}
				
	for (int i = 0; i < desks.size(); ++i)
		if (desks[i].client == currentEvent.name)
		{
			desks[i].taken  = false;
			desks[i].client = "Empty";
			break;
		}
				
	if (!error)
	{
		desks[currentEvent.deskNumber - 1].taken 			= true;
		desks[currentEvent.deskNumber - 1].startedUsingTime = currentEvent.time;
		desks[currentEvent.deskNumber - 1].client 			= currentEvent.name;
	}
}

void HandleClientWaits(std::vector<Event>& events, const Event& currentEvent,
					   ComputerClub& computerClub, std::vector<Desk>& desks,
					   bool& error, int nDesks)
{
	if (!AllDesksTaken(desks))
	{
		Event ErrorEvent(currentEvent.time, OUT_ERROR, "ICanWaitNoLonger");
		events.push_back(ErrorEvent);
		error = true;
	}
	
	if (computerClub.waitingClients.size() > nDesks)
	{
		computerClub.clients.erase(currentEvent.name);
		Event LeavingEvent(currentEvent.time, OUT_CLIENT_LEAVES, currentEvent.name);
		events.push_back(LeavingEvent);
		error = true;
	}
	
	if (!error)
		computerClub.waitingClients.push(currentEvent.name);
}

void HandleClientLeaves(std::vector<Event>& events, const Event& currentEvent,
					    ComputerClub& computerClub, std::vector<Desk>& desks,
						int hourCost, bool& error)
{
	auto search = computerClub.clients.find(currentEvent.name);
	if (search == computerClub.clients.end())
	{					
		Event ErrorEvent(currentEvent.time, OUT_ERROR, "ClientUnknown");
		events.push_back(ErrorEvent);
		error = true;
	}
				
	if (!error)
	{
		for (int i = 0; i < desks.size(); ++i)
			if (desks[i].client == currentEvent.name)
			{
				desks[i].taken = false;
				desks[i].AddTime(currentEvent.time, hourCost);
				if (computerClub.waitingClients.size())
				{
					desks[i].client = computerClub.waitingClients.front();
					computerClub.waitingClients.pop();
					Event TakingDeskEvent(currentEvent.time, OUT_CLIENT_SITS, 
										  desks[i].client, i + 1);
					events.push_back(TakingDeskEvent);
					desks[i].taken = true;
					desks[i].startedUsingTime = currentEvent.time;
				}
				else
					desks[i].client = "Empty";
				break;
			}
	}
} 

void KickOutRemainingClients(std::vector<Event>& events,
							 std::vector<Desk>& desks,
							 const WorkingHours& workingHours, int hourCost)
{
	for (int i = 0; i < desks.size(); ++i)
		if (desks[i].taken)
		{
			Event LeavingEvent(workingHours.closeTime, OUT_CLIENT_LEAVES, desks[i].client);
			events.push_back(LeavingEvent);
			desks[i].AddTime(workingHours.closeTime, hourCost);
		}
}

void PrintOutputToConsole(const std::vector<Event>& events,
						  const std::vector<Desk>& desks,
						  const WorkingHours& workingHours)
{
	std::cout << workingHours.openTime.Print() << std::endl;
	
	for (Event CurrentEvent: events)
		std::cout << CurrentEvent.Print() << std::endl;
	
	std::cout << workingHours.closeTime.Print() << std::endl;	
	
	for (int i = 0; i < desks.size(); ++i)
		std::cout << i + 1 << " " << desks[i].money
						   << " " << desks[i].allUsageTime.Print() << std::endl;
}

bool ValidOpenCloseTime(const std::string& currentLine)
{
	const std::string format = "XX:XX YY:YY";
	if (currentLine.length() < format.length()) return false;
	if (!std::isdigit(currentLine[0])) 		    return false;
	if (!std::isdigit(currentLine[1])) 		    return false;
	if (currentLine[2] != ':') 		  		    return false;
	if (!std::isdigit(currentLine[3]))  		return false;
	if (!std::isdigit(currentLine[4]))  		return false;
	if (currentLine[5] != ' ') 		    		return false;
	if (!std::isdigit(currentLine[6]))  		return false;
	if (!std::isdigit(currentLine[7]))  		return false;
	if (currentLine[8] != ':') 		    		return false;
	if (!std::isdigit(currentLine[9]))  		return false;
	if (!std::isdigit(currentLine[10])) 		return false;
	
	if (currentLine[0] - '0' > 2)			    		       return false;
	if (currentLine[0] - '0' == 2 && currentLine[1] - '0' > 3) return false;
	if (currentLine[3] - '0' > 5)			   			       return false;
	if (currentLine[6] - '0' > 2)			   			       return false;
	if (currentLine[6] - '0' == 2 && currentLine[7] - '0' > 3) return false;
	if (currentLine[9] - '0' > 5)			    		       return false;
	return true;
}

bool ValidEventTimeAndID(const std::string& currentLine)
{
	const std::string format = "XX:XX Y ";
	if (currentLine.length() < format.length()) return false;
	if (!std::isdigit(currentLine[0]))  		return false;
	if (!std::isdigit(currentLine[1]))  		return false;
	if (currentLine[2] != ':') 		    		return false;
	if (!std::isdigit(currentLine[3]))  		return false;
	if (!std::isdigit(currentLine[4]))  		return false;
	if (currentLine[5] != ' ') 		    		return false;
	if (!std::isdigit(currentLine[6])) 			return false;
	if (currentLine[7] != ' ') 		    		return false;
	
	if (currentLine[0] - '0' > 2)			    		       return false;
	if (currentLine[0] - '0' == 2 && currentLine[1] - '0' > 3) return false;
	if (currentLine[3] - '0' > 5)							   return false;
	
	int ThisEventID = currentLine[6] - '0';
	AssertForInput(ThisEventID == IN_CLIENT_COMES || ThisEventID == IN_CLIENT_SITS || 
				   ThisEventID == IN_CLIENT_WAITS || ThisEventID == IN_CLIENT_LEAVES, currentLine);

	return true;
}