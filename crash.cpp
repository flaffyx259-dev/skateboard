#include "includes.h"
#include "crash.h"

#include <psapi.h>

// note: no dbghelp needed - we only need module + offset, and symbol
// resolution is done later by loading the pdb into a debugger.
// psapi is not linked either: on Win7+ EnumProcessModules / GetModuleInformation
// / GetModuleBaseName forward to K32* which already lives in kernel32.dll.

// ---------------------------------------------------------------------------
// small self contained helpers. this code runs inside an exception handler, so
// it avoids anything that could allocate or take a lock.
// ---------------------------------------------------------------------------
namespace {

	constexpr int RING_SIZE = 512;
	constexpr int RING_MASK = RING_SIZE - 1;
	constexpr int LINE_MAX = 512;

	// bounded vsnprintf wrapper - returns number of chars written (no null).
	int Fmt( char* out, size_t cap, const char* fmt, va_list args ) {
		if ( !out || cap == 0 )
			return 0;

		int n = _vsnprintf_s( out, cap, _TRUNCATE, fmt, args );

		if ( n < 0 ) {
			out[ 0 ] = '\0';
			return 0;
		}

		// make sure we always return a terminated string.
		out[ cap - 1 ] = '\0';

		return ( int )strlen( out );
	}

	int Fmt( char* out, size_t cap, const char* fmt, ... ) {
		va_list args;
		va_start( args, fmt );
		int n = Fmt( out, cap, fmt, args );
		va_end( args );

		return n;
	}

	void CopyStr( char* dst, size_t cap, const char* src ) {
		if ( !dst || cap == 0 )
			return;

		if ( !src ) {
			dst[ 0 ] = '\0';
			return;
		}

		size_t i = 0;

		for ( ; i + 1 < cap && src[ i ]; ++i )
			dst[ i ] = src[ i ];

		dst[ i ] = '\0';
	}

	struct Ring {
		char          m_lines[ RING_SIZE ][ LINE_MAX ];
		volatile long m_head;
		volatile long m_count;

		__forceinline void reset( ) {
			m_head = 0;
			m_count = 0;
		}
	};

	Ring g_ring;
	char g_path[ MAX_PATH ] = { };
	char g_tag[ 64 ] = { };
	volatile long g_installed = 0;
	volatile long g_reported = 0;

	struct ModuleEntry {
		char     m_name[ MAX_PATH ];
		uintptr_t m_base;
		uintptr_t m_size;
	};

	ModuleEntry g_modules[ 256 ];
	volatile long g_module_count = 0;

	HANDLE g_file = nullptr;

	void Raw( const char* text ) {
		if ( !g_file || !text )
			return;

		WriteFile( g_file, text, ( DWORD )strlen( text ), nullptr, nullptr );
	}

	void RawFmt( const char* fmt, ... ) {
		if ( !g_file )
			return;

		char buf[ 2048 ];

		va_list args;
		va_start( args, fmt );
		va_list copy;
		va_copy( copy, args );
		(void)Fmt( buf, sizeof( buf ), fmt, copy );
		va_end( copy );
		va_end( args );

		Raw( buf );
	}

	void BuildModuleList( ) {
		HMODULE mods[ 256 ];
		HANDLE process = GetCurrentProcess( );

		DWORD needed = 0;

		if ( !EnumProcessModules( process, mods, sizeof( mods ), &needed ) )
			return;

		const DWORD count = needed / sizeof( HMODULE );

		long n = 0;

		for ( DWORD i = 0; i < count && n < 256; ++i ) {
			MODULEINFO info{ };

			if ( !GetModuleInformation( process, mods[ i ], &info, sizeof( info ) ) )
				continue;

			ModuleEntry& e = g_modules[ n ];

			e.m_base = ( uintptr_t )info.lpBaseOfDll;
			e.m_size = ( uintptr_t )info.SizeOfImage;

			// MODULEINFO has no name field, ask the loader for it.
			if ( GetModuleBaseNameA( process, mods[ i ], e.m_name, MAX_PATH ) == 0 )
				CopyStr( e.m_name, MAX_PATH, "<unknown>" );

			++n;
		}

		g_module_count = n;
	}

	const ModuleEntry* Resolve( uintptr_t addr, uintptr_t* out_offset ) {
		for ( long i = 0; i < g_module_count; ++i ) {
			const ModuleEntry& e = g_modules[ i ];

			if ( addr >= e.m_base && addr < e.m_base + e.m_size ) {
				if ( out_offset )
					*out_offset = addr - e.m_base;

				return &e;
			}
		}

		if ( out_offset )
			*out_offset = 0;

		return nullptr;
	}

	void OpenFile( ) {
		if ( g_file )
			return;

		// next to the dll, so you always know where to look.
		HMODULE self = nullptr;

		if ( GetModuleHandleExA(
			 GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			 ( LPCSTR )&OpenFile, &self ) )
			GetModuleFileNameA( self, g_path, MAX_PATH );

		char* slash = strrchr( g_path, '\\' );

		if ( slash )
			*slash = '\0';
		else
			CopyStr( g_path, MAX_PATH, "." );

		char file[ MAX_PATH ];
		Fmt( file, sizeof( file ), "%s\\crash-%d.log", g_path, GetCurrentProcessId( ) );

		g_file = CreateFileA( file, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );

		if ( !g_file ) {
			// game folder not writable - fall back to temp.
			if ( !GetTempPathA( MAX_PATH, g_path ) )
				CopyStr( g_path, MAX_PATH, "." );

			Fmt( file, sizeof( file ), "%scrash-%d.log", g_path, GetCurrentProcessId( ) );
			g_file = CreateFileA( file, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
		}
	}

	const char* ExceptionName( DWORD code ) {
		switch ( code ) {
		case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
		case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
		case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
		case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
		case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
		default: return "EXCEPTION_UNKNOWN";
		}
	}

	void DumpRing( ) {
		const long count = g_ring.m_count;

		Raw( "\r\n---- last operations (newest last) ----\r\n" );

		const long first = ( count > RING_SIZE ) ? ( count - RING_SIZE ) : 0;

		for ( long i = first; i < count; ++i ) {
			Raw( "  " );
			Raw( g_ring.m_lines[ i & RING_MASK ] );
			Raw( "\r\n" );
		}

		Raw( "---- end of trace ----\r\n" );
		RawFmt( "(%d entries, %d dropped)\r\n", count, count > RING_SIZE ? count - RING_SIZE : 0 );
	}

	void DumpContext( const CONTEXT* ctx ) {
		Raw( "\r\n---- registers ----\r\n" );
		RawFmt( "EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X\r\n", ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx );
		RawFmt( "ESI=%08X  EDI=%08X  EBP=%08X  ESP=%08X\r\n", ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp );
		RawFmt( "EIP=%08X  EFlags=%08X\r\n", ctx->Eip, ctx->EFlags );
		RawFmt( "CS=%04X SS=%04X DS=%04X ES=%04X\r\n", ctx->SegCs, ctx->SegSs, ctx->SegDs, ctx->SegEs );
	}

	void DumpStack( const CONTEXT* ctx ) {
		Raw( "\r\n---- stack walk ----\r\n" );
		RawFmt( "  EIP %08X\r\n", ctx->Eip );

		uintptr_t off = 0;
		const ModuleEntry* mod = Resolve( ctx->Eip, &off );

		if ( mod )
			RawFmt( "       -> %s+%08X\r\n", mod->m_name, off );
		else
			RawFmt( "       -> <not inside any module>\r\n" );

		// return addresses live between ESP and the top of the stack.
		uintptr_t bottom = ctx->Esp + 0x8000;

		MEMORY_BASIC_INFORMATION mbi{ };

		if ( VirtualQuery( ( void* )ctx->Esp, &mbi, sizeof( mbi ) ) == sizeof( mbi ) ) {
			const uintptr_t region_end = ( uintptr_t )mbi.BaseAddress + mbi.RegionSize;

			if ( region_end > ctx->Esp && region_end < bottom )
				bottom = region_end;
		}

		int printed = 0;

		for ( uintptr_t p = ctx->Esp; p + 4 <= bottom && printed < 48; p += 4 ) {
			uintptr_t value = 0;

			__try {
				value = *( uintptr_t* )p;
			}
			__except ( EXCEPTION_EXECUTE_HANDLER ) {
				break;   // unreadable page - we walked off the top of the stack
			}

			uintptr_t roff = 0;
			const ModuleEntry* rmod = Resolve( value, &roff );

			if ( !rmod )
				continue;

			// a return address is never at the very start of a module, and
			// never megabytes into a small one.
			if ( roff == 0 || roff > 0x100000 )
				continue;

			RawFmt( "  [%02d] %s+%08X\r\n", printed, rmod->m_name, roff );
			++printed;
		}

		if ( !printed )
			Raw( "  <no usable frames>\r\n" );
	}

	void DumpInstruction( uintptr_t addr ) {
		Raw( "\r\n---- bytes at fault address ----\r\n" );

		uint8_t bytes[ 16 ] = { };

		__try {
			memcpy( bytes, ( const void* )addr, sizeof( bytes ) );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
			Raw( "  <could not read instruction bytes>\r\n" );
			return;
		}

		char line[ 128 ];
		int used = Fmt( line, sizeof( line ), "  " );

		for ( int i = 0; i < 16; ++i )
			used += Fmt( line + used, sizeof( line ) - used, "%s%02X", i ? " " : "", bytes[ i ] );

		RawFmt( "%s\r\n", line );
		Raw( "  (paste into a disassembler at <module>+<offset> printed above)\r\n" );
	}

	void WriteHeader( const char* reason ) {
		Raw( "================================================================\r\n" );
		Raw( "  skateboard2018 crash report\r\n" );
		Raw( "================================================================\r\n" );

		char exe[ MAX_PATH ] = { };
		GetModuleFileNameA( nullptr, exe, MAX_PATH );

		SYSTEMTIME st{ };
		GetLocalTime( &st );

		RawFmt( "time        : %04d-%02d-%02d %02d:%02d:%02d\r\n",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
		RawFmt( "pid         : %d\r\n", GetCurrentProcessId( ) );
		RawFmt( "process     : %s\r\n", exe );
		RawFmt( "reason      : %s\r\n", reason );

		if ( g_tag[ 0 ] )
			RawFmt( "build tag   : %s\r\n", g_tag );

		RawFmt( "log path    : %s\r\n", g_path );
		Raw( "pdb         : <same folder> skateboard2018.pdb\r\n" );
		Raw( "map         : <same folder> skateboard2018.map\r\n" );
		Raw( "              load the pdb in a debugger to turn module+offset into file:line\r\n" );

		Raw( "\r\n---- loaded modules (interesting ones) ----\r\n" );

		for ( long i = 0; i < g_module_count; ++i ) {
			const ModuleEntry& e = g_modules[ i ];

			if ( !strstr( e.m_name, "skateboard" ) &&
				 !strstr( e.m_name, "client" ) &&
				 !strstr( e.m_name, "engine" ) &&
				 !strstr( e.m_name, "studiorender" ) &&
				 !strstr( e.m_name, "tier0" ) &&
				 !strstr( e.m_name, "vstdlib" ) )
				continue;

			RawFmt( "  %-24s base=%08X size=%08X\r\n", e.m_name, e.m_base, e.m_size );
		}
	}

	void WriteFooter( ) {
		Raw( "\r\n================================================================\r\n" );
		Raw( "  end of report\r\n" );
		Raw( "================================================================\r\n" );
	}

	LONG CALLBACK Handler( EXCEPTION_POINTERS* info ) {
		// the vectored handler and the unhandled-exception filter both fire for
		// the same fault, so only write the report once.
		if ( InterlockedExchange( &g_reported, 1 ) != 0 )
			return EXCEPTION_CONTINUE_SEARCH;

		// the very first fault is the one worth reporting.
		OpenFile();

		if ( !g_file || !info )
			return EXCEPTION_CONTINUE_SEARCH;

		const DWORD code = info->ExceptionRecord ? info->ExceptionRecord->ExceptionCode : 0;
		const uintptr_t addr = info->ExceptionRecord ? ( uintptr_t )info->ExceptionRecord->ExceptionAddress : 0;

		BuildModuleList();
		WriteHeader( "unhandled exception" );

		RawFmt( "\r\nexception  : 0x%08X  %s\r\n", code, ExceptionName( code ) );
		RawFmt( "address    : 0x%08X\r\n", addr );

		uintptr_t off = 0;
		const ModuleEntry* mod = Resolve( addr, &off );

		if ( mod ) {
			RawFmt( "module     : %s\r\n", mod->m_name );
			RawFmt( "module+off : %s+%08X    <-- the exact spot, resolve via the pdb\r\n", mod->m_name, off );
		}
		else {
			RawFmt( "module     : <not inside any loaded module>\r\n" );
			RawFmt( "              this is a wild pointer - almost always a null or stale\r\n" );
			RawFmt( "              entity / record / animstate deref. check the trace below.\r\n" );
		}

		if ( code == EXCEPTION_ACCESS_VIOLATION && info->ExceptionRecord->NumberParameters >= 2 ) {
			const DWORD kind = info->ExceptionRecord->ExceptionInformation[ 0 ];

			RawFmt( "av type    : %s\r\n",
				kind == 0 ? "READ" : kind == 1 ? "WRITE" : kind == 2 ? "EXECUTE" : "unknown" );
			RawFmt( "av address : 0x%08X\r\n", info->ExceptionRecord->ExceptionInformation[ 1 ] );
		}

		if ( code == EXCEPTION_INT_DIVIDE_BY_ZERO || code == EXCEPTION_FLT_DIVIDE_BY_ZERO ) {
			Raw( "note       : integer divide by zero.\r\n" );
			Raw( "              if the module above is skateboard2018, the offending\r\n" );
			Raw( "              line is doing 'x / 0' or 'x % 0' on an integer.\r\n" );
		}

		if ( info->ContextRecord ) {
			DumpContext( info->ContextRecord );
			DumpInstruction( addr );
			DumpStack( info->ContextRecord );
		}

		DumpRing( );
		WriteFooter( );

		FlushFileBuffers( g_file );

		// let the game / steam crash handler do its thing as usual.
		return EXCEPTION_CONTINUE_SEARCH;
	}

} // namespace

void CrashLog::Init( const char* tag ) {
	if ( g_installed )
		return;

	g_installed = 1;
	g_ring.reset( );

	CopyStr( g_tag, sizeof( g_tag ), tag );

	OpenFile();
	BuildModuleList();

	SetUnhandledExceptionFilter( Handler );

	// vectored handler too - it runs before the SEH unwind, so we still get a
	// usable CONTEXT even when the stack is already trashed. the g_reported
	// one-shot guard makes sure the pair only produces a single report.
	AddVectoredExceptionHandler( 1, Handler );

	RawFmt( "CrashLog installed (tag='%s'), log=%s\r\n", g_tag, g_path );
}

void CrashLog::Trace( const char* fmt, ... ) {
	if ( !g_installed || !fmt )
		return;

	char buf[ LINE_MAX ];

	va_list args;
	va_start( args, fmt );
	(void)Fmt( buf, sizeof( buf ), fmt, args );
	va_end( args );

	const long slot = g_ring.m_head & RING_MASK;

	CopyStr( g_ring.m_lines[ slot ], LINE_MAX, buf );

	InterlockedIncrement( &g_ring.m_head );
	InterlockedIncrement( &g_ring.m_count );

	// mirror live so a hang or a silent lock-up still leaves a trail.
	if ( g_file ) {
		Raw( "  [T] " );
		Raw( buf );
		Raw( "\r\n" );
	}
}

void CrashLog::TraceBuf( const char* what, const void* data, int len ) {
	if ( !data || len <= 0 )
		return;

	const uint8_t* bytes = ( const uint8_t* )data;

	char line[ LINE_MAX ];
	int used = Fmt( line, sizeof( line ), "%s:", what ? what : "buf" );

	for ( int i = 0; i < len && used > 0 && used < ( int )sizeof( line ) - 5; ++i )
		used += Fmt( line + used, sizeof( line ) - used, " %02X", bytes[ i ] );

	Trace( "%s", line );
}

size_t CrashLog::Pending( ) {
	return ( size_t )g_ring.m_count;
}

const char* CrashLog::Path( ) {
	return g_path;
}

void CrashLog::Flush( const char* reason ) {
	if ( !g_installed )
		return;

	OpenFile();
	BuildModuleList();
	WriteHeader( reason ? reason : "manual flush" );
	DumpRing( );
	WriteFooter( );

	if ( g_file )
		FlushFileBuffers( g_file );
}
