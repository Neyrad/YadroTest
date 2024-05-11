#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <set>
#include <cassert>

class Time final
{
public:
	int hours;
	int minutes;
	
	Time(std::ifstream& inFile);
	Time(int hours = 0, int minutes = 0) : hours(hours), minutes(minutes) {}
	std::string Get();
	
	bool operator<(const Time& other) const
	{
        if (this->hours < other.hours)
			return true;
		if (this->hours == other.hours)
			return this->minutes < other.minutes;
		return false;
    }
};

class Event final
{
public:
	Time time;
	int ID;
	std::string name;
	int deskNumber;	

	bool Set(std::ifstream& inFile);
	Event(const Time& time = Time(), int ID = 0, const std::string& name = "Empty", int deskNumber = -1) : time(time), ID(ID), name(name), deskNumber(deskNumber) {}
	std::string Get();
};

class Desk final
{
public:
	Time startedUsingTime;
	Time allUsageTime;
	std::string client;
	bool taken;
	int money;
	
	void AddTime(const Time& currentSessionTime, int hourCost)
	{	
		int intAllUsageTime 	   =     this->allUsageTime.hours * 60 + 	 this->allUsageTime.minutes;
		int intCurrentSessionTime  =     currentSessionTime.hours * 60 +     currentSessionTime.minutes;
		int intStartedUsingTime    = this->startedUsingTime.hours * 60 + this->startedUsingTime.minutes;
		
		this->money += ((intCurrentSessionTime - intStartedUsingTime) / 60 + 1) * hourCost;
		intAllUsageTime += intCurrentSessionTime - intStartedUsingTime;
		
		this->allUsageTime.hours   = intAllUsageTime / 60;
		this->allUsageTime.minutes = intAllUsageTime % 60;
	}
	
	Desk(const Time& startedUsingTime = Time(), const Time& allUsageTime = Time(), const std::string& client = "Empty", bool taken = false, int money = 0) :
		startedUsingTime(startedUsingTime), allUsageTime(allUsageTime), client(client), taken(taken), money(money) {}
};

class ComputerClub
{
public:
	std::queue<std::string> waitingClients;
	std::set<std::string> 	clients;
};

bool AllDesksTaken(const std::vector<Desk>& desks)
{
	for (int i = 0; i < desks.size(); ++i)
		if (!desks[i].taken)
			return false; 
	return true;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: ./task test_file.txt" << std::endl;
		return 1;
	}
	
	const std::string Filename = argv[1];
	ComputerClub ComputerClub;
	std::ifstream InFile;
	InFile.open(Filename);
	
	if (InFile.fail())
	{
		std::cout << "Error opening file." << std::endl;
		return 1;
	}
	
	int NDesks;
	InFile >> NDesks;

	std::vector<Desk> Desks(NDesks);
	
	Time OpenTime(InFile);
	Time CloseTime(InFile);
	
	int HourCost;
	InFile >> HourCost;
	
	std::cout << OpenTime.Get() << std::endl;
	
	std::vector<Event> Events;
	Event CurrentEvent;
	while(CurrentEvent.Set(InFile))
	{
		bool error = false;
		Events.push_back(CurrentEvent);
		switch (CurrentEvent.ID)
		{
			case 1:
			{
				auto search = ComputerClub.clients.find(CurrentEvent.name);
				if (search != ComputerClub.clients.end())
				{
					Event ErrorEvent(CurrentEvent.time, 13, "YouShallNotPass");
					Events.push_back(ErrorEvent);
					error = true;
					
				}
				if (CurrentEvent.time < OpenTime)
				{
					Event ErrorEvent(CurrentEvent.time, 13, "NotOpenYet");
					Events.push_back(ErrorEvent);
					error = true;
				}
				
				if (!error)
					ComputerClub.clients.insert(CurrentEvent.name);
				break;
			}
			case 2:
			{
				if (Desks[CurrentEvent.deskNumber - 1].taken)
				{
					Event ErrorEvent(CurrentEvent.time, 13, "PlaceIsBusy");
					Events.push_back(ErrorEvent);
					error = true;
				}
				
				auto search = ComputerClub.clients.find(CurrentEvent.name);
				if (search == ComputerClub.clients.end())
				{					
					Event ErrorEvent(CurrentEvent.time, 13, "ClientUnknown");
					Events.push_back(ErrorEvent);
					error = true;
				}
				
				for (int i = 0; i < Desks.size(); ++i)
					if (Desks[i].client == CurrentEvent.name)
					{
						Desks[i].client = "Empty";
						break;
					}
				
				if (!error)
				{
					Desks[CurrentEvent.deskNumber - 1].taken 			= true;
					Desks[CurrentEvent.deskNumber - 1].startedUsingTime = CurrentEvent.time;
					Desks[CurrentEvent.deskNumber - 1].client 			= CurrentEvent.name;
				}
				break;
			}
			case 3:
				if (!AllDesksTaken(Desks))
				{
					Event ErrorEvent(CurrentEvent.time, 13, "ICanWaitNoLonger");
					Events.push_back(ErrorEvent);
					error = true;
				}
				if (ComputerClub.waitingClients.size() > NDesks)
				{
					ComputerClub.clients.erase(CurrentEvent.name);
					Event LeavingEvent(CurrentEvent.time, 11, CurrentEvent.name);
					Events.push_back(LeavingEvent);
					error = true;
				}
				if (!error)
					ComputerClub.waitingClients.push(CurrentEvent.name);
				break;
			case 4:
			{
				auto search = ComputerClub.clients.find(CurrentEvent.name);
				if (search == ComputerClub.clients.end())
				{					
					Event ErrorEvent(CurrentEvent.time, 13, "ClientUnknown");
					Events.push_back(ErrorEvent);
					error = true;
				}
				
				if (!error)
				{
					for (int i = 0; i < Desks.size(); ++i)
						if (Desks[i].client == CurrentEvent.name)
						{
							Desks[i].taken = false;
							Desks[i].AddTime(CurrentEvent.time, HourCost);
							if (ComputerClub.waitingClients.size())
							{
								Desks[i].client = ComputerClub.waitingClients.front();
								ComputerClub.waitingClients.pop();
								Event TakingDeskEvent(CurrentEvent.time, 12, Desks[i].client, i + 1);
								Events.push_back(TakingDeskEvent);
								Desks[i].taken = true;
								Desks[i].startedUsingTime = CurrentEvent.time;
							}
							else
								Desks[i].client = "Empty";
							break;
						}
				}
				break;
			}
			default:
				std::cout << "Error! Invalid Event ID" << std::endl;
				return 1;
		}
	}
	
	for (int i = 0; i < Desks.size(); ++i)
		if (Desks[i].taken)
		{
			Event LeavingEvent(CloseTime, 11, Desks[i].client);
			Events.push_back(LeavingEvent);
			Desks[i].AddTime(CloseTime, HourCost);
		}
	
	for (Event CurrentEvent: Events)
		std::cout << CurrentEvent.Get() << std::endl;
	
	std::cout << CloseTime.Get() << std::endl;	
	
	
	for (int i = 0; i < Desks.size(); ++i)
		std::cout << i + 1 << " " << Desks[i].money << " " << Desks[i].allUsageTime.Get() << std::endl;
	
	
	InFile.close();
}

Time::Time(std::ifstream& inFile)
{
    char delimiter;
    inFile >> this->hours >> delimiter >> this->minutes;
}

std::string Time::Get()
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

bool Event::Set(std::ifstream& inFile)
{
    this->time = Time(inFile);
	inFile >> this->ID;
	
	if (!inFile)
		return false;
	
	inFile >> this->name;
	if (this->ID == 2)
	{	
		if (!inFile)
			return false;
		inFile >> this->deskNumber;
	}
	else
		this->deskNumber = -1;
	
	return true;
}

std::string Event::Get()
{
	std::string out;	
	out += this->time.Get();
	out += ' ';
	out += std::to_string(this->ID);
	out += ' ';
	out += this->name;
	if (this->ID == 2 || this->ID == 12)
	{
		out += ' ';
		out += std::to_string(this->deskNumber);
	}
	return out;
}