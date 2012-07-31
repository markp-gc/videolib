#include "RobotServer.h"

const int IMG_WIDTH  = 320;
const int IMG_HEIGHT = 240;

#include <time.h>

static double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

/**
    Setup a robot server with specified TCP and serial ports.
    
    TCP is used for a remote tele-link and the serial port is used for differential drive control (if available).
**/
RobotServer::RobotServer( int tcpPort, const char* motorSerialPort )
:
    m_serialPort( motorSerialPort ),
    m_drive ( 0 ),
    m_motors( 0 ),
    m_server( 0 ),
    m_con   ( 0 ),
    m_camera( 0 )
{   
    // Setup a server socket for receiving client commands:
    m_server = new TcpSocket();
    m_server->Bind( tcpPort );
}

RobotServer::~RobotServer()
{
    delete m_camera;
    delete m_con;
    delete m_server;
    delete m_server;
    delete m_drive;
    delete m_motors;
}

/**
    Blocks until robot gets a connection.
**/
void RobotServer::Listen()
{
    fprintf( stderr, "Waiting for new connection...\n" );
    m_server->Listen( 0 ); // Wait for connection - no queue
    m_con = m_server->Accept(); // Create connection
    m_con->SetBlocking( false );

    PostConnectionSetup();
}

/**
    Perform post conection processing.
    
    Attempts to access camera and wheels.
**/
void RobotServer::PostConnectionSetup()
{
    // Setup comms to motors:
    m_motors = new MotionMind( m_serialPort.cStr() );
    if ( m_motors->Available() )
    {
        m_drive = new DiffDrive( *m_motors );
        float amps = 1.5f;
        int32_t currentLimit = roundf( amps/0.02f );
        int32_t pwmLimit = (72*1024)/120; // motor voltage / battery voltage
        
        m_motors->WriteRegister( 1, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 2, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 1, MotionMind::PWMLIMIT, pwmLimit );
        m_motors->WriteRegister( 2, MotionMind::PWMLIMIT, pwmLimit );
    }
    else
    {
        delete m_motors;
        m_motors = 0;
    }

    // Setup camera:
    m_camera = new UnicapCamera();
    size_t imageBufferSize = m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * sizeof(uint8_t);
    if ( m_camera->IsOpen() )
    {
        m_camera->StartCapture();
        int err = posix_memalign( (void**)&m_lum, 16, imageBufferSize );
        assert( err == 0 );
    }
    else
    {
        delete m_camera;
        m_camera = 0;
    }
}

/**
    Cleans up resources once the comms loop has finished.
**/
void RobotServer::PostCommsCleanup()
{
    // These must be deleted in this order:
    delete m_drive;
    delete m_motors;
    m_drive = 0;
    m_motors = 0;
    
    if ( m_camera )
    {
        m_camera->StopCapture();
        delete m_camera;
        free( m_lum );
    }
}

/**
    Runs the robot's comms loop until the connection terminates or fails.
**/
void RobotServer::RunCommsLoop()
{
    // Setup a TeleJoystick object:
    TeleJoystick* teljoy = 0;
    if ( m_con )
    {
        Ipv4Address clientAddress;
        m_con->GetPeerAddress( clientAddress );

        {
            std::string name;
            clientAddress.GetHostName( name );
            fprintf( stderr, "Client %s connected to robot.\n", name.c_str() );
        }

        teljoy = new TeleJoystick( *m_con, m_drive ); // Will start receiving and processing remote joystick cammands immediately.
        teljoy->Go();

        GLK::Thread::Sleep( 100 );
        fprintf( stderr, "running: %d\n", teljoy->IsRunning() );

        // Start capturing and transmitting images:
        if ( m_camera )
        {
            StreamVideo( *teljoy );
        }
        else
        {
            while ( teljoy->IsRunning() ) {
                sleep(100);
            }
        }

        fprintf( stderr, "Control terminated\n" );
        
    } // end if

    delete teljoy;
    
    PostCommsCleanup();
}

/**
    Assumptions:
        m_con is not null.
*/
void RobotServer::StreamVideo( TeleJoystick& joy )
{
    assert( m_con );

    // Create a video writer object that uses socket IO:
    FFMpegSocketIO videoIO( *m_con, true );
    LibAvWriter streamer( videoIO );

    // Setup an MPEG4 video stream:
    streamer.AddVideoStream( m_camera->GetFrameWidth()/2, m_camera->GetFrameHeight()/2, 30, LibAvWriter::FourCc( 'F','M','P','4' ) );

    struct timespec t1;
    struct timespec t2;

    bool sentOk = true;
    clock_gettime( CLOCK_MONOTONIC, &t1 );
    while ( sentOk && joy.IsRunning() && m_camera->GetFrame() )
    {
        clock_gettime( CLOCK_MONOTONIC, &t2 );

        sentOk = streamer.PutYUYV422Frame( m_camera->UnsafeBufferAccess(), m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );
        m_camera->DoneFrame();
        sentOk &= videoIO.GetAVIOContext()->error >= 0;

        double grabTime = milliseconds(t2) - milliseconds(t1);
        fprintf( stderr, "%f %f %f %f\n", grabTime, streamer.lastConvertTime_ms, streamer.lastEncodeTime_ms, streamer.lastPacketWriteTime_ms );
        clock_gettime( CLOCK_MONOTONIC, &t1 );
    }
}


