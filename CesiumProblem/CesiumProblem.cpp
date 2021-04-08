// CesiumProblem.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include <string>

constexpr int lengthPerUnit = 30;
constexpr int heightPerUnit = 11;
constexpr int numRows = 512;
constexpr int numColumns = 512;

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
		float theta = atan2(y,x);
		float tempX = cos(theta);
		float tempY = sin(theta);
		return(Vec2(tempX,tempY));
	}

	float GetLength() const
	{
		return( sqrt( ( x * x ) + ( y * y ) ) );
	}
};


void* FileReadToBuffer( std::string const& filename , size_t* outSize )
{
	FILE* fp = nullptr;

	fopen_s( &fp , filename.c_str() , "r" );
	if ( fp == nullptr )
	{
		return nullptr;
	}

	fseek( fp , 0 , SEEK_END );
	long file_size = ftell( fp );

	unsigned char* buffer = new unsigned char[ file_size + 1 ];
	if ( buffer != nullptr )
	{
		fseek( fp , 0 , SEEK_SET );
		size_t bytes_read = fread( buffer , 1 , ( size_t ) file_size + 1 , fp );
		buffer[ bytes_read ] = NULL;
	}

	if ( outSize != nullptr )
	{
		*outSize = ( size_t ) file_size;
	}

	fclose( fp );

	return buffer;
}

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
	float horizontalLength = 0.f;
	float verticalLength = 0.f;

	Vec2 startPosition = Vec2( ( float ) startPixelCoordX , ( float ) startPixelCoordY );
	Vec2 endPosition = Vec2( ( float ) endPixelCoordX , ( float ) endPixelCoordY );

	Vec2 horizontalVec = endPosition - startPosition;
	horizontalLength = horizontalVec.GetLength() * lengthPerUnit;

	Vec2 direction = horizontalVec.GetNormalized();
	float distaceToCoverForVerticalLength = horizontalVec.GetLength();
	float distanceCovered = 0.f;
	float stepSize = 1.f;

	Vec2 previousPosition = startPosition;
	Vec2 currentPosition = previousPosition + ( direction * stepSize );
	distanceCovered += ( currentPosition - previousPosition ).GetLength();
	
	while ( distanceCovered <= distaceToCoverForVerticalLength )
	{
		int previousPostionX = ( int ) previousPosition.x;
		int previousPostionY = ( int ) previousPosition.y;

		int currentPositionX = ( int ) currentPosition.x;
		int currentPositionY = ( int ) currentPosition.y;

		unsigned int difference = abs( array[ currentPositionX ][ currentPositionY ] - array[ previousPostionX ][ previousPostionY ] );
		verticalLength += difference * heightPerUnit;

		previousPosition = currentPosition;
		currentPosition = previousPosition + ( direction * stepSize );
		distanceCovered += ( currentPosition - previousPosition ).GetLength();
	}

	totalLength = horizontalLength + verticalLength;
	return totalLength;
}


int main()
{
	unsigned char preEruptionData[ numRows ][ numColumns ] = { 0 };
	unsigned char postEruptionData[ numRows ][ numColumns ] = { 0 };

	std::string preEruptionFilePath;
	std::string postEruptionFilePath;

	std::cout << "Enter the file path for pre eruption data"<<std::endl;
	std::cin >> preEruptionFilePath;

	size_t preEruptionOutSize;
	void* preEruptionBuffer = FileReadToBuffer( preEruptionFilePath , &preEruptionOutSize );

	if ( preEruptionBuffer == nullptr )
	{
		std::cout << "Invalid file path for pre eruption" << std::endl;
		return -1;
	}
	else if ( preEruptionOutSize != numRows * numColumns )
	{
		std::cout << "File size must be 512 * 512 bytes" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "Pre eruption file read successfully" << std::endl;
	}

	CopyDataFromBufferToArray( preEruptionData , preEruptionBuffer );

	std::cout << "Enter the file path for post eruption data" << std::endl;
	std::cin >> postEruptionFilePath;

	size_t postEruptionOutSize;
	void* postEruptionBuffer = FileReadToBuffer( postEruptionFilePath , &postEruptionOutSize );

	if ( postEruptionBuffer == nullptr )
	{
		std::cout << "Invalid file path for post eruption" << std::endl;
		return -1;
	}
	else if ( postEruptionOutSize != numRows * numColumns )
	{
		std::cout << "File size must be 512 * 512 bytes" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "Post eruption file read successfully" << std::endl;
	}
	
	CopyDataFromBufferToArray( postEruptionData , postEruptionBuffer );

	int startPixelCoordX;
	int startPixelCoordY;
	int endPixelCoordX;
	int endPixelCoordY;

	std::cout << "Enter the start pixel coordinates" << std::endl;
	std::cin >> startPixelCoordX >> startPixelCoordY;

	std::cout << "Enter the end pixel coordinates" << std::endl;
	std::cin >> endPixelCoordX >> endPixelCoordY;

	float preEruptionSurfaceLength = GetSurfaceLength( preEruptionData , startPixelCoordX , startPixelCoordY , endPixelCoordX , endPixelCoordY );
	float postEruptionSurfaceLength = GetSurfaceLength( postEruptionData , startPixelCoordX , startPixelCoordY , endPixelCoordX , endPixelCoordY );

	printf( "The pre eruption surfaceLenght is %f meters \n",preEruptionSurfaceLength);
	printf( "The post eruption surfaceLength is %f meters \n" , postEruptionSurfaceLength );
	printf( "The difference is %f meters \n" , postEruptionSurfaceLength - preEruptionSurfaceLength );
	
	return 0;
}


