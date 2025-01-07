set -xe

gcc -O1 -o prog main.c -lraylib -lm -ggdb -Wall -Wextra -Werror \
 -Wpedantic \
 -Wfloat-equal \
 -Wshadow \
 -Wswitch-enum \
 -Wcast-qual \
 -Wconversion \
 -Wundef \
 -Wunused-macros \
 -Wduplicated-cond \
 -Wduplicated-branches \
 #-fsanitize=undefined \
 #-fsanitize=float-divide-by-zero \
 #-fsanitize=integer-divide-by-zero \
 #-fsanitize=leak \
 #-fsanitize=address \
 #-Wbad-function-cast \
