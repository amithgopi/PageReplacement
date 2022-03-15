/**
 * @file gopi_proj2_main.c
 * @author Amith Gopi (amithgopi@tamu.edu) (431000927)
 * @brief CSCE611 Project 2. Complie using `gcc gopi_proj2_main.c -o gopi_proj2.exe -lm`
 * @version 1.0
 * @date 2022-03-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "gopi_proj2_list.c"
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define INPUT_FILENAME "gopi_proj2_input.txt"
#define OUTPUT_FILENAME_4FRAMES "gopi_proj2_output_4 frames.txt"
#define OUTPUT_FILENAME_8FRAMES "gopi_proj2_output_8 frames.txt"
#define INPUT_LENGTH 100
#define NUMBER_OF_PAGES 16
#define TIMESTAMP_FORMAT "%02d/%02d/%04d %02d:%02d:%02d:%03d"
#define STANDARD_DEVIATION 2.5
#define ENABLE_LOGGING_PAGEFRAME_LIST 0 // Set to 1 to show the page frame queue in stdout

#define FOREACH_ALGORITHM(ALGORITHM) \
        ALGORITHM(FIFO)   \
        ALGORITHM(LRU)  \
        ALGORITHM(SECOND_CHANCE) 
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum ReplacementAlgorithms {
    FOREACH_ALGORITHM(GENERATE_ENUM)
};
static const char *ReplacementAlgorithmStrings[] = {
    FOREACH_ALGORITHM(GENERATE_STRING)
};

//Global paramters
int numPageFrames = 4;
int *pageFault;
char *outputFileName = OUTPUT_FILENAME_4FRAMES;

//Function declarations
void createInputFile();
int getUserInput();
int getPageFromInputFile(FILE*);
void setNumberOfFrames();
void runReplacementAlgorithm(int, FILE*);

int runFIFO(struct List*, struct List*, int);
int runLRU(struct List*, struct List*, int);
int runSecondChance(struct List*, struct List*, int, int*);

/**
 * @brief Main function
 * 
 * @return int 
 */
int main() {

    int userChoice = 0;
    enum ReplacementAlgorithms userChosenAlgorithm;
    // Seed rand
    srand(time(0));
    // Create input file if not present
    createInputFile();
    // Loop till exit
    while(1) {
        // Open input file
        FILE* inputFilePtr = fopen(INPUT_FILENAME, "r");
        // Get user input for algorithm
        userChoice = getUserInput();
        // Check exit condition
        if (userChoice == 4) break;
        // Get number of page frames from user
        setNumberOfFrames();
        // Run the selected algorithm on input
        runReplacementAlgorithm(userChoice, inputFilePtr);
        //Close input file
        fclose(inputFilePtr);
    }
    return 0;
}

void setNumberOfFrames() {
    //Get number of page frames from user
    do {
        printf("Choose Number of frames (4 or 8): ");
        scanf("%d", &numPageFrames);
        if(numPageFrames != 4 && numPageFrames != 8) {
            printf("Invalid Choice.\n");
        }
    } while (numPageFrames != 4 && numPageFrames != 8);

    printf("Selected memory with %d page frames.\n", numPageFrames);
    // Set output fileName
    if(numPageFrames == 4) {
        outputFileName = OUTPUT_FILENAME_4FRAMES;
    } else {
        outputFileName = OUTPUT_FILENAME_8FRAMES;
    }
}

/**
 * @brief Read one line of input from input file
 * This would give the number of the next page to take as input
 * 
 * @param filePtr 
 * @return int 
 */
int getPageFromInputFile(FILE *filePtr) {
    int page = -1;
    fscanf(filePtr, "%d\n", &page);
    return page;
}

/**
 * @brief Generate the current system timestanp and format it as a string
 * 
 * @return char* 
 */
char* getTimeStamp() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    struct tm *now = localtime(&currentTime);
    char *timeStamp = malloc(sizeof(char)*24);
    snprintf(timeStamp, 24, TIMESTAMP_FORMAT, now->tm_mon + 1, now->tm_mday, now->tm_year + 1900, now->tm_hour, now->tm_min, now->tm_sec, currentTime.tv_usec/1000);
    return timeStamp;
}

/**
 * @brief Print thr formatted output for each page input, to the output file with timestamp, page and page fault info.
 * This also prints the same string to stdout.
 * 
 * @param filePtr File pointer for the output file
 * @param page Input page
 * @param fault 1 if page gets a fault else 0
 */
void printPageStatusToFile(FILE* filePtr, int page, int fault) {
    char pageStatusString[50];
    snprintf(pageStatusString, 50, "%s %02d %s\n", getTimeStamp(), page, fault ? "FAULT" : "HIT");
    fprintf(filePtr, pageStatusString);
    printf(pageStatusString);
}

/**
 * @brief Run the selected algorithm in the input file and generate the output
 * 
 * @param algorithmChoice User choice of the algorithm
 * @param filePtr Pointer to input file
 */
void runReplacementAlgorithm(int algorithmChoice, FILE* filePtr) {
    // Init variables
    int page = NUMBER_OF_PAGES + 1; // Init page to a value not possible in the input list that is not -1
    int fault;  //Indicate if current page genrates a fault
    int faultCount = 0;
    // Open output file
    FILE *outputPtr = fopen(outputFileName, "a");
    //Print output header
    fprintf(outputPtr, "Running %s with %d Page frames\n", ReplacementAlgorithmStrings[algorithmChoice-1], numPageFrames);
    //Init lists/queues to store page frames
    struct List *memory = malloc(sizeof(struct List));
    memory->front = NULL;
    memory->back = NULL;
    memory->size = 0;
    struct List *recent = malloc(sizeof(struct List));
    recent->front = NULL;
    recent->back = NULL;
    recent->size = 0;
    // Array to hold the referenced bit
    int referenced[NUMBER_OF_PAGES] = {[0 ... (NUMBER_OF_PAGES-1)] = 0};
    // Loop over all entries in the input
    for(int i=0; i<INPUT_LENGTH; i++) {
        page = getPageFromInputFile(filePtr);
        if(page == -1) break;
        // selct algorithm to run based on user input     
        switch (algorithmChoice) {
            case 1:
                fault = runFIFO(memory, recent, page);
                break;
            case 2:
                fault = runLRU(memory, recent, page);
                break;
            case 3:
                fault = runSecondChance(memory, recent, page, referenced);
                break;
            default:
                break;
        }
        // Increment number of faults if page added creates a fault  
        if(fault) faultCount++;
        // Print the log for the current page
        printPageStatusToFile(outputPtr, page, fault);
        if(ENABLE_LOGGING_PAGEFRAME_LIST) print_list(memory);
    }
    // Print details of total number of faults in the run
    fprintf(outputPtr, "Total Number of Faults: %d", faultCount);
    fprintf(outputPtr, "\n\n");
    printf("\n");
    // Free resources
    fclose(outputPtr);
    free(memory);
    free(recent);
}


/**
 * @brief Get user input to choose a replacement algorithm
 * 
 * @return int 
 */
int getUserInput() {
    int userChoice = 0;
    printf("1. FIFO\n2. LRU\n3. Second Chance\n4. Exit\n");
    printf("Choose a page replacemnt algorithm (1, 2, or 3): ");
    scanf("%d", &userChoice);
    while(userChoice == 0) {
        if(userChoice <= 0 || userChoice > 4) {
        printf("Invaid Choice\n");
        userChoice = 0;
        }
    }
    return userChoice;
}

/**
 * @brief Create an input file with a random sequence of integers from [0, NUMBER_OF_PAGES-1]. The file is created
 * if it does not exist, else do nothing
 * 
 */
void createInputFile() {
    FILE *filePtr = fopen(INPUT_FILENAME, "r");
    // Check if file does not exist
    if (!filePtr) {
        filePtr = fopen(INPUT_FILENAME, "w");
        for(int i=0; i<INPUT_LENGTH; i++) {
            // Generate normally distributed random variable with mean (NUMBER_OF_PAGES-1)/2 = 7.5 and standard deviation = STANDARD_DEVIATION
            float r1 = ((rand() / (RAND_MAX + 1.0)));
            float r2 = ((rand() / (RAND_MAX + 1.0)));
            float normalRand = sqrt(-2.0*log(r1))*cos(2*3.14159*r2)*STANDARD_DEVIATION + (NUMBER_OF_PAGES-1)/2.0;
            if (normalRand < 0.5) normalRand = 0;
            if (normalRand > NUMBER_OF_PAGES-1.5) normalRand = NUMBER_OF_PAGES-1;
            fprintf(filePtr, "%d\n", (int)round(normalRand) );
        }
    }
    fclose(filePtr);
}

/**
 * @brief FIFO Algorithm
 * 
 */
int runFIFO(struct List* memory, struct List* recent, int page) {
        struct Node* frame = find(memory, page);
        int pageFault = 1;
        //If next page is in recent histroy queue remove it
        struct Node* recentFrame = find(recent, page);
        if(recentFrame != NULL) {
            remove_element(recent, recentFrame);
        }
        //Check if input page in memory
        if(frame == NULL) {
            //Check if memory is full
            if(memory->size == numPageFrames) {
                int last = pop_back(memory);
                //Remove page first arrived at back of queue
                //If recent history is full remove an entry
                if(recent->size == numPageFrames) {
                    pop_back(recent); 
                }
                //Push removed page to recent history list
                push_front(recent, last);    
            }
            //Put new page at front of queue
            push_front(memory, page);
        } else {
            // Set pagefault to false if present in memory
            pageFault = 0;
        }
        return pageFault;
}



/**
 * @brief Least recently used algorithm
 * 
 */
int runLRU(struct List* memory, struct List* recent, int page) {
    struct Node* frame = find(memory, page);
    int pageFault = 1;
    //If next page is in recent histroy queue remove it
    struct Node* recentFrame = find(recent, page);
    if(recentFrame != NULL) {
        remove_element(recent, recentFrame);
    }
    //Check if input page in memory
    if(frame == NULL) {
        pageFault = 1;
        //Check if memory is full
        if(memory->size == numPageFrames) {
            int last = pop_back(memory); //Remove least recently used oage at bottom of the stack
            //If recent history is full remove an entry
            if(recent->size == numPageFrames) {
                pop_back(recent); 
            }
            push_front(recent, last); 
        }
    } else {
        // Set pagefault to false if present in memory
        pageFault = 0;
        // Remove the page from the current stack position
        remove_element(memory, frame);
    }
    // Place it back on top of the stack
    push_front(memory, page);
    
    return pageFault;

}


/**
 * @brief Second change replacement algorithm
 * 
 */
int runSecondChance(struct List* memory, struct List* recent, int page, int* referenced) {
    struct Node* frame = find(memory, page);
    int pageFault = 1;
    //If next page is in recent histroy queue remove it
    struct Node* recentFrame = find(recent, page);
    if(recentFrame != NULL) {
        remove_element(recent, recentFrame);
    }
    //Check if input page in memory
    if(frame == NULL) {
        if(memory->size == numPageFrames) {
            int last = get_back(memory);
            // Keep checking referenced bit till it is 0
            while(referenced[last]) {
                // Pop out a page from back of queue with r = 1 
                pop_back(memory);
                // Clear r bit and place it back at the front
                referenced[last] = 0;
                push_front(memory, last);
                last = get_back(memory);
            }
            // Pop out a page from back of queue with ref bit set to 0
            last = pop_back(memory);
            //If recent history is full remove an entry
            if(recent->size == numPageFrames) {
                pop_back(recent); 
            }
            //Add the page removed from memory to the recent list
            push_front(recent, last);
            // Unset referenced flag for the removed page 
            referenced[last] = 0;   
        }
        // Place new page at the front of the queue
        push_front(memory, page);
        // referenced[page] = 1;
    } else {
        // Page in memory, set pageFault to false and do nothing
        pageFault = 0;
        referenced[page] = 1;
    }
    return pageFault;   
}