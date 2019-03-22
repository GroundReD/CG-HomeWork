#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <GL/glut.h>
using namespace std;

#include "WaveFrontOBJ.h" 

//------------------------------------------------------------------------------
// ∞¥√º∏¶ ª˝º∫«œ∏Èº≠ filename ø°º≠ object ∏¶ ¿–¥¬¥Ÿ
// Construct object and read object from filename
WaveFrontOBJ::WaveFrontOBJ(char *filename) 
{
	isFlat = true;
	mode = GL_POLYGON;

	char *line = new char[200];
	char *line_back = new char[200];
	char wspace[] = " \t\n";
	char separator[] = "/";
	char *token;
	int indices[3];
	float x, y, z;
	float tex_u, tex_v;

	ifstream file(filename);
	if ( !file ) {
		cerr <<"Cannot open file: " <<filename <<" exiting." <<endl;
		exit ( -1 );
	}

	while ( !file.eof() ) {
		file.getline( line, 199 );
		// first, strip off comments
		if ( line[0] == '#' )
			continue;
		else if ( !strcmp( line, "" ) )
			continue;
		else {
			strcpy( line_back, line ); // strtok destroys line.
      
			token = strtok( line, wspace);

			if ( !strcmp( token, "v" ) ) {
				x = atof( strtok( NULL, wspace ) );
				y = atof( strtok( NULL, wspace ) );
				z = atof( strtok( NULL, wspace ) );
				verts.push_back( Vertex( x, y, z ) );
			}

			else if ( !strcmp( token, "vn" ) ) {
				x = atof( strtok( NULL, wspace ) );
				y = atof( strtok( NULL, wspace ) );
				z = atof( strtok( NULL, wspace ) );
				Vector vn(x, y, z);
				vn.Normalize();
				normals.push_back( vn );
			}

			else if ( !strcmp( token, "vt" ) ) {
				tex_u = atof( strtok( NULL, wspace ) );
				tex_v = atof( strtok( NULL, wspace ) );
				texCoords.push_back( TexCoord(tex_u, tex_v ) );
			}

			else if ( !strcmp( token, "f" ) ) {
				int vi = (int)vIndex.size();
                
//                std:cout << vi<<"\n";
                
				faces.push_back( Face( vi ) );
				Face& curFace = faces.back();
				for (char *p = strtok( NULL, wspace ); p ; p = strtok( NULL, wspace ) ) {
					indices[0] = 0; 
					indices[1] = 0;
					indices[2] = 0;
					char* pos = p;
					int len = (int)strlen(p);

//                    std::cout << "p : "<< *p <<"\n";
                    
					for ( int j=0, i=0;  j <= len && i < 3; j++ ) {
						if ( p[j] == '/' || p[j] == 0) {
							p[j] = 0;
							indices[i++] = atoi( pos );
							pos = p + j+1;
						}
					}
                    
//                    std::cout <<"indices : "<< indices[0]<<" "<<indices[1]<<" "<<indices[2]<<"\n";
                    
					vIndex.push_back( indices[0] - 1 );
					tIndex.push_back( indices[1] - 1 );
					nIndex.push_back( indices[2] - 1 );                        
					curFace.vCount++;
                    
//                    std::cout<<"vIndex : ";
//                    std:: for_each(vIndex.begin(), vIndex.end(), [](int i)->void { std::cout << i<<"\n";});
//                    std::cout<<"tIndex : ";
//                    std:: for_each(vIndex.begin(), tIndex.end(), [](int i)->void { std::cout << i<<"\n";});
//                    std::cout<<"vIndex : ";
//                    std:: for_each(vIndex.begin(), nIndex.end(), [](int i)->void { std::cout << i<<"\n";});
                    
					if (indices[2] != 0)
						isFlat = false;
				}
                
//                std::cout<<"end\n";
                
				if( curFace.vCount > 2 ){
                    
                    // std::cout << "vi : "<< vi << "\n";
                    // std::cout << "vIndex : "<< vIndex[vi]<< "\n";
                    // std::cout << "verts : "<< &verts[vIndex[vi]]<< "\n";
                    
					curFace.normal = faceNormal(verts[vIndex[vi]], verts[vIndex[vi+1]], verts[vIndex[vi+2]] );
                    curFace.center_pos = verts[vIndex[vi]].pos.operator+(verts[vIndex[vi+1]].pos.operator+(verts[vIndex[vi+2]].pos)).operator*(1.0/3.0);
					curFace.normal.Normalize();
					faceNormals.push_back(curFace.normal);
				}
                
			}


			else if ( !strcmp( token, "g" ) ) {      // group
			}
			else if ( !strcmp( token, "s" ) ) {      // smoothing group
			}
			else if ( !strcmp( token, "u" ) ) {      // material line
			}
			else if ( !strcmp( token, "" ) ) {       // blank line
			}
			else {
				cout <<line_back <<endl;
			}
		}
	}
	
	vertexNormal();

	computeBoundingBox();
}


//------------------------------------------------------------------------------
Vector WaveFrontOBJ::faceNormal(Vertex& v0, Vertex& v1, Vertex& v2) {
	/*******************************************************************/
	//(PA #4) : ºº ¡°¿« ¡¬«•∏¶ ¿ÃøÎ«œø© face normal¿ª ∞ËªÍ«œ¥¬ «‘ºˆ∏¶ øœº∫«œΩ Ω√ø¿.
	// - ∞ËªÍ«— face normal¿Ã ∞¢ Face class¿« normal ø° ¿˙¿Â µ«µµ∑œ ±∏«ˆ«œΩ Ω√ø¿.
	/*******************************************************************/

	// face의 두 벡터 u, v 를 외적
    Vector u = v1.pos.operator-(v0.pos);
    Vector v = v2.pos.operator-(v0.pos);
    
	return u.CrossProduct(v);
}

void WaveFrontOBJ::vertexNormal() {
	/*******************************************************************/
	//(PA #4) : ¡÷∫Ø face normal¿ª ¿ÃøÎ«œø© vertex normal¿ª ∞ËªÍ«œ¥¬ «‘ºˆ∏¶ øœº∫«œΩ Ω√ø¿.
	// - ∞ËªÍ«— vertex normal¿Ã ∞¢ Vertex Class¿« normalø° ¿˙¿Â µ«µµ∑œ ±∏«ˆ«œΩ Ω√ø¿.
	/*******************************************************************/
    int i;
    
    //vertex에 face normal 벡터를 다 더한 후 count 로 나눠줌

    for ( i = 0 ; i < faces.size() ; i++ ) {
        int vi = faces[i].vIndexStart;
        
        verts[vIndex[vi]].normal =  verts[vIndex[vi]].normal.operator+(faces[i].normal);
        verts[vIndex[vi]].count++;
        
        verts[vIndex[vi+1]].normal = verts[vIndex[vi+1]].normal.operator+(faces[i].normal);
        verts[vIndex[vi+1]].count++;
        
        verts[vIndex[vi+2]].normal = verts[vIndex[vi+2]].normal.operator+(faces[i].normal);
        verts[vIndex[vi+2]].count++;
    }
    
    for ( i = 0 ; i < verts.size() ; i++ ) {
        verts[i].normal = verts[i].normal.operator*(1.0/verts[i].count);
        verts[i].normal.Normalize();
        normals.push_back(verts[i].normal);
        
    }
    
    
}

//------------------------------------------------------------------------------
// OpenGL API ∏¶ ªÁøÎ«ÿº≠ ∆ƒ¿œø°º≠ ¿–æÓµ– object ∏¶ ±◊∏Æ¥¬ «‘ºˆ.
// Draw object which is read from file
void WaveFrontOBJ::Draw() {
	int i;

	for (int f = 0; f < (int)faces.size(); f++) {
		Face& curFace = faces[f];        
		glBegin(mode);
		for (int v = 0; v < curFace.vCount; v++) {
			int vi = curFace.vIndexStart + v;
			
			if (isFlat) {
				if (v == 0) {
					glNormal3f(curFace.normal.x, curFace.normal.y, curFace.normal.z);
				}
			}
			
			else if ((i = vIndex[vi]) >= 0) {
				glNormal3f(verts[i].normal.x, verts[i].normal.y, verts[i].normal.z);
			}
			
			if ((i = tIndex[vi]) >= 0) {
				glTexCoord2f(texCoords[i].u, texCoords[i].v);
			}
			if ((i = vIndex[vi]) >= 0) {
				glVertex3f(verts[i].pos.x, verts[i].pos.y, verts[i].pos.z);
			}
		}
		glEnd();
	}
}

void WaveFrontOBJ::Draw_FN() {
	glDisable(GL_LIGHTING);
	/*******************************************************************/
	//(PA #4) : ∞¢ faceø° ¥Î«ÿ face normal¿ª ±◊∏Æ¥¬ «‘ºˆ∏¶ ¿€º∫«œΩ Ω√ø¿.
	// - ¿€º∫«— «‘ºˆ¥¬ drawCow, drawbunnyø° »∞øÎ«œΩ Ω√ø¿.
	/*******************************************************************/
    
    // face의 pos_center부터 normal verctor의 끝점 까지 선을 그어줌

    for (int f = 0; f < (int)faces.size(); f++) {
        Face& curFace = faces[f];
        Vector normal_endPos = curFace.center_pos.operator+(curFace.normal);

        glBegin(GL_LINES);
            glColor3f(0.0, 1.0, 0.0);
            glVertex3f(curFace.center_pos.x, curFace.center_pos.y, curFace.center_pos.z);
            glVertex3f(normal_endPos.x, normal_endPos.y, normal_endPos.z);
        
        glEnd();
    }
    
    
	glEnable(GL_LIGHTING);
}

void WaveFrontOBJ::Draw_VN() {
	glDisable(GL_LIGHTING);
	/*******************************************************************/
	//(PA #4) : ∞¢ vertexø° ¥Î«ÿ vertex normal¿ª ±◊∏Æ¥¬ «‘ºˆ∏¶ ¿€º∫«œΩ Ω√ø¿.
	// - ¿€º∫«— «‘ºˆ¥¬ drawCow, drawbunnyø° »∞øÎ«œΩ Ω√ø¿.
	/*******************************************************************/
    
    //vertex pos 부터 vertex normal 벡터의 끝 점까지 선을 그
    for (int i = 0; i<verts.size() ; i++) {
        Vertex& curVertex = verts[i];
        Vector normal_endPos = curVertex.pos.operator+(curVertex.normal);
        
        glBegin(GL_LINES);
            glColor3f(0.0, 0.0, 0.0);
            glVertex3f(curVertex.pos.x, curVertex.pos.y, curVertex.pos.z);
            glVertex3f(normal_endPos.x, normal_endPos.y, normal_endPos.z);
        
        glEnd();
    }
    
	glEnable(GL_LIGHTING);
}

//------------------------------------------------------------------------------
void WaveFrontOBJ::computeBoundingBox()
{
	if( verts.size() > 0 )
	{
		bbmin.pos.x = verts[0].pos.x;
		bbmin.pos.y = verts[0].pos.y;
		bbmin.pos.z = verts[0].pos.z;
		bbmax.pos.x = verts[0].pos.x;
		bbmax.pos.y = verts[0].pos.y;
		bbmax.pos.z = verts[0].pos.z;
		for( int i=1; i < (int)verts.size(); i++ )
		{
			if( verts[i].pos.x < bbmin.pos.x ) bbmin.pos.x = verts[i].pos.x;
			if( verts[i].pos.y < bbmin.pos.y ) bbmin.pos.y = verts[i].pos.y;
			if( verts[i].pos.z < bbmin.pos.z ) bbmin.pos.z = verts[i].pos.z;
			if( verts[i].pos.x > bbmax.pos.x ) bbmax.pos.x = verts[i].pos.x;
			if( verts[i].pos.y > bbmax.pos.y ) bbmax.pos.y = verts[i].pos.y;
			if( verts[i].pos.z > bbmax.pos.z ) bbmax.pos.z = verts[i].pos.z;
		}
	}
}
