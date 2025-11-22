#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/TransformedView.h"

#include "/home/codeleaded/System/Static/Library/Circle.h"
#include "/home/codeleaded/System/Static/Library/CircleLine.h"

typedef struct Ball {
    Vec2 p;
    F32 r;
    F32 m;
    Vec2 v;
} Ball;

Ball Ball_New(Vec2 p,Vec2 v,F32 r,F32 m){
    Ball b;
    b.p = p;
    b.v = v;
    b.r = r;
    b.m = m;
    return b;
}
void Ball_Update(Ball* b,Rect border,float ElapsedTime){
    //if(b->p.x < -b->r)          b->p.x = 1.0f + b->r;
    //if(b->p.x > 1.0f + b->r)    b->p.x = 0.0f - b->r;
    //if(b->p.y < -b->r)          b->p.y = 1.0f + b->r;
    //if(b->p.y > 1.0f + b->r)    b->p.y = 0.0f - b->r;

    if(b->p.x < border.p.x + b->r){
        b->p.x = border.p.x + b->r;
        b->v.x *= -1.0f;
    }
    if(b->p.x > border.p.x + border.d.x - b->r){
        b->p.x = border.p.x + border.d.x - b->r;
        b->v.x *= -1.0f;
    }
    if(b->p.y < border.p.y + b->r){
        b->p.y = border.p.y + b->r;
        b->v.y *= -1.0f;
    }
    if(b->p.y > border.p.y + border.d.y - b->r){
        b->p.y = border.p.y + border.d.y - b->r;
        b->v.y *= -1.0f;
    }

    b->v = Vec2_Add(b->v,Vec2_Mulf((Vec2){ 0.0f,9.0f },1.0f * ElapsedTime));
    b->v = Vec2_Add(b->v,Vec2_Mulf(Vec2_Neg(b->v),0.05f * ElapsedTime));
    if(Vec2_Mag(b->v) < 0.0001f) b->v = (Vec2){ 0.0f,0.0f };
    
    b->p = Vec2_Add(b->p,Vec2_Mulf(b->v,ElapsedTime));
}
void Ball_Render(Ball* b,TransformedView* tv){
    Vec2 p = TransformedView_WorldScreenPos(tv,b->p);
    float r = TransformedView_WorldScreenLX(tv,b->r);
    RenderCircleWire(p,r,WHITE,1.0f);
    
    Vec2 t = Vec2_Add(p,Vec2_Mulf(Vec2_Norm(b->v),r));
    RenderLine(p,t,BLUE,1.0f);
}


typedef struct Edge {
    Vec2 s;
    Vec2 e;
    F32 r;
    F32 m;
} Edge;

Edge Edge_New(Vec2 s,Vec2 e,F32 r,F32 m){
    Edge b;
    b.s = s;
    b.e = e;
    b.r = r;
    b.m = m;
    return b;
}
void Edge_Render(Edge* b,TransformedView* tv){
    Vec2 s = TransformedView_WorldScreenPos(tv,b->s);
    Vec2 e = TransformedView_WorldScreenPos(tv,b->e);
    float r = TransformedView_WorldScreenLX(tv,b->r);
    CircleLine_RenderX(WINDOW_STD_ARGS,s,e,r,WHITE);
}



int Selected;
Vec2* SelectedPos;
Vector Balls;
Vector Edges;
Rect Border;
TransformedView tv;

void Setup(AlxWindow* w){
    RGA_Set(Time_Nano());

    tv = TransformedView_Make((Vec2){ GetWidth(),GetHeight() },(Vec2){ 0.0f,0.0f },(Vec2){ 1.0f,1.0f },(float)GetWidth() / (float)GetHeight());
    Border = Rect_New((Vec2){ 0.0f,0.0f },(Vec2){ 15.0f,15.0f });
    Balls = Vector_New(sizeof(Ball));
    Edges = Vector_New(sizeof(Edge));

    Selected = -1;
    SelectedPos = NULL;
    
    for(int i = 0;i<100;i++){
        float r = Random_f64_MinMax(0.025f,0.1f);
        Vector_Push(&Balls,(Ball[]){ Ball_New(
            (Vec2){ Random_f64_MinMax(Border.p.x,Border.p.x + Border.d.x),Random_f64_MinMax(Border.p.y,Border.p.y + Border.d.y) },
            (Vec2){ 0.0f,0.0f },
            r,
            r * 100.0f
        )});
    }

    for(int i = 0;i<5;i++){
        float r = Random_f64_MinMax(0.1f,0.3f);
        Vector_Push(&Edges,(Edge[]){ Edge_New(
            (Vec2){ Random_f64_MinMax(Border.p.x,Border.p.x + Border.d.x),Random_f64_MinMax(Border.p.y,Border.p.y + Border.d.y) },
            (Vec2){ Random_f64_MinMax(Border.p.x,Border.p.x + Border.d.x),Random_f64_MinMax(Border.p.y,Border.p.y + Border.d.y) },
            r,
            r * 10000.0f
        )});
    }
}
void Update(AlxWindow* w){
    TransformedView_Output(&tv,(Vec2){ GetWidth(),GetHeight() });
    TransformedView_HandlePanZoom(&tv,window.Strokes,GetMouse());
    Vec2 Mouse = TransformedView_ScreenWorldPos(&tv,GetMouse());

    if(Stroke(ALX_MOUSE_L).PRESSED){
        SelectedPos = NULL;
        for(int i = 0;i<Balls.size;i++){
            Ball* b = (Ball*)Vector_Get(&Balls,i);

            if(Circle_Point((Circle*)b,Mouse)){
                SelectedPos = &b->p;
            }
        }
        for(int i = 0;i<Edges.size;i++){
            Edge* b = (Edge*)Vector_Get(&Edges,i);

            if(Circle_Point((Circle[]){ Circle_New(b->s,b->r) },Mouse)){
                SelectedPos = &b->s;
            }
            if(Circle_Point((Circle[]){ Circle_New(b->e,b->r) },Mouse)){
                SelectedPos = &b->e;
            }
        }
    }
    if(Stroke(ALX_MOUSE_L).RELEASED){
        if(SelectedPos){
            SelectedPos = NULL;
        }
    }

    if(Stroke(ALX_MOUSE_R).PRESSED){
        Selected = -1;
        for(int i = 0;i<Balls.size;i++){
            Ball* b = (Ball*)Vector_Get(&Balls,i);

            if(Circle_Point((Circle*)b,Mouse)){
                Selected = i;
            }
        }
    }
    if(Stroke(ALX_MOUSE_R).RELEASED){
        if(Selected>=0 && Selected<Balls.size){
            Ball* b = (Ball*)Vector_Get(&Balls,Selected);
            b->v = Vec2_Neg(Vec2_Sub(Mouse,b->p));
            Selected = -1;
        }
    }

    if(SelectedPos) *SelectedPos = Mouse;

    Clear(BLACK);

    Vec2 BorderP = TransformedView_WorldScreenPos(&tv,Border.p);
    Vec2 BorderD = TransformedView_WorldScreenLength(&tv,Border.d);
    RenderRectWire(BorderP.x,BorderP.y,BorderD.x,BorderD.y,WHITE,1.0f);

    for(int i = 0;i<Balls.size;i++){
        Ball* b1 = (Ball*)Vector_Get(&Balls,i);
        
        Ball_Update(b1,Border,w->ElapsedTime);

        for(int j = 0;j<Balls.size;j++){
            if(i==j) continue;
            
            Ball* b2 = (Ball*)Vector_Get(&Balls,j);
            if(Circle_Circle_Overlap((Circle*)b1,(Circle*)b2)){
                Circle_Circle_Dynamic((Circle*)b1,&b1->v,b1->m,(Circle*)b2,&b2->v,b2->m,0.75f);
            }
        }
    }
    for(int i = 0;i<Edges.size;i++){
        Edge* b1 = (Edge*)Vector_Get(&Edges,i);

        for(int j = 0;j<Balls.size;j++){
            Ball* b2 = (Ball*)Vector_Get(&Balls,j);
            
            //Vec2 closest = CircleLine_Point_Closest((CircleLine*)b1,b2->p);
            //Vec2 p = TransformedView_WorldScreenPos(&tv,closest);
            //float r = TransformedView_WorldScreenLX(&tv,0.05f);
            //RenderCircle(p,r,RED);

            if(CircleLine_Circle_Overlap((CircleLine*)b1,(Circle*)b2)){
                //CircleLine_Circle_Static((CircleLine*)b1,(Circle*)b2);
                CircleLine_Circle_Dynamic((CircleLine*)b1,NULL,b1->m,(Circle*)b2,&b2->v,b2->m,0.75f);
            }
        }
    }
    
    for(int i = 0;i<Balls.size;i++){
        Ball* b1 = (Ball*)Vector_Get(&Balls,i);
        Ball_Render(b1,&tv);
    }
    for(int i = 0;i<Edges.size;i++){
        Edge* b1 = (Edge*)Vector_Get(&Edges,i);
        Edge_Render(b1,&tv);
    }

    if(Selected>=0){
        Ball* b = (Ball*)Vector_Get(&Balls,Selected);

        Vec2 p = TransformedView_WorldScreenPos(&tv,b->p);
        Vec2 t = GetMouse();
        RenderLine(p,t,RED,1.0f);
    }
}
void Delete(AlxWindow* w){
    for(int i = 0;i<Balls.size;i++){
        Ball* b = (Ball*)Vector_Get(&Balls,i);
        //Ball_Free(b);
    }
	Vector_Free(&Balls);
}

int main(){
    if(Create("Balls",1920,1080,1,1,Setup,Update,Delete))
        Start();
    return 0;
}