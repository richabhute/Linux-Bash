#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 

#define max 2000

struct bg_pro{
 pid_t pid;
 char command[1024];
 struct bg_pro* next;
}bg_pro; 

int bg_count = 0;
struct bg_pro* root = NULL;;

void bg_list()
{
	//printf("%d\n", bg_count );
	if (bg_count > 0)
	{
        if(root->next == NULL)
        {
	    	printf("%s %d\n",root->command, root->pid);
        }
        else
        {
		    struct bg_pro* current = root->next;
		    printf("%s %d\n",root->command, root->pid);
		    while (current->next != NULL)
		    {
		        current = current->next;
		        printf("%s %d\n",current->command, current->pid);
	    	}
        }
    	
    	printf("Background Processes: %d\n", bg_count);
    }else
    {
		printf("No processes running\n");
    }

}

// this function is based on the 
//psudocode from the slides
void check_bgProcess()
{
	if(bg_count > 0)
	{
		pid_t ter = waitpid(0, NULL, WNOHANG);
		while(ter > 0)
		{
			if(ter > 0)
			{
				if(root->pid == ter)
				{
					printf("%d %s has terminated\n", root->pid, root->command);
					root = root->next;
				}else
				{
					struct bg_pro* curr = root;
					while(curr->next->pid == ter)
					{
						printf("%d %s has terminated\n", curr->next->pid, curr->next->command);
						curr->next = curr->next->next;
					}
				}
			}

			ter = waitpid(0, NULL, WNOHANG);
		}
	}
}

// background execution along with the count 
void backgroundExec(char** input, int bg_count)
{	

	//printf("%s\n",*input );
	
	pid_t pid = fork();

	if(pid == -1)
	{
		printf("Failed\n");
		return;
	}else if (pid > 0)
	{
		
		if(bg_count == 0)
		{
			bg_count++;
			root = malloc(sizeof(bg_pro)*5);
			root->pid = pid;
			root->next = NULL;
			strcat(root->command, input[0]);	
		}	
		else
		{	
			bg_count++;

			struct bg_pro* current = root;
			while(current -> next != NULL)
			{
				current = current->next;
			}

			current->next->pid = pid;
			strcat(current->next->command, input[0]);
			current->next->next = NULL;

			return;
		}
	
		waitpid(pid, NULL, WNOHANG);
		return;
		
	}else
	{
		execvp(input[0], input);
		exit(0);
	}	
}

//executes all user input
void basic_execution(char** input)
{
	pid_t pid = fork();

	if(pid == -1)
	{
		printf("Failed\n");
		return;
	}else if (pid > 0)
	{
		wait(NULL);	
	}else
	{		
		execvp(input[0], input);
		//exit(1);
	}
	//return 0;
}

// this function executes non-built-in commands
int more_commands(char* input)
{
	char* command1 = "cd\0";
	char* command2 = "cd ~";
	char* command3 = "~\0";

	if ((strcmp(input,command1) == 0) || (strcmp(input,command2) == 0) || (strcmp(input,command3) == 0))
	{
		chdir(getenv("HOME"));
		return 1;
	}

	return 0;
}

int command(char** input)
{
	char* command = "cd";
	char* bg = "bg";
	char* bglist = "bglist";
	
	if (strcmp(input[0],command) == 0)
	{
		chdir(input[1]);
		return 1;
	}

	if(strcmp(input[0], bg) == 0)
	{
		backgroundExec(input+1, bg_count);
	}

	if(strcmp(input[0], bglist) == 0)
	{
		bg_list();
	}

	return 0;	
}


// function separates the spaces from user input
int parsed(char* input, char** list)
{
	if(more_commands(input))
	{
		return 0;
	}
	else
		for(int x = 0; x < max; x++)
		{
			list[x] = strsep(&input, " ");

		 	if(list[x] == NULL)
		 	{
		 		break;
		 	}

		 	if(strlen(list[x]) == 0)
		 	{
		 		x--;
		 	}
		} 

		if(command(list))
		{
			return 0;
		}
	
	return 1;
}

// executes the SSI output
void start_myshell()
{
	char* username = getlogin();
	char hostname[1024];
    gethostname(hostname, 1024);
    char cwd[1024]; 
	getcwd(cwd, sizeof(cwd));

    printf("SSI: %s@%s: %s ",username, hostname, cwd); 
}

int read_line(char* input)
{
	char* line = readline(" > ");
	if(strlen(line)!= 0)
	{
		add_history(line);
		strcpy(input, line);
		return 0;
	}
	else
	{
		return 1;
	}
}

int main(int argc, char const *argv[])
{
	char user_input[max];
	char* list[max];

	while(1)
	{
		//SSI output for the shell
		start_myshell();
		if(read_line(user_input))
		{
			continue;
		}
		
		// when true represents builtin commmands	
		if(parsed(user_input, list) == 1) 
		{
			basic_execution(list);
		}

		check_bgProcess();
	}

	return 0;
}


