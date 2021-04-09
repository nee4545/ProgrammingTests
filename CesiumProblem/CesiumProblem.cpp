// CesiumProblem.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include <string>

constexpr int lengthPerUnit = 30;
constexpr int heightPerUnit = 11;
constexpr int numRows = 512;
constexpr int numColumns = 512;

// A 2D vector struct used for distance calculation and directions
struct Vec2
{
	float x = 0.f;
	float y = 0.f;

	Vec2() {}
	~Vec2() {}

	Vec2( float x , float y)
	{
		this->x = x;
		this->y = y;
	}

	Vec2 operator+( const Vec2& vecToAdd ) const
	{
		Vec2 r = Vec2();
		r.x = x + vecToAdd.x;
		r.y = y + vecToAdd.y;
		return r;
	}

	Vec2 operator-( const Vec2& vecToSubtract ) const
	{
		Vec2 r = Vec2();
		r.x = x - vecToSubtract.x;
		r.y = y - vecToSubtract.y;
		return r;
	}

	Vec2 operator*( float uniformScale ) const
	{
		Vec2 r;
		r.x = x * uniformScale;
		r.y = y * uniformScale;
		return r;
	}

	Vec2 GetNormalized() const
	{
		float theta = atan2( y , x );
		float tempX = cos( theta );
		float tempY = sin( theta );
		return( Vec2( tempX , tempY ) );
	}

	float GetLength() const
	{
		return( sqrt( ( x * x ) + ( y * y ) ) );
	}
};


// Reads a file into a buffer and returns it along with the size of the file that is read
// Returns nullptr if anything goes wrong
void* FileReadToBuffer( std::string const& filename , size_t* outSize )
{
	FILE* fp = nullptr;

	fopen_s( &fp , filename.c_str() , "r" );
	if ( fp == nullptr )
	{
		return nullptr;
	}

	fseek( fp , 0 , SEEK_END );
	long fileSize = ftell( fp );

	unsigned char* buffer = new unsigned char[ fileSize + 1 ];
	if ( buffer != nullptr )
	{
		fseek( fp , 0 , SEEK_SET );
		size_t bytesRead = fread( buffer , 1 , ( size_t ) fileSize + 1 , fp );
		buffer[ bytesRead ] = NULL;
	}

	if ( outSize != nullptr )
	{
		*outSize = ( size_t ) fileSize;
	}

	fclose( fp );

	return buffer;
}

//Put the height values from the file that is read into a buffer
void CopyDataFromBufferToArray( unsigned char array[numRows][numColumns] , const void* buffer )
{
	unsigned char* dataPtr = ( unsigned char* ) buffer;

	for ( int i = 0; i < numRows; i++ )
	{
		for ( int j = 0; j < numColumns; j++ )
		{
			array[ i ][ j ] =  *dataPtr;
			dataPtr++;
		}
	}
}

float GetSurfaceLength( unsigned char array[ numRows ][ numColumns ] , int startPixelCoordX , int startPixelCoordY , int endPixelCoordX , int endPixelCoordY )
{
	float totalLength = 0.f;
	
	Vec2 startPosition = Vec2( ( float ) startPixelCoordX , ( float ) startPixelCoordY );
	Vec2 endPosition = Vec2( ( float ) endPixelCoordX , ( float ) endPixelCoordY );

	Vec2 distanceVec = endPosition - startPosition;

	Vec2 direction = distanceVec.GetNormalized();
	float distaceToCover = distanceVec.GetLength();
	float distanceCovered = 0.f;
	float stepSize = 0.1f;

	Vec2 previousPosition = startPosition;
	Vec2 currentPosition = previousPosition + ( direction * stepSize );
	distanceCovered += ( currentPosition - previousPosition ).GetLength();
	
	while ( true )
	{
		int previousPostionX = ( int ) previousPosition.x;
		int previousPostionY = ( int ) previousPosition.y;

		int currentPositionX = ( int ) currentPosition.x;
		int currentPositionY = ( int ) currentPosition.y;

		unsigned int difference = abs( array[ currentPositionX ][ currentPositionY ] - array[ previousPostionX ][ previousPostionY ] );
		float verticalLength = (float)(difference * heightPerUnit * stepSize);
		float horizontalLength = (currentPosition-previousPosition).GetLength() * lengthPerUnit;

		//Pythagoras theorem
		totalLength += sqrt( ( horizontalLength * horizontalLength ) + ( verticalLength * verticalLength ) );

		if ( distanceCovered >= distaceToCover )
		{
			break;
		}

		previousPosition = currentPosition;
		if ( distanceCovered + (  direction * stepSize ).GetLength() > distaceToCover )
		{
			currentPosition = previousPosition + ( direction * ( distaceToCover - distanceCovered ) );
		}
		else
		{
			currentPosition = previousPosition + ( direction * stepSize );
		}
		distanceCovered += ( currentPosition - previousPosition ).GetLength();

	}

	return totalLength;
}


int main()
{
	unsigned char preEruptionData[ numRows ][ numColumns ] = { 0 };
	unsigned char postEruptionData[ numRows ][ numColumns ] = { 0 };

	std::string preEruptionFilePath;
	std::string postEruptionFilePath;

	size_t preEruptionOutSize;
	void* preEruptionBuffer = nullptr;
	do 
	{
		std::cout << "Enter the file path for pre eruption data" << std::endl;
		std::cin >> preEruptionFilePath;
		preEruptionBuffer = FileReadToBuffer( preEruptionFilePath , &preEruptionOutSize );

		if ( preEruptionBuffer == nullptr )
		{
			std::cout << "Invalid file path for pre eruption" << std::endl;
		}
		else if ( preEruptionOutSize != numRows * numColumns )
		{
			std::cout << "File size must be 512 * 512 bytes" << std::endl;
		}
		else
		{
			std::cout << "Pre eruption file read successfully" << std::endl;
		}

	} while ( ( preEruptionBuffer == nullptr ) || ( preEruptionOutSize != numRows * numColumns ) );
	
	CopyDataFromBufferToArray( preEruptionData , preEruptionBuffer );

	size_t postEruptionOutSize;
	void* postEruptionBuffer = nullptr;

	do 
	{
		std::cout << "Enter the file path for post eruption data" << std::endl;
		std::cin >> postEruptionFilePath;

		postEruptionBuffer = FileReadToBuffer( postEruptionFilePath , &postEruptionOutSize );


		if ( postEruptionBuffer == nullptr )
		{
			std::cout << "Invalid file path for post eruption" << std::endl;
		}
		else if ( postEruptionOutSize != numRows * numColumns )
		{
			std::cout << "File size must be 512 * 512 bytes" << std::endl;
		}
		else
		{
			std::cout << "Post eruption file read successfully" << std::endl;
		}


	} while ( ( postEruptionBuffer == nullptr ) || ( postEruptionOutSize != numRows * numColumns ) );
	
	CopyDataFromBufferToArray( postEruptionData , postEruptionBuffer );

	int startPixelCoordX = -1;
	int startPixelCoordY = -1;
	int endPixelCoordX = -1;
	int endPixelCoordY = -1;

	do 
	{
		std::cout << "Enter the start pixel coordinates" << std::endl;
		std::cin >> startPixelCoordX >> startPixelCoordY;

		if ( !( startPixelCoordX >= 0 && startPixelCoordX < numRows && startPixelCoordY >= 0 && startPixelCoordY < numColumns ) )
		{
			std::cout << "Enter a valid coordinate"<<std::endl;
		}

	} while ( !( startPixelCoordX >= 0 && startPixelCoordX < numRows && startPixelCoordY >= 0 && startPixelCoordY < numColumns ) );

	do 
	{
		std::cout << "Enter the end pixel coordinates" << std::endl;
		std::cin >> endPixelCoordX >> endPixelCoordY;

		if ( !( endPixelCoordX >= 0 && endPixelCoordX < numRows && endPixelCoordY >= 0 && endPixelCoordY < numColumns ) )
		{
			std::cout << "Enter a valid coordinate" << std::endl;
		}

	} while ( !( endPixelCoordX >= 0 && endPixelCoordX < numRows && endPixelCoordY >= 0 && endPixelCoordY < numColumns ) );
	
	float preEruptionSurfaceLength = GetSurfaceLength( preEruptionData , startPixelCoordX , startPixelCoordY , endPixelCoordX , endPixelCoordY );
	float postEruptionSurfaceLength = GetSurfaceLength( postEruptionData , startPixelCoordX , startPixelCoordY , endPixelCoordX , endPixelCoordY );

	printf( "The pre eruption surface length is %f meters \n",preEruptionSurfaceLength);
	printf( "The post eruption surface length is %f meters \n" , postEruptionSurfaceLength );
	printf( "The difference is %f meters \n" , postEruptionSurfaceLength - preEruptionSurfaceLength );

	// getch function was not working, so a temp hack just to show the results before program terminates
	std::string temp;
	std::cout << "Press any key and enter to exit"<<std::endl;
	std::cin >> temp;

	return 0;
}


