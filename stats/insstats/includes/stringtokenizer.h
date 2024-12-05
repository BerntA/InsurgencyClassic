//========= Copyright © 2005, James "Pongles" Mansfield, All Rights Reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef _STRINGTOKENIZER_H_
#define _STRINGTOKENIZER_H_

#include <stdio.h>
#include <string.h>

//=========================================================
//=========================================================
class StringTokenizer
{
public:
	StringTokenizer( char *pszData, char *pszDelimiter )
	{
		m_pszData = pszData;
		m_iDataLength = strlen( pszData );
		m_pszDelimiter = pszDelimiter;
		m_iDelimiterLength = strlen( pszDelimiter );
		m_iOffset = 0;
		m_bDone = false;
	}

	virtual ~StringTokenizer( )
	{
	}

	char* NextToken( void )
	{
		m_iTokenSize = 0;
		m_szToken[ 0 ] = '\0';

		for( int i = 0; m_iOffset < m_iDataLength; i++ ) 
		{
			m_szToken[ i ] = m_pszData[ m_iOffset ];
			m_szToken[ i + 1 ] = '\0';

			for( int j = 0; j < m_iDelimiterLength; j++ )
			{
		        if( m_szToken[ i ] != m_pszDelimiter[ j ] )
					continue;
				
				m_szToken[ i ] = '\0';

				if( m_iOffset == m_iDataLength )
					m_bDone = true;

				m_iOffset++;
				m_iTokenSize = i;

				return m_szToken;
			}

			m_iOffset++;
	    }

		m_bDone = true;

		return m_szToken;
	}

	bool HasMoreTokens( void ) const
	{
	    return !m_bDone;
	}

	int GetDataLength( void ) const
	{
	    return m_iDataLength;
	}

	int GetTokenSize( void ) const
	{
	    return m_iTokenSize;
	}

	int GetNumTokens( void )
	{
	    int iTemp = m_iOffset;
	    int iNumber = 0;
		bool bDoneSwap = m_bDone;

		while( HasMoreTokens( ) )
		{
			if( NextToken( ) != NULL )
		        ++iNumber;
		}
		
		m_bDone = bDoneSwap;
    	m_iOffset = iTemp;

		return iNumber;
	}

	void Reset( void )
	{
		m_iOffset = 0;
	}

private:
  char *m_pszData;
  char *m_pszDelimiter;
  int m_iOffset;
  bool m_bDone;
  int m_iDataLength;
  int m_iDelimiterLength;
  int m_iTokenSize;
  char m_szToken[ 512 ];
};

#endif // _STRINGTOKENIZER_H_