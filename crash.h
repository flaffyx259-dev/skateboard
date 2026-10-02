#pragma once

// Debug crash diagnostics.
//
// The cheat runs inside csgo.exe, so when it faults there is usually no
// debugger attached and the game's own crash handler only tells you the
// exception code. this installs a vectored exception handler that writes a
// full report to disk BEFORE the process dies:
//
//   crash-<pid>.log   exception code, faulting address, owning module,
//                     module-relative offset, CONTEXT register dump,
//                     instruction bytes at the fault, and a stack walk
//                     with every frame resolved to module+offset.
//
// it also keeps a rolling ring buffer of Trace() calls, so the log ends with
// the last few hundred things the hack was doing right before it died. that
// is the "why" - the faulting frame alone never tells you which record,
// which enemy, or which resolve mode was being processed.

class CrashLog {
public:
	// call once, as early as possible ( DllMain / init thread ).
	static void Init( const char* tag = nullptr );

	// rolling trace. keep the messages short and cheap - no allocation, no
	// locks, safe to call from anywhere including the exception handler.
	static void Trace( const char* fmt, ... );

	// convenience: trace + a hex dump of a region, for spotting bad pointers.
	static void TraceBuf( const char* what, const void* data, int len );

	// number of entries currently in the ring buffer.
	static size_t Pending( );

	// path of the log file for this process.
	static const char* Path( );

	// dump the ring buffer now without crashing ( so you can inspect it live ).
	static void Flush( const char* reason );
};
