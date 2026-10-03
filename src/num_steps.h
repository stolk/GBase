
static inline int num_120Hz_steps( double elapsed )
{
	// Our simulation frequency is 120Hz, an 8⅓  (eight one third) ms period.
	double e = 1 / 120.0;

	// We will pretend our display sync rate is one of these:
	     if ( elapsed > 4.5 * e )
		return 5;			// 24 Hz	( .. to 26.67 Hz )
	else if ( elapsed > 3.5 * e )
		return 4;			// 30 Hz	( 26.67 Hz to 34.29 Hz )
	else if ( elapsed > 2.5 * e )
		return 3;			// 40 Hz	( 34.29 Hz to 48.00 Hz )
	else if ( elapsed > 1.5 * e )
		return 2;			// 60 Hz	( 48.00 Hz to 80.00 Hz )
	else
		return 1;			// 120 Hz	( 80.00 Hz to .. )
}


static inline int num_240Hz_steps( double elapsed )
{
	// Our simulation frequency is 240Hz, a 4⅙  (four one sixth) ms period.
	double e = 1 / 240.0;

	// We will pretend our display sync rate is one of these:
	     if ( elapsed > 11.5 * e )
		return 12;			// 20.00 Hz	( .. to 20.87 Hz )
	else if ( elapsed > 10.5 * e )
		return 11;			// 21.82 Hz	( 20.87 Hz to 22.86 Hz )
	else if ( elapsed > 9.5 * e )
		return 10;			// 24 Hz	( 22.86 Hz to 25.26 Hz )
	else if ( elapsed > 8.5 * e )
		return 9;			// 26.67 Hz	( 25.26 Hz to 28.24 Hz )
	else if ( elapsed > 7.5 * e )
		return 8;			// 30 Hz	( 28.24 Hz to 32 Hz )
	else if ( elapsed > 6.5 * e )
		return 7;			// 34.29 Hz	( 32 Hz to 36.92 Hz )
	else if ( elapsed > 5.5 * e )
		return 6;			// 40 Hz	( 36.92 Hz to 43.64 Hz )
	else if ( elapsed > 4.5 * e )
		return 5;			// 48 Hz	( 43.64 Hz to 53.33 Hz )
	else if ( elapsed > 3.5 * e )
		return 4;			// 60 Hz	( 53.33 Hz to 68.57 Hz )
	else if ( elapsed > 2.5 * e )
		return 3;			// 80 Hz	( 68.57 Hz to 96 Hz )
	else if ( elapsed > 1.5 * e )
		return 2;			// 120 Hz	( 96 Hz to 160 Hz )
	else
		return 1;			// 240 Hz	( 160 Hz to .. )
}


static inline int num_480Hz_steps( double elapsed )
{
	// Our simulation frequency is 480Hz, a 2𐧶 (two one twelfth) ms period.
	double e = 1 / 480.0;

	// We will pretend our display sync rate is one of these:
	if ( elapsed > 15.5 * e )
		return 16;			// 30 Hz	( .. to 30.97 Hz )
	else if ( elapsed > 14.5 * e )
		return 15;			// 32 Hz	( 30.97 Hz to 33.10 Hz )
	else if ( elapsed > 13.5 * e )
		return 14;			// 34.29 Hz	( 33.10 Hz to 35.56 Hz )
	else if ( elapsed > 12.5 * e )
		return 13;			// 36.92 Hz	( 35.56 Hz to 38.40 Hz )
	else if ( elapsed > 11.5 * e )
		return 12;			// 40 Hz	( 38.40 Hz to 41.74 Hz )
	else if ( elapsed > 10.5 * e )
		return 11;			// 43.64 Hz	( 41.74 Hz to 45.71 Hz )
	else if ( elapsed >  9.5 * e )
		return 10;			// 48 Hz	( 45.71 Hz to 50.53 Hz )
	else if ( elapsed >  8.5 * e )
		return 9;			// 53.33 Hz	( 50.53 Hz to 56.47 Hz )
	else if ( elapsed >  7.5 * e )
		return 8;			// 60 Hz	( 56.47 Hz to 64 Hz )
	else if ( elapsed >  6.5 * e )
		return 7;			// 68.57 Hz	( 64 Hz to 73.85 Hz )
	else if ( elapsed >  5.5 * e )
		return 6;			// 80 Hz	( 73.85 Hz to 87.27 Hz )
	else if ( elapsed >  4.5 * e )
		return 5;			// 96 Hz	( 87.27 Hz to 106.67 Hz )
	else if ( elapsed >  3.5 * e )
		return 4;			// 120 Hz	( 106.67 Hz to 137.14 Hz )
	else if ( elapsed >  2.5 * e )
		return 3;			// 160 Hz	( 137.14 Hz to 192 Hz )
	else if ( elapsed >  1.5 * e )
		return 2;			// 240 Hz	( 192 Hz to 320 Hz )
	else
		return 1;			// 480 Hz	( 320 Hz to .. )
}

