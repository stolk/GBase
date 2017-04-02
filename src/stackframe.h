// FROM:
// https://spin.atomicobject.com/2013/01/13/exceptions-stack-traces-c/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <execinfo.h>

#include "logx.h"

#if defined( __APPLE__ )
#	include <mach/mach_init.h>
#	include <sys/sysctl.h>
#	include <mach/mach_vm.h>
#endif

static void (*stackframe_presenter)( const char* );

static const char* stackframe_program_name;

#if defined( __APPLE__ )
static vm_map_offset_t process_load_address( void )
{
	mach_port_name_t task = current_task();
	vm_map_offset_t vmoffset;
	vm_map_size_t vmsize;
	uint32_t nesting_depth = 0;
	struct vm_region_submap_info_64 vbr;
	mach_msg_type_number_t vbrcount = 16;
	kern_return_t kr;

	kr = mach_vm_region_recurse(task, &vmoffset, &vmsize, &nesting_depth, (vm_region_recurse_info_t)&vbr, &vbrcount);

	if ( kr != KERN_SUCCESS )
		LOGE( "mach_vm_region_recurse() failed." );
	return vmoffset;
}
#endif
 
/* Resolve symbol name and source location given the path to the executable and an address */
const char* addr2line( char const * const program_name, void const * const addr )
{
	static char addr2line_cmd[ 1024 ] = {0};
	static char output[ 1024 ];
 
	/* have addr2line map the address to the relent line in the code */
#ifdef __APPLE__
	/* apple does things differently... */
	vm_map_offset_t loadaddr = process_load_address();
	snprintf( addr2line_cmd, sizeof(addr2line_cmd), "atos -o %.256s -l %p %p", stackframe_program_name, (void*)loadaddr, addr );
	LOGI( "cmd: %s", addr2line_cmd );
#else
	snprintf( addr2line_cmd, sizeof(addr2line_cmd), "addr2line -C -f -p -e %.256s %p", stackframe_program_name, addr );
#endif
 
	/* This will print a nicely formatted string specifying the function and source line of the address */
	FILE* f = popen( addr2line_cmd, "r" );
	output[ 0 ] = 0;
	if ( f )
	{
		size_t rv = fread( output, 1, sizeof( output ) - 1, f );
		output[ rv ] = 0;
		if ( rv > 0 ) output[ rv-1 ] = 0;	// remove newline character.
		pclose( f );
		LOGE( "%s", output );
	}
	return output;
}


#define MAX_STACK_FRAMES 96
static void *stack_traces[ MAX_STACK_FRAMES ];
static char frametext[ 4096 ];
void posix_print_stack_trace( void )
{
	int i, trace_size = 0;
	int filled=0;
	char **messages = (char **)NULL;
	frametext[ 0 ] = 0;
 
	trace_size = backtrace(stack_traces, MAX_STACK_FRAMES);
	messages = backtrace_symbols(stack_traces, trace_size);
 
	/* skip the first couple stack frames (as they are this function and our handler) and also skip the last frame as it's (always?) junk. */
	// for (i = 3; i < (trace_size - 1); ++i)
	for (i = 0; i < trace_size; ++i)
	{
		const char* line = addr2line( stackframe_program_name, stack_traces[ i ] );
		const size_t len = strlen( line );
		if ( len >= 8 )
		{
			strncat( frametext, line, sizeof(frametext) - filled );
			filled += len;
			strncat( frametext, "\n", sizeof(frametext) - filled );
			filled += 1;
		}
	}
	stackframe_presenter( frametext );
	if (messages) { free(messages); } 
}

 
static void posix_signal_handler( int sig, siginfo_t *siginfo, void *context )
{
	(void)context;
	const char* m = "Huh?";
	switch(sig)
	{
		case SIGSEGV:
			m = "Caught SIGSEGV: Segmentation Fault";
			break;
		case SIGINT:
			m = "Caught SIGINT: Interactive attention signal, (usually ctrl+c)";
			break;
		case SIGFPE:
			switch(siginfo->si_code)
			{
				case FPE_INTDIV:
					m = "Caught SIGFPE: (integer divide by zero)";
					break;
				case FPE_INTOVF:
					m = "Caught SIGFPE: (integer overflow)";
					break;
				case FPE_FLTDIV:
					m = "Caught SIGFPE: (floating-point divide by zero)";
					break;
				case FPE_FLTOVF:
					m = "Caught SIGFPE: (floating-point overflow)";
					break;
				case FPE_FLTUND:
					m = "Caught SIGFPE: (floating-point underflow)";
					break;
				case FPE_FLTRES:
					m = "Caught SIGFPE: (floating-point inexact result)";
					break;
				case FPE_FLTINV:
					m = "Caught SIGFPE: (floating-point invalid operation)";
					break;
				case FPE_FLTSUB:
					m = "Caught SIGFPE: (subscript out of range)";
					break;
				default:
					m = "Caught SIGFPE: Arithmetic Exception";
					break;
			}
			break;
		case SIGILL:
			switch(siginfo->si_code)
			{
				case ILL_ILLOPC:
					m = "Caught SIGILL: (illegal opcode)";
					break;
				case ILL_ILLOPN:
					m = "Caught SIGILL: (illegal operand)";
					break;
				case ILL_ILLADR:
					m = "Caught SIGILL: (illegal addressing mode)";
					break;
				case ILL_ILLTRP:
					m = "Caught SIGILL: (illegal trap)";
					break;
				case ILL_PRVOPC:
					m = "Caught SIGILL: (privileged opcode)";
					break;
				case ILL_PRVREG:
					m = "Caught SIGILL: (privileged register)";
					break;
				case ILL_COPROC:
					m = "Caught SIGILL: (coprocessor error)";
					break;
				case ILL_BADSTK:
					m = "Caught SIGILL: (internal stack error)";
					break;
				default:
					m = "Caught SIGILL: Illegal Instruction";
					break;
			}
			break;
		case SIGTERM:
			m = "Caught SIGTERM: a termination request was sent to the program";
			break;
		case SIGABRT:
			m = "Caught SIGABRT: usually caused by an abort() or assert()";
			break;
		default:
			break;
	}
	LOGE( "Signal Handler: %s", m );
	posix_print_stack_trace();
	fclose( logx_file );
	_Exit( 1 );
}


static uint8_t alternate_stack[ SIGSTKSZ ];
static void stackframe_set_signal_handler( const char* progname )
{
	stackframe_program_name = progname;
	/* setup alternate stack */
	{
		stack_t ss = {};
		/* malloc is usually used here, I'm not 100% sure my static allocation is valid but it seems to work just fine. */
		ss.ss_sp = (void*)alternate_stack;
		ss.ss_size = SIGSTKSZ;
		ss.ss_flags = 0;
 
		if (sigaltstack(&ss, NULL) != 0) { err(1, "sigaltstack"); }
	}
 
	/* register our signal handlers */
	{
		struct sigaction sig_action = {};
		sig_action.sa_sigaction = posix_signal_handler;
		sigemptyset(&sig_action.sa_mask);
 
#ifdef __APPLE__
		/* for some reason we backtrace() doesn't work on osx when we use an alternate stack */
		sig_action.sa_flags = SA_SIGINFO;
#else
		sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
#endif
 
		if (sigaction(SIGSEGV, &sig_action, NULL) != 0) { err(1, "sigaction"); }
		if (sigaction(SIGFPE,  &sig_action, NULL) != 0) { err(1, "sigaction"); }
		//if (sigaction(SIGINT,  &sig_action, NULL) != 0) { err(1, "sigaction"); }
		if (sigaction(SIGILL,  &sig_action, NULL) != 0) { err(1, "sigaction"); }
		if (sigaction(SIGTERM, &sig_action, NULL) != 0) { err(1, "sigaction"); }
		if (sigaction(SIGABRT, &sig_action, NULL) != 0) { err(1, "sigaction"); }
	}
}


