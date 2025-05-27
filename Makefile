NAME        = ircserv
FSANITNAME    = $(NAME)Fsan
VALGRNAME    = $(NAME)Val

CXX            = c++
FLAGS        = -std=c++17 -Wall -Wextra -Werror -Wpedantic -Wshadow #-O3 #-DNDEBUG
DEBUGFLAGS    = -g -O0 -fsanitize=address -fsanitize=undefined #-v
VALGRFLAGS    = -g -O0
RM            = rm -rf


INCDIR		= inc/
HEADERS    =    $(INCDIR)Server.hpp \
				$(INCDIR)error.hpp \
				$(INCDIR)Client.hpp \
				$(INCDIR)Channel.hpp \
				$(INCDIR)Socket.hpp \
				$(INCDIR)Config.hpp \

SRCDIR	= 	src/
SRCS    =    $(SRCDIR)main.cpp \
			 $(SRCDIR)Config.cpp \
			 $(SRCDIR)Socket.cpp \
			 $(SRCDIR)Channel.cpp \
			 $(SRCDIR)Client.cpp \
			 $(SRCDIR)Error.cpp \
			 $(SRCDIR)ProcessCmd.cpp \
			 $(SRCDIR)Server.cpp \
			 $(SRCDIR)cmd/Join.cpp \
			 $(SRCDIR)cmd/Kick.cpp \
			 $(SRCDIR)cmd/Mode.cpp \
			 $(SRCDIR)cmd/Nick.cpp \
			 $(SRCDIR)cmd/Ping.cpp \
			 $(SRCDIR)cmd/PrivMsg.cpp \
			 $(SRCDIR)cmd/Topic.cpp \
			 $(SRCDIR)cmd/User.cpp 



all: $(NAME)

$(NAME): $(SRCS) $(HEADERS)
	$(CXX) $(FLAGS) -o $(NAME) $(SRCS)

clean:
	$(RM) $(FSANITNAME) $(VALGRNAME)

fclean: clean
	$(RM) $(NAME) a.out

re: fclean all




fs: fclean
	echo ########################################################
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -o $(FSANITNAME) $(SRCS)
	./$(FSANITNAME) 6667 4242

val: fclean
	echo ########################################################
	$(CXX) $(FLAGS) $(VALGRFLAGS) -o $(VALGRNAME) $(SRCS)
	valgrind --leak-check=full --track-fds=yes --show-leak-kinds=all ./$(VALGRNAME) 6667 4242

git: fclean
	clear
	git status
	git add .
	git status
	git commit -m "$(NAME) - save state"
	git push
	git status




.PHONY: all clean fclean re fs val git

