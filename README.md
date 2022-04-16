# GraphicsEngine_DX11
DirectX 11 기반으로 제작한 Graphics Engine의 소스입니다.

.

# 엔진 구조 설계

팀 내의 다른 프로그래머들과 협업하여 게임 엔진 구조를 설계하였습니다.

![image](https://user-images.githubusercontent.com/97003592/163679747-3ea6683f-aceb-4b74-ac2a-883c07dee8a3.png)

위에서 회색으로 칠해져있는 부분을 제가 담당하여 개발하였습니다.

.
 
Graphics Engine구조를 설계할 때 고려했던 부분은 다음과 같습니다.

1. dll을 사용하여 Game Engine과 Graphics Engine 개발을 분리하기
2. 명시적 링킹을 사용하여 프로그램 실행 중에 dll 교체 가능하도록 하기
3. Graphics Engine을 제외하고는 DirectX를 비롯한 그래픽 API를 모르도록 하기

.

### (1) dll을 사용하여 Game Engine과 Graphics Engine 개발을 분리하기

게임 엔진과 그래픽스 엔진을 개발하는 작업자가 따로 있었기 때문에, 그래픽스 엔진 개발이 게임 엔진 개발에 영향을 주지 않기 위해서 dll을 사용했습니다. 제가 그래픽스 엔진을 수정하더라도 게임 엔진 개발자는 그래픽스 엔진쪽은 컴파일 하지 않아도되서 개발 시에 더 효율적이었습니다.

.

### (2) 명시적 링킹을 사용하여 프로그램 실행 중에 dll 교체 가능하도록 하기

DirectX로 자체 엔진을 만들어 프로젝트를 한번 경험해보고 난 뒤에, 다음 프로젝트에서는 게임 옵션에서 DirectX 11과 DirectX 12를 교체할 수 있는 구조로 만들어야겠다고 생각했습니다. 따라서 dll의 명시적 링킹을 사용하여 프로그램 실행 중에 dll을 교체할 수 있도록 해야겠다고 생각했습니다.

하지만, 암시적 링킹과 달리 명시적 링킹은 매번 함수포인터를 얻어와 함수를 사용해야되기 때문에 코드가 매우 복잡해져 개선이 필요했습니다.

제가 개선한 방법은 두 가지입니다.

.

1. 가변인자 매크로 활용
    
    ![image](https://user-images.githubusercontent.com/97003592/163679774-b7202540-a2e5-4e93-9d08-32aa577cf5b7.png)
    
    ![image](https://user-images.githubusercontent.com/97003592/163679779-64aae52a-b44b-4aa3-9563-9c991543077b.png)
    
    함수 포인터를 얻어오고 함수를 사용하는 과정을 가변인자 매크로 한 줄로 처리가능하도록 하였습니다. 첫번째에는 함수의 이름, 두번째부터 인자들을 넣어주면 됩니다.
    
.

2. 인터페이스 클래스 활용
    
    그래픽스 엔진을 사용하는 쪽에서 인터페이스 클래스 형식으로 선언을 합니다.
    
    ![image](https://user-images.githubusercontent.com/97003592/163679784-9e7d82c0-cbfb-4343-b238-695225e8d0fa.png)
    
    ![image](https://user-images.githubusercontent.com/97003592/163679788-7318c116-7d95-41e3-b100-bfe576931065.png)
    
    .
    
    엔진을 생성할 때는, 내부에서 IGraphicsEngine 인터페이스 클래스를 생성하는게 아니라 실제 사용될 함수들을 담아놓은 GraphicsEngine 클래스를 생성해 리턴해줍니다.
    
    ![image](https://user-images.githubusercontent.com/97003592/163679792-cfbc13bb-5dd7-4bc8-86fb-846ed83ddbe5.png)
    
    ![image](https://user-images.githubusercontent.com/97003592/163679798-a77b779d-6f27-417d-9464-ffc0e7115bf4.png)
    
   .
   
   사용자는 IGraphicsEngine 인터페이스 클래스의 함수를 호출했지만 다형적 동작에 의해서 GraphicsEngine 클래스의 함수가 호출됩니다.
    
    ![image](https://user-images.githubusercontent.com/97003592/163679803-fda18722-989f-48b9-b83c-643488b626a4.png)
    
    ![image](https://user-images.githubusercontent.com/97003592/163679810-5e8bf715-ab78-4f9e-baab-624abf9f3f7d.png)
    
    이 방식으로, 명시적 dll에서 함수 포인터를 직접 얻어오지 않고 C++의 다형성을 이용해 함수를 호출할 수 있었습니다.
    

.

결론적으로 사용자는 다음과 같은 두 가지 방법 중에서 한 가지를 선택해그래픽스 함수를 호출하여 사용할 수 있습니다.

![image](https://user-images.githubusercontent.com/97003592/163679815-3517da79-3ba6-4e65-82fe-a74faa58bc1a.png)

.

### (3) Graphics Engine을 제외하고는 DirectX를 비롯한 그래픽 API를 모르도록 하기

기존에는 게임 엔진 전체에서 DirectX의 SimpleMath를 사용하여 Vector, Matrix등의 수학 클래스를 사용했습니다. 그러나 SimpleMath를 사용하기 위해서는 반드시 DirectXMath 헤더를 include해야 했습니다.

이러한 구조에서 벗어나기 위해 Math, Collision 등의 DirectX에 종속적인 클래스들을 랩핑하여 사용했습니다.

![image](https://user-images.githubusercontent.com/97003592/163679822-4b3f609e-f042-4423-8569-493e512a5e41.png)

SimpleMath.h

![image](https://user-images.githubusercontent.com/97003592/163679834-6e4ea9a5-29a5-4e75-9b0d-de241963593f.png)

Math를 랩핑한 EMath.h

.

다음은, 한 프레임이 렌더링 되기까지를 간단하게 나타낸 흐름도입니다. 게임 엔진이 관여하는 부분과 그래픽스 엔진이 관여하는 부분을 명확하게 나눠 독립성을 높이고자 했습니다. 그래픽스 엔진에서는 렌더링 관련한 부분만 처리할 수 있도록 노력했습니다.
![image](https://user-images.githubusercontent.com/97003592/163679840-f766cee5-9933-4b50-b319-32b9086dcbb2.png)
