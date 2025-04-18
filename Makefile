NAME			= ircserv
FSANITNAME		= ircFsan
VALGRNAME		= ircVal

CXX				= c++
FLAGS			= -std=c++20 -Wall -Wextra -Werror #-O3
DEBUGFLAGS		= -Wpedantic -Wshadow -g -O0 -fsanitize=address -fsanitize=undefined #-v
VALGRFLAGS		= -Wpedantic -Wshadow -g -O0
RM				= rm -rf


INC			= inc/
HEADERS		=	$(INC)irc.hpp \
				$(INC)Socket.hpp

SRCDIR	=	src/
SRCS	=	$(SRCDIR)main.cpp \
			$(SRCDIR)ArgCheck.cpp \
			$(SRCDIR)Server.cpp \
			$(SRCDIR)Socket.cpp



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
	./$(FSANITNAME)

val: fclean
	echo ########################################################
	$(CXX) $(FLAGS) $(VALGRFLAGS) -o $(VALGRNAME) $(SRCS)
	valgrind --leak-check=full --show-leak-kinds=all ./$(VALGRNAME)

git: fclean
	clear
	git status
	git add .
	git status
	git commit -m "$(NAME) - save state"
	git push
	git status




.PHONY: all clean fclean re fs val git