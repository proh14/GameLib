// Copyright (C) 2011 Valeriano Alfonso Rodriguez (Kableado)

#include <stdlib.h>
#include <math.h>

#include "GameLib.h"
extern int gamelib_debug;

#include "GameEnts.h"

DrawImg img_barrel;
DrawImg img_barrel2;
DrawImg img_column;
DrawImg img_column_faded;
DrawImg img_rock;
DrawImg img_lamp;
DrawImg img_floor;
DrawImg img_floor_left;
DrawImg img_floor_right;
DrawImg img_floor_center;
DrawImg img_hole_spiked;
Anim anim_hole_lava;
DrawImg img_player_down;
DrawImg img_player_up;
DrawImg img_player_left;
DrawImg img_player_right;
DrawImg img_savepoint;
Anim anim_savepoint_active;
DrawImg img_endpoint;
Anim anim_exitpoint;
DrawImg img_arrowshooter_up;
DrawImg img_arrowshooter_down;
DrawImg img_arrowshooter_left;
DrawImg img_arrowshooter_right;
DrawImg img_arrow_up;
DrawImg img_arrow_down;
DrawImg img_arrow_left;
DrawImg img_arrow_right;
Anim anim_fire;
DrawImg img_player_broken;


AudioSnd snd_arrowhit;
AudioSnd snd_savepoint;
AudioSnd snd_exitpoint;
AudioSnd snd_shootarrow;
AudioSnd snd_burn;
AudioSnd snd_fillhole;
AudioSnd snd_drag;

Entity ent_player;
Entity ent_barrel;
Entity ent_column;
Entity ent_column_faded;
Entity ent_rock;
Entity ent_lamp;
Entity ent_floor;
Entity ent_floor_right;
Entity ent_floor_left;
Entity ent_floor_center;
Entity ent_hole_spiked;
Entity ent_hole_filled;
Entity ent_hole_lava;
Entity ent_arrowshooter_up;
Entity ent_arrowshooter_down;
Entity ent_arrowshooter_left;
Entity ent_arrowshooter_right;
Entity ent_arrow_up;
Entity ent_arrow_down;
Entity ent_arrow_left;
Entity ent_arrow_right;
Entity ent_exitpoint;
Entity ent_endpoint;
Entity ent_savepoint;
Entity ent_teleporter;
Entity ent_teleporter_dest;


Entity ent_fire;
Entity ent_player_broken;

extern int game_level;
extern int game_level_point;
extern int game_level_reset;

void LoadGame();
void SaveGame();


void player_proc(Entity e,int ft){
	vec2 vel;
	int pos[2],size[2],delta[2];

	if (gamelib_debug) {
		if (Input_GetKey(InputKey_Jump)==InputKey_Pressed) {
			if (!(e->flags&EntityFlag_Collision)) {
				e->flags|=(EntityFlag_Collision|EntityFlag_Overlap);
				GameLib_EntitySetLight(e,0.4f,0.4f,0.4f,5*32.0f);
			}else {
				e->flags&=~(EntityFlag_Collision|EntityFlag_Overlap);
				GameLib_EntitySetLight(e,0.7f,0.7f,0.7f,20*32.0f);
			}
		}
	}


	if(Input_GetDir(vel)){
		vec2 up,right;
		float updown,leftright;

		vec2_set(up,0,-1);
		vec2_set(right,1,0);
		updown=vec2_dot(up,vel);
		leftright=vec2_dot(right,vel);
		if(fabs(updown)>=fabs(leftright)){
			if(updown>0.0f){
				AnimPlay_SetImg(&e->anim,img_player_up);
			}else{
				AnimPlay_SetImg(&e->anim,img_player_down);
			}
		}else{
			if(leftright>0.0f){
				AnimPlay_SetImg(&e->anim,img_player_right);
			}else{
				AnimPlay_SetImg(&e->anim,img_player_left);
			}
		}

		vec2_scale(vel,vel,7);
		Entity_AddVelLimit(e,vel,15.0f);
	}

	GameLib_MoveToPos(e->pos,0.3f);
}

int player_collision(Entity e1,Entity e2,float t,vec2 n){
	if(e2->type==Ent_Barrel){
		float vlen;
		vec2 vdir;
		vlen=sqrtf(vec2_dot(e1->vel,e1->vel));
		if(vlen>0.0f){
			vec2_scale(vdir,e1->vel,1.0f/vlen);
			if (vec2_dot(vdir,n) < -0.9f) {
				vec2_orthogonalize4(n);
				Entity_CollisionResponseCircle(e2,e1,t,n);
				return(2);
			}
		}
	}
	return(1);
}

void barrel_proc(Entity e,int ft){
	float qvel;
	int tnow;

	qvel=vec2_dot(e->vel,e->vel);
	if(qvel>0.0f){
		tnow=Time_GetTime()/1000;
		if(tnow-250>e->A){
			GameLib_PlaySound(snd_drag,(int)e->pos[0],(int)e->pos[1]);
			e->A=tnow;
		}
	}
}


void hole_spiked_overlap(Entity e1,Entity e2){
	Entity e;

	if(e2->type==Ent_Barrel){
		Entity e;

		// Disable future overlaps
		e1->overlap=NULL;

		// "Fill the hole"
		e=Entity_Copy(ent_hole_filled);
		vec2_copy(e->pos,e1->pos);
		GameLib_AddEntity(e);
		GameLib_PlaySound(snd_fillhole,(int)e2->pos[0],(int)e2->pos[1]);

		// Remove the two entities
		GameLib_DelEntity(e1);
		GameLib_DelEntity(e2);

	}
	if(e2->type==Ent_Player){
		// "Kill the player"

		// Broken player
		e=Entity_Copy(ent_player_broken);
		vec2_copy(e->pos,e2->pos);
		GameLib_AddEntity(e);

		GameLib_PlaySound(snd_burn,(int)e2->pos[0],(int)e2->pos[1]);
		GameLib_DelEntity(e2);

		// HACK
		game_level_reset=1;
	}
}


void hole_lava_oncopy(Entity ent){
	AnimPlay_IncTime(&ent->anim,rand()%1000);
}

void hole_lava_overlap(Entity e1,Entity e2){
	Entity e;

	if(e2->type==Ent_Barrel){
		// "Burn the barrel"

		// Make fire
		e=Entity_Copy(ent_fire);
		vec2_copy(e->pos,e2->pos);
		GameLib_AddEntity(e);

		GameLib_PlaySound(snd_burn,(int)e2->pos[0],(int)e2->pos[1]);
		GameLib_DelEntity(e2);
	}
	if(e2->type==Ent_Player){
		// "Burn the player"

		// Make fire
		e=Entity_Copy(ent_fire);
		vec2_copy(e->pos,e2->pos);
		GameLib_AddEntity(e);

		// Broken player
		e=Entity_Copy(ent_player_broken);
		vec2_copy(e->pos,e2->pos);
		GameLib_AddEntity(e);

		GameLib_PlaySound(snd_burn,(int)e2->pos[0],(int)e2->pos[1]);
		GameLib_DelEntity(e2);
		game_level_reset=1;
	}
}


int arrow_collision(Entity e1,Entity e2,float t,vec2 n){
	Entity e;

	if(e1->postproc)
		return(0);

	if(e2->type==Ent_ArrowShooter)
		return(0);
	if(e2->type==Ent_Arrow)
		return(0);

	if(e2->type==Ent_Player){
		// KILL the player
		e=Entity_Copy(ent_player_broken);
		vec2_copy(e->pos,e2->pos);
		GameLib_AddEntity(e);
		GameLib_DelEntity(e2);
		GameLib_PlaySound(snd_burn,(int)e2->pos[0],(int)e2->pos[1]);
		game_level_reset=1;
	}
	GameLib_DelEntity(e1);
	GameLib_PlaySound(snd_arrowhit,(int)e1->pos[0],(int)e1->pos[1]);

	return(0);
}

void arrowshooter_oncopy(Entity e){
	e->A=(rand()%15);
}

void arrowshooter_proc(Entity e,int ft){
	if(e->A==0){
		Entity e2;

		e2=Entity_Copy(e->child);
		vec2_copy(e2->pos,e->pos);
		GameLib_AddEntity(e2);
		GameLib_PlaySound(snd_shootarrow,(int)e->pos[0],(int)e->pos[1]);

		e->A=15;
	}else{
		e->A--;
	}
}

Entity _savepoint=NULL;
void savepoint_ondelete(Entity e){
	if(_savepoint==e){
		_savepoint=NULL;
	}
}
void savepoint_overlap(Entity e1,Entity e2){
	if(e2->type==Ent_Player){
		// Save the point
		if(game_level_point!=e1->A){
			game_level_point=e1->A;
			GameLib_PlaySound(snd_savepoint,(int)e1->pos[0],(int)e1->pos[1]);
			SaveGame();
		}
		if(e1!=_savepoint){
			AnimPlay_SetAnim(&e1->anim,anim_savepoint_active);
			GameLib_EntitySetLight(e1,0.0f,0.0f,0.5f,4*32.0f);
			if(_savepoint){
				AnimPlay_SetImg(&_savepoint->anim,img_savepoint);
				GameLib_EntitySetLight(_savepoint,0.0f,0.0f,0.5f,2*32.0f);
			}
			_savepoint=e1;
		}
	}
}

void exitpoint_overlap(Entity e1,Entity e2){
	if(e2->type==Ent_Player){
		// Exit the level
		game_level++;
		game_level_point=1;
		game_level_reset=2;

		// HACK: Delete the player
		GameLib_DelEntity(e2);

		GameLib_PlaySound(snd_exitpoint,(int)e1->pos[0],(int)e1->pos[1]);
		SaveGame();
	}
}

void endpoint_overlap(Entity e1,Entity e2){
	if(e2->type==Ent_Player){
		// Go to end
		game_level_reset=3;

		// HACK: Delete the player
		GameLib_DelEntity(e2);

		GameLib_PlaySound(snd_exitpoint,(int)e1->pos[0],(int)e1->pos[1]);
		SaveGame();
	}
}

void timeoutent_proc(Entity e,int ft){
	if(e->A==0){
		GameLib_DelEntity(e);
	}else{
		e->A--;
	}
}

int teleporter_searchdest(Entity ent,void *d){
	int a=*(int*)d;
	if(ent->type!=Ent_Teleporter_Dest){
		return 0;
	}

	if(ent->A==a){
		return 1;
	}
	return 0;
}

void teleporter_overlap(Entity e1,Entity e2){
	Entity dest=NULL;

	// Search the destination
	dest=GameLib_SearchEnt(teleporter_searchdest,&e1->A);

	if(dest){
		vec2_copy(e2->pos,dest->pos);
	}
}


void GameEnts_Init(){
	Entity ent;

	//////////////////////////////
	// Load Resources

	img_barrel=Draw_LoadImage("data/barrel.png");
	Draw_SetOffset(img_barrel,-16,-32);

	img_barrel2=Draw_LoadImage("data/barrel2.png");
	Draw_SetOffset(img_barrel2,-16,-16);

	img_floor=Draw_LoadImage("data/floor.png");
	Draw_SetOffset(img_floor,-16,-16);
	img_floor_left=Draw_LoadImage("data/floor_left.png");
	Draw_SetOffset(img_floor_left,-16,-16);
	img_floor_right=Draw_LoadImage("data/floor_right.png");
	Draw_SetOffset(img_floor_right,-16,-16);
	img_floor_center=Draw_LoadImage("data/floor_center.png");
	Draw_SetOffset(img_floor_center,-16,-16);

	img_column=Draw_LoadImage("data/column.png");
	Draw_SetOffset(img_column,-16,-80);
	img_column_faded=Draw_LoadImage("data/column_faded.png");
	Draw_SetOffset(img_column_faded,-16,-80);
	img_rock=Draw_LoadImage("data/rock.png");
	Draw_SetOffset(img_rock,-16,-32);
	img_lamp=Draw_LoadImage("data/lamp.png");
	Draw_SetOffset(img_lamp,-16,-48);

	img_hole_spiked=Draw_LoadImage("data/hole_spiked.png");
	Draw_SetOffset(img_hole_spiked,-16,-16);

	anim_hole_lava=Anim_LoadAnim("data/hole_lava.png",32,2,5);
	Anim_SetOffset(anim_hole_lava,-16,-16);

	img_player_up=Draw_LoadImage("data/player_up.png");
	Draw_SetOffset(img_player_up,-16,-48);
	img_player_down=Draw_LoadImage("data/player_down.png");
	Draw_SetOffset(img_player_down,-16,-48);
	img_player_left=Draw_LoadImage("data/player_left.png");
	Draw_SetOffset(img_player_left,-16,-48);
	img_player_right=Draw_LoadImage("data/player_right.png");
	Draw_SetOffset(img_player_right,-16,-48);

	img_savepoint=Draw_LoadImage("data/save_point.png");
	Draw_SetOffset(img_savepoint,-16,-16);

	anim_savepoint_active=Anim_LoadAnim("data/save_point_active.png",32,2,5);
	Anim_SetOffset(anim_savepoint_active,-16,-16);

	anim_exitpoint=Anim_LoadAnim("data/exit_point.png",32,2,10);
	Anim_SetOffset(anim_exitpoint,-16,-48);

	img_endpoint=Draw_LoadImage("data/end_point.png");
	Draw_SetOffset(img_endpoint,-16,-32);

	img_arrowshooter_up=Draw_LoadImage("data/arrowshooter_up.png");
	Draw_SetOffset(img_arrowshooter_up,-16,-16);
	img_arrowshooter_down=Draw_LoadImage("data/arrowshooter_down.png");
	Draw_SetOffset(img_arrowshooter_down,-16,-16);
	img_arrowshooter_left=Draw_LoadImage("data/arrowshooter_left.png");
	Draw_SetOffset(img_arrowshooter_left,-16,-16);
	img_arrowshooter_right=Draw_LoadImage("data/arrowshooter_right.png");
	Draw_SetOffset(img_arrowshooter_right,-16,-16);

	img_arrow_up=Draw_LoadImage("data/arrow_up.png");
	Draw_SetOffset(img_arrow_up,-16,-16);
	img_arrow_down=Draw_LoadImage("data/arrow_down.png");
	Draw_SetOffset(img_arrow_down,-16,-16);
	img_arrow_left=Draw_LoadImage("data/arrow_left.png");
	Draw_SetOffset(img_arrow_left,-16,-16);
	img_arrow_right=Draw_LoadImage("data/arrow_right.png");
	Draw_SetOffset(img_arrow_right,-16,-16);

	anim_fire=Anim_LoadAnim("data/fire.png",32,3,5);
	Anim_SetOffset(anim_fire,-16,-48);

	img_player_broken=Draw_LoadImage("data/player_broken.png");
	Draw_SetOffset(img_player_broken,-16,-48);



	snd_arrowhit=Audio_LoadSound("data/Hit_Hurt10.wav");
	snd_exitpoint=Audio_LoadSound("data/Powerup10.wav");
	snd_savepoint=Audio_LoadSound("data/Powerup30.wav");
	snd_shootarrow=Audio_LoadSound("data/Laser_Shoot2.wav");
	snd_burn=Audio_LoadSound("data/Explosion2.wav");
	snd_fillhole=Audio_LoadSound("data/Hit_Hurt16.wav");
	snd_drag=Audio_LoadSound("data/Explosion16.wav");






	///////////////////////////////////////
	//  Create the entity templates

	ent=Entity_New();
	ent->mass=-1.0f;
	ent->flags=0;
	//Entity_SetLight(ent,0.2f,0.2f,0.2f,1.0f);
	Entity_SetLight(ent,0,0,0,1);


	ent_player=Entity_Copy(ent);
	ent_player->type=Ent_Player;
	ent_player->radius=16.0f;
	ent_player->width=24;
	ent_player->height=24;
	ent_player->mass=30.0f;
	ent_player->backFric_static=2.5f;
	ent_player->backFric_dynamic=0.3f;
	ent_player->flags=
		EntityFlag_Collision|EntityFlag_Overlap|EntityFlag_Light;
	Entity_SetLight(ent_player,0.4f,0.4f,0.4f,3*32.0f);
	AnimPlay_SetImg(&ent_player->anim,img_player_down);
	ent_player->proc=player_proc;
	ent_player->collision=player_collision;


	ent_barrel=Entity_Copy(ent);
	ent_barrel->type=Ent_Barrel;
	ent_barrel->flags=
		EntityFlag_Collision|EntityFlag_Overlap;
	ent_barrel->radius=16.0f;
	ent_barrel->width=24;
	ent_barrel->height=24;
	ent_barrel->mass=25.0f;
	ent_barrel->backFric_static=2.0f;
	ent_barrel->backFric_dynamic=0.5f;
	ent_barrel->proc=barrel_proc;
	AnimPlay_SetImg(&ent_barrel->anim,img_barrel);


	ent_column=Entity_Copy(ent);
	ent_column->type=Ent_Column;
	//ent_column->flags=EntityFlag_BlockCollision;
	ent_column->flags=EntityFlag_Collision;
	ent_column->radius=12;
	ent_column->width=24;
	ent_column->height=24;
	AnimPlay_SetImg(&ent_column->anim,img_column);
	ent_column_faded=Entity_Copy(ent_column);
	AnimPlay_SetImg(&ent_column_faded->anim,img_column_faded);
	ent_rock=Entity_Copy(ent_column);
	AnimPlay_SetImg(&ent_rock->anim,img_rock);
	ent_rock->flags=EntityFlag_Collision;
	ent_lamp=Entity_Copy(ent_rock);
	AnimPlay_SetImg(&ent_lamp->anim,img_lamp);
	ent_lamp->flags=
		EntityFlag_Collision|EntityFlag_Light;
	Entity_SetLight(ent_lamp,0.4f,0.4f,0.4f,5*32.0f);



	ent_floor=Entity_Copy(ent);
	ent_floor->type=Ent_Floor;
	ent_floor->zorder=-1;
	ent_floor->flags=0;
	AnimPlay_SetImg(&ent_floor->anim,img_floor);
	ent_floor_left=Entity_Copy(ent_floor);
	AnimPlay_SetImg(&ent_floor_left->anim,img_floor_left);
	ent_floor_right=Entity_Copy(ent_floor);
	AnimPlay_SetImg(&ent_floor_right->anim,img_floor_right);
	ent_floor_center=Entity_Copy(ent_floor);
	AnimPlay_SetImg(&ent_floor_center->anim,img_floor_center);

	ent_hole_spiked=Entity_Copy(ent);
	ent_hole_spiked->type=Ent_Hole_Spiked;
	ent_hole_spiked->zorder=-1;
	ent_hole_spiked->flags=EntityFlag_Overlap;
	ent_hole_spiked->radius=18;
	AnimPlay_SetImg(&ent_hole_spiked->anim,img_hole_spiked);
	ent_hole_spiked->overlap=hole_spiked_overlap;

	ent_hole_filled=Entity_Copy(ent);
	ent_hole_filled->type=Ent_Hole_Filled;
	ent_hole_filled->zorder=-1;
	ent_hole_filled->flags=0;
	AnimPlay_SetImg(&ent_hole_filled->anim,img_barrel2);

	ent_hole_lava=Entity_Copy(ent);
	ent_hole_lava->type=Ent_Hole_Lava;
	ent_hole_lava->zorder=-1;
	ent_hole_lava->flags=EntityFlag_Overlap|EntityFlag_Light;
	ent_hole_lava->radius=18;
	AnimPlay_SetAnim(&ent_hole_lava->anim,anim_hole_lava);
	Entity_SetLight(ent_hole_lava,1.0f,0.0f,0.0f,4*32.0f);
	ent_hole_lava->oncopy=hole_lava_oncopy;
	ent_hole_lava->overlap=hole_lava_overlap;


	ent_arrow_up=Entity_Copy(ent);
	ent_arrow_up->type=Ent_Arrow;
	//ent_arrow_up->flags=EntityFlag_Collision;
	ent_arrow_up->flags=EntityFlag_Collision|EntityFlag_Light;
	Entity_SetLight(ent_arrow_up,0.2f,0.2f,0.2f,2*32.0f);
	ent_arrow_up->radius=4;
	ent_arrow_up->mass=0.1f;
	ent_arrow_up->collision=arrow_collision;
	ent_arrow_up->proc=timeoutent_proc;
	ent_arrow_up->A=120;
	AnimPlay_SetImg(&ent_arrow_up->anim,img_arrow_up);
	vec2_set(ent_arrow_up->vel,0,-16);
	ent_arrow_down=Entity_Copy(ent_arrow_up);
	AnimPlay_SetImg(&ent_arrow_down->anim,img_arrow_down);
	vec2_set(ent_arrow_down->vel,0,16);
	ent_arrow_left=Entity_Copy(ent_arrow_up);
	AnimPlay_SetImg(&ent_arrow_left->anim,img_arrow_left);
	vec2_set(ent_arrow_left->vel,-16,0);
	ent_arrow_right=Entity_Copy(ent_arrow_up);
	AnimPlay_SetImg(&ent_arrow_right->anim,img_arrow_right);
	vec2_set(ent_arrow_right->vel,16,0);


	ent_arrowshooter_up=Entity_Copy(ent);
	ent_arrowshooter_up->type=Ent_ArrowShooter;
	ent_arrowshooter_up->flags=EntityFlag_Collision;
	ent_arrowshooter_up->radius=15;
	ent_arrowshooter_up->oncopy=arrowshooter_oncopy;
	ent_arrowshooter_up->proc=arrowshooter_proc;
	AnimPlay_SetImg(&ent_arrowshooter_up->anim,img_arrowshooter_up);
	ent_arrowshooter_up->child=ent_arrow_up;
	ent_arrowshooter_down=Entity_Copy(ent_arrowshooter_up);
	AnimPlay_SetImg(&ent_arrowshooter_down->anim,img_arrowshooter_down);
	ent_arrowshooter_down->child=ent_arrow_down;
	ent_arrowshooter_left=Entity_Copy(ent_arrowshooter_up);
	AnimPlay_SetImg(&ent_arrowshooter_left->anim,img_arrowshooter_left);
	ent_arrowshooter_left->child=ent_arrow_left;
	ent_arrowshooter_right=Entity_Copy(ent_arrowshooter_up);
	AnimPlay_SetImg(&ent_arrowshooter_right->anim,img_arrowshooter_right);
	ent_arrowshooter_right->child=ent_arrow_right;


	ent_savepoint=Entity_Copy(ent);
	ent_savepoint->type=Ent_SavePoint;
	ent_savepoint->sortYOffset=-5;
	ent_savepoint->flags=EntityFlag_Overlap|EntityFlag_Light;
	ent_savepoint->radius=20;
	Entity_SetLight(ent_savepoint,0.0f,0.0f,0.5f,2*32.0f);
	AnimPlay_SetImg(&ent_savepoint->anim,img_savepoint);
	ent_savepoint->overlap=savepoint_overlap;
	ent_savepoint->ondelete=savepoint_ondelete;


	ent_exitpoint=Entity_Copy(ent);
	ent_exitpoint->type=Ent_ExitPoint;
	ent_exitpoint->flags=EntityFlag_Overlap|EntityFlag_Light;
	Entity_SetLight(ent_exitpoint,0.5f,0.5f,0.5f,5*32.0f);
	ent_exitpoint->radius=20;
	AnimPlay_SetAnim(&ent_exitpoint->anim,anim_exitpoint);
	ent_exitpoint->overlap=exitpoint_overlap;
	ent_endpoint=Entity_Copy(ent_exitpoint);
	AnimPlay_SetImg(&ent_endpoint->anim,img_endpoint);
	ent_endpoint->overlap=endpoint_overlap;

	ent_teleporter=Entity_Copy(ent);
	ent_teleporter->zorder=0;
	ent_teleporter->type=Ent_Teleporter;
	ent_teleporter->flags=EntityFlag_Overlap|EntityFlag_Light;
	Entity_SetLight(ent_teleporter,0.5f,0.5f,0.5f,5*32.0f);
	ent_teleporter->radius=20;
	AnimPlay_SetImg(&ent_teleporter->anim,img_savepoint);
	ent_teleporter->overlap=teleporter_overlap;

	ent_teleporter_dest=Entity_Copy(ent);
	ent_teleporter_dest->zorder=0;
	ent_teleporter_dest->type=Ent_Teleporter_Dest;
	ent_teleporter_dest->flags=0;
	AnimPlay_SetImg(&ent_teleporter_dest->anim,img_savepoint);





	ent_fire=Entity_Copy(ent);
	ent_fire->type=Ent_Effect;
	ent_fire->flags=EntityFlag_Light;
	Entity_SetLight(ent_fire,1.0f,0.0f,0.0f,3*32.0f);
	AnimPlay_SetAnim(&ent_fire->anim,anim_fire);
	ent_fire->proc=timeoutent_proc;
	ent_fire->A=15;
	ent_fire->sortYOffset=1;

	ent_player_broken=Entity_Copy(ent);
	ent_player_broken->type=Ent_Effect;
	ent_player_broken->flags=0;
	AnimPlay_SetImg(&ent_player_broken->anim,img_player_broken);

}


