#include "Server.hpp"

int is_digit(char *str)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] >= '0' && str[i] <= '9')
            i++;
        else
            return(0); 
    }
    return(1);
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Error: not valid arguments" << std::endl;
		return (1);
	}
	if (!is_digit(av[1]) || (atoi(av[1]) < 5000 || atoi(av[1]) > 32768))
	{
		std::cout << "Error: not valid arguments" << std::endl;
		return (1);
	}
	Server server(atoi(av[1]), av[2]);
	server.run();
}