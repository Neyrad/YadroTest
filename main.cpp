#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <set>

#define POISON_DESK_NUMBER -1

enum
{
	IN_CLIENT_COMES   = 1,
	IN_CLIENT_SITS	  = 2,
	IN_CLIENT_WAITS   = 3,
	IN_CLIENT_LEAVES  = 4,
	
	OUT_CLIENT_LEAVES = 11,
	OUT_CLIENT_SITS   = 12,
	OUT_ERROR		  = 13
};

class Time final
{
public:
	int hours;
	int minutes;
	
	Time(int hours = 0, int minutes = 0) : hours(hours), minutes(minutes) {}
	
	std::string Print()
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
bool InvalidOpenCloseTime(const std::string& currentLine);
bool InvalidEventTimeOrID(const std::string& currentLine);

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
	int ID;
	std::string name;
	int deskNumber;	
	
	Event(const Time& time = Time(), int ID = 0, const std::string& name = "Empty",
		  int deskNumber = POISON_DESK_NUMBER) : 
			time(time), ID(ID), name(name), deskNumber(deskNumber) {}
	
	Event(const std::string& currentLine, int nDesks)
	{
		if (InvalidEventTimeOrID(currentLine))
		{
			std::cout << currentLine << std::endl;
			abort();
		}
		
		std::istringstream iss(currentLine);
		char delimiter;
		iss >> this->time.hours >> delimiter >> this->time.minutes;
		iss >> this->ID >> this->name;
		if (!this->name.length())
		{
			std::cout << currentLine << std::endl;
			abort();
		}
		
		for (char c: this->name)
			if (!(c >= 'a' && c <= 'z' || c >= '0' && c <= '9' ||
				  c == ' ' || c == '-' || c == '_'))
			{
				std::cout << currentLine << std::endl;
				abort();
			}
	
		std::string DeskNumberString;
		iss >> DeskNumberString;
		for (char c: DeskNumberString)
			if (!(c >= '0' && c <= '9' || c == ' '))
			{
				std::cout << currentLine << std::endl;
				abort();
			}
		
		if (this->ID == IN_CLIENT_SITS)
		{
			if (!DeskNumberString.length())
			{
				std::cout << currentLine << std::endl;
				abort();
			}
			this->deskNumber = std::stoi(DeskNumberString);
		}
		else
			this->deskNumber = POISON_DESK_NUMBER;
		
		if ((this->deskNumber < 1 || this->deskNumber > nDesks) &&
			 this->deskNumber != POISON_DESK_NUMBER)
		{
			std::cout << currentLine << std::endl;
			abort();
		}
		
		std::string Remainder;
		iss >> Remainder;
		for (char c: Remainder)
			if (c != ' ')
			{
				std::cout << currentLine << std::endl;
				abort();
			}
	}
	
	std::string Print()
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

class ComputerClub final
{
public:
	std::queue<std::string> waitingClients;
	std::set<std::string> 	clients;
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: ./task test_file.txt" << std::endl;
		abort();
	}
	
	const std::string Filename = argv[1];
	ComputerClub ComputerClub;
	std::ifstream InFile;
	InFile.open(Filename);
	
	if (InFile.fail())
	{
		std::cout << "Error opening file." << std::endl;
		abort();
	}
	
	std::string CurrentLine;
	std::getline(InFile, CurrentLine);
	
	for (char c: CurrentLine)
		if (!(c >= '0' && c <= '9' || c == ' '))
		{
			std::cout << CurrentLine << std::endl;
			abort();
		}
	int NDesks = std::stoi(CurrentLine);
	if (NDesks <= 0)
	{
		std::cout << CurrentLine << std::endl;
		abort();
	}
	
	std::vector<Desk> Desks(NDesks);

	std::getline(InFile, CurrentLine);
	if (InvalidOpenCloseTime(CurrentLine))
	{
		std::cout << CurrentLine << std::endl;
		abort();
	}
	
	int OpenHours    = (CurrentLine[0] - '0') * 10 + (CurrentLine[1] - '0');
	int OpenMinutes  = (CurrentLine[3] - '0') * 10 + (CurrentLine[4] - '0');
	int CloseHours   = (CurrentLine[6] - '0') * 10 + (CurrentLine[7] - '0');
	int CloseMinutes = (CurrentLine[9] - '0') * 10 + (CurrentLine[10] - '0');
	
	Time OpenTime(OpenHours, OpenMinutes);
	Time CloseTime(CloseHours, CloseMinutes);
	
	if (CloseTime < OpenTime)
	{
		std::cout << CurrentLine << std::endl;
		abort();
	}
	
	std::getline(InFile, CurrentLine);
	for (char c: CurrentLine)
		if (!(c >= '0' && c <= '9' || c == ' '))
		{
			std::cout << CurrentLine << std::endl;
			abort();
		}
	int HourCost = std::stoi(CurrentLine);
	if (HourCost <= 0)
	{
		std::cout << CurrentLine << std::endl;
		abort();
	}
	
	std::cout << OpenTime.Print() << std::endl;
	
	std::vector<Event> Events;
	Time LastEventTime = OpenTime;
	while(true)
	{
		std::getline(InFile, CurrentLine);
		if (!CurrentLine.length())
			break;
		
		Event CurrentEvent(CurrentLine, NDesks);
		bool error = false;
		Events.push_back(CurrentEvent);
		switch (CurrentEvent.ID)
		{
			case IN_CLIENT_COMES:
			{
				auto search = ComputerClub.clients.find(CurrentEvent.name);
				if (search != ComputerClub.clients.end())
				{
					Event ErrorEvent(CurrentEvent.time, OUT_ERROR, "YouShallNotPass");
					Events.push_back(ErrorEvent);
					error = true;
					
				}
				if (CurrentEvent.time < OpenTime)
				{
					Event ErrorEvent(CurrentEvent.time, OUT_ERROR, "NotOpenYet");
					Events.push_back(ErrorEvent);
					error = true;
				}
				
				if (CloseTime < CurrentEvent.time)
				{
					Event ErrorEvent(CurrentEvent.time, OUT_ERROR, "AlreadyClosed");
					Events.push_back(ErrorEvent);
					error = true;
				}
				
				
				if (!error)
					ComputerClub.clients.insert(CurrentEvent.name);
				break;
			}
			case IN_CLIENT_SITS:
			{
				if (Desks[CurrentEvent.deskNumber - 1].taken)
				{
					Event ErrorEvent(CurrentEvent.time, OUT_ERROR, "PlaceIsBusy");
					Events.push_back(ErrorEvent);
					error = true;
				}
				
				auto search = ComputerClub.clients.find(CurrentEvent.name);
				if (search == ComputerClub.clients.end())
				{					
					Event ErrorEvent(CurrentEvent.time, OUT_ERROR, "ClientUnknown");
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
			case IN_CLIENT_WAITS:
				if (!AllDesksTaken(Desks))
				{
					Event ErrorEvent(CurrentEvent.time, OUT_ERROR, "ICanWaitNoLonger");
					Events.push_back(ErrorEvent);
					error = true;
				}
				if (ComputerClub.waitingClients.size() > NDesks)
				{
					ComputerClub.clients.erase(CurrentEvent.name);
					Event LeavingEvent(CurrentEvent.time, OUT_CLIENT_LEAVES, CurrentEvent.name);
					Events.push_back(LeavingEvent);
					error = true;
				}
				if (!error)
					ComputerClub.waitingClients.push(CurrentEvent.name);
				break;
			case IN_CLIENT_LEAVES:
			{
				auto search = ComputerClub.clients.find(CurrentEvent.name);
				if (search == ComputerClub.clients.end())
				{					
					Event ErrorEvent(CurrentEvent.time, OUT_ERROR, "ClientUnknown");
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
								Event TakingDeskEvent(CurrentEvent.time, OUT_CLIENT_SITS, 
													  Desks[i].client, i + 1);
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
				abort();
		}
		
		if (!error)
		{
			if (CurrentEvent.time < LastEventTime)
			{
				std::cout << CurrentLine << std::endl;
				abort();
			}
			LastEventTime = CurrentEvent.time;
		}
	}
	
	for (int i = 0; i < Desks.size(); ++i)
		if (Desks[i].taken)
		{
			Event LeavingEvent(CloseTime, OUT_CLIENT_LEAVES, Desks[i].client);
			Events.push_back(LeavingEvent);
			Desks[i].AddTime(CloseTime, HourCost);
		}
	
	for (Event CurrentEvent: Events)
		std::cout << CurrentEvent.Print() << std::endl;
	
	std::cout << CloseTime.Print() << std::endl;	
	
	for (int i = 0; i < Desks.size(); ++i)
		std::cout << i + 1 << " " << Desks[i].money
						   << " " << Desks[i].allUsageTime.Print() << std::endl;
	
	InFile.close();
}

bool AllDesksTaken(const std::vector<Desk>& desks)
{
	for (int i = 0; i < desks.size(); ++i)
		if (!desks[i].taken)
			return false; 
	return true;
}

bool InvalidOpenCloseTime(const std::string& currentLine)
{
	if (currentLine.length() < 11) 	    return true;
	if (!std::isdigit(currentLine[0]))  return true;
	if (!std::isdigit(currentLine[1]))  return true;
	if (currentLine[2] != ':') 		    return true;
	if (!std::isdigit(currentLine[3]))  return true;
	if (!std::isdigit(currentLine[4]))  return true;
	if (currentLine[5] != ' ') 		    return true;
	if (!std::isdigit(currentLine[6]))  return true;
	if (!std::isdigit(currentLine[7]))  return true;
	if (currentLine[8] != ':') 		    return true;
	if (!std::isdigit(currentLine[9]))  return true;
	if (!std::isdigit(currentLine[10])) return true;
	
	if (currentLine[0] - '0' > 2)			    		       return true;
	if (currentLine[0] - '0' == 2 && currentLine[1] - '0' > 3) return true;
	if (currentLine[3] - '0' > 5)			   			       return true;
	if (currentLine[6] - '0' > 2)			   			       return true;
	if (currentLine[6] - '0' == 2 && currentLine[7] - '0' > 3) return true;
	if (currentLine[9] - '0' > 5)			    		       return true;
	return false;
}

bool InvalidEventTimeOrID(const std::string& currentLine)
{
	if (currentLine.length() < 8)  	    return true;
	if (!std::isdigit(currentLine[0]))  return true;
	if (!std::isdigit(currentLine[1]))  return true;
	if (currentLine[2] != ':') 		    return true;
	if (!std::isdigit(currentLine[3]))  return true;
	if (!std::isdigit(currentLine[4]))  return true;
	if (currentLine[5] != ' ') 		    return true;
	if (!std::isdigit(currentLine[6])) 	return true;
	if (currentLine[7] != ' ') 		    return true;
	
	if (currentLine[0] - '0' > 2)			    		       return true;
	if (currentLine[0] - '0' == 2 && currentLine[1] - '0' > 3) return true;
	if (currentLine[3] - '0' > 5)							   return true;
	
	int EventID = currentLine[6] - '0';
	if (EventID != IN_CLIENT_COMES && EventID != IN_CLIENT_SITS && 
		EventID != IN_CLIENT_WAITS && EventID != IN_CLIENT_LEAVES)
	{
		std::cout << currentLine << std::endl;
		abort();
	}
	return false;
}