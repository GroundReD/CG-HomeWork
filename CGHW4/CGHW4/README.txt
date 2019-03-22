README.txt

WaveFrontOBJ.h

	1. Face normal vector를 그리기 위해 Face class에 각 face의 무게중심을 저장하는 vector 변수를 추가	

WaveFrontOBJ.cpp

	1. Vector WaveFrontOBJ::faceNormal(Vertex& v0, Vertex& v1, Vertex& v2)
		: triangle face의 u, v 벡터를 구해 외적하여 face normal vector를 구함

 	2. void WaveFrontOBJ::vertexNormal()
 		: vIndex를 이용해 각각의 face를 탐색하면서 face의 vertex normal vector 변수에 face normal vector를 더하면서 count 변수에 1씩 더함.
 		  이후 각각의 vertex를 탐색하면서 count로 vertext normal vector를 나눠준 후 정규화

 	3. void WaveFrontOBJ::Draw_FN()
 		: Face class의 center_pos vector를 시작점과 face normal 벡터를 더한 벡터를 끝 점으로 한 후 선을 그림

 	4. void WaveFrontOBJ::Draw_VN()
		: Vertex class의 pos를 시작점으로, vertex normal vector를 끝점으로 해 선을 그림.

pa4_skeleton.cpp

	1. void drawCow(), void drawBunny()
		: select_vector 'f' 일 때 face normal vector를 그림. 'n'일 때 vertex normal vector 그림

	2. void Lighting()
		: light1 -> point light 
		  light2 -> directional light
		  light3 -> spot light
		  각각의 빛의 특성 변수를 입력한 후, 광원과 빛의 방향을 하얀색 구로 출력

	3. void SpecialKey(int key, int x, int y)
		: 특수키 입력.
			F1 -> direction light. light mode를 d로 변경
			F2 -> point light. light mode를 p로 변경
			F3 -> spot light. light mode를 s로 변경