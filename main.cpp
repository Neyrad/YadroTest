#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
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
	
	bool operator<(const Time& other) const
	{
        if (this->hours < other.hours)
			return true;
		if (this->hours == other.hours)
			return this->minutes < other.minutes;
		return false;
    }
	
	friend std::ostream& operator<<(std::ostream& os, const Time& time);
};

std::ostream& operator<<(std::ostream& os, const Time& time)
{
	if (time.hours < 10)
		os << '0';
	os << time.hours << ':';
	
	if (time.minutes < 10)
		os << '0';
	os << time.minutes;
	return os;
}

int GetDigit(char c);
bool IsTime(const std::string& str);
Time GetTime(const std::string& str, const std::string& currentLine);

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
		const std::string time_stamp = "XX:XX";
		unsigned target_length = time_stamp.length() + 3; // "XX:XX Y "
		AssertForInput(currentLine.length() >= target_length
					   && std::isspace(currentLine[time_stamp.length()])
					   && std::isdigit(currentLine[time_stamp.length() + 1])
					   && std::isspace(currentLine[time_stamp.length() + 2])
					   && (GetDigit(currentLine[time_stamp.length() + 1]) == IN_CLIENT_COMES
					   ||  GetDigit(currentLine[time_stamp.length() + 1]) == IN_CLIENT_LEAVES
					   ||  GetDigit(currentLine[time_stamp.length() + 1]) == IN_CLIENT_SITS
					   ||  GetDigit(currentLine[time_stamp.length() + 1]) == IN_CLIENT_WAITS), currentLine);
		
		this->time = GetTime(currentLine.substr(0, time_stamp.length()), currentLine);
		
		std::istringstream iss(currentLine.substr(time_stamp.length() + 1));
		
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
	
	friend std::ostream& operator<<(std::ostream& os, const Event& event);
};

std::ostream& operator<<(std::ostream& os, const Event& event)
{	
	os << event.time << ' ' << event.ID << ' ' << event.name;
	if (event.ID == IN_CLIENT_SITS || event.ID == OUT_CLIENT_SITS)
		os << ' ' << event.deskNumber;
	return os;
}

struct WorkingHours final
{
	Time openTime;
	Time closeTime;
};

class ComputerClub final
{
public:
	std::queue<std::string> waitingClients;
	std::set<std::string> clients;
	std::vector<Event> events;
	std::vector<Desk> desks;
	WorkingHours workingHours;
	bool error;
	int nDesks;
	int hourCost;
	
	ComputerClub(int nDesks) : desks(nDesks) {} 
	
	void HandleClientComes(const Event& currentEvent);
	void HandleClientSits(const Event& currentEvent);
	void HandleClientWaits(const Event& currentEvent);
	void HandleClientLeaves(const Event& currentEvent);

	void KickOutRemainingClients();
	void PrintOutputToConsole();
	void HandleEvents(std::ifstream& inFile);
	bool AllDesksTaken();
	void AddEventRaiseError(const Time& time, EventID eventID, const std::string& message);
};

std::ifstream GetInputFile(int argc, char* path);
WorkingHours GetWorkingHours(std::ifstream& inFile);
int GetNumber(std::ifstream& inFile);

int main(int argc, char* argv[])
{
	std::ifstream InFile = GetInputFile(argc, argv[1]);
	int NDesks 				  = GetNumber(InFile);
	ComputerClub ComputerClub(NDesks);
	ComputerClub.nDesks	      = NDesks;
	ComputerClub.workingHours = GetWorkingHours(InFile);
	ComputerClub.hourCost 	  = GetNumber(InFile);
	ComputerClub.HandleEvents(InFile);
	ComputerClub.PrintOutputToConsole();
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

std::string GetLine(std::ifstream& inFile)
{
	std::string str;
	std::getline(inFile, str);
	return str;
}

void ComputerClub::HandleEvents(std::ifstream& inFile)
{
	Time LastEventTime = this->workingHours.openTime;
	while(true)
	{
		std::string CurrentLine = GetLine(inFile);
		if (CurrentLine.empty())
			break;
		
		Event CurrentEvent(CurrentLine, this->nDesks);
		this->events.push_back(CurrentEvent);
		this->error = false;
		switch (CurrentEvent.ID)
		{
			case IN_CLIENT_COMES:
				HandleClientComes(CurrentEvent);
				break;
			case IN_CLIENT_SITS:
				HandleClientSits(CurrentEvent);
				break;
			case IN_CLIENT_WAITS:
				HandleClientWaits(CurrentEvent);
				break;
			case IN_CLIENT_LEAVES:
				HandleClientLeaves(CurrentEvent);
				break;
			default:
				std::cout << "Error! Invalid Event ID" << std::endl;
				exit(1);
		}
		
		if (!this->error)
		{
			AssertForInput(!(CurrentEvent.time < LastEventTime), CurrentLine);
			LastEventTime = CurrentEvent.time;
		}
	}
	
	KickOutRemainingClients();
}

bool ComputerClub::AllDesksTaken()
{
	for (const Desk& Desk: this->desks)
		if (!Desk.taken)
			return false; 
	return true;
}

WorkingHours GetWorkingHours(std::ifstream& inFile)
{
	std::string CurrentLine = GetLine(inFile);
	const std::string time_stamp = "XX:XX";
	unsigned target_length = time_stamp.length() * 2 + 1; // XX:XX YY:YY
	AssertForInput(CurrentLine.length() == target_length
				   && std::isspace(CurrentLine[time_stamp.length()]), CurrentLine);
	WorkingHours Temp;   
	Temp.openTime  = GetTime(CurrentLine.substr(0, time_stamp.length()), 				 CurrentLine);
    Temp.closeTime = GetTime(CurrentLine.substr(time_stamp.length() + 1, target_length), CurrentLine);
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
	std::string CurrentLine = GetLine(inFile);
	for (char c: CurrentLine)
		AssertForInput(std::isdigit(c) || std::isspace(c), CurrentLine);
	int Number = std::stoi(CurrentLine);
	AssertForInput(Number > 0, CurrentLine);
	return Number;
}

void ComputerClub::AddEventRaiseError(const Time& time, EventID eventID, const std::string& message)
{
	Event Event(time, eventID, message);
	this->events.push_back(Event);
	this->error = true;
}

void ComputerClub::HandleClientComes(const Event& currentEvent)
{
	auto Search = this->clients.find(currentEvent.name);
	if (Search != this->clients.end())
		this->AddEventRaiseError(currentEvent.time, OUT_ERROR, "YouShallNotPass");
	
	if (currentEvent.time < this->workingHours.openTime)
		this->AddEventRaiseError(currentEvent.time, OUT_ERROR, "NotOpenYet");
	
	if (this->workingHours.closeTime < currentEvent.time)
		this->AddEventRaiseError(currentEvent.time, OUT_ERROR, "AlreadyClosed");
	
	if (!this->error)
		this->clients.insert(currentEvent.name);
}

void ComputerClub::HandleClientSits(const Event& currentEvent)
{
	if (this->desks[currentEvent.deskNumber - 1].taken)
		this->AddEventRaiseError(currentEvent.time, OUT_ERROR, "PlaceIsBusy");
				
	auto search = this->clients.find(currentEvent.name);
	if (search == this->clients.end())
		this->AddEventRaiseError(currentEvent.time, OUT_ERROR, "ClientUnknown");
	
	for (Desk& Desk: this->desks)
		if (Desk.client == currentEvent.name)
		{
			Desk.taken  = false;
			Desk.client = "Empty";
			break;
		}

	if (!this->error)
	{
		this->desks[currentEvent.deskNumber - 1].taken 			  = true;
		this->desks[currentEvent.deskNumber - 1].startedUsingTime = currentEvent.time;
		this->desks[currentEvent.deskNumber - 1].client 		  = currentEvent.name;
	}
}

void ComputerClub::HandleClientWaits(const Event& currentEvent)
{
	if (!this->AllDesksTaken())
		this->AddEventRaiseError(currentEvent.time, OUT_ERROR, "ICanWaitNoLonger");
	
	if (this->waitingClients.size() > this->nDesks)
	{
		this->clients.erase(currentEvent.name);
		this->AddEventRaiseError(currentEvent.time, OUT_CLIENT_LEAVES, currentEvent.name);
	}
	
	if (!this->error)
		this->waitingClients.push(currentEvent.name);
}

void ComputerClub::HandleClientLeaves(const Event& currentEvent)
{
	auto search = this->clients.find(currentEvent.name);
	if (search == this->clients.end())
		this->AddEventRaiseError(currentEvent.time, OUT_ERROR, "ClientUnknown");
				
	if (!this->error)
	{
		auto search = std::find_if(this->desks.begin(), this->desks.end(),
								  [currentEvent](Desk leavingDesk) {return leavingDesk.client == currentEvent.name;});
								
        int num = std::distance(this->desks.begin(), search);
        Desk& Desk = this->desks[num];
		Desk.taken = false;
		Desk.AddTime(currentEvent.time, this->hourCost);
		if (!this->waitingClients.empty())
		{
			Desk.client = this->waitingClients.front();
			this->waitingClients.pop();
			Event TakingDeskEvent(currentEvent.time, OUT_CLIENT_SITS, Desk.client, num + 1);
			this->events.push_back(TakingDeskEvent);
			Desk.taken = true;
			Desk.startedUsingTime = currentEvent.time;
		}
		else
			Desk.client = "Empty";
	}
} 

void ComputerClub::KickOutRemainingClients()
{
	for (Desk& Desk: this->desks)
		if (Desk.taken)
		{
			Event LeavingEvent(this->workingHours.closeTime, OUT_CLIENT_LEAVES, Desk.client);
			this->events.push_back(LeavingEvent);
			Desk.AddTime(this->workingHours.closeTime, this->hourCost);
		}
}

void ComputerClub::PrintOutputToConsole()
{
	std::cout << this->workingHours.openTime << std::endl;
	
	for (const Event& CurrentEvent: this->events)
		std::cout << CurrentEvent << std::endl;
	
	std::cout << this->workingHours.closeTime << std::endl;	
	
	for (int i = 0; i < this->desks.size(); ++i)
		std::cout << i + 1 << " " << this->desks[i].money
						   << " " << this->desks[i].allUsageTime << std::endl;
}

int GetDigit(char c)
{
	return c - '0';
}

bool IsTime(const std::string& str)
{
	const std::string time_stamp = "XX:XX";
    return str.length() == time_stamp.length()
		   && std::isdigit(str[0]) && std::isdigit(str[1])
           && str[2] == ':'
           && std::isdigit(str[3]) && std::isdigit(str[4]);
}

Time GetTime(const std::string& str, const std::string& currentLine)
{
	AssertForInput(IsTime(str), currentLine);
	Time Temp;
	Temp.hours   = GetDigit(str[0]) * 10 + GetDigit(str[1]);
	Temp.minutes = GetDigit(str[3]) * 10 + GetDigit(str[4]);
	AssertForInput(Temp.hours < 24 && Temp.minutes < 60, currentLine);
    return Temp;
}