// serialize.h
//
// Macros to write/read scalars and arrays to/from file.


#define	SERIALIZESCALAR(F, OUT, V) \
{	\
	if (OUT) \
	{ \
		const size_t written = fwrite(&(V), sizeof(V), 1, F); \
		ASSERT(written != static_cast<size_t>(-1)); \
		ASSERT(written == 1); \
	} \
	else \
	{ \
		const size_t numread = fread(&(V), sizeof(V), 1, F); \
		ASSERT(numread == 1); \
	} \
}


#define	SERIALIZEARRAY(F, OUT, V, CNT) \
{	\
	ASSERT(CNT>0); \
	if (OUT) \
	{ \
		const size_t written = fwrite(V, sizeof(V[0]), CNT, F); \
		ASSERT(written != static_cast<size_t>(-1)); \
		ASSERT(written == CNT); \
	} \
	else \
	{ \
		const size_t numread = fread( V, sizeof(V[0]), CNT, F); \
		ASSERT(numread == CNT); \
	} \
}


