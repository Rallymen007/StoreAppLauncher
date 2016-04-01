#include "Controller.h"

typedef DWORD( WINAPI* XInputGetStateEx_t )( DWORD dwUserIndex, XINPUT_STATE *pState );
XInputGetStateEx_t XInputGetStateEx = NULL;

Controller::Controller( float x, float y )
	:m_XInputLib( nullptr ), m_Status( false )
{
	if( Init( x, y ) )
	{
		m_Status = true;
	}
	else
	{
		m_Status = false;
	}

	m_Id = -1;
	XInputGetStateEx( m_Id, &m_State );
	m_Deadzone = { x, y };
}

Controller::~Controller( )
{

}

bool Controller::GetXInputLibrary( HINSTANCE& lib )
{
	lib = LoadLibrary( L"xinput1_4.dll" );
	if( lib )
	{
		return true;
	}
	lib = LoadLibrary( L"xinput9_1_0.dll" );
	if( lib )
	{
		return true;
	}
	lib = LoadLibrary( L"xinput1_3.dll" );
	if( lib )
	{
		return true;
	}
	return false;
}

bool Controller::Init( float x, float y )
{
	if( GetXInputLibrary( m_XInputLib ) )
	{
		XInputGetStateEx = (XInputGetStateEx_t)GetProcAddress( m_XInputLib, (LPCSTR)100 );
		if( !XInputGetStateEx )
		{
			XInputGetStateEx = (XInputGetStateEx_t)GetProcAddress( m_XInputLib, "XInputGetState" );
		}
		return true;
	}
	return false;
}

int Controller::GetPort( )
{
	return m_Id;
}

XINPUT_GAMEPAD * Controller::GetState( )
{
	return &m_State.Gamepad;
}

bool Controller::CheckConnection( )
{
	int id = -1;
	for( ulong i = 0; i < XUSER_MAX_COUNT && id == -1; i++ )
	{
		XINPUT_STATE state;
		ZeroMemory( &state, sizeof( state ) );
		if( XInputGetStateEx( i, &state ) == ERROR_SUCCESS )
		{
			id = i;
		}
	}
	m_Id = id;
	return id != -1;
}

bool Controller::Update( uint timeout )
{
	if( timeout > 0 )
	{
		Sleep( timeout );
	}
	if( m_Id == -1 )
	{
		CheckConnection( );
	}
	else
	{
		ZeroMemory( &m_State, sizeof( m_State ) );
		if( XInputGetStateEx( m_Id, &m_State ) != ERROR_SUCCESS )
		{
			m_Id = -1;
			return false;
		}

		float normLX = fmaxf( -1, static_cast<float>( m_State.Gamepad.sThumbLX ) / 32767 );
		float normLY = fmaxf( -1, static_cast<float>( m_State.Gamepad.sThumbLY ) / 32767 );

		m_LeftStick = 
		{
			( abs( normLX ) < m_Deadzone.X ) ? 0 : ( abs( normLX ) - m_Deadzone.X ) * ( normLX / abs( normLX ) ),
			( abs( normLY ) < m_Deadzone.Y ) ? 0 : ( abs( normLY ) - m_Deadzone.Y ) * ( normLY / abs( normLY ) )
		};

		if( m_Deadzone.X > 0 ) 
		{
			m_LeftStick.X *= 1 / ( 1 - m_Deadzone.X );
		}
		if( m_Deadzone.Y > 0 )
		{
			m_LeftStick.Y *= 1 / ( 1 - m_Deadzone.Y );
		}

		float normRX = fmaxf( -1, static_cast<float>( m_State.Gamepad.sThumbRX ) / 32767 );
		float normRY = fmaxf( -1, static_cast<float>( m_State.Gamepad.sThumbRY ) / 32767 );

		m_RightStick =
		{
			( abs( normRX ) < m_Deadzone.X ) ? 0 : ( abs( normRX ) - m_Deadzone.X ) * ( normRX / abs( normRX ) ),
			( abs( normRY ) < m_Deadzone.Y ) ? 0 : ( abs( normRY ) - m_Deadzone.Y ) * ( normRY / abs( normRY ) )
		};

		if( m_Deadzone.X > 0 )
		{
			m_RightStick.X *= 1 / ( 1 - m_Deadzone.X );
		}
		if( m_Deadzone.Y > 0 )
		{
			m_RightStick.Y *= 1 / ( 1 - m_Deadzone.Y );
		}

		m_LeftTrigger.Value = m_State.Gamepad.bLeftTrigger;
		m_RightTrigger.Value = m_State.Gamepad.bRightTrigger;
		return true;
	}
	return false;
}

bool Controller::IsPressed( uint val )
{
	return ( m_State.Gamepad.wButtons & val ) != 0;
}

void Controller::Shutdown( )
{
	if( m_XInputLib != nullptr )
	{
		FreeLibrary( m_XInputLib );
	}	
}
