#include "list.c"
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define INPUT_FILENAME "input.txt"
#define OUTPUT_FILENAME_4FRAMES "output_4 frames.txt"
#define OUTPUT_FILENAME_8FRAMES "output_8 frames.txt"
#define INPUT_LENGTH 100
#define NUMBER_OF_PAGES 16
#define TIMESTAMP_FORMAT "%02d/%02d/%04d %02d:%02d:%02d:%03d"

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

int numPageFrames = 4;
int *pageFault;
char *outputFileName = OUTPUT_FILENAME_4FRAMES;


void createInputFile();
int getUserInput();
int getPageFromInputFile(FILE*);
void runReplacementAlgorithm(enum ReplacementAlgorithms, FILE*);
int runFIFO(struct List*, struct List*, int);
int runLRU(struct List*, struct List*, int);
int runSecondChance(struct List*, struct List*, int, int*);
void setNumberOfFrames();

int main() {

    int userChoice = 0;
    enum ReplacementAlgorithms userChosenAlgorithm;

    srand(time(0));
    createInputFile();

    FILE* inputFilePtr = fopen(INPUT_FILENAME, "r");
    
    while (userChoice == 0) {
        userChoice = getUserInput();
    }

    switch (userChoice)
    {
        case 1:
            printf("Selected - FIFO\n");
            userChosenAlgorithm = FIFO;
            break;
        case 2:
            printf("Selected - LRU\n");
            userChosenAlgorithm = LRU;
            break;
        case 3:
            printf("Selected - Second Chance\n");
            userChosenAlgorithm = SECOND_CHANCE;
            break;
        default:
            break;
    }

    setNumberOfFrames();

    runReplacementAlgorithm(userChosenAlgorithm, inputFilePtr);

    fclose(inputFilePtr);
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

    if(numPageFrames == 4) {
        outputFileName = OUTPUT_FILENAME_4FRAMES;
    } else {
        outputFileName = OUTPUT_FILENAME_8FRAMES;
    }
}

int getPageFromInputFile(FILE *filePtr) {
    int page = -1;
    fscanf(filePtr, "%d\n", &page);
    return page;
}

char* getTimeStamp() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    struct tm *now = localtime(&currentTime);
    char *timeStamp = malloc(sizeof(char)*24);
    snprintf(timeStamp, 24, TIMESTAMP_FORMAT, now->tm_mon + 1, now->tm_mday, now->tm_year + 1900, now->tm_hour, now->tm_min, now->tm_sec, currentTime.tv_usec/1000);
    return timeStamp;
}

void printPageStatusToFile(FILE* filePtr, int page, int fault) {
    char pageStatusString[50];
    snprintf(pageStatusString, 50, "%s %02d %s\n", getTimeStamp(), page, fault ? "FAULT" : "HIT");
    fprintf(filePtr, pageStatusString);
    printf(pageStatusString);
}

void runReplacementAlgorithm(enum ReplacementAlgorithms algorithm, FILE* filePtr) {
    int page = NUMBER_OF_PAGES + 1; // Init page to a value not possible in the input list that is not -1
    int fault;
    int faultCount = 0;

    FILE *outputPtr = fopen(outputFileName, "a");
    fprintf(outputPtr, "Running %s with %d Page frames\n", ReplacementAlgorithmStrings[algorithm], numPageFrames);

    struct List *memory = malloc(sizeof(struct List));
    memory->front = NULL;
    memory->back = NULL;
    struct List *recent = malloc(sizeof(struct List));
    recent->front = NULL;
    recent->back = NULL;
    int referenced[NUMBER_OF_PAGES] = {[0 ... (NUMBER_OF_PAGES-1)] = 0};

    for(int i=0; i<INPUT_LENGTH; i++) {
        page = getPageFromInputFile(filePtr);
        if(page == -1) break;
               
        switch (algorithm) {
            case FIFO:
                fault = runFIFO(memory, recent, page);
                break;
            case LRU:
                fault = runLRU(memory, recent, page);
                break;
            case SECOND_CHANCE:
                fault = runSecondChance(memory, recent, page, referenced);
                break;
            default:
                break;
        }
        
        if(fault) faultCount++;
        printPageStatusToFile(outputPtr, page, fault);
        print_list(memory);
    }
    fprintf(outputPtr, "Total Number of Faults: %d", faultCount);
    fprintf(outputPtr, "\n\n");
    printf("\n");

    fclose(outputPtr);
    free(memory);
    free(recent);
}



int getUserInput() {
    int userChoice = 0;
    printf("1. FIFO\n2. LRU\n3. Second Chance\n4. Exit\n");
    printf("Choose a page replacemnt algorithm (1, 2, or 3): ");
    scanf("%d", &userChoice);
    if(userChoice <= 0 || userChoice > 4) {
        printf("Invaid Choice\n");
        userChoice = 0;
    }
    return userChoice;

}

void createInputFile() {
    FILE *filePtr = fopen(INPUT_FILENAME, "r");
    // Check if file does not exist
    if (!filePtr) {
        filePtr = fopen(INPUT_FILENAME, "w");
        for(int i=0; i<INPUT_LENGTH; i++) {
            fprintf(filePtr, "%d\n", rand() / (RAND_MAX + 1.0) * (NUMBER_OF_PAGES) );
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
        referenced[page] = 1;
    } else {
        // Page in memory, set pageFault to false and do nothing
        pageFault = 0;
        referenced[page] = 1;
    }
    return pageFault;   
}