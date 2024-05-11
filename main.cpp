#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <set>

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
bool InvalidEventTime(const std::string& currentLine);

class Desk final
{
public:
	Time startedUsingTime;
	Time allUsageTime;
	std::string client;
	bool taken;
	int money;
	
	Desk(const Time& startedUsingTime = Time(), const Time& allUsageTime = Time(), const std::string& client = "Empty", bool taken = false, int money = 0) :
		startedUsingTime(startedUsingTime), allUsageTime(allUsageTime), client(client), taken(taken), money(money) {}
	
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
};
bool AllDesksTaken(const std::vector<Desk>& desks);

class Event final
{
public:
	Time time;
	int ID;
	std::string name;
	int deskNumber;	
	
	Event(const Time& time = Time(), int ID = 0, const std::string& name = "Empty", int deskNumber = -1) : time(time), ID(ID), name(name), deskNumber(deskNumber) {}
	
	Event(const std::string& currentLine, int nDesks)
	{
		if (InvalidEventTime(currentLine))
		{
			std::cout << currentLine << std::endl;
			abort();
		}
		
		std::istringstream iss(currentLine);
		char delimiter;
		iss >> this->time.hours >> delimiter >> this->time.minutes;
		iss >> this->ID;
		if (this->ID < 1 || this->ID > 4)
		{
			std::cout << currentLine << std::endl;
			abort();
		}
		
		iss >> this->name;
		if (!this->name.length())
		{
			std::cout << currentLine << std::endl;
			abort();
		}
		
		for (char c: this->name)
			if (!(c >= 'a' && c <= 'z' || c >= '0' && c <= '9' || c == ' ' || c == '-'))
			{
				std::cout << currentLine << std::endl;
				abort();
			}
	
		std::string deskNumberString;
		iss >> deskNumberString;	
		if (this->ID == 2)
		{
			if (!deskNumberString.length())
			{
				std::cout << currentLine << std::endl;
				abort();
			}
			this->deskNumber = std::stoi(deskNumberString);
		}
		else
			this->deskNumber = -1;
		
		
		if (this->deskNumber < -1 || this->deskNumber > nDesks)
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
		if (this->ID == 2 || this->ID == 12)
		{
			out += ' ';
			out += std::to_string(this->deskNumber);
		}
		return out;
	}
};

class ComputerClub
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
	
	std::string CurrentLine;
	std::getline(InFile, CurrentLine);
	if (!std::isdigit(CurrentLine[0]))
	{
		std::cout << CurrentLine << std::endl;
		return 1;
	}
	int NDesks = std::stoi(CurrentLine);
	if (NDesks <= 0)
	{
		std::cout << CurrentLine << std::endl;
		return 1;
	}
	
	std::vector<Desk> Desks(NDesks);

	std::getline(InFile, CurrentLine);
	if (InvalidOpenCloseTime(CurrentLine))
	{
		std::cout << CurrentLine << std::endl;
		return 1;
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
		return 1;
	}
	
	std::getline(InFile, CurrentLine);
	if (!std::isdigit(CurrentLine[0]))
	{
		std::cout << CurrentLine << std::endl;
		return 1;
	}
	int HourCost = std::stoi(CurrentLine);
	if (HourCost <= 0)
	{
		std::cout << CurrentLine << std::endl;
		return 1;
	}
	
	std::cout << OpenTime.Print() << std::endl;
	
	std::vector<Event> Events;
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
		std::cout << CurrentEvent.Print() << std::endl;
	
	std::cout << CloseTime.Print() << std::endl;	
	
	
	for (int i = 0; i < Desks.size(); ++i)
		std::cout << i + 1 << " " << Desks[i].money << " " << Desks[i].allUsageTime.Print() << std::endl;
	
	
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

bool InvalidEventTime(const std::string& currentLine)
{
	if (currentLine.length() < 6)  	    return true;
	if (!std::isdigit(currentLine[0]))  return true;
	if (!std::isdigit(currentLine[1]))  return true;
	if (currentLine[2] != ':') 		    return true;
	if (!std::isdigit(currentLine[3]))  return true;
	if (!std::isdigit(currentLine[4]))  return true;
	if (currentLine[5] != ' ') 		    return true;
	
	if (currentLine[0] - '0' > 2)			    		       return true;
	if (currentLine[0] - '0' == 2 && currentLine[1] - '0' > 3) return true;
	if (currentLine[3] - '0' > 5)							   return true;
	return false;
}