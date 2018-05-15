#ifndef SLEEPSIMULATOR_H
#define SLEEPSIMULATOR_H

/* From https://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
 *
 * QWaitCondition is designed to coordinate mutex waiting between different threads.
 * But what makes this work is the wait method has a timeout on it. When called this way,
 * it functions exactly like a sleep function, but it uses Qt's event loop for the timing.
 * So, no other events or the UI are blocked like normal windows sleep function does.
 *
 * As a bonus, we added the CancelSleep function to allows another part of the program to cancel the "sleep" function.
 *
 * What we liked about this is that it lightweight, reusable and is completely self contained.
 */

class SleepSimulator{
     QMutex localMutex;
     QWaitCondition sleepSimulator;
public:
    SleepSimulator()
    {
        localMutex.lock();
    }
    void sleep(unsigned long sleepMS)
    {
        sleepSimulator.wait(&localMutex, sleepMS);
    }
    void CancelSleep()
    {
        sleepSimulator.wakeAll();
    }
};


#endif // SLEEPSIMULATOR_H
