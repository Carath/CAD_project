#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H


///////////////////////////////////////////////////////////////////////////
// Frameworks to be used:


// Does what 'diagnosticFullProcess()' do, albeit periodically.
// This process can be stopped by pressing either the 'q' key or ESC.
void diagnosticEventLoop(void);


// Fetches the prediagnostics from their source directory, makes a diagnostic for each of them,
// stores the result into the database, moves the prediagnostics file to other directories,
// and collects some data along the way. Returns the elapsed time. Note that this shall work
// properly no matter if new prediagnostics are added in the meantime.
double diagnosticFullProcess(void);


// Cleans the directory of processed prediagnostics, and prints some data along the way.
// The directory of failed prediagnostics is never cleaned up, for debugging purpose.
void cleanupProcess(void);


///////////////////////////////////////////////////////////////////////////
// Proof of concept:


// Loop which stops when the user presses 'q' or ESC.
void demoLooping(void);


#endif
